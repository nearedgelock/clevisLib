
#FetchContent_Declare(
#  openssl
#  GIT_REPOSITORY https://github.com/openssl/openssl.git
#  GIT_TAG openssl-3.2.0
#)
#set(OPENSSL_NO_DEPRECATED ON CACHE BOOL "" FORCE)
#set(OPENSSL_NO_DOCS ON CACHE BOOL "" FORCE)
#set(OPENSSL_NO_TESTS ON CACHE BOOL "" FORCE)
#set(OPENSSL_NO_EXAMPLES ON CACHE BOOL "" FORCE)
#set(OPENSSL_NO_WEAK_SSL_CIPHERS ON CACHE BOOL "" FORCE)
#set(OPENSSL_NO_DYNAMIC_ENGINE ON CACHE BOOL "" FORCE)
#set(OPENSSL_ENABLE_KTLS OFF CACHE BOOL "" FORCE)
#set(OPENSSL_NO_STATIC_ENGINE ON CACHE BOOL "" FORCE)
#set(OPENSSL_NO_SHARED ON CACHE BOOL "" FORCE)

#FetchContent_MakeAvailable(openssl)
#get_target_property(OPENSSL_INCLUDE_DIR openssl INTERFACE_INCLUDE_DIRECTORIES)
#get_filename_component(OPENSSL_ROOT_DIR "${OPENSSL_INCLUDE_DIR}" PATH)


FetchContent_Declare(
  openssl
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG openssl-3.2.0
)

FetchContent_GetProperties(openssl)
if(NOT openssl_POPULATED)
  FetchContent_Populate(openssl)
  
  # Prepare configuration options
  set(OPENSSL_CONFIG_OPTIONS
    no-deprecated
    no-docs
    no-tests
    no-weak-ssl-ciphers
    no-dynamic-engine
    no-ktls
    no-static-engine
    no-shared
  )

  # Check if OpenSSL is already built
  if(NOT EXISTS "${openssl_SOURCE_DIR}/libssl.a" OR NOT EXISTS "${openssl_SOURCE_DIR}/libcrypto.a")
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
  else()
    message(STATUS "OpenSSL already built, skipping build step")
  endif()
endif()

# Set OpenSSL paths manually
set(OPENSSL_ROOT_DIR ${openssl_SOURCE_DIR})
set(OPENSSL_INCLUDE_DIR ${openssl_SOURCE_DIR}/include)
set(OPENSSL_CRYPTO_LIBRARY ${openssl_SOURCE_DIR}/libcrypto.a)
set(OPENSSL_SSL_LIBRARY ${openssl_SOURCE_DIR}/libssl.a)
