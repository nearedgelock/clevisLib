

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.3.1
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_MINIZIP OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(zlib)
get_property(targets DIRECTORY "${zlib_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
message("Available targets: ${targets} for zlib")


