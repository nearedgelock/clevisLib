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
#include "clevisTang.h"
#include "jose/joseCommon.h"
#include "jose/joseTang.h"

#include <stdlib.h>

namespace binding {

  const returnWithStatus_t createJWKAdvertisement(const std::string& jwk, const std::string& signKey) {
    returnWithStatus_t   result;

    if ( (jwk.empty() == true) or (signKey.empty() == true) ) {
      result.success = false;
      result.msg = "Missing JWK or Signing key. Can not create an advertisement";
      return result;
    }

    json_error_t         error;
    json_auto_t*         jwk_j = json_loads(jwk.data(), 0, &error);
    json_auto_t*         signKey_j= nullptr;
    if (jwk_j == nullptr) {
      result.success = false;
      result.msg = "Error while loading key to generate the advertisement";
      return result;
    }

    joseLibWrapper::removePrivate(jwk_j);

    signKey_j= json_loads(signKey.data(), 0, &error);
    if (signKey_j == nullptr) {
      result.success = false;
      result.msg = "Error while loading signature key to generate the advertisement";
      return result;
    }

    joseLibWrapper::removePrivate(signKey_j);

    json_auto_t*        jws = json_object();
    json_auto_t*        payload = json_object();
    json_auto_t*        keys = json_array();

    // Generate the payload data
    if (json_array_append(keys, jwk_j) < 0) {
      result.success = false;
      result.msg = "Error while preparing the advertisement";
      return result;
    }

    if (json_array_append(keys, signKey_j) < 0) {
      result.success = false;
      result.msg = "Error while preparing the advertisement";
      return result;
    }

    if (json_object_set(payload, "keys", keys) < 0) {
      result.success = false;
      result.msg = "Error while preparing the advertisement";
      return result;
    }

    // Sign the payload
    binding::returnWithStatus_t   resultSign;
    resultSign = sign(joseLibWrapper::prettyPrintJson(payload), signKey);
    if (resultSign.success == false) {
      result.success = false;
      result.msg = "Failure signing the advertisement paylaod";
      return result;
    }

    result.msg = resultSign.msg;
    result.success = true;
    return result;
  }

const returnWithStatus_t keyExchange(const std::string& jwk, const std::string& req) {
    returnWithStatus_t        result;
    json_error_t              error;
    json_auto_t*              jwk_j = json_loads(jwk.data(), 0, &error);
    json_auto_t*              req_j = json_loads(req.data(), 0, &error);

    // Perform the exchange
    json_auto_t*              rep = joseLibWrapper::tang::keyExchange(jwk_j, req_j);

    if (rep == nullptr) {
      result.msg = "Failed to perform the ECMR Key exchange";
      result.success = false;
      return result;
    }

    result.msg = joseLibWrapper::printFlatJson(rep);
    result.success = true;
    return result;
}

} // namesapce binding



