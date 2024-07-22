

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.3.1
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_MINIZIP OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL "Build static library" FORCE)
set(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library" FORCE)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)

    # Check if zlib is already built
    if(NOT EXISTS "${zlib_BINARY_DIR}/lib/libz.a")
      message (STATUS "Building zlib at ${zlib_BINARY_DIR}")
      
      # Configure zlib
      execute_process(
        COMMAND ${CMAKE_COMMAND}
          -DCMAKE_INSTALL_PREFIX=${zlib_BINARY_DIR}
          -DZLIB_BUILD_EXAMPLES=OFF
          -DZLIB_BUILD_TESTS=OFF
          -DZLIB_BUILD_MINIZIP=OFF
          -DBUILD_SHARED_LIBS=OFF
          -DZLIB_BUILD_STATIC_LIBS=ON
          -DZLIB_BUILD_SHARED_LIBS=OFF
          ${zlib_SOURCE_DIR}
        WORKING_DIRECTORY ${zlib_BINARY_DIR}
        RESULT_VARIABLE result
      )

      if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to configure zlib")
      endif()

      # Build zlib
      execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${zlib_BINARY_DIR}
      )

      if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to build zlib")
      endif()
    else()
    message(STATUS "zlib already built, skipping build step. We found ${zlib_SOURCE_DIR}/libz.a")
  endif()
endif()

# Set up zlib variables for the rest of the project
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" "${ZLIB_BINARY_DIR}")
set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/libz.a")

add_library(ZLIB::ZLIB STATIC IMPORTED)
set_target_properties(ZLIB::ZLIB PROPERTIES
  IMPORTED_LOCATION "${ZLIB_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}"
)

