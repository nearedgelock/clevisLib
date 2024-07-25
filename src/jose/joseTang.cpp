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
#include "joseTang.h"

#include <cstring>
#include <stdlib.h>

extern "C" {
#include "jose/jwk.h"
}

namespace joseLibWrapper {
namespace tang {

  json_t* keyExchange(json_t* jwk, json_t* req) {

    if ( (jwk == nullptr) or (req == nullptr) ) {
      return nullptr;
    }

    // ECMR key exchange
    json_auto_t*              rep = jose_jwk_exc(nullptr, jwk, req);

    if (rep == nullptr) {
      return nullptr;
    }

    // Add the alg and key_ops fields
    if (json_object_set_new(rep, "alg", json_string("ECMR")) < 0) {
      return nullptr;
    }

    if (json_object_set_new(rep, "key_ops", json_pack("[s]", "deriveKey")) < 0) {
      return nullptr;
    }

    json_incref(rep);
    return rep;

  }
} // namespace tang
} // namespace joseLibWrapper



