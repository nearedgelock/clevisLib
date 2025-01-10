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

#include "clevisLib.h"
#include "jose/joseCommon.h"

#include <string>
#include <jansson.h>

extern "C" {
#include "jose/jwe.h"
}

namespace binding {
  const std::string         decomposeAdvertisement(const std::string& adv);

  const std::string         getServerKeyFromAdvertisement(const std::string& adv, bool signing = false, bool keepOps = false);
  json_t*                   prepareSealing(const std::string& adv, const std::string& url, json_t* cek, const std::string& ancillary = "");
  const std::string         sealSecret(const std::string& adv, const std::string& url, const std::string& secret, const std::string& ancillary = "");

#ifdef WEB_TARGET
  const std::string         sealSecretVal(const std::string& adv, const std::string& url, const emscripten::val& data, const std::string& ancillary = "");
#endif

  bool                      validateAdvertisement(const std::string& adv);

  // A class object that wraps and persists what is needed to encrypt large payload using
  // a streaming methodology.
  class encryptLarge {
  public:
    encryptLarge(const std::string& adv, const std::string& url, const std::string& ancillary = "");
    ~encryptLarge() { freeJson(); };

    const std::string       feedData(const std::string&, bool final = false);
#ifdef WEB_TARGET
    const std::string       feedDataVal(const emscripten::val& data, bool final);
#endif
    bool                    finalize(); 

    const std::string       getFinalJWE() const { return joseLibWrapper::compact(jwe); };
    const std::string       getJWEProtected() const { return joseLibWrapper::compactPartialProtected(jwe); };
    const std::string       getJWEIV() const { return joseLibWrapper::compactPartialIV(jwe); };
    const std::string       getJWECiphertext() const { return joseLibWrapper::compactPartialCiphertext(jwe); };
    const std::string       getJWETag() const { return joseLibWrapper::compactPartialTag(jwe); };

    void                    printInfo() const;
    json_t*                 getCEK() { return cek; };
  protected:
    void                    freeJson();

    mutable json_t*         jwe = nullptr;
    mutable json_t*         cek = nullptr;

    mutable jose_io_t*      input = nullptr;
    mutable jose_io_t*      output = nullptr;
    mutable void*           ct = nullptr;   // The cipher text
    std::size_t             ctl = 0;        // And its length
  };

} // namespace binding



