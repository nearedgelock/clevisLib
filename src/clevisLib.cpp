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

namespace wasmBinding {
  
  bool      isLogEnabled = false;

  void log(const std::string& msg) {
    if (isLogEnabled) {
      std::cout << (msg.empty() ? "" : msg) << std::endl;
    }
  };

} // namespace wasmBinding

#ifdef WEB_TARGET
EMSCRIPTEN_BINDINGS(clevisLib) {
  emscripten::function("decomposeAdvertisement", &wasmBinding::decomposeAdvertisement);
  emscripten::function("generateKey", &wasmBinding::generateKey);
  emscripten::function("getServerKeyFromAdvertisement", &wasmBinding::getServerKeyFromAdvertisement);
  emscripten::function("sealSecret", &wasmBinding::sealSecret);
  emscripten::function("sealSecretVal", &wasmBinding::sealSecretVal);

  emscripten::function("validateAdvertisement", &wasmBinding::validateAdvertisement);

  emscripten::value_object<wasmBinding::returnWithStatus_t>("returnWithStatus_t")
    .field("msg", &wasmBinding::returnWithStatus_t::msg)
    .field("success", &wasmBinding::returnWithStatus_t::success)
    ;

  emscripten::class_<wasmBinding::decrypt>("Decryptor")
    .constructor<const std::string, bool>()
    .function("isPrintable", &wasmBinding::decrypt::isPrintable)
	  .function("recoveryUrl", &wasmBinding::decrypt::recoveryUrl)
    .function("transportKey", &wasmBinding::decrypt::transportKey)
    .function("unSealSecret", &wasmBinding::decrypt::unSealSecret)
    .function("secret", &wasmBinding::decrypt::secret)
    .function("feedDataLargeVal", &wasmBinding::decrypt::feedDataLargeVal)
    .function("setupLarge", &wasmBinding::decrypt::setupLarge)
    .function("checkTag", &wasmBinding::decrypt::checkTag)
    .function("getLeftOver", &wasmBinding::decrypt::getLeftOver)
    .function("printInfo", &wasmBinding::decrypt::printInfo)
    .function("getProtectedRawSize", &wasmBinding::decrypt::getProtectedRawSize)
    .function("getIVRawSize", &wasmBinding::decrypt::getIVRawSize)
    ;

  emscripten::class_<wasmBinding::encryptLarge>("Encryptor")
    .constructor<const std::string, const std::string>()
    .function("feedData", &wasmBinding::encryptLarge::feedData)
    .function("feedDataVal", &wasmBinding::encryptLarge::feedDataVal)
    .function("finalize", &wasmBinding::encryptLarge::finalize)
    .function("getFinalJWE", &wasmBinding::encryptLarge::getFinalJWE)
    .function("getJWEProtected", &wasmBinding::encryptLarge::getJWEProtected)
    .function("getJWEIV", &wasmBinding::encryptLarge::getJWEIV)
    .function("getJWECiphertext", &wasmBinding::encryptLarge::getJWECiphertext)
    .function("getJWETag", &wasmBinding::encryptLarge::getJWETag)
    .function("printInfo", &wasmBinding::decrypt::printInfo)
    ;
}
#endif
