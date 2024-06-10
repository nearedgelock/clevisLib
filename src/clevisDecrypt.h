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

#include <string>

#include "jose/joseCommon.h"
#include "jose/joseClevisDecrypt.h"

namespace wasmBinding {

  class decrypt {
  public:
    decrypt(std::string jwe, bool streamMode = false);    // The JWE is expected to be complete, with the sealed secret, etc as per clevis

    const std::string                       recoveryUrl(bool full = true) const;                  // The complete URL needed to access the tang rec api, including the kid and query string
    const std::string                       transportKey() const;                                 // This is the payload of the POST request sent as the rec api call to tang
    const returnWithStatus_t                unSealSecret(const std::string responseFromTang);     // The tang interaction on JS side provides the reply here

    void                                    addCEK(json_t* c) { cek = c; };                       // For testing purpose.
    void                                    setupLarge(const std::string responseFromTang);       // Setup the object for streaming mode (supporting large data set).
    const returnWithStatus_t                feedDataLarge(const std::string data);                // Partial ciphertext
#ifdef WEB_TARGET
    const returnWithStatus_t                feedDataLargeVal(const emscripten::val& data);
#endif
    bool                                    checkTag(const std::string tag);                     // Presumably, all the ciphetext was provided and the only thinhg left to so is to check the tag
    std::string                             getLeftOver() const;

    const std::string                       secret() const { return unsealed; };
    bool                                    isPrintable() const { return printable; };

    void                                    printInfo() const;
    void                                    replaceTransportKey(const std::string pub, const std::string priv) { transport = joseLibWrapper::decrypt::transportKey(pub, priv); };
    const std::size_t                       getProtectedRawSize() const { if (jwe_j != nullptr) { return json_integer_value(json_object_get(jwe_j, "protected_rawsize")); } return 0; };
    const std::size_t                       getIVRawSize() const { if (jwe_j != nullptr) { return json_integer_value(json_object_get(jwe_j, "iv_rawsize")); } return 0; };
    
  protected:
    mutable json_t*                         jwe_j = nullptr;
    joseLibWrapper::decrypt::checkJWE       checker;
    joseLibWrapper::decrypt::transportKey   transport;
    mutable json_t*                         unwrappingJWK_j = nullptr;
    mutable json_t*                         cek = nullptr;

    std::string                             unsealed;
    bool                                    printable = false;

    mutable jose_io_t*                      input = nullptr;
    mutable jose_io_t*                      output = nullptr;
    mutable void*                           pt = nullptr;   // The plain text (portion)
    std::size_t                             ptl = 0;        // And its length

    std::string                             lastPT;         // After runnig the checkTag, some of the B64 leftover might have resulted in few extra PT bytes

    void                                    performExchange(const std::string& responseFromTang);    // The tang interaction on JW side provides the reply here
  };

} // namesapce wasmBinding



