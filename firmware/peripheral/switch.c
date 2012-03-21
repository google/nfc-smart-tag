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
 * Read switch status connected to PD.
 * CAUTION: if a switch is not implemented, it is *ON*.
 */

#include "switch.h"

#include <util/delay.h>

#define SWITCH_OUT_PORT PORTD
#define SWITCH_IN_PORT PIND
#define SWITCH_DDR_PORT DDRD

/**
 * Read switch status.
 *
 * Arguments:
 *      id: id of a switch.
 * Returns:
 *      true if switch is on (open), or switch does not exist.
 *      false if switch is off (closed).
 *
 * Note that behavior is undefined against unknown id.
 */
bool switch_is_on(enum SWITCH id)
{
  SWITCH_DDR_PORT &= ~id; // configure input
  SWITCH_OUT_PORT |= id; // enables pull-up
  _delay_us(10);
  bool on = ((SWITCH_IN_PORT & id) != 0);
  SWITCH_OUT_PORT &= ~id; // disables pull-up not to consume power.
  return on;
}
