/*
 * Copyright 2012 Google Inc. All Rights Reserved.
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
 *
 * Allow the base station to act as a target in the following modes:
 *   Type 3 NFC tag (Felica)
 *   SNEP NDEF Push over LLCP (ISO 18092)
 */

#ifndef __TARGET_H__
#define __TARGET_H__

#define TG_COMM_TIMEOUT_MS 512
#define MAX_TARGET_LOOP_TIMES 16

enum target_res { TGT_COMPLETE, TGT_TIMEOUT, TGT_ERROR, TGT_RETRY };

// Switch into target mode and respond to Felica or ISO 18092 requests
enum target_res target(char* label);

#endif /* !__TARGET_H__ */
