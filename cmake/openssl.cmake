
FetchContent_Declare(
  openssl
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG openssl-3.2.0
)

set(OPENSSL_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/openssl-install")

FetchContent_GetProperties(openssl)
if(NOT openssl_POPULATED)
  FetchContent_Populate(openssl)
  set(openssl_POPULATED TRUE CACHE BOOL "openssl populated" FORCE)

  message (STATUS "Using ${OPENSSL_INSTALL_DIR} for the openSSL installation directory")
  
  # Prepare configuration options
  if(EMSCRIPTEN)
    set(OPENSSL_CONFIG_OPTIONS
      --prefix=${OPENSSL_INSTALL_DIR}
      linux-generic32
      no-asm
      no-shared
      no-afalgeng
      no-engine
      no-dynamic-engine
      no-tests
      no-deprecated
      no-docs
      no-weak-ssl-ciphers
      no-ktls
      no-static-engine
      CC=emcc
      CXX=em++
      CFLAGS="-O3"
      CXXFLAGS="-O3"
    ) 
  else () 
    set(OPENSSL_CONFIG_OPTIONS
      --prefix=${OPENSSL_INSTALL_DIR}
      no-deprecated
      no-docs
      no-tests
      no-weak-ssl-ciphers
      no-dynamic-engine
      no-ktls
      no-static-engine
      no-shared
    )
  endif()

  # Check if OpenSSL is already built
  if(NOT EXISTS "${OPENSSL_INSTALL_DIR}/lib/libcrypto.a")
    # Configure OpenSSL
    execute_process(
      COMMAND ./config ${OPENSSL_CONFIG_OPTIONS}
      WORKING_DIRECTORY ${openssl_SOURCE_DIR}
    )
    
    # Build OpenSSL
    execute_process(
      COMMAND make
      WORKING_DIRECTORY ${openssl_SOURCE_DIR}
    )

    # Install OpenSSL
      execute_process(
        COMMAND make install_sw
        WORKING_DIRECTORY ${openssl_SOURCE_DIR}
    )
  else()
    message(STATUS "OpenSSL already built, skipping build step. We found ${openssl_SOURCE_DIR}/libssl.a and ${openssl_SOURCE_DIR}/libcrypto.a")
  endif()
endif()

# Set OpenSSL paths manually
set(OPENSSL_ROOT_DIR ${openssl_SOURCE_DIR})
set(OPENSSL_INCLUDE_DIR ${openssl_SOURCE_DIR}/include)
set(OPENSSL_CRYPTO_LIBRARY ${openssl_SOURCE_DIR}/libcrypto.a)
set(OPENSSL_SSL_LIBRARY ${openssl_SOURCE_DIR}/libssl.a)

set(ENV{PKG_CONFIG_PATH} "${OPENSSL_INSTALL_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
