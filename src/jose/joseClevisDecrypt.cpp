/*
 * Copyright 2024 NearEDGE, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "joseCommon.h"
#include "joseClevisDecrypt.h"

#include <stdlib.h>
#include <string>

extern "C" {
#include "jose/jwe.h"
#include "jose/jwk.h"
}

#include "stringSplit.h"

namespace joseLibWrapper {
namespace decrypt {
  /*
  //See http://www.dwheeler.com/secure-programs/Secure-Programs-HOWTO/protect-secrets.html

  // It would be nice to use this but jose itself prevent this by directly using free on pointer
  // returned by the low level jansson :-(

  void *guaranteed_memset(void *v,int c,size_t n) {
    volatile char*    p = (volatile char*)v;
    while (n--)
      *p++=c;
    return v;
  }

  static void *secure_malloc(size_t size) {
    // Store the memory area size in the beginning of the block
    char*             ptr = (char *)malloc(size + sizeof(char *));
    *((size_t *)ptr) = size;

    return (void *) (ptr + sizeof(char *));
  }

  static void secure_free(void *ptr) {
    size_t size = 0;

    ptr = (char*)ptr - sizeof(char *);
    size = *((size_t *)ptr);

    guaranteed_memset(ptr, 0, size + sizeof(char *));
    free(ptr);
  }

  static void __attribute__((constructor))
  constructor(void) {
    json_set_alloc_funcs(secure_malloc, secure_free);
  }
  */

  json_t* decomposeCompactJWE(const std::string& jwe, bool partial) {
   // We decompose the compact JWE. The underlying split function does not care if multiple
    // '.' separator are present. We harcode the fact that the JWE Encrypted Key is not
    // supposed to be present. So, in effect, the 5 components is returned as an deque of 4
    // elements.
    //
    // Updated. When partial is true, we expect to only find the protected header and IV. And
    // potentially a  portion of the CT (which will probably be incomplete). This is useful when
    // using a streaming mode for decryption.
    std::deque<std::string>     decomposedJWT =  misc::split(jwe, '.');
    if ( (decomposedJWT.size() != 4) and (partial == false) ) {
      // This is the classic, none streaming mode were we expect to find all of the 
      // components.
      std::size_t               count = 1;
      for (const auto& element : decomposedJWT) {
        try {
          json_t*                 el = joseLibWrapper::extractB64ToJson(element);
        } catch (std::exception& exc) {
          // Probably the element was not B64 encoded. We presume that it is a printable string
        }
      }

      logJson("Is payload missing?", nullptr);      
      throw joseLibWrapper::decrypt::invalidJWT();
    } else if ( (decomposedJWT.size() < 2) and (partial == true) ) {
      // We expect to at least have 2 elements.
      logJson("Is IV missing?", nullptr);      
      throw joseLibWrapper::decrypt::invalidJWT();      
    }

    // Keep a copy of the full JWE in JSON format (an array of object)
    // Note that some of the value are actual JSON but at this stage they are still
    // stored as string.
    json_t*                     jwe_j = json_object();
    if (jwe_j != nullptr) {
      json_object_set(jwe_j, "protected", json_string(decomposedJWT[0].data()));
      json_object_set(jwe_j, "protected_rawsize", json_integer(decomposedJWT[0].size()));

      json_object_set(jwe_j, "iv", json_string(decomposedJWT[1].data()));
      json_object_set(jwe_j, "iv_rawsize", json_integer(decomposedJWT[1].size()));
      if (partial == false) {
        json_object_set(jwe_j, "ciphertext", json_string(decomposedJWT[2].data()));
        json_object_set(jwe_j, "ciphertext_rawsize", json_integer(decomposedJWT[2].size()));

        json_object_set(jwe_j, "tag",  json_string(decomposedJWT[3].data()));
        json_object_set(jwe_j, "tag_rawsize", json_integer(decomposedJWT[3].size()));
        
      }
      //logJson("Full JWE is ", jwe_j);
    }

    return jwe_j;
  }

  std::string recoverPayload(const json_t* jwk, const json_t* jwe) {
    // We return a string, which may contain binary data. We must make sure
    // to properly handle this and not expect a null terminated result.
    char*             result = nullptr;
    std::size_t       plaintextSize = 0;
    result = (char*)jose_jwe_dec(nullptr, jwe, nullptr, jwk, &plaintextSize);

    if (result != nullptr) {
      return std::string(result, plaintextSize);
    }
    throw failedGetServerKey("Failed at recovering the payload.");
  }

  json_t* validateKey(const json_t* kid, const json_t* keys) {
    // In fact, despite the function name, what we want if to find the key (in keys) that
    // has the same thumbpint as kid.

    // First, verify that the length of the hash of the thumbprints (which is the key ID) is valid
    // The thumbprint is B64 encoded (URL safe)
    if (json_string_length(kid) != 43) {
      throw invalidKIDHash("Wrong length");
    }

    // We expect keys to be an array.
    if (json_is_array(keys) == false) {
      throw failedGetServerKey("Can not find an array of key candidates");
    }

    std::size_t       index;
    json_t*           value = nullptr;

    // Scan the arrays, compute the thumbprint and test for kid
    json_array_foreach(keys, index, value) {
      if (thumbprint(value) == json_string_value(kid)) {
        return value;
      }
    }

    throw failedGetServerKey("Failed to find the advertized key from the server corresponding to KID");
  }

  checkJWE::checkJWE(const json_t* jwe) {
    //
    // We check the validity of the JWE essentially by extracting the data we need. If something
    // is missing, then an exception is thrown
    //

    if (jwe == nullptr) {
      throw joseLibWrapper::decrypt::notClevisTang();
    }

    // Decode B64 and verify that the JWE was produced via clevis (or something compatible)
    jweProtectedHeaders_j = joseLibWrapper::extractB64ToJson(json_object_get(jwe, "protected"));

    if (jweProtectedHeaders_j != nullptr) {
      json_incref(jweProtectedHeaders_j);
      // The meta data must have a JSON key with "clevis" at the top level
      if (json_is_object(jweProtectedHeaders_j) == false) {
        throw joseLibWrapper::decrypt::notClevisTang();
      }

      // The clevis custom header includes the main data we need
      // The sub checker below (tang, sss and tpm will futher extract data
      json_t*                 clevis_j = json_object_get(jweProtectedHeaders_j, "clevis");
      if (clevis_j == nullptr) {
        throw joseLibWrapper::decrypt::notClevisTang();
      };

      // We may have a tang or sss or tpm variant
      if (json_object_get(clevis_j, "tang") != nullptr) {
        if (checkJWEValidity_tang(jweProtectedHeaders_j) == true)
          return;
      }

      if (json_object_get(clevis_j, "sss") != nullptr) {
        if (checkJWEValidity_sss(jweProtectedHeaders_j) == true)
          return;
      }

      if (json_object_get(clevis_j, "tpm") != nullptr) {
        if (checkJWEValidity_tpm(jweProtectedHeaders_j) == true)
          return;
      }
    }

    logJson("Invalid JWE, we bail out", nullptr);
    throw joseLibWrapper::decrypt::notClevisTang();
  }

  void checkJWE::printInfo() const {
    printEPK();
    printEPKCurve();
    printKID();
    printAllKeys();
    printSelectedKey();
    printSelectedServerKey();
    printProtectedHeader(false);
  }

  bool checkJWE::checkJWEValidity_tang(const json_t* hdr) {
    // A valid header for the tang pin is when the epk, kid, url and 
    // key array are present.
    epk_j = json_object_get(hdr, "epk");
    if (epk_j != nullptr) {
      epkCurve_j = json_object_get(epk_j, "crv");
    }

    kid_j = json_object_get(hdr, "kid");

    json_t*           clevis = json_object_get(hdr, "clevis");
    json_t*           tangInfo = json_object_get(clevis, "tang");
    json_t*           advertisement = nullptr;
    if (tangInfo != nullptr) {
      extractedUrl = json_string_value(json_object_get(tangInfo, "url"));
      advertisement = json_object_get(tangInfo, "adv");
      if (advertisement != nullptr) { 
        allKeys_j = json_object_get(advertisement, "keys");
      }
    }

    activeServerKey_j = joseLibWrapper::decrypt::validateKey(kid_j, allKeys_j);

    if ( (epk_j != nullptr) and (kid_j != nullptr) and (extractedUrl.empty() == false) and (allKeys_j != nullptr) ) {
      return true;
    } else {
      logJson("Invalid tang header", nullptr);
      return false;
    }
  }

  bool checkJWE::checkJWEValidity_sss(const json_t* hdr) {
    return false;
  }

  bool checkJWE::checkJWEValidity_tpm(const json_t* hdr) {
    return false;
  }

  transportKey::transportKey(const json_t* crv, const json_t* epk) {
    try {
      // This series of action may throw an exception
      ephemeralKey_j = joseLibWrapper::generateKey(crv);   // This is a full pairwise key, i.e. the private part is present
      json_incref(ephemeralKey_j);

      json_auto_t*                  ex_j = joseLibWrapper::keyExchange(epk, ephemeralKey_j);    // Perform a key extract between the EPK and the new ephemeral key
      ephemeralKey_pub = joseLibWrapper::printFlatJson(ex_j);
    } catch (std::exception& exc) {
      throw;
    }
  }

  void transportKey::printInfo() const {
    logJson("Ephemeral Key (private): ", getEphemeralKey());
    logJson("Ephemeral Key (pub): " + getEphemeralKeyPub(), nullptr);
  }

  transportKey::transportKey(const std::string pub, const std::string priv) {
    ephemeralKey_pub = pub;
    
    json_error_t                  error;
    ephemeralKey_j = json_loads(priv.data(), 0, &error);
    json_incref(ephemeralKey_j);
  }

} // namespace decrypt
} // namespace joseLibWrapper

