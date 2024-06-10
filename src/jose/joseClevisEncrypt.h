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
#pragma once

#include <string>
#include <stdexcept>

#include <jansson.h>

namespace joseLibWrapper {
namespace encrypt {

  json_t*           decomposeAdvertisement(const std::string& adv) ;
  json_t*           encryptIntoJWE (json_t* baseJWE, json_t* cek, void* payload, std::size_t size);
  json_t*           getServerKeyFromAdvertisement(const json_t* adv);
  json_t*           skeletonJWE(const json_t* advKeys, const std::string& url, const std::string& kid);
  bool              validateAdvertisement(const json_t* adv);

  class failedEncrypt: public joseException {
  public:
    failedEncrypt(const std::string& msg = ""): joseException ("Failed to encrypt a payload and generate a JWE "+ ((msg.empty() == false) ? (" - " + msg) : "")) { };
  };

  class failedExtractPublicKey: public joseException {
  public:
    failedExtractPublicKey(): joseException ("Failed to extract the public part of a Key ") { };
  };

  class failedGetServerKey: public joseException {
  public:
    failedGetServerKey(const std::string& msg = ""): joseException ("Invalid or missing server Key " + ((msg.empty() == false) ? (" - " + msg) : "")) { };
  };

} // namespace encrypt
} // namespace joseLibWrapper



