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
#include "joseClevisEncrypt.h"

#include <cstring>
#include <stdlib.h>

extern "C" {
#include "jose/jwe.h"
#include "jose/jwk.h"
}

namespace joseLibWrapper {
namespace encrypt {
  // The advertisement as received verbatim from the server. It is a serialize JSON string with just the payload element
  json_t* decomposeAdvertisement(const std::string& adv) {
    json_error_t        error;
    json_auto_t*        adv_j = json_loads(adv.data(), 0, &error);

    if (adv_j != nullptr) {
      // A valid JSON string, we expect to find the payload field, which is base64 encoded
      json_t*           payload_j = joseLibWrapper::extractB64ToJson(json_object_get(adv_j, "payload"));
      if (payload_j != nullptr) {
        return payload_j;
      }
    }

    throw joseLibWrapper::invalidJson();
  }

  json_t* encryptIntoJWE (json_t* baseJWE, json_t* cek, void* payload, std::size_t size) {
    // We presume that a skeleton base JWE is provided and the cek is suitable

    //joseLibWrapper::logJson("JWE before encryption ", baseJWE);
    //joseLibWrapper::logJson("CEK before encryption ", cek);
    if (jose_jwe_enc_cek(nullptr, baseJWE, cek, payload, size) == true) {
      return baseJWE;
    }

    throw failedEncrypt();
  }

  jose_io_t* encryptIntoJWEIO (json_t* baseJWE, json_t* cek, jose_io_t *next) {
    // We presume that a skeleton base JWE is provided and the cek is suitable

    //joseLibWrapper::logJson("JWE before encryption ", baseJWE);
    //joseLibWrapper::logJson("CEK before encryption ", cek);
    jose_io_t*      input = jose_jwe_enc_cek_io(nullptr, baseJWE, cek, next);
    return input; 
    //throw failedEncrypt();
  }

  // Return a pointer to an existing server key if found
  json_t* getServerKeyFromAdvertisement(const json_t* adv) {
    bool                    result = validateAdvertisement(adv);

    if (result == true) {
      // The advertisement is valid but this only tells us there are 2 keys.
      // We want to extract the deriveKey
      size_t                index;
      json_t*               key;

      json_array_foreach(json_object_get(adv, "keys"), index, key) {
        json_t*             key_ops = json_object_get(key, "key_ops");
        if ( (key_ops != nullptr) and (json_is_array(key_ops) == true) ) {
          // key_ops is an array
          size_t            index;
          json_t*           keytype;

          json_array_foreach(key_ops, index, keytype) {
            if (strcmp (json_string_value(keytype), "deriveKey") == 0) {
              // This becoems the JWK that we will use to create the CEK
              // We must make a copy and stip some of the fields so as to not
              // affect the CEK (with a wrong algo, etc.)
              json_t*       serverkey = json_copy(key);
              json_object_del(serverkey, "alg");
              json_object_del(serverkey, "key_ops");
              return serverkey;
              break;
            }
          }
        }
      }
    }

    throw failedGetServerKey("");
  }

  json_t* skeletonJWE(const json_t* advKeys, const std::string& url, const std::string& kid) {
    // Create a clevis compatible skeleton JWE
    const std::string     adv_s = prettyPrintJson(advKeys);

    if ( (adv_s.empty() == true) or (url.empty() == true) or (kid.empty() == true) ) {
      throw failedEncrypt("Missing data to create base JWE");
    }

    std::string           jweSkeleton = R"({)"
      R"("protected": {)"
      R"(  "alg": "ECDH-ES",)"
      R"(  "enc": "A256GCM",)"
      R"(  "clevis": {)"
      R"(    "pin": "tang",)"
      R"(    "tang": {)"
      R"(      "adv": )";
    jweSkeleton += adv_s + ",";
    jweSkeleton +=
      R"(      "url": ")";
    jweSkeleton += url + "\"";
    jweSkeleton +=
      R"(    })"
      R"(  },)"
      R"(  "kid": ")";
    jweSkeleton += kid + R"("}})";

    json_error_t        error;
    json_t*             jwe = json_loads(jweSkeleton.data(), 0, &error);

    if (jwe == nullptr) {
      throw failedEncrypt("Failed to convert skeleton to JSON");
    }
    return jwe;
  }

  bool validateAdvertisement(const json_t* adv) {
    json_t*             keys_j = json_object_get(adv, "keys");

    if ( (keys_j != nullptr) and (json_array_size(keys_j) == 2) ) {
      return true;
    }

    return false;
  }

} // namespace encrypt
} // namespace joseLibWrapper

