/*
 * Copyright 2012 Google Inc. All Rights Reserved.
 * Author: ghohpe@google.com (Gregor Hohpe)
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
 * Measure battery voltage and determine low battery condition.
 *
 * Note: Discard first reading so band gap voltage can stabilize
 * Note: Accuracy depends on the band gap reference, which can vary
 *       between 1.0 and 1.2V from chip to chip
 */

#ifndef __BATTERY_H__
#define __BATTERY_H__

#include <stdbool.h>

#define BATT_LOW_LEVEL 3.5 // Volt for LiPoly batteries
#define BATT_DEAD_LEVEL 3.1 // Volt NFC module needs 3.3V +/-5%

/*
 * Initializes the ADC and the voltage reference,
 * and waits until they are ready to be used (~100uS).
 */
void adc_init(void);

/*
 * Because the ADC and the voltage reference consume power (200-300uA
 * combined), they should be turned off before deep sleep.
 */
void adc_disable(void);

/*
 * Returns VCC voltage as a ratio of Band gap voltage (1.1v nominal)
 * to system voltage. For example, 4.4V returns 64. Voltage can be
 * computed as 256 / (v+0.5) * 1.1
 */
uint8_t read_voltage(void);

/*
 * Returns true iff battery voltage (returned from read_voltage) is below
 * a specified threshold.
 */
bool is_battery_low(uint8_t voltage);
bool is_battery_dead(uint8_t voltage);

/*
 * Returns true iff the device is plugged into external power.
 */
bool is_on_external_power(void);

#endif /* __BATTERY_H__ */
