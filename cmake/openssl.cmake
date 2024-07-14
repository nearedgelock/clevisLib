
FetchContent_Declare(
  openssl
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG openssl-3.2.0
)
set(OPENSSL_NO_DEPRECATED ON CACHE BOOL "" FORCE)
set(OPENSSL_NO_DOCS ON CACHE BOOL "" FORCE)
set(OPENSSL_NO_TESTS ON CACHE BOOL "" FORCE)
set(OPENSSL_NO_EXAMPLES ON CACHE BOOL "" FORCE)
set(OPENSSL_NO_WEAK_SSL_CIPHERS ON CACHE BOOL "" FORCE)
set(OPENSSL_NO_DYNAMIC_ENGINE ON CACHE BOOL "" FORCE)
set(OPENSSL_ENABLE_KTLS OFF CACHE BOOL "" FORCE)
set(OPENSSL_NO_STATIC_ENGINE ON CACHE BOOL "" FORCE)
set(OPENSSL_NO_SHARED ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(openssl)

