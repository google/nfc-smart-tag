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
 * Web safe base64 encoding.
 */

#ifndef __WEBSAFE_BASE64_ENCODE_H__
#define __WEBSAFE_BASE64_ENCODE_H__

#include <stdbool.h>
#include <stdint.h>

// Encodes a data block in web safe base 64 encoding.
bool websafe_base64_encode(char output[], int output_size,
                           const uint8_t input[], int input_size);

#endif /* __WEBSAFE_BASE64_ENCODE_H__ */
