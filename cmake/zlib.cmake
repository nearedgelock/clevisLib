

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
  
  # Add zlib subdirectory with specific options
  add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

