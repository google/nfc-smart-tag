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
 * Controls power to NFC module via a MOSFET connected to an I/O pin.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "module_power.h"
#include "usart.h"

/*
 * Turns on power to NFC module and waits until it is
 * ready to receive commands.
 */
void module_power_up(void)
{
  MODULE_POWER_DDR |= _BV(MODULE_POWER_PIN);
  // Output port low = power on
  MODULE_POWER_PORT &= ~_BV(MODULE_POWER_PIN);
  // RC-620 Startup time ~70ms
  _delay_ms(100);
  usart_init();
}

/*
 * Disables the serial port and turns off power to the NFC module.
 */
void module_power_down(void)
{
  // Sets USART pins to high impedance to avoid powering the module
  // through the I/O pins.
  usart_disable();
  _delay_ms(1);
  // Output high impedance = power off (bias transistor)
  MODULE_POWER_DDR &= ~_BV(MODULE_POWER_PIN);
  MODULE_POWER_PORT &= ~_BV(MODULE_POWER_PIN);
}
