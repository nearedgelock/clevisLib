cmake_minimum_required(VERSION 3.22)

set(CLEVISLIB_MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CLEVISLIB_MAIN_PROJECT ON)
endif()

# Options
if(CLEVISLIB_MAIN_PROJECT)
  set(BUILD_EXECUTABLE_DEFAULT ON)
  set(NATIVE_DEFAULT ON)
else()
  set(BUILD_EXECUTABLE_DEFAULT OFF)
  set(NATIVE_DEFAULT ON)
endif()

option(BUILD_EXECUTABLE "Build an executable binary" ${BUILD_EXECUTABLE_DEFAULT})
option(NATIVE "Build using the native / local compiler" ${NATIVE_DEFAULT})

set(CMAKE_CXX_STANDARD 20)

#
# Top level stuff 
#
project(clevisLib VERSION 0.0.1)

if (NATIVE)
  message (NOTICE "Performing a native build")
else()
  message (NOTICE "Performing a cross build (emscripten)")
  set(EMSDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
  list(APPEND CMAKE_FIND_ROOT_PATH "${PROJECT_BINARY_DIR}/_deps/jansson-build/install")
  list(APPEND CMAKE_FIND_ROOT_PATH "${PROJECT_BINARY_DIR}/_deps")
endif()

add_compile_options(-fexceptions)  # Add support for exception (usefull mostly for emscripten but does not hurt otherwise)
if (BUILD_EXECUTABLE)
  add_executable(${CMAKE_PROJECT_NAME})
else()
  add_library(${CMAKE_PROJECT_NAME})
endif()

#
# Add the various libraries, either directly (jose) or as a package
#
include(cmake/targetInfo.cmake)
include(cmake/addExternal.cmake)
include (jose.cmake)

message (STATUS "Done with including 3rd party libraries. We now need to find them....")
message (STATUS "Botan location is ${botan_BINARY_DIR}")

message(STATUS "botan_BINARY_DIR = ${botan_BINARY_DIR}")
message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_FIND_ROOT_PATH = ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "CMAKE_LIBRARY_PATH = ${CMAKE_LIBRARY_PATH}")
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
message(STATUS "BUILD TYPE = ${CMAKE_BUILD_TYPE}")
if(WEB_TARGET)
    message(STATUS "Building for Web/WASM")
endif()


find_package(PkgConfig REQUIRED) 
set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(Botan REQUIRED PATHS ${botan_BINARY_DIR}/lib/cmake/Botan-3.5.0)
print_target_info(Botan::Botan-static)
find_package(OpenSSL REQUIRED)
print_target_info(OpenSSL::Crypto)

#set(CMAKE_SIZEOF_VOID_P "")   # Required to avoid a failure with jansson when check for 32/64 bitness. Hopefully this is not impactful

message (STATUS "Searched ${FETCHCONTENT_BASE_DIR}/jansson-build/install/lib/cmake/jansson")
find_package(jansson REQUIRED PATHS ${FETCHCONTENT_BASE_DIR}/jansson-build/install/lib/cmake/jansson)
print_target_info(jansson::jansson)

# Define final WASM module (static library)
if (WEB_TARGET)
  # This should not be necessary to propagate to the compiler but it is....
  add_definitions(-DWEB_TARGET)
endif()

#
# Our sources
#
add_subdirectory(src)

#
# clevisLib target adjustment - Note that JOSE_INCLUDE_OBJECT is a list of objects that MUST always be linked, despite their symbol not being called (implicitly called)
#
# Link sublibraries
add_dependencies(${CMAKE_PROJECT_NAME} libjose_static)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
 "-Wl,--whole-archive $<TARGET_OBJECTS:libjose_static>  -Wl,--no-whole-archive"  
)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  ZLIB::ZLIB 
  jansson::jansson
  Botan::Botan-static
  OpenSSL::Crypto
)

if (NATIVE)
  #set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS)
else()
  # Set WASM target - For browser
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "-fexceptions -lembind -s WASM=1 -s MODULARIZE=1 -s ENVIRONMENT=\"web\" -sASSERTIONS -s EXPORT_NAME=\"createModule\"")
endif()

#
# Installation - Only when we are a library (not for WASM)
# 
if (NOT BUILD_EXECUTABLE AND NOT WEB_TARGET)
  include(GNUInstallDirs)

  # Remove all libraries from the target and makes sure they are not included
  # in the installation location, thus leaving full control on those to the
  # other project.
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY INTERFACE_LINK_LIBRARIES "")

  install(TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(FILES
    src/clevisLib.h
    src/clevisDecrypt.h
    src/clevisEncrypt.h
    src/clevisTang.h

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
  )

  install(FILES
    src/jose/joseCommon.h    
    src/jose/joseClevisDecrypt.h
    src/jose/joseClevisEncrypt.h
    src/jose/joseTang.h

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jose/
  )

  install(FILES ${jose_BINARY_DIR}/lib/libjose_static.a
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Development
  )

  # Install jose headers
  install(DIRECTORY ${INSTALL_DIR}/include/jose
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
  )
  # Install the export set for use with the install-tree
  install(EXPORT ${CMAKE_PROJECT_NAME}Targets
    FILE ${CMAKE_PROJECT_NAME}Targets.cmake
    NAMESPACE ${CMAKE_PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}
    EXPORT_LINK_INTERFACE_LIBRARIES
  ) 

  # Create and install the Config file
  include(CMakePackageConfigHelpers)
  configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME} 
    PATH_VARS
      CMAKE_INSTALL_LIBDIR
      CMAKE_INSTALL_INCLUDEDIR
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}
  )
endif()
