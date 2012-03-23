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
 * Initiator feature of the smart tag.
 * Polls for a Felica-equipeed mobile phone and pushes a URL.
 */

#ifndef __INITIATOR_H__
#define __INITIATOR_H__

#include <stdbool.h>

// The minimum retry count that worked will all tested handsets
#define NUM_RETRY_POLL 2
#define NUM_RETRY_COMM 0
#define NUM_RETRY_INITIATOR_LOOP 5
#define TIMEOUT_STYLE 0x0d /* 50 * 2^(TIMEOUT_STYLE) [us] i.e. ~400ms*/
#define COMM_TIMEOUT_MS 1000

// Main initiator feature. Pools for phone and pushes URL.
bool initiator(const char push_label[]);

// Set default values for time-out to Felica module.
void initiator_set_defaults(void);

#endif /* !__INITIATOR_H__ */
