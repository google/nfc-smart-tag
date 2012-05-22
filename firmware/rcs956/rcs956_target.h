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
 * Commands to control the RC-956 NFC chip as target or peer-to-peer.
 */

#ifndef __RCS956_TARGET_H__
#define __RCS956_TARGET_H__

#include <stdbool.h>
#include <stdint.h>

#include "rcs956_common.h"

// Time out from target mode after specified milliseconds
#define TG_INIT_WAIT_MS 500

/* target mode (mode 0, 1, 2, 3) */
int rcs956_tg_init(const uint8_t idm[]);
int rcs956_tg_wait_initiator(uint8_t *resp, size_t resp_len);
bool rcs956_tg_set_general_bytes(uint8_t *payload, size_t payload_len);

/* Felica target (mode 5) */
int rcs956_tg_get_init_cmd(uint8_t *resp, size_t resp_len);
bool rcs956_tg_resp2init(uint8_t *payload, size_t payload_len);

/* ISO18092 Peer-to-peer (DEP) */
int rcs956_tg_get_dep_data(uint8_t *resp, size_t resp_len);
bool rcs956_tg_set_dep_data(uint8_t *data, size_t data_len, uint8_t *status);

/* General purpose */
bool rcs956_set_param(uint8_t flags);
bool rcs956_write_register(uint16_t adr, uint8_t val);

#endif /* __RCS956_TARGET_H__ */
