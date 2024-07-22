

FetchContent_Declare(
  jansson
  GIT_REPOSITORY https://github.com/akheron/jansson.git
  GIT_TAG v2.14
)
set(JANSSON_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(JANSSON_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(JANSSON_WITHOUT_TESTS ON CACHE BOOL "" FORCE)
set(JANSSON_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_GetProperties(jansson)
if(NOT jansson_POPULATED)
  message(STATUS "Populating (download) jansson at ${jansson_SOURCE_DIR}")
  FetchContent_Populate(jansson)
  set(jansson_POPULATED TRUE CACHE BOOL "jansson populated" FORCE)
endif()

# Check if jansson is already built
if(NOT EXISTS "${jansson_SOURCE_DIR}/lib/libjansson.a")
  
  # Configure jansson
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -DJANSSON_BUILD_DOCS=OFF
      -DJANSSON_BUILD_SHARED_LIBS=OFF
      -DJANSSON_WITHOUT_TESTS=ON
      -DJANSSON_EXAMPLES=OFF
      -DJANSSON_INSTALL=ON
      -DCMAKE_INSTALL_PREFIX=${jansson_BINARY_DIR}/install
      -S ${jansson_SOURCE_DIR} 
      -B ${jansson_BINARY_DIR}        
  )

  # Build and install jansson
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${jansson_BINARY_DIR} --target install
    RESULT_VARIABLE result
  )
  if(result)
    message(FATAL_ERROR "Build step for jansson failed: ${result}")
  endif()

else()
  message(STATUS "jansson already built, skipping build step")
endif()    

