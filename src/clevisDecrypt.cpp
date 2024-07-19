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
#include "clevisDecrypt.h"
#include "jose/joseCommon.h"
#include "jose/joseClevisDecrypt.h"

#include <stdlib.h>
#include <sstream>
#include <iomanip>

extern "C" {
#include "jose/jwe.h"
#include "jose/b64.h"
}

namespace wasmBinding {
  decrypt::decrypt(std::string jwe, bool streamMode ) {
    try {
      log("Constructing a decrypt object");

      // Remove all trailing \n
      while( (jwe.back() == '\n') and (jwe.empty() == false) ) {
        jwe.pop_back();
      }

      jwe_j = joseLibWrapper::decrypt::decomposeCompactJWE(jwe, streamMode);    // With streamMode, the jwe only needs to have the protected header and the IV.
    
      //joseLibWrapper::logJson("The JWE to check is ", jwe_j);
      checker = joseLibWrapper::decrypt::checkJWE(jwe_j);

      //checker.printProtectedHeader();
      //checker.printEPK();
      //checker.printSelectedServerKey();

      transport = joseLibWrapper::decrypt::transportKey(checker.getEpkCurve(), checker.getEpk());
      //transport.printInfo();
    } catch (std::exception& exc) {
      log("Exception while constructing decrypt - " + std::string(exc.what()));
    }
  }

  const std::string decrypt::recoveryUrl(bool full) const {
    if (full == true) {
      return std::string (checker.getUrl() +"/rec/" + kid());
    } else {
      return std::string (checker.getUrl());
    }
  }

  const std::string decrypt::transportKey() const {
    return transport.getEphemeralKeyPub();    
  }

  const returnWithStatus_t decrypt::unSealSecret(const std::string responseFromTang) {
    try {
      log("Unseal a secret into a regular std::string");

      // exchange
      performExchange(responseFromTang);

      //
      // Actual secret unsealing
      //
      joseLibWrapper::logJson("Now, lets recover the payload, the unwrappingkey is ", unwrappingJWK_j);
      unsealed = joseLibWrapper::decrypt::recoverPayload(unwrappingJWK_j, jwe_j);
      log("Payload size (after decrypt) is " + std::to_string(unsealed.size()));

      printable = joseLibWrapper::isPrintable(unsealed);
      //printable = false;    // Force to always use B64 between us and JS...
      //if (printable == false) {
        // There are some non printable character in the unsealed secret. Lets base64 encode the result
        //log("Non printable character, we base64 encode the result");
        return {joseLibWrapper::encodeB64(unsealed, false), true};
      //} else {
        // Text data
      //  log("Normal text data");
      //  return unsealed;
      //}
    } catch (std::exception& exc) {
      log("Exception while unsealing a secret - " + std::string(exc.what()));
      return {"No secret !!!", false};
    }
  }

  void decrypt::setupLarge(const std::string responseFromTang) {
    jose_io_auto_t*     d = nullptr;

    try {
      if (cek == nullptr) {
        // exchange
        performExchange(responseFromTang);
        cek = jose_jwe_dec_jwk(nullptr, jwe_j, nullptr, unwrappingJWK_j);
      }

      output = jose_io_malloc(nullptr, &pt, &ptl);
      d = jose_jwe_dec_cek_io(nullptr, jwe_j, cek, output);
      input = jose_b64_dec_io(d);

    } catch(std::exception& exc) {
      throw;
    }

    if ( (output == nullptr) or ( input == nullptr) or ( d == nullptr) or ( cek == nullptr) ) {
      throw std::runtime_error("One of the expected object were not created");
    }
  }

  const returnWithStatus_t decrypt::feedDataLarge(const std::string data) {
    try {
      if (data.size() != 0) {
        if (input->feed(input, (void *)data.data(), data.size())== false) {
          // Something went wrong
          log("Something wrong when performing a decryption operation");
        }
      } else {
        if(input->done(input) == false) {
          // Something went wrong
          log("Something wrong when performing the last (using done) decryption operation");
        }        
      }

      // Pull the plain text data from the output buffer
      joseLibWrapper::io_malloc_t*        o = containerof(output, joseLibWrapper::io_malloc_t, io);
      std::string         retval((char*)*o->buf, *o->len);
      *o->len = 0;
      return {retval, true};
    } catch(std::exception& exc) {
      log("Got a failure during feedDataLarge (streaming) - " + std::string(exc.what()));
      return {"Failure", false};
    }
  }

#ifdef WEB_TARGET
  const returnWithStatus_t decrypt::feedDataLargeVal(const emscripten::val& data) {
    try {
      // Convert JavaScript array to std::vector<char> using vecFromJSArray
      std::vector<uint8_t>  dataVector = emscripten::vecFromJSArray<uint8_t>(data);
      //log("Payload size (to decrypt) is " + std::to_string(dataVector.size()));

      if (input->feed(input, dataVector.data(), dataVector.size())== false) {
        // Something went wrong
        log("Unknown failure at feeding data to the input io object");
      }

      // Pull the plain text data from the output buffer
      joseLibWrapper::io_malloc_t*        o = containerof(output, joseLibWrapper::io_malloc_t, io);
      std::string         retval((char*)*o->buf, *o->len);
      //log("Decrypted payload size (before B64 reencoding) " + std::to_string(retval.size()));
      
      *o->len = 0;
      const std::string     result = joseLibWrapper::encodeB64(retval, false);
      return {std::move(result), true};
    } catch (std::exception& exc) {
      log("Top level exception - " + std::string(exc.what()));
      return {"Got an exception", false};
    }
    }
#endif

  bool decrypt::checkTag(const std::string tag) {
    // Insert the tag inot the JWE
    json_object_set(jwe_j, "tag",  json_string(tag.data()));
    bool                tagVerificationResult = input->done(input);

    // The last feed operation might have left some bytes in the input buifer (because of bae64 leftover). 
    // We need to recover them
    joseLibWrapper::io_malloc_t*        o = containerof(output, joseLibWrapper::io_malloc_t, io);
    std::string         pt((char*)*o->buf, *o->len);
    //log("Decrypted payload size (before B64 reencoding) " + std::to_string(pt.size()));

    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(pt.front());
    //log("First of the remaining byte is " + ss.str());
      
    *o->len = 0;
    lastPT = joseLibWrapper::encodeB64(pt, false);
    //log("Last encoded bytes are " + lastPT);

    return tagVerificationResult;
  }

  std::string decrypt::getLeftOver() const {
    //log("Returning the last few bytes, as B64 " + lastPT);
    return lastPT;
  }

  void decrypt::printInfo() const {
    joseLibWrapper::logJson("The base JWE is " + std::string((jwe_j != nullptr) ? "defined, and its value is " : "Undefined"), jwe_j);
    checker.printInfo();
    transport.printInfo();
  }

  void decrypt::performExchange(const std::string& responseFromTang) {
    try {
      //
      // Equivalent to tang interaction but using the provided response
      //
      log("Performing the exchange");

      json_auto_t*                  exchangedKey = nullptr;
      json_auto_t*                  recoveredKey = nullptr;

      recoveredKey = joseLibWrapper::extractB64ToJson(responseFromTang, true);

      exchangedKey = joseLibWrapper::keyExchange(transport.getEphemeralKey(), checker.getActiveKey());

      joseLibWrapper::removePrivate(recoveredKey);
      unwrappingJWK_j = joseLibWrapper::keyExchange(recoveredKey, exchangedKey, true);
      //Probably not a good idea to show this, even for debug DEBUG() << "Unwrapping JWK: " << joseLibWrapper::prettyPrintJson(unwrappingJWK_j) << std::endl;

    } catch (std::exception& exc) {
      log("Exception while unsealing a secret - " + std::string(exc.what()));
      throw std::runtime_error(std::string("Error during key exchange."));
    }
  }

  void decrypt::freeJson() {
    json_decref(jwe_j);
    json_decref(unwrappingJWK_j);
    json_decref(cek);

    jose_io_decref(input);
    jose_io_decref(output);
  }

} // namesapce wasmBinding


