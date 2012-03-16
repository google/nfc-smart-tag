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
 
 * Measure battery voltage and determine low battery condition.
 *
 * Note: Discard first reading so band gap voltage can stabilize
 * Note: Accuracy depends on the band gap reference, which can vary
 *       between 1.0 and 1.2V from chip to chip
 */

#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "battery.h"

#define VOLT2ADC(v) (uint8_t)(256 / v * 1.1)
#define ADMUX_BANDGAP 0x0E
#define PORT_POWER_SENSE PD3

/*
 * Initializes the ADC and the voltage reference,
 * and waits until they are ready to be used.
 */
void adc_init(void)
{
  // Enable the ADC
  ADCSRA = _BV(ADEN);

  // Set the Band Gap voltage as the ADC input (0x0E) and VCC as
  // voltage reference. Left-adjust the ADC data register, so we can
  // read top 8 bits easily (we do not need 10 bits precision).
  ADMUX = _BV(REFS0) | _BV(ADLAR) | ADMUX_BANDGAP;

  // The bandgap takes max 70us to initialize (Sec 28.5 AVR Ref Manual).
  _delay_us(70);
}

/*
 * Because the ADC and the voltage reference consume power (200-300uA
 * combined), they should be turned off before deep sleep.
 */
void adc_disable(void)
{
  // Make sure ADC does not use voltage ref
  ADMUX = 0;

  // Stop ADC conversions and disable the ADC.
  ADCSRA = 0;
}

/*
 * Returns VCC voltage as a ratio of Band gap voltage (1.1v nominal)
 * to system voltage. For example, 4.4V returns 64. Voltage can be
 * computed as 256 / (v+0.5) * 1.1.
 *
 * Must call init_adc before calling this method.
 */
uint8_t read_voltage(void)
{
  // Start conversion, clock / 32 ~ 100kHz. Full conversion takes 25 cycles.
  ADCSRA |= _BV(ADIE) | _BV(ADSC) | 5;
  // Sleep during conversion to reduce noise
  set_sleep_mode(SLEEP_MODE_IDLE);
  // We need interrupt to wake up
  sei();
  // sleep until the conversion is complete (in case other interrupt hits)
  do {
    sleep_mode();
  } while (ADCSRA & _BV(ADSC));
  // Read only high 8 bits
  return ADCH;
}

/*
 * Returns true iff battery voltage (returned from read_voltage) is below
 * a specified threshold.
 */
bool is_battery_low(uint8_t voltage)
{
  return (voltage >= VOLT2ADC(BATT_LOW_LEVEL));
}

bool is_battery_dead(uint8_t voltage)
{
  return (voltage >= VOLT2ADC(BATT_DEAD_LEVEL));
}

/*
 * Returns true iff the device is plugged into external power.
 */
bool is_on_external_power(void)
{
#ifdef HAS_CHARGER
  // Configure PD3 (PCINT19) as Input
  DDRD &= ~_BV(PORT_POWER_SENSE);
  // No Pull-up
  PORTD &= ~_BV(PORT_POWER_SENSE);
  // stabilize
  _delay_us(10);
  // High pin means ext power
  return (PIND & _BV(PORT_POWER_SENSE));
#else
  return true;
#endif /* HAS_CHARGER */
}

/*
 * Do nothing on AD Conversion complete.
 */
EMPTY_INTERRUPT(ADC_vect)
