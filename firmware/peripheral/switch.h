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
 * Read switch status connected to PD5 and PD7.
 * CAUTION: if switch does not exist, returns *on*.
 */


#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <avr/io.h>

#include <stdbool.h>

enum SWITCH {
  SW1 = _BV(PD5),
  SW2 = _BV(PD7),
};

// Read switch status (true = on or switch does not exist)
bool switch_is_on(enum SWITCH id);

#endif /* __SWITCH_H__ */
