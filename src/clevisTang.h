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
#include <jansson.h>

namespace binding {
  const returnWithStatus_t      createJWKAdvertisement(const std::string& jwk, const std::string& signKey);
  const returnWithStatus_t      keyExchange(const std::string& jwk, const std::string& req);
} // namespace binding



