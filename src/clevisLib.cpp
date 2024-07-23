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
      std::cout << (msg.empty() ? "" : msg) << std::endl;
    }
  };

} // namespace binding

#ifdef WEB_TARGET
EMSCRIPTEN_BINDINGS(clevisLib) {
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
    .function("isPrintable", &binding::decrypt::isPrintable)
	  .function("recoveryUrl", &binding::decrypt::recoveryUrl)
    .function("transportKey", &binding::decrypt::transportKey)
    .function("unSealSecret", &binding::decrypt::unSealSecret)
    .function("secret", &binding::decrypt::secret)
    .function("feedDataLargeVal", &binding::decrypt::feedDataLargeVal)
    .function("setupLarge", &binding::decrypt::setupLarge)
    .function("checkTag", &binding::decrypt::checkTag)
    .function("getLeftOver", &binding::decrypt::getLeftOver)
    .function("printInfo", &binding::decrypt::printInfo)
    .function("getProtectedRawSize", &binding::decrypt::getProtectedRawSize)
    .function("getIVRawSize", &binding::decrypt::getIVRawSize)
    ;

  emscripten::class_<binding::encryptLarge>("Encryptor")
    .constructor<const std::string, const std::string>()
    .function("feedData", &binding::encryptLarge::feedData)
    .function("feedDataVal", &binding::encryptLarge::feedDataVal)
    .function("finalize", &binding::encryptLarge::finalize)
    .function("getFinalJWE", &binding::encryptLarge::getFinalJWE)
    .function("getJWEProtected", &binding::encryptLarge::getJWEProtected)
    .function("getJWEIV", &binding::encryptLarge::getJWEIV)
    .function("getJWECiphertext", &binding::encryptLarge::getJWECiphertext)
    .function("getJWETag", &binding::encryptLarge::getJWETag)
    .function("printInfo", &binding::decrypt::printInfo)
    ;
}
#endif
