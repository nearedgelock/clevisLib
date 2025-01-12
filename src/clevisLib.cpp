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
#include "clevisLib.h"
#include "clevisDecrypt.h"
#include "clevisEncrypt.h"

#include <string>
#include <stdlib.h>
#include <iostream>

#ifdef WEB_TARGET
// Binding to call JS code
EM_JS(void, logToJS, (const char* msg), {
  Module.logFromWasm(UTF8ToString(msg));
});
#endif

extern "C" {
  void      logToJS(const char* msg);
}

namespace binding {
  
  bool      isLogEnabled = false;
  bool      allowException = false;

  const std::string generateKey(bool forSigning) {
    try {
      json_auto_t*        key = joseLibWrapper::generateKey(nullptr, forSigning);

      if (key != nullptr) {
        const std::string   result = joseLibWrapper::prettyPrintJson(key);
        return result;
      } else {
        return nullptr;
      }
    } catch (std::exception &exc) {
      log("Top level exeception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception while generating a new key";
    }
  }

  void log(const std::string& msg) {
    if (isLogEnabled) {
#ifdef WEB_TARGET
      logToJS(msg.c_str());
#else
      std::cout << (msg.empty() ? "" : msg) << std::endl;
#endif
    }
  };

  const returnWithStatus_t sign(const std::string& payload, const std::string& signKey) {
    returnWithStatus_t        result;

    json_t* sign(json_t* payload, json_t* sign);

    json_error_t              error;
    json_auto_t*              sign_j = json_loads(signKey.data(), 0, &error);
    json_auto_t*              payload_j = json_loads(payload.data(), 0, &error);
    json_auto_t*              sig_template = json_pack("{s:{s:s}}", "protected", "cty", "jwk-set+json");

    json_auto_t*              payloadSigned = joseLibWrapper::sign(payload_j, sign_j);

    if (payloadSigned == nullptr) {
      result.success = false;
      result.msg = "Error while signing, using jose_jws_sig";
      return result;
    }

    result.success = true;
    result.msg = joseLibWrapper::printFlatJson(payloadSigned);
    return result;    
  };

  const returnWithStatus_t thumbprint(const std::string& jwk) {
    returnWithStatus_t        result;
    json_error_t              error;
    json_auto_t*              jwk_j = json_loads(jwk.data(), 0, &error);

    if (jwk_j != nullptr) {
      result.success = true;
      result.msg = joseLibWrapper::thumbprint(jwk_j);
      return result;
    }

    result.success = false;
    return result;
  } 


} // namespace binding

#ifdef WEB_TARGET
//Binding to enable calling from JS
EMSCRIPTEN_BINDINGS(clevisLib) {
  emscripten::function("enableLog", &binding::enableLog);

  emscripten::function("decomposeAdvertisement", &binding::decomposeAdvertisement);
  emscripten::function("generateKey", &binding::generateKey);
  emscripten::function("getServerKeyFromAdvertisement", &binding::getServerKeyFromAdvertisement);
  emscripten::function("sealSecret", &binding::sealSecret);
  emscripten::function("sealSecretVal", &binding::sealSecretVal);

  emscripten::function("validateAdvertisement", &binding::validateAdvertisement);

  emscripten::value_object<binding::returnWithStatus_t>("returnWithStatus_t")
    .field("msg", &binding::returnWithStatus_t::msg)
    .field("success", &binding::returnWithStatus_t::success)
    ;

  emscripten::class_<binding::decrypt>("Decryptor")
    .constructor<const std::string, bool>()
    .function("ancillary", &binding::decrypt::ancillary)
    .function("checkTag", &binding::decrypt::checkTag)
    .function("feedDataLargeVal", &binding::decrypt::feedDataLargeVal)
    .function("getIVRawSize", &binding::decrypt::getIVRawSize)
    .function("getLeftOver", &binding::decrypt::getLeftOver)
    .function("getProtectedRawSize", &binding::decrypt::getProtectedRawSize)
    .function("isPrintable", &binding::decrypt::isPrintable)
    .function("printInfo", &binding::decrypt::printInfo)
    .function("recoveryUrl", &binding::decrypt::recoveryUrl)
    .function("secret", &binding::decrypt::secret)
    .function("setupLarge", &binding::decrypt::setupLarge)
    .function("transportKey", &binding::decrypt::transportKey)
    .function("unSealSecret", &binding::decrypt::unSealSecret)
    ;

  emscripten::class_<binding::encryptLarge>("Encryptor")
    .constructor<const std::string, const std::string, const std::string>()
    .function("isValid", &binding::encryptLarge::isValid)
    .function("feedData", &binding::encryptLarge::feedData)
    .function("feedDataVal", &binding::encryptLarge::feedDataVal)
    .function("finalize", &binding::encryptLarge::finalize)
    .function("getFinalJWE", &binding::encryptLarge::getFinalJWE)
    .function("getJWEProtected", &binding::encryptLarge::getJWEProtected)
    .function("getJWEIV", &binding::encryptLarge::getJWEIV)
    .function("getJWECiphertext", &binding::encryptLarge::getJWECiphertext)
    .function("getJWETag", &binding::encryptLarge::getJWETag)
    .function("printInfo", &binding::encryptLarge::printInfo)
    ;
}
#endif
