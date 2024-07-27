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
#include <deque>

#include <jansson.h>
//#include "helpers/log.h"
namespace joseLibWrapper {
namespace decrypt {
  json_t*                     checkJWEValidity(const std::string& jwe);
  void                        checkJWEValidity_tang(const json_t* hdr);
  void                        checkJWEValidity_sss(json_t* hdr);
  void                        checkJWEValidity_tpm(json_t* hdr);

  json_t*                     decomposeCompactJWE(const std::string& jwe, bool partial = false);
  std::string                 recoverPayload(const json_t* jwk, const json_t* jwe);
  json_t*                     validateKey(const json_t* kid, const json_t* keys);

  // Small class whose job is to validate the validity fo the JWE provided by the user. Since
  // we support tang, ss and tpm, and may be mode, we need a way to provide
  // different tests as well as different pice of data.
  class checkJWE {
  public:
    checkJWE() { };
    checkJWE(const json_t* jwe);
    ~checkJWE() { if (jweProtectedHeaders_j != nullptr) json_decref(jweProtectedHeaders_j); };

    json_t*                   getHeader() const { return jweProtectedHeaders_j; };

    json_t*                   getEpk() const { return epk_j; };
    json_t*                   getEpkCurve() const { return epkCurve_j; };
    json_t*                   getKid() const { return kid_j; };
    json_t*                   getKeys() const { return allKeys_j; }
    json_t*                   getActiveKey() const { return activeServerKey_j; };

    std::string               getUrl() const { return extractedUrl; };

    void                      printInfo() const;
  protected:
    bool                      checkJWEValidity_tang(const json_t* hdr);
    bool                      checkJWEValidity_sss(const json_t* hdr);
    bool                      checkJWEValidity_tpm(const json_t* hdr);

    json_t*                   jweProtectedHeaders_j = nullptr;

    // For the tang pin
    // Borrowed references, which must NOT be freed
    json_t*                   epk_j = nullptr;
    json_t*                   epkCurve_j = nullptr;
    json_t*                   kid_j = nullptr;
    json_t*                   allKeys_j = nullptr;
    json_t*                   activeServerKey_j = nullptr;

    std::string               extractedUrl;

  public:
    void                      printEPK() const { logJson("EPK: ", epk_j); };
    void                      printEPKCurve() const { logJson("EPK Curve: ", epkCurve_j); };
    void                      printKID() const { logJson("KID: ", kid_j); };
    void                      printAllKeys() const { logJson("Keys: ", allKeys_j); };
    void                      printSelectedKey() const { logJson("Selected key: ", activeServerKey_j); };
    void                      printSelectedServerKey() const {logJson("Active server key: ", activeServerKey_j); };
    void                      printProtectedHeader(bool force = false) const { logJson("Protected header: \n", jweProtectedHeaders_j); };
  };
  
  // This class handles the ephemeral transport key. It generate a pairwise key, extract the public part, etc.
  // serialize and deserialize it. 
  class transportKey {
  public:
    transportKey() { };
    transportKey(const json_t* crv, const json_t* epk);
    transportKey(const std::string pub, const std::string priv);    // For testing with known value
    ~transportKey() { if (ephemeralKey_j != nullptr) json_decref(ephemeralKey_j); };

    json_t*                   getEphemeralKey() const { return  ephemeralKey_j; };
    std::string               getEphemeralKeyPub() const { return ephemeralKey_pub; };

    void                      printInfo() const;
  protected:
    json_t*                   ephemeralKey_j = nullptr;
    std::string               ephemeralKey_pub;
  };
 
  class invalidJWT: public joseException {
  public:
    invalidJWT(): joseException("JWE base format is invalid" ) { };
  };

  class notClevisTang: public joseException {
  public:
    notClevisTang(): joseException("The provided JWE does not include the clevis header or is not the correct pin" ) { };
  };

  class invalidKIDHash: public joseException {
  public:
    invalidKIDHash(const std::string& msg = ""): joseException ("Key ID hash is invalid " + ((msg.empty() == false) ? (" - " + msg) : "")) { };
  };
  
  class failedGetServerKey: public joseException {
  public:
    failedGetServerKey(const std::string& msg = ""): joseException ("Invalid or missing server Key " + ((msg.empty() == false) ? (" - " + msg) : "")) { };
  };

  class permanentTangFailure: public joseException {
  public:
    permanentTangFailure(const std::string& msg = ""): joseException ("Permanent failure from tang " + ((msg.empty() == false) ? (" - " + msg) : "")) { };
  };
} // namespace decrypt
} // namespace joseLibWrapper



