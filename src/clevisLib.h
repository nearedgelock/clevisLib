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

#ifdef WEB_TARGET
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

namespace binding {
  extern bool               isLogEnabled;
  extern bool               allowException;     // Top level function / methods may generate exceptions. If not, returning normally.

  inline void               enableLog() { isLogEnabled = true; };
  inline void               enableException() { allowException = true; };
  inline void               rethrowIfAllowed() { if (allowException == true) { throw; } };

  using returnWithStatus_t =    struct returnWithStatus {
                                  std::string   msg;
                                  bool          success;
                                };


  const std::string         generateKey(bool forSigning = false);
  void                      log(const std::string& msg);
  const returnWithStatus_t  sign(const std::string& payload, const std::string& sign);
  const returnWithStatus_t  thumbprint(const std::string& jwk);  

} // namespace binding



