#
# Drive the josé build process - We need to complete this before we can add
# its library to our main project.
#

set(jose_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/jose")
set(INSTALL_DIR "${jose_BINARY_DIR}/install")

# Manually configuring and building jose

message (STATUS "Starting configuration, build and installation of jose")
execute_process(
  COMMAND mkdir -p ${jose_BINARY_DIR}
)

message (STATUS "jose setup at location ${jose_BINARY_DIR}, using ${CMAKE_CURRENT_SOURCE_DIR}/jose")
set(ENV{PKG_CONFIG} "/usr/bin/pkg-config")

# Build arguments list
set(MESON_SETUP_ARGS
  --buildtype=debug
  -Dbuild_static=true
  -Dbuild_dynamic=false
  -Dbuild_executable=false
  --default-library=static
  --prefix=${INSTALL_DIR}
)

# Some cross-build specific
if(NATIVE)
else()
  message(STATUS "Preparing the meson machine file")

  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/wasm-cross-meson.txt.in" MACHINEFILE_IN)
  string(CONFIGURE "${MACHINEFILE_IN}" MACHINEFILE_OUT)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/jose/wasm-cross-meson.txt" "${MACHINEFILE_OUT}")

  list(APPEND MESON_SETUP_ARGS "--cross-file=${CMAKE_CURRENT_BINARY_DIR}/jose/wasm-cross-meson.txt")
endif()

# Dependencies
set(MESON_PKG_CONFIG_PATH
  "${FETCHCONTENT_BASE_DIR}/jansson-build/install/lib/pkgconfig"
  "${FETCHCONTENT_BASE_DIR}/openssl-install/lib/pkgconfig"
  "${FETCHCONTENT_BASE_DIR}/botan-build/lib/pkgconfig"
  "${FETCHCONTENT_BASE_DIR}/zlib-build"
)

string(JOIN ":" MESON_PKG_CONFIG_PATH_ARG ${MESON_PKG_CONFIG_PATH})
list(APPEND MESON_SETUP_ARGS "--pkg-config-path=${MESON_PKG_CONFIG_PATH_ARG}")

message (STATUS "meson arguments (for setup) are ${MESON_SETUP_ARGS}")

execute_process(
  WORKING_DIRECTORY ${jose_BINARY_DIR}
  COMMAND meson setup  ${MESON_SETUP_ARGS} . ${CMAKE_CURRENT_SOURCE_DIR}/jose     
  RESULT_VARIABLE JOSE_SETUP_RESULT
)

include_directories(${jose_BINARY_DIR}/install/include)
link_directories(${jose_BINARY_DIR}/lib)

#
# The custom command (running during build time) to build finish creating the libjose)static file
#
# First, create a custom command for the jose build process
add_custom_command(
  OUTPUT ${jose_BINARY_DIR}/lib/libjose_static.a
  COMMAND ninja
  COMMAND ninja install
  COMMAND ${CMAKE_COMMAND} -E echo "jose build and install completed"
  COMMAND ${CMAKE_COMMAND} -E rename ${jose_BINARY_DIR}/lib/libjose_static.a ${jose_BINARY_DIR}/lib/libjose_static_thin.a

  COMMAND bash -c "ar -t lib/libjose_static_thin.a | xargs ar -qc lib/libjose_static.a"
  #COMMAND ${CMAKE_COMMAND} -E chdir ${jose_BINARY_DIR}/lib  ar -t libjose_static_thin.a | xargs ar -qc libjose_static.a
  WORKING_DIRECTORY ${jose_BINARY_DIR}
  COMMENT "Building and installing jose, then converting to regular archive"
  VERBATIM
)

# Create a custom target that depends on the output of the custom command
add_custom_target(jose_build ALL
  DEPENDS ${jose_BINARY_DIR}/lib/libjose_static.a
)

# Set up the jose library as an imported target
add_library(libjose_static STATIC IMPORTED GLOBAL)
set_target_properties(libjose_static PROPERTIES
  IMPORTED_LOCATION ${jose_BINARY_DIR}/lib/libjose_static.a
  INTERFACE_INCLUDE_DIRECTORIES "${jose_BINARY_DIR}/install/include"
)
add_dependencies(libjose_static jose_build)
