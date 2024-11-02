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

#ifndef WEB_TARGET
//
// Adding a main entry point for testing
//

#include "clevisLib.h"
#include "clevisDecrypt.h"
#include "clevisEncrypt.h"
#include "stringSplit.h"

#include <string>
#include <iostream>
#include <sstream>

// This TANG advertisement corresponds to https://tang.nearedge-it.com/0123482c-0510-4227-a920-ba7aa83084c0
const std::string           testAdv = R"({"payload":"eyJrZXlzIjpbeyJhbGciOiJFQ01SIiwiY3J2IjoiUC01MjEiLCJrZXlfb3BzIjpbImRlcml2ZUtleSJdLCJrdHkiOiJFQyIsIngiOiJBQzIzLXJMc21rMDlKOUJVck9xWmtaeXR4Vjd4QTEwdS1LdV9RN0I0T1gtOUpEZ0dld012NnZKSGhoeE9TNjVZUEI1ejRQbFdEYkxlSkJmenNEOGVjQ0ppIiwieSI6IkFDOEhkSWpSOE1CRHVGbHR4TG9abzItUXc2ZHdfVjQwZzhiSEs2SnJKZVRmWEU3bm9pdFNkVHdXQ2t2dVlsaUlqMmFLLTd2MlhhNzQzVm5haXFsZW1rZ3YifSx7ImFsZyI6IkVTNTEyIiwiY3J2IjoiUC01MjEiLCJrZXlfb3BzIjpbInZlcmlmeSJdLCJrdHkiOiJFQyIsIngiOiJBSUs3cmtpcWVzREFrYUFkUDkxbjRBZ2Z4OGFod1JsTlJxeWZqbzd3SjNsbVktejNnamhyeEYtVml1NUdMZHZzWGNvbW52Zm84QXZqcVQtYjFNYzU1QzNLIiwieSI6IkFia0UtYkN0ZWRUWEFrdUxoMGR2Nk1OWkNBdG1Ua0FfN3p4djJUa1cyc0w0M1Vlb2RIS2ZRRk93Nzh5Qk51WXN3NTYtal9TMThNY1NIVE5ZM0NsOVRoVnoifV19","protected":"eyJhbGciOiJFUzUxMiIsImN0eSI6Imp3ay1zZXQranNvbiJ9","signature":"AJFflgtGZWe0ZFuWdSNhjbh8_wXW_jujGss0U_-Fgp6aOPU5A4TE4JQkE922ly5-PEAmXWNKAGqgB0REWkii7t0qAB9HJDyUA5VCnjvXlQZ3rqg5cJXKcQimXKDHDxK6JpBMConnrU4PyZ6-f8m5csdaDrfUKuGysqB8O1WaG96Y6g3E"})";

void testB64() {
  std::string     data("\n");

  std::string     encoded = joseLibWrapper::encodeB64((void*) data.data(), data.size(), true);
  std::cout << "Encoded data is " << encoded << std::endl;

}

int main () {
  std::cout << "Hello from main.cpp!!" << std::endl;
  //testB64();

  const std::string jwe = binding::sealSecret(testAdv,  "https://tang.nearedge-it.com/0123482c-0510-4227-a920-ba7aa83084c0", "Hello!!");
  std::cout << "The JWE is " << jwe << std::endl;


//
// Streaming mode
//


binding::encryptLarge   jweLarge(testAdv,  "https://tang.nearedge-it.com/0123482c-0510-4227-a920-ba7aa83084c0");
std::stringstream           finalJWE;

finalJWE << jweLarge.getJWEProtected() << jweLarge.getJWEIV();
finalJWE << jweLarge.feedData("Hello ", false);
finalJWE << jweLarge.feedData("&", false);    // Just to make the string not a multipe of 3B (and hence force B64 remaining bytes)
finalJWE << jweLarge.feedData("World!", true) << ".";
finalJWE << jweLarge.getJWETag();
std::cout << "The Final JWE (encrypted payload) is \n" << finalJWE.str() << std::endl;

json_t*                     cek = jweLarge.getCEK();

binding::decrypt        decryptor(finalJWE.str(), true);
decryptor.addCEK(cek);
decryptor.setupLarge("");

std::deque<std::string>     decomposedJWE =  misc::split(finalJWE.str(), '.');
std::string                 segment1 = decomposedJWE[2].substr(0, 8);
std::string                 segment2 = decomposedJWE[2].substr(8);
std::cout << "The cipher text size is (B64 encoded) " << decomposedJWE[2].size() << std::endl;

std::cout << "1st segment " << decryptor.feedDataLarge(segment1).msg << std::endl;
std::cout << "2nd segment " << decryptor.feedDataLarge(segment2).msg << std::endl;
//std::cout << "2nd segment " << decryptorStream.feedDataLarge("").msg << std::endl;

std::cout << "Decryption result is " << (decryptor.checkTag(decomposedJWE[3]) ? "valid" : "invalid") << std::endl;

/*

// Try to decrypt the just encrypted message
// The secret is 12B long, it is "Hello World\n"
const std::string           testJWE = "eyJhbGciOiJFQ0RILUVTIiwiY2xldmlzIjp7InBpbiI6InRhbmciLCJ0YW5nIjp7ImFkdiI6eyJrZXlzIjpbeyJhbGciOiJFQ01SIiwiY3J2IjoiUC01MjEiLCJrZXlfb3BzIjpbImRlcml2ZUtleSJdLCJrdHkiOiJFQyIsIngiOiJBQzIzLXJMc21rMDlKOUJVck9xWmtaeXR4Vjd4QTEwdS1LdV9RN0I0T1gtOUpEZ0dld012NnZKSGhoeE9TNjVZUEI1ejRQbFdEYkxlSkJmenNEOGVjQ0ppIiwieSI6IkFDOEhkSWpSOE1CRHVGbHR4TG9abzItUXc2ZHdfVjQwZzhiSEs2SnJKZVRmWEU3bm9pdFNkVHdXQ2t2dVlsaUlqMmFLLTd2MlhhNzQzVm5haXFsZW1rZ3YifSx7ImFsZyI6IkVTNTEyIiwiY3J2IjoiUC01MjEiLCJrZXlfb3BzIjpbInZlcmlmeSJdLCJrdHkiOiJFQyIsIngiOiJBSUs3cmtpcWVzREFrYUFkUDkxbjRBZ2Z4OGFod1JsTlJxeWZqbzd3SjNsbVktejNnamhyeEYtVml1NUdMZHZzWGNvbW52Zm84QXZqcVQtYjFNYzU1QzNLIiwieSI6IkFia0UtYkN0ZWRUWEFrdUxoMGR2Nk1OWkNBdG1Ua0FfN3p4djJUa1cyc0w0M1Vlb2RIS2ZRRk93Nzh5Qk51WXN3NTYtal9TMThNY1NIVE5ZM0NsOVRoVnoifV19LCJ1cmwiOiJodHRwczovL3RhbmcubmVhcmVkZ2UtaXQuY29tLzAxMjM0ODJjLTA1MTAtNDIyNy1hOTIwLWJhN2FhODMwODRjMCJ9fSwiZW5jIjoiQTI1NkdDTSIsImVwayI6eyJjcnYiOiJQLTUyMSIsImt0eSI6IkVDIiwieCI6IkFGNHBESFNHbVdhWlZ0TmhGTThiQmNjNFdwcHZnSlBKSlJsbHBtWVdsM3hzemJTQ2VLNHFuREdubDNydi14RzBlbWtza1hoX2d0MlFrZGhaaGt3WnJsSWQiLCJ5IjoiQVJEbVBGaktmV0JhNFI0VVRxRU9TRUlrZzdBbWVMTGQtVURvR1ZXajZhdDhvNVZrLWdGdnB4ZmNRcUtHdXM5d3JHNzFBUlZlY0duZ0NROEZfNTZsSExSbCJ9LCJraWQiOiJOanZLZUdRNUprd2Zhd1RKRXhvbGpOTmxDeGNCRVJqckZwNVZfMGVnd1RjIn0..2Ww1xod5u4QPxX98.pWg3WGt7AYU2S_AZ.3ygLqHlJEfEelCVA1MQ2aA";  
const std::string           ephemeral_priv = R"({ "alg": "ECMR", "crv": "P-521", "kty": "EC", "x": "AexxdSOgvI5A5i4lPtmFkj8N9S1oDMfNLcdy7HBX7JogactAfZo0NfdXDTHYwfKVUMTCVV5rZpJ73O7MQ3UhS1mt", "y": "APDV5glB6_Ngj_FkbRAk__HLWdOzkSAJKaMNXooZZwSfvzfZ_xV0Zwc9Ay1bPDmikgZhHgJmygrP-ksFN-ezy3_I", "d": "AG178Swdjhcr8Fm8-p_HPlzp-f9MzyPD0E5VH1J5AW9WQUDunC4HMaCJgN08uTZVivZ2Q_nDOvN6SGx-_YZDA0f5", "key_ops": [ "deriveKey" ] })";
const std::string           ephemeral_pub = R"({"kty": "EC", "crv": "P-521", "x": "AHQX4VfSVXnYBytgZ819qYfmNFROcLPv6ZckEI4KCeevZ_Z545Nn-tvYCC60r5gFCcTOSCumCVchMFzHAWBeqgCB", "y": "AXRP39W2MOAWLbt_wjkA_l4Vw5NTXg284CoLahVJFbuEX4mjeT129v__PCsnkCnlEc1Zmc_GFvgtW-AytydDsAMY"})";
const std::string           serverRecResponse = R"({"alg":"ECMR","crv":"P-521","key_ops":["deriveKey"],"kty":"EC","x":"APCBKLnNyAbpzqoN-xldbSlSqJn_skZFkJGRZYGEqPq9VUm1o38cY7j1_8sclqE39ljy3eyE4tnRhfY5Pgu0Ah64","y":"AfG2Ryf_p5_-w5YFKEa17u3EK7sKooS7U1fVCqCftTdI7_XwWiOaw9uPjdfX3NESr8ksNT8sKKKSlXY_0C64Fhnj"})";

// The secret is 505B long, It is a README file
const std::string           testJWE2 = "eyJhbGciOiJFQ0RILUVTIiwiY2xldmlzIjp7InBpbiI6InRhbmciLCJ0YW5nIjp7ImFkdiI6eyJrZXlzIjpbeyJhbGciOiJFQ01SIiwiY3J2IjoiUC01MjEiLCJrZXlfb3BzIjpbImRlcml2ZUtleSJdLCJrdHkiOiJFQyIsIngiOiJBQzIzLXJMc21rMDlKOUJVck9xWmtaeXR4Vjd4QTEwdS1LdV9RN0I0T1gtOUpEZ0dld012NnZKSGhoeE9TNjVZUEI1ejRQbFdEYkxlSkJmenNEOGVjQ0ppIiwieSI6IkFDOEhkSWpSOE1CRHVGbHR4TG9abzItUXc2ZHdfVjQwZzhiSEs2SnJKZVRmWEU3bm9pdFNkVHdXQ2t2dVlsaUlqMmFLLTd2MlhhNzQzVm5haXFsZW1rZ3YifSx7ImFsZyI6IkVTNTEyIiwiY3J2IjoiUC01MjEiLCJrZXlfb3BzIjpbInZlcmlmeSJdLCJrdHkiOiJFQyIsIngiOiJBSUs3cmtpcWVzREFrYUFkUDkxbjRBZ2Z4OGFod1JsTlJxeWZqbzd3SjNsbVktejNnamhyeEYtVml1NUdMZHZzWGNvbW52Zm84QXZqcVQtYjFNYzU1QzNLIiwieSI6IkFia0UtYkN0ZWRUWEFrdUxoMGR2Nk1OWkNBdG1Ua0FfN3p4djJUa1cyc0w0M1Vlb2RIS2ZRRk93Nzh5Qk51WXN3NTYtal9TMThNY1NIVE5ZM0NsOVRoVnoifV19LCJ1cmwiOiJodHRwczovL3RhbmcubmVhcmVkZ2UtaXQuY29tLzAxMjM0ODJjLTA1MTAtNDIyNy1hOTIwLWJhN2FhODMwODRjMCJ9fSwiZW5jIjoiQTI1NkdDTSIsImVwayI6eyJjcnYiOiJQLTUyMSIsImt0eSI6IkVDIiwieCI6IkFCUnFIZVRKOTdHOHhJdG5DM0NjcDU2WWcydTJBcC10V2hXTVV2TWlMV1h0bWVfWUdKYnNHQllYQ05ldHJObElqUU4zdFg3X2ZlOUxoNFdaVVVySWlWZzkiLCJ5IjoiQVlLRkpaN0R5ZGFBLUx5ZENJV2Q4bkdYLVYwWXhydkJQY3NDYTlNNHhjZ2FUc1hQV2N5ZEhpc1J0d3JoYW5aakpnaUsxdVZ0TW5VQVhZbDVrWU5oQmZwTiJ9LCJraWQiOiJOanZLZUdRNUprd2Zhd1RKRXhvbGpOTmxDeGNCRVJqckZwNVZfMGVnd1RjIn0..q8oxA_8whHY6__S5._iB-wGa6MvAz4OMiURJNZTEhGKl9EUcdJxiPQpZm7mJ7DFNHV1SAATbIYfOq0B19BRGWF-hHAC6EnE6tFFWxIBWQUmHKpahuWHugxigfBKCwADLJyLtEAvNlpSFA5XfvdSwLQDATbKpmW74v1ucijVvQgFisahy1OCNsq7fCpCdW8eNDcM1Dlwrqsgxd6bhFx9CdBoaoEVMujCAN2ObKtdV8m7zHeJ7psqH4IoIG_ITNQpay7ekFdTNti5GSVSou1vcUwNPrp20OoioewazCW5VHLhnCFNk-2i2Sp-ofQwPsapGPr0ebDc9p12kv29Ijp6_csoZzvzB9oLB4MJgffUbEmQmTJpiy23BlZ1TbU0ZdBgHM5k7TjCt41F_7kLcF54FYBuu8Jah_OCKOFtXBL0GWdMNQb5wrQzmLqPJcbCBH6hN0q0txiwrKfN7giwoYR3-S8CBqUOLwWl7gZwmBQvMimMVYutlDEKTLjLeDFKkhgA8U7PvZ8W3CgRuUQKLQeZ7YQkLKj5AbjDMvHjzv8Y6nOuYG4Wd3MuT4wSzj8GA69ijn8Y70XLlUfC-RP22XS5lN81iiMLu6gXykUMEAO0axVFupXG7qr32m_nlcSnj2zJESUASewmICrygqbx3Zj77XkRyBu-goj25KyOMJO1Eoko1OMczZNg.L8VdfI2K7Gz-xd7VlaxXrA";
const std::string           ephemeral2_priv = R"({ "alg": "ECMR","crv": "P-521","kty": "EC","x": "AHJrZ5CKDcTEonv7bJYOwNBXth9ZVC5UeG5U1KgWIDV7SLOOpDKTiakobXq6F8ZXoDMN0VVQGX2gvAngmkUXTZBB","y":"AKs8KxyZ4WL186YomwWn2u58aRLUGS7Nqdgue0QhRc2PVLYUbgm3OAovSmZHSzt-DXJ3CfoNIyqSbhzY9WINRFzu","d": "APWbMS39WdAYVyWpMl0FBfkXu-libzBvSsoNUrSW5DTIm1vF27FwLS3TW4voae58C3Bl5VZxGUcPDfWEHsm2ZxMC","key_ops": ["deriveKey"]})";
const std::string           ephemeral2_pub = R"({"kty": "EC","crv": "P-521","x": "ACGAOWwHEaL3A1HIqK7J2YkHWFNSjQko5vSIvNqFaLFdDocxk9XBUW2zOkPr5Lg3aveief91o0d0ihZ4TcduS4zF","y": "AY5lD-U5Y1KcEqpK66aqwRvvi2CyADvevEXa04N-9B0o7s6SL6jhr6pWgk7alT-HZVma1VhHqWgSIoQEXaZdRmL9"})";
const std::string           serverRecResponse2 = R"({"alg":"ECMR","crv":"P-521","key_ops":["deriveKey"],"kty":"EC","x":"AbWxSFBXqYLQhO2k1wAb9jhTFxEE1xSZ7MjGdSUhfKTRX7C9yUowsaA1ysFJz1fReP2VP6c8qJ5_kECCj6aVGk0L","y":"AEwMyBdou1FHeIp12zG9piwZHMEjxWVMpY4zQGIgqAeEMFAWCNZh4sFlrEOpy914DLsPDa6JknMtWJUzQF4XlcvV"})";

// Test with the non stream mode to verify that the data above is valid
binding::decrypt            decryptor(testJWE);
decryptor.replaceTransportKey(ephemeral_pub, ephemeral_priv);
std::cout << "Decrypted secret is \n" << decryptor.unSealSecret(serverRecResponse) << std::endl;

// Now, use the stream mode
std::deque<std::string>     decomposedJWE =  misc::split(testJWE2, '.');
std::string                 partialJWE = decomposedJWE[0] + "." + decomposedJWE[1];

binding::decrypt            decryptorStream(partialJWE, true);
decryptorStream.replaceTransportKey(ephemeral2_pub, ephemeral2_priv);
decryptorStream.setupLarge(serverRecResponse2);

std::string                 segment1 = decomposedJWE[2].substr(0, 8);
std::string                 segment2 = decomposedJWE[2].substr(8);

std::cout << "1st segment " << decryptorStream.feedDataLarge(segment1) << std::endl;
std::cout << "2nd segment " << decryptorStream.feedDataLarge(segment2) << std::endl;
//std::cout << "2nd segment " << decryptorStream.feedDataLarge("") << std::endl;


std::cout << "Decryption result is " << decryptorStream.checkTag(decomposedJWE[3]) << std::endl;
*/

}
#endif
