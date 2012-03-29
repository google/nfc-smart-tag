/*
 * Copyright 2011 Google Inc. All Rights Reserved.
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
 * Constructs a digitally signed one-time URL.
 */

#ifndef __GENERATE_URL_H__
#define __GENERATE_URL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef URL
#define URL "http://nfc-smart-tag.appspot.com/nfc?nv="
#undef WITHOUT_V_FIELD
#endif /* !URL */

#define IDM_BYTES 8
#define STATION_ID_BYTES 8
#define COUNTER_BYTES 4
#define VERSION_BYTES 1
#define STATION_KEY_BYTES 16
#define URL_VERSION 2
#define URL_LENGTH 128

/* build URL */
bool build_url(char *url_buffer, size_t url_buffer_size, uint8_t *idm);

/* set extra data to be transmitted as part of URL */
void set_extra_url_data(uint8_t voltage);

#endif /* __GENERATE_URL_H__ */
