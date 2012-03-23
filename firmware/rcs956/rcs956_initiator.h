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
 * Commands to control the RC-956 NFC chip as initiator.
 */

#ifndef __RCS956_INITIATOR_H__
#define __RCS956_INITIATOR_H__

#include <stdint.h>
#include <stdbool.h>

// Turn off RF field.
void rcs956_rf_off(void);

// Checks whether a target (card, phone) is present.
bool initiator_poll(uint8_t idm[], uint8_t pmm[], uint16_t syscode);

// Sends data to the NFC module and receives a response.
int initiator_command(uint8_t *payload, size_t payload_len,
                      uint8_t *resp, size_t resp_len,
                      uint16_t timeout);

// Defines the retry count for RF communication for InListPassiveTarget
bool rcs956_set_retry(uint8_t retry);

// Defines the retry count for RF communication for InCommunicateThrough
bool rcs956_set_retry_com(uint8_t retry);

// Sets RF Communication Timeout Value used by InCommunicateThru
bool rcs956_set_timeout(uint8_t timeout);

#endif /* __RCS956_INITIATOR_H__ */
