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
#include "clevisEncrypt.h"
#include "jose/joseClevisEncrypt.h"

#include <stdlib.h>
#include <sstream>
#include <iomanip>

namespace binding {
  // The advertisement as received verbatim from the server. It is a serialize JSON string with just the payload element
  const std::string decomposeAdvertisement(const std::string& adv) {
    try {
      json_auto_t*            payload_j = joseLibWrapper::encrypt::decomposeAdvertisement(adv);
      return joseLibWrapper::prettyPrintJson(payload_j);
    } catch (std::exception& exc) {
      log("Top level exception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception";
    }
  }

  // The advertisement as received verbatim from the server. It is a serialize JSON string with just the payload element
  const std::string getServerKeyFromAdvertisement(const std::string& adv) {
    try {
      json_auto_t*            payload_j = joseLibWrapper::encrypt::decomposeAdvertisement(adv);
      json_t*                 serverKey = joseLibWrapper::encrypt::getServerKeyFromAdvertisement(payload_j);

      joseLibWrapper::encrypt::skeletonJWE(payload_j, "http://example.com", "Some KID");

      return joseLibWrapper::prettyPrintJson(serverKey);
    } catch (std::exception& exc) {
      log("Top level exeception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception";
    }
  }

  json_t* prepareSealing(const std::string& adv, const std::string& url, json_t* cek) {
    if (cek == nullptr) {
      throw std::runtime_error(std::string("Missing CEK objects"));
    }

    //joseLibWrapper::logJson("The advertisement string is " + adv, nullptr);
    //joseLibWrapper::logJson("And the URL (will not fetch ourselves) is " + url, nullptr);

    json_auto_t*            payload_j = joseLibWrapper::encrypt::decomposeAdvertisement(adv);                     // Get the tang advertisement
    json_t*                 serverKey = joseLibWrapper::encrypt::getServerKeyFromAdvertisement(payload_j);        // Get the deriveKey
    const std::string       kid = joseLibWrapper::thumbprint(serverKey);    // Its thumbprint

    json_t*                 jwe = joseLibWrapper::encrypt::skeletonJWE(payload_j, url, kid);

    bool                    keyChangeResult = jose_jwe_enc_jwk(nullptr, jwe, nullptr, serverKey, cek);      // Do the ECMR key exchange

    joseLibWrapper::mergeHeaderIntoProtected(jwe);
    return jwe;
  }

  const std::string sealSecret(const std::string& adv, const std::string& url, const std::string& secret) {
    joseLibWrapper::logJson("Starting the procedure to seal a secret, size is " + std::to_string(secret.size()), nullptr);

    try {
      json_auto_t*          jwe = nullptr;
      json_auto_t*          cek = json_object();
      jwe = prepareSealing(adv, url, cek);

      log("Ready to encrypt");
      joseLibWrapper::logJson("The skeleton JWE is ", jwe);
      joseLibWrapper::logJson("And the CEK is ", cek);
      joseLibWrapper::encrypt::encryptIntoJWE (jwe, cek, (void*) secret.data(), secret.size());

      return joseLibWrapper::compact(jwe);
    } catch (std::exception& exc) {
      log("Top level exception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception";
    }
  }

#ifdef WEB_TARGET
  const std::string sealSecretVal(const std::string& adv, const std::string& url, const emscripten::val& data) {
    joseLibWrapper::logJson("Starting the procedure to seal a secret using transliteration (from a ArrayBuffer)", nullptr);

    try {
      json_auto_t*          jwe = nullptr;
      json_auto_t*          cek = json_object();
      jwe = prepareSealing(adv, url, cek);

      log("Ready to encrypt");
      joseLibWrapper::logJson("The skeleton JWE is ", jwe);
      joseLibWrapper::logJson("And the CEK is ", cek);


      // Convert JavaScript array to std::vector<char> using vecFromJSArray
      std::vector<uint8_t>  dataVector = emscripten::vecFromJSArray<uint8_t>(data);
      log("Payload size (to encrypt) is " + std::to_string(dataVector.size()));

      joseLibWrapper::encrypt::encryptIntoJWE (jwe, cek, (void*) dataVector.data(), dataVector.size());

      return joseLibWrapper::compact(jwe);
    } catch (std::exception& exc) {
      log("Top level exception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception";
    }
  }
#endif

  // We expect to receive a base64 decoded JSON string corresponding to the payload returned by the server.
  bool validateAdvertisement(const std::string& adv) {
    try {
      json_error_t            error;
      json_auto_t*            adv_j = json_loads(adv.data(), 0, &error);

      if (adv_j != nullptr) {
        bool result = joseLibWrapper::encrypt::validateAdvertisement(adv_j);
        return result;
      }
      throw joseLibWrapper::invalidJson();
    } catch (std::exception& exc) {
      log("Top level exeception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception";      
    }
  }

  //
  // Calass object that implements the ability to encrypt in a stream fashion, supporting limitless plain-text (and hence
  // cipher-text) size.
  //
  
  encryptLarge::encryptLarge(const std::string& adv, const std::string& url) {
    cek = json_object();

    try {
      // Perform the advertisment analysis and prepare the CEK (content Encryption Key)
      jwe = prepareSealing(adv, url, cek);

      // Prepare the streaming IO
      output = jose_io_malloc(nullptr, &ct, &ctl);
      input = jose_jwe_enc_cek_io(nullptr, jwe, cek, output);

      if ( (jwe == nullptr) or (output == nullptr) or ( input == nullptr) ) {
        throw std::runtime_error("One of the expected object were not created");
      }
    } catch (std::exception& exc) {
      log("Failed to create the encryptLarge object - " + std::string(exc.what()));
      rethrowIfAllowed();
    }
  }

  const std::string encryptLarge::feedData(const std::string& data, bool final) {
    bool                retval = input->feed(input, data.data() , data.size());
    if (retval == false) {
      return "Error while feed encryptor";
    }

    const std::string   ctPartialB64 = joseLibWrapper::extractPartial(output, final, true);

    if (final == true) {
      if (finalize() == false) {
        return "Error while finalizing encryption";
      };
    }

    return ctPartialB64;
  };

#ifdef WEB_TARGET
  const std::string encryptLarge::feedDataVal(const emscripten::val& data, bool final) {
    //joseLibWrapper::logJson("Starting the procedure to seal a secret using transliteration (from a ArrayBuffer)", nullptr);

    try {
      // Convert JavaScript array to std::vector<char> using vecFromJSArray
      std::vector<uint8_t>  dataVector = emscripten::vecFromJSArray<uint8_t>(data);
      //log("Payload size (to encrypt) is " + std::to_string(dataVector.size()));

      if (dataVector.size() != 0) {
        bool                  retval = input->feed(input, dataVector.data() , dataVector.size());
        if (retval == false) {
          return "Error while feed encryptor";
        }
      }

      if (final == true) {
        bool retval = input->done(input);

        if (retval == false) {
          log("Failed to complete the encryption operation via done");
          return "Error doing the final (done) step)";
        }
      }

      const std::string   ctPartialB64 = joseLibWrapper::extractPartial(output, final, true);

      //if (final == true) {
      //  if (finalize() == false) {
      //    return "Error while finalizing encryption";
      //  };
      //}

      return ctPartialB64;
    } catch (std::exception& exc) {
      log("Top level exception - " + std::string(exc.what()));
      rethrowIfAllowed();
      return "Got an exception";
    }
  }
#endif

  bool encryptLarge::finalize() {
    // Finalyze the encryption. The resulting tag will be added to the JWE object by the lower
    //printInfo();
    bool retval = input->done(input);
    //printInfo();

    if (retval == false) {
      log("Failed to complete the encryption operation via done");
      return retval;
    }

    // All is left is to append the ciphter text to the JWE
    json_t*           ctB64 = joseLibWrapper::encodeB64ToJson(ct, ctl, true);
    int i = json_object_set_new(jwe, "ciphertext", ctB64);
    if (i < 0) {
      return false;
    }
    return true;
  }

  void encryptLarge::printInfo() const {
    joseLibWrapper::logJson("The JWE (large data) ", jwe); 
  }

  void encryptLarge::freeJson() {
    if (input != nullptr) {
      jose_io_auto(&input);
    }

    if (output != nullptr) {
      jose_io_auto(&output);
    }

    if (jwe != nullptr) {
      json_decref(jwe);
    }

    if (cek != nullptr) {
      json_decref(cek);
    }

  }

} // namesapce binding



