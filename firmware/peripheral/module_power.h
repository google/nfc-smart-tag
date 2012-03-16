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
 * Controls power to NFC module.
 */

#ifndef __MODULE_POWER_H__
#define __MODULE_POWER_H__

// Which pin the MOSFET is connected
#define MODULE_POWER_DDR DDRD
#define MODULE_POWER_PORT PORTD
#define MODULE_POWER_PIN 4

// Turns on power to NFC module and waits until it is ready to receive commands
void module_hard_power_up(void);

// Disables the serial port and turns off power to the NFC module.
void module_hard_power_down(void);

#endif /* __MODULE_POWER_H__ */
