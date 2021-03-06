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
 * Common commands for RC-S956 NFC Chip
 *
 * See http://www.sony.co.jp/Products/felica/business/tech-support
 *
 */

#ifndef __RC956_COMMON_H__
#define __RC956_COMMON_H__

#include <stdbool.h>

// Sends and receives data through NFC
int rcs956_comm_thru_ex(uint8_t *payload, size_t payload_len,
                        uint8_t *resp, size_t resp_len,
                        uint16_t timeout);

// Sets the NFC module into mode 0.
bool rcs956_reset(void);

// Wakes up NFC module from soft power down.
void rcs956_serial_wake_up(void);

#endif /* !__RC956_COMMON_H__ */
