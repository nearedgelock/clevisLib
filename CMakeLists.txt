cmake_minimum_required(VERSION 3.22)

# Options
option(BUILD_EXECUTABLE "Build an executable binary " ON)
option(NATIVE "Build using the native / local compiler" OFF)

set(CMAKE_CXX_STANDARD 20)

set(VCPKG_ROOT vcpkg)

if (NATIVE)
  message (NOTICE "Performing a native build")
else()
  message (NOTICE "Performing a cross build (emscripten)")
  set(VCPKG_DEFAULT_TRIPLET "wasm32-emscripten" CACHE STRING "")
  set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE STRING "")
  set(VCPKG_TARGET_TRIPLET "wasm32-emscripten" CACHE STRING "")

  set(EMSDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
endif()

project(clevisLib VERSION 0.0.1)
add_compile_options(-fexceptions)  # Add support for exception
if (BUILD_EXECUTABLE)
  add_executable(${CMAKE_PROJECT_NAME})
else()
  add_library(${CMAKE_PROJECT_NAME})
endif()

include(cmake/addExternal.cmake)
#message(FATAL_ERROR "Stop requested")

include (jose.cmake)

find_package(Botan REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

set(CMAKE_SIZEOF_VOID_P "")   # Required to avoid a failure with jansson when check for 32/64 bitness. Hopefully this is not impactful
#find_package(jansson REQUIRED)

# Define final WASM module (static library)
if (WEB_TARGET)
  # This should not be necessary to propagate to the compiler but it is....
  add_definitions(-DWEB_TARGET)
endif()

add_subdirectory(src)

# Link sublibraries
target_link_libraries(${CMAKE_PROJECT_NAME} ${JOSE_INCLUDE_OBJECT} Botan::Botan-static libjose_static jansson zlibstatic OpenSSL::Crypto)

if (NATIVE)
  #set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS)
else()
  # Set WASM target - For browser
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "-fexceptions -lembind -s WASM=1 -s MODULARIZE=1 -s ENVIRONMENT=\"web\" -sASSERTIONS -s EXPORT_NAME=\"createModule\"")
endif()
