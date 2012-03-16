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
 * Controls an LED connected to an output port.
 */

#ifndef __LED_H__
#define __LED_H__

// The port to which the LED is connected
#define LED_PORT PORTB0

#include <stdint.h>

void led_on(void);
void led_off(void);
void led_toggle(void);
void blink(uint8_t delay_100ms, uint8_t count);
#endif /* __LED_H__ */
