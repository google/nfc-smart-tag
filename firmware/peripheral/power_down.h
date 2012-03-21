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
 * Helper routines to put the AVR into power save mode for
 * a specified amount of time or until external interrupt. Power consumption is
 * dramatically reduced, but the sleep time is not very accurate.
 */

#ifndef __POWER_DOWN_H_
#define __POWER_DOWN_H_

#include <stdbool.h>

// Use this to compute how many times to call sleep_until_timer, passing
// the desired duration in milliseconds. This avoids floating point computation
// at runtime.
// The macro rounds up, i.e. actual sleep time may be longer than
// the specified value in ms.
#define SLEEP_COUNT_CLK_DOWN(x) ((x - 1) / (8 * 1024L * 255 * 1000 / F_CPU)) + 1
#define SLEEP_COUNT(x) ((x - 1) / (1024L * 255 * 1000 / F_CPU)) + 1

// Disable AVR modules not being used to save power
void disable_unused_circuits();

// Sleeps in low power mode until Timer 2 overflows.
void sleep_until_timer(uint8_t mode, bool clock_down);

// Sleep AVR in lowest power state (set wake-up condition before!).
void sleep_forever();

// Configures AVR to wake up when PCINT1 goes low
void wakeup_on_external_interrupt(void);

// Triggers a hard reset when the voltage on pin PD3 changes.
void reset_on_power_change(void);

// Disables automatic reboot on ext. power change.
void disable_reset_on_power_change(void);

#endif  // __POWER_DOWN_H_
