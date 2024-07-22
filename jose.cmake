#
# Drive the josé build process - We need to complete this before we can add
# its library to our main project.
#

set(jose_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/jose")
set(INSTALL_DIR "${jose_BINARY_DIR}/install")

# Manually configuring and building jose

message (NOTICE "Starting configuration, build and installation of jose")
execute_process(
  COMMAND mkdir -p ${jose_BINARY_DIR}
)

message (NOTICE "jose setup at location ${jose_BINARY_DIR}, using ${CMAKE_CURRENT_SOURCE_DIR}/jose")
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
  message(NOTICE "Preparing the meson machine file")

  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/wasm-cross-meson.txt.in" MACHINEFILE_IN)
  string(CONFIGURE "${MACHINEFILE_IN}" MACHINEFILE_OUT)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/jose/wasm-cross-meson.txt" "${MACHINEFILE_OUT}")

  list(APPEND MESON_SETUP_ARGS "--cross-file=${CMAKE_CURRENT_BINARY_DIR}/jose/wasm-cross-meson.txt")
endif()

# Dependencies
set(MESON_PKG_CONFIG_PATH
  "${FETCHCONTENT_BASE_DIR}/jansson-build/install/lib/pkgconfig"
  "${FETCHCONTENT_BASE_DIR}/openssl-src"
  "${FETCHCONTENT_BASE_DIR}/botan-build/lib/pkgconfig"
  "${FETCHCONTENT_BASE_DIR}/zlib-build"
)

string(JOIN ":" MESON_PKG_CONFIG_PATH_ARG ${MESON_PKG_CONFIG_PATH})
list(APPEND MESON_SETUP_ARGS "--pkg-config-path=${MESON_PKG_CONFIG_PATH_ARG}")
#list(APPEND MESON_SETUP_ARGS "--pkg-config-path=
#${FETCHCONTENT_BASE_DIR}/jansson-build/install/lib/pkgconfig:${FETCHCONTENT_BASE_DIR}/openssl-src:${FETCHCONTENT_BASE_DIR}/botan-build/lib/pkgconfig:${FETCHCONTENT_BASE_DIR}/zlib-src")

message (NOTICE "meson arguments (for setup) are ${MESON_SETUP_ARGS}")

execute_process(
  WORKING_DIRECTORY ${jose_BINARY_DIR}
  COMMAND meson setup  ${MESON_SETUP_ARGS} . ${CMAKE_CURRENT_SOURCE_DIR}/jose     
  RESULT_VARIABLE JOSE_SETUP_RESULT
)

message (NOTICE "********************** meson setup done")

if(NOT ${JOSE_SETUP_RESULT} EQUAL 0)
    message(NOTICE "Output is ${JOSE_SETUP_OUTPUT}")
    message(FATAL_ERROR "Failed to setup jose")
endif()

message (NOTICE "jose build")
execute_process(
  WORKING_DIRECTORY ${jose_BINARY_DIR}
  COMMAND ninja
)

message (NOTICE "jose installation")
execute_process(
  WORKING_DIRECTORY ${jose_BINARY_DIR}
  COMMAND ninja install

  RESULT_VARIABLE JOSE_INSTALL_RESULT
)
if(NOT ${JOSE_INSTALL_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to build / install jose")
endif()

message (NOTICE "convert thin archive to regular / classic format")
execute_process(
  COMMAND sh -c "cd ${jose_BINARY_DIR}/lib/libjose_static.a.p && mv ../libjose_static.a ../libjose_static_thin.a | ar -t ../libjose_static_thin.a | xargs ar -qc ../libjose_static.a"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}  # Adjust this if needed
  RESULT_VARIABLE RESULT

)

include_directories(${jose_BINARY_DIR}/install/include)
link_directories(${jose_BINARY_DIR}/lib)

add_library(libjose_static STATIC IMPORTED)
set_target_properties(libjose_static PROPERTIES IMPORTED_LOCATION ${jose_BINARY_DIR}/lib/libjose_static.a)
set_target_properties(libjose_static PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${jose_BINARY_DIR}/install/include"
)

