

# Botan setup
FetchContent_Declare(
  botan
  GIT_REPOSITORY https://github.com/randombit/botan.git
  GIT_TAG 3.5.0
)

FetchContent_GetProperties(botan)
if(NOT botan_POPULATED)
  FetchContent_Populate(botan)
  
  # Determine the compiler type
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(BOTAN_COMPILER_TYPE "gcc")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(BOTAN_COMPILER_TYPE "clang")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(BOTAN_COMPILER_TYPE "msvc")
  else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()

  # Create a separate script to build Botan
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/build_botan.cmake"
    "execute_process(
       COMMAND python3 ${botan_SOURCE_DIR}/configure.py 
               --prefix=${botan_BINARY_DIR}
               --with-cmake
               --disable-shared
               --no-install-python-module
               --without-documentation
               --disable-modules=tests,test_tools,benchmarks
               --cc=${BOTAN_COMPILER_TYPE}
               --cc-bin=${CMAKE_CXX_COMPILER}
       WORKING_DIRECTORY ${botan_SOURCE_DIR}
       RESULT_VARIABLE BOTAN_CONFIGURE_RESULT
     )
     if(NOT BOTAN_CONFIGURE_RESULT EQUAL 0)
       message(FATAL_ERROR \"Botan configuration failed\")
     endif()
     execute_process(
       COMMAND make -j18
       WORKING_DIRECTORY ${botan_SOURCE_DIR}
       RESULT_VARIABLE BOTAN_BUILD_RESULT
     )
     if(NOT BOTAN_BUILD_RESULT EQUAL 0)
       message(FATAL_ERROR \"Botan build failed\")
     endif()
     execute_process(
       COMMAND make install
       WORKING_DIRECTORY ${botan_SOURCE_DIR}
       RESULT_VARIABLE BOTAN_INSTALL_RESULT
     )
     if(NOT BOTAN_INSTALL_RESULT EQUAL 0)
       message(FATAL_ERROR \"Botan installation failed\")
     endif()"
  )

  # Execute the Botan build script in a clean environment
  execute_process(
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/build_botan.cmake
    RESULT_VARIABLE BOTAN_BUILD_SCRIPT_RESULT
  )

  if(NOT BOTAN_BUILD_SCRIPT_RESULT EQUAL 0)
    message(FATAL_ERROR "Botan build script failed")
  endif()

  # Now try to find the installed Botan
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${botan_BINARY_DIR})
  find_package(botan REQUIRED)
endif()

# If everything worked, you should be able to use Botan targets now
# target_link_libraries(your_target PRIVATE botan::botan)



