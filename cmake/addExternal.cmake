#
# Thi file should be included by the top level CMakeLists to defined the
# needed external dependencies
#

# CLEVISLIB_DEP_PREFIXES collects the install prefixes of our dependencies. It
# is used for find_package() (through CMAKE_PREFIX_PATH) and to build the
# pkg-config search path handed to meson for jose (see jose.cmake), so that
# clevisLib and jose are compiled against the very same headers / libraries.
set(CLEVISLIB_DEP_PREFIXES "")

if(CLEVISLIB_FETCH_DEPS)
  include(FetchContent)

  include(cmake/jansson.cmake)
  include(cmake/botan.cmake)
  include(cmake/zlib.cmake)
  include(cmake/openssl.cmake)

  list(APPEND CMAKE_PREFIX_PATH ${CLEVISLIB_DEP_PREFIXES})
else()
  if(OPENSSL_ROOT_DIR)
    list(APPEND CLEVISLIB_DEP_PREFIXES ${OPENSSL_ROOT_DIR})
  endif()
  list(APPEND CLEVISLIB_DEP_PREFIXES ${CMAKE_PREFIX_PATH})
  message(STATUS "CLEVISLIB_FETCH_DEPS=OFF: using dependencies found in ${CLEVISLIB_DEP_PREFIXES}")
endif()


