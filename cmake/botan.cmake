

# Botan setup
FetchContent_Declare(
  botan
  GIT_REPOSITORY https://github.com/randombit/botan.git
  GIT_TAG 3.5.0
)

FetchContent_GetProperties(botan)
set(botan_POPULATED FALSE CACHE BOOL "botan populated" FORCE)
message(STATUS "Botan flag is ${botan_POPULATED}")
if(NOT botan_POPULATED)
  message(STATUS "Populating (download) Botan at ${botan_SOURCE_DIR}")
  FetchContent_Populate(botan)
  set(botan_POPULATED TRUE CACHE BOOL "botan populated" FORCE)
 
  # Determine the compiler type
  if(EMSCRIPTEN)
    set(BOTAN_CONFIGURE_FLAGS
      --prefix=${botan_BINARY_DIR}
      --with-cmake
      --disable-shared
      --no-install-python-module
      --without-documentation
      --disable-modules=tests,test_tools,benchmarks
      --cpu=wasm
      --os=emscripten
      --cc=emcc
      --cc-bin=${CMAKE_CXX_COMPILER}
      --module-policy=modern
      --minimized-build
      --disable-modules=pkcs11,tpm,compression
    ) 
  else()  
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      set(BOTAN_COMPILER_TYPE "gcc")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(BOTAN_COMPILER_TYPE "clang")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      set(BOTAN_COMPILER_TYPE "msvc")
    else()
      message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()

    set(BOTAN_CONFIGURE_FLAGS
    --prefix=${botan_BINARY_DIR}
    --with-cmake
    --disable-shared
    --no-install-python-module
    --without-documentation
    --disable-modules=tests,test_tools,benchmarks
    --cc=${BOTAN_COMPILER_TYPE}
    --cc-bin=${CMAKE_CXX_COMPILER}
    )
  endif()

  # Create a separate script to build Botan
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/build_botan.cmake"
    "execute_process(
       COMMAND python3 ${botan_SOURCE_DIR}/configure.py 
       ${BOTAN_CONFIGURE_FLAGS}
       WORKING_DIRECTORY ${botan_SOURCE_DIR}
       RESULT_VARIABLE BOTAN_CONFIGURE_RESULT
     )
     if(NOT BOTAN_CONFIGURE_RESULT EQUAL 0)
       message(FATAL_ERROR \"Botan configuration failed\")
     endif()
     execute_process(

       #COMMAND make -j\${CMAKE_BUILD_PARALLEL_LEVEL}
       COMMAND make -j 18
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
endif()

# Check if Botan is already built
if(NOT EXISTS "${botan_BINARY_DIR}/lib/libbotan-3.a")
  # Execute the Botan build script in a clean environment
  execute_process(
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/build_botan.cmake
    RESULT_VARIABLE BOTAN_BUILD_SCRIPT_RESULT
  )
  
  if(NOT BOTAN_BUILD_SCRIPT_RESULT EQUAL 0)
    message(FATAL_ERROR "Botan build script failed")
  endif()
else()
  message(STATUS "Botan already built, skipping build step. We found ${botan_BINARY_DIR}/lib/libbotan-3.a")
endif()





