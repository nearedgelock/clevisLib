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

extern "C" {
#include "jose/io.h"
}

namespace joseLibWrapper {  
  std::string       compact(const json_t* jwe);
  std::string       compactPartialProtected(const json_t* jwe);
  std::string       compactPartialIV(const json_t* jwe);
  std::string       compactPartialCiphertext(const json_t* jwe);
  std::string       compactPartialTag(const json_t* jwe);

  std::string       encodeB64(const std::string& payload, bool urlFriendly);
  std::string       encodeB64(const void* payload, size_t input_length, bool urlFriendly);
  json_t*           encodeB64ToJson(const std::string& payload, bool urlFriendly);
  json_t*           encodeB64ToJson(const void* payload, size_t input_length, bool urlFriendly);
  std::string       extractB64(const std::string& encoded, bool urlFriendly = false);
  std::string       extractB64(const char input[], size_t input_length, bool urlFriendly = false);
  json_t*           extractB64ToJson(const std::string& encoded, bool nodecode = false, bool urlFriendly = false);
  json_t*           extractB64ToJson(const json_t* encoded, bool nodecode = false, bool urlFriendly = false);

  const std::string extractPartial(jose_io_t* out, bool final, bool urlFriendly);    // This extracts the partial CT from the given object and return it base64 encoded


  bool              isPrintable(const std::string& data);

  void              logJson(const std::string& msg, const json_t* val);
  void              logJWE(const std::string& msg, json_t* jwe);
  const std::string prettyPrintJson(const json_t* val);

  json_t*           generateKey(const json_t* crv = nullptr, bool forSigning = false);
  json_t*           keyExchange(const json_t* key1, const json_t* key2, bool defaultAlgo = false);
  json_t*           keyExchange(const std::string& key1, const json_t* key2, bool defaultAlgo = false);
  void              mergeHeaderIntoProtected(json_t* jwe);
  void              removePrivate(json_t* key);
  std::string       thumbprint(const json_t* jwk);

  class joseException: public std::runtime_error {
  public:
    joseException(const std::string& msg = ""): runtime_error("Exception in JOSE (use --trace to get more info)" + ((msg.empty() == false) ? (" - " + msg) : "")) { };
  };
  
  // Exception classes, including an overall base exception class
  class failCompact: public joseException {
  public:
    failCompact(): joseException("Failed to create a compact representation" ) { };
  };

  class failRemovePriv: public joseException {
  public:
    failRemovePriv(): joseException("Failed to remove the private part of a key pair" ) { };
  };

  class invalidB64: public joseException {
  public:
    invalidB64(): joseException("Invalid, missing B64 data, or malformed JSON" ) { };
  };

  class invalidJson: public joseException {
  public:
    invalidJson(): joseException("Invalid or malformed JSON" ) { };
  };

  class failedGenerateKey: public joseException {
  public:
    failedGenerateKey(): joseException ("Unable to generate an Key ") { };
  };


  //
  // Some of the following is taken verbatim from the JOSE sources. They are not defined in headers but we need them :-(
  //
  typedef struct {
    jose_io_t io;
    void **buf;
    size_t *len;
  } io_malloc_t;

} // namespace joseLibWrapper

#define containerof(ptr, type, member) \
    ((type *)((char *) ptr - offsetof(type, member)))



