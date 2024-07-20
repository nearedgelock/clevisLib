

FetchContent_Declare(
  jansson
  GIT_REPOSITORY https://github.com/akheron/jansson.git
  GIT_TAG v2.14
)
set(JANSSON_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(JANSSON_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(JANSSON_WITHOUT_TESTS ON CACHE BOOL "" FORCE)
set(JANSSON_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JANSSON_INSTALL OFF CACHE BOOL "" FORCE)

#FetchContent_MakeAvailable(jansson)
FetchContent_GetProperties(jansson)
if(NOT jansson_POPULATED)
  FetchContent_Populate(jansson)

  # Check if jansson is already built
  if(NOT EXISTS "${jansson_SOURCE_DIR}/lib/libjansson.a")
  
    # Configure jansson
    execute_process(
      COMMAND ${CMAKE_COMMAND} -DJANSSON_BUILD_DOCS=OFF -DJANSSON_BUILD_SHARED_LIBS=OFF -DJANSSON_WITHOUT_TESTS=ON -DJANSSON_EXAMPLES=OFF -DJANSSON_INSTALL=OFF .
      WORKING_DIRECTORY ${jansson_SOURCE_DIR}
    )
  
    # Build jansson
    execute_process(
      COMMAND ${CMAKE_COMMAND} --build .
      WORKING_DIRECTORY ${jansson_SOURCE_DIR}
    )
  else()
    message(STATUS "jansson already built, skipping build step")
  endif()    
endif()

