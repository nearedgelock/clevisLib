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
#include "joseCommon.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <iomanip>

#include <stdlib.h>

extern "C" {
  #include "jose/jwk.h"
  #include "jose/jws.h"
}

#include "botan/base64.h"

namespace joseLibWrapper {

  bool      isLogEnabled = false;

  std::string compact(const json_t* jwe) {
    // Convert the given JWE to its compact representation
    return compactPartialProtected(jwe) + compactPartialIV(jwe) + compactPartialCiphertext(jwe) + compactPartialTag(jwe);
  }

  std::string compactPartialProtected(const json_t* jwe) {
    std::string   protectedHdr = json_string_value(json_object_get(jwe, "protected"));
    if (protectedHdr.empty() == true) {
      throw failCompact();
    }
    return protectedHdr + "..";
  };

  std::string compactPartialIV(const json_t* jwe) {
    std::string   iv = json_string_value(json_object_get(jwe, "iv"));
    if (iv.empty() == true) {
      throw failCompact();
    }
    return iv + ".";
  };

  std::string compactPartialCiphertext(const json_t* jwe) {
    std::string   ciphertext = json_string_value(json_object_get(jwe, "ciphertext"));
    if (ciphertext.empty() == true) {
      throw failCompact();
    }
    return ciphertext + ".";
  };

  std::string compactPartialTag(const json_t* jwe) {
    std::string   tag = json_string_value(json_object_get(jwe, "tag"));
    if (tag.empty() == true) {
      throw failCompact();
    }
    return tag;
  };

  std::string encodeB64(const std::string& payload, bool urlFriendly) {
    return encodeB64((void*) payload.data(), payload.size(), urlFriendly);
  };

  std::string encodeB64(const void* payload, size_t input_length, bool urlFriendly) {
    std::string      encoded = Botan::base64_encode((uint8_t*) payload, input_length);

    if (urlFriendly == true) {
      std::replace(encoded.begin(), encoded.end(), '+', '-');
      std::replace(encoded.begin(), encoded.end(), '/', '_');
      encoded.erase(std::remove(encoded.begin(), encoded.end(), '='), encoded.end());
    }
 
    return encoded;
  };

  json_t* encodeB64ToJson(const std::string& payload, bool urlFriendly) {
    return encodeB64ToJson((void*) payload.data(), payload.size(), urlFriendly);
  };

  json_t* encodeB64ToJson(const void* payload, size_t input_length, bool urlFriendly) {
    json_error_t        error;
    json_t*             retval = nullptr;

    const std::string   encoded = encodeB64(payload, input_length, urlFriendly);

    retval = json_string((char *)encoded.c_str());

    return retval;
  };

  std::string extractB64(const std::string& encoded, bool urlFriendly) {
    if (urlFriendly == true) {
      std::string     notUrlSafe = encoded;
      std::replace(notUrlSafe.begin(), notUrlSafe.end(), '-', '+');
      std::replace(notUrlSafe.begin(), notUrlSafe.end(), '_', '/');

      const Botan::secure_vector<uint8_t>  decoded = Botan::base64_decode(notUrlSafe, true);
      return std::string(decoded.begin(), decoded.end());
    }

    const Botan::secure_vector<uint8_t>  decoded = Botan::base64_decode(encoded, true);
    return std::string(decoded.begin(), decoded.end());
  };

  std::string extractB64(const char input[], size_t input_length, bool urlFriendly) {
    if (urlFriendly == true) {
      std::string     notUrlSafe(input, input_length);
      return  extractB64(notUrlSafe, true);
    }
    const Botan::secure_vector<uint8_t>  decoded = Botan::base64_decode(input, input_length, true);
    return std::string(decoded.begin(), decoded.end());
  };

  json_t* extractB64ToJson(const std::string& encoded, bool nodecode, bool urlFriendly) {
    json_error_t                  error;
    json_t*                       retval = nullptr;

    try {
      if (nodecode == false) {
        std::string                 b64Decoded = extractB64(encoded, urlFriendly);
        retval = json_loads((char *)b64Decoded.data(), 0, &error);
      } else {
        retval = json_loads((char *)encoded.data(), 0, &error);
      }

      if (retval != nullptr) {
        return retval;
      }

      throw invalidB64();
    } catch (std::exception& exc) {
      throw invalidB64();
    }
  }

  json_t* extractB64ToJson(const json_t* encoded, bool nodecode, bool urlFriendly) {
    std::string       value = json_string_value(encoded);
    return extractB64ToJson(value, nodecode, urlFriendly);
  }

  bool isPrintable(const std::string& data) {
    std::size_t     pos = 0;
    for (const auto& character : data) {
      if ((std::isprint(character) == false) and (character != '\n')  and (character != '\t')) {
        logJson("Found a non printable character at position " + std::to_string(pos), nullptr);
        return false;
      }
      ++pos;
    }

    return true;
  }

  const std::string extractPartial(jose_io_t* io, bool final, bool urlFriendly) {
    // This extracts the partial CT from the given object and return it base64 encoded
    //
    // If final is false, the extraction size will be modulo 3 and any remaining bytes (upto 2) will be left at the
    // beginng of the buffer. This is done so that subsequent calls (where more data is expected have been added to the
    // buffer) constinues the same base64 stream.
    //
    // If final is true, then the whole buffer is encoded.
    //
    // This whole functilon is levearing the jose IO concepts
    //
    io_malloc_t*        i = containerof(io, io_malloc_t, io);
    size_t              processSize = *i->len;
    size_t              notProcessSize = 0;

    // Determine the number of bytes to include in the base64 encoding and how
    // many will be left over
    if (final == false) {
      notProcessSize = processSize % 3;            // Any remaining will be left over
      processSize = processSize - notProcessSize;  // So that we get a multiple of 3 bytes
    }

    // Perform the base64 encoding
    const std::string   retval = encodeB64(*i->buf, processSize, urlFriendly);

    // Empty the buffer and add back the left over bytes (up to 2)
    if ( (final == false) and (notProcessSize != 0) ) {
      unsigned char     remainingBytes[2];
      // There is 1 or 2 bytes to grab.
      remainingBytes[0] = ((unsigned char*)*i->buf)[processSize];
      if (notProcessSize == 2) {
        remainingBytes[1] = ((unsigned char*)*i->buf)[processSize+1];
      }

      // Reset the size of the buffer - This is inspire by malloc_feed
      *i->len = notProcessSize;
      void *tmp = realloc(*i->buf, *i->len);
      *i->buf = tmp;

      // Insert the left over bytes
      ((unsigned char*)*i->buf)[0] = remainingBytes[0];
      if (notProcessSize == 2) { 
        ((unsigned char*)*i->buf)[1] = remainingBytes[1];
      }
    } else {
      *i->len = 0;
    }
    return retval;
  }

  const std::string prettyPrintJson(const json_t* val) {
    const char*   result = json_dumps(val, JSON_INDENT(2));
    if (result != nullptr) {
      std::string   retval(result);
      free((void*)result);
      return retval;
    } else if (json_is_string(val)) {
      return std::string(json_string_value(val));
    } else {
      return "Invalid or empty JSON value";
    }
  }

  const std::string printFlatJson(const json_t* val) {
    const char*   result = json_dumps(val, JSON_INDENT(0) | JSON_SORT_KEYS | JSON_COMPACT);
    if (result != nullptr) {
      std::string   retval(result);
      free((void*)result);
      return retval;
    } else if (json_is_string(val)) {
      return std::string(json_string_value(val));
    } else {
      return "Invalid or empty JSON value";
    }    
  };

  void logJson(const std::string& msg, const json_t* val) {
    if (isLogEnabled) {
      std::cout << (msg.empty() ? "" : msg) << (val != nullptr ? prettyPrintJson(val) : "") << std::endl;
    }
  };

  void logJWE(const std::string& msg, json_t* jwe) {
    // The protected header may be base64 encoded. If so we decode it before logging the JWE
    json_t*         protectedHdr = json_object_get(jwe, "protected");
    if (json_is_string(protectedHdr) == false) {
      logJson(msg, jwe);
    } else {
      json_auto_t*  decodedHdr = extractB64ToJson(protectedHdr, false);
      json_incref(protectedHdr);
      json_object_set(jwe, "protected", decodedHdr);
      logJson(msg, jwe);
      json_object_set(jwe, "protected", protectedHdr);
      json_decref(protectedHdr);
    }
  };

  json_t* generateKey(const json_t* crv, bool forSigning) {
    try {
      //"ES512"
      json_auto_t*  jwk = nullptr;
      if (crv == nullptr) {
        jwk = json_pack("{s:s}", "alg", (forSigning ? "ES512" : "ECMR"));
      } else {
        jwk = json_pack("{s:s, s:s}", "alg", (forSigning ? "ES512" : "ECMR"), "crv", json_string_value(crv));
      }
      if (jose_jwk_gen(nullptr, jwk) == true) {      
        return json_incref(jwk);
      }

      throw failedGenerateKey();
    } catch (std::exception& exc) {
      throw;
    }
  };

  json_t* keyExchange(const json_t* key1, const json_t* key2, bool defaultAlgo) {
    json_auto_t*      exchangedKey = nullptr;

    exchangedKey = jose_jwk_exc(nullptr, key1, key2);
    if (exchangedKey != nullptr) {
      return json_incref(exchangedKey);
    }

    throw failedGenerateKey();
  }

  json_t* keyExchange(const std::string& key1, const json_t* key2, bool defaultAlgo) {
    return keyExchange(extractB64ToJson(key1, true), key2, defaultAlgo);
  }

  void mergeHeaderIntoProtected(json_t* jwe) {
    // When using the compact format, the data from the header section must be included in the protected header.
    json_t*       protectedHdr = json_object_get(jwe, "protected");
    json_t*       header = json_object_get(jwe, "header");

      json_object_update(protectedHdr, header);
      json_object_del(jwe,"header"); 
  };

  void removePrivate(json_t* key) {
    if ( (key != nullptr) and (jose_jwk_pub(nullptr, key) == true) ) {
      return ;
    }
    throw failRemovePriv();
  }

  json_t* sign(json_t* payload, json_t* sign) {
    json_auto_t*              sig_template = json_pack("{s:{s:s}}", "protected", "cty", "jwk-set+json");
    json_auto_t*              jws = json_pack("{s:o}", "payload", encodeB64ToJson(printFlatJson(payload), false));

    if (jose_jws_sig(nullptr, jws, sig_template, sign) == false) {
      return nullptr;
    }

    json_incref(jws);
    return jws; 
  }

  std::string thumbprint(const json_t* jwk) {
    // Compute the SHA256 thumbprint of the given key
    // First, we need to find the required size for the buffer
    std::size_t       retval = jose_jwk_thp_buf(nullptr, jwk, "S256", nullptr, (char)0);

    // Now, compute the actual thumbprint
    std::string       thumbprint(retval, 0);
    retval = jose_jwk_thp_buf(nullptr, jwk, "S256", (uint8_t*)thumbprint.data(), retval);

    if (retval == -1) {
      // Failure, the jwk is probably not a JWK....
      throw joseException("Can not compute thumbprint - The provided JWK is invalid");
    }
    
    return encodeB64(thumbprint, true);
  };


} // namespace joseLibWrapper


