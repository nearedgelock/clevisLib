

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

set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip install for zlib" FORCE)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)

    # Check if zlib is already built
    if(NOT EXISTS "${zlib_SOURCE_DIR}/libz.a")

      # Configure zlib
      execute_process(
        COMMAND ${CMAKE_COMMAND} -DZLIB_BUILD_EXAMPLES=OFF -DZLIB_BUILD_TESTS=OFF -DZLIB_BUILD_MINIZIP=OFF -DBUILD_SHARED_LIBS=OFF -DZLIB_BUILD_STATIC_LIBS=ON -DZLIB_BUILD_SHARED_LIBS=OFF -DSKIP_INSTALL_ALL=ON .
        WORKING_DIRECTORY ${zlib_SOURCE_DIR}
      )
    
      # Build zlib
      execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${zlib_SOURCE_DIR}
      )
    else()
    message(STATUS "zlib already built, skipping build step")
  endif()
endif()

