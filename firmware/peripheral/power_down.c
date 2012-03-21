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
 *
 * Uses Timer 2 to wake up. Use when no other interrupts are happening,
 * so chip actually stays asleep.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "power_down.h"

/*
 * Disable AVR modules not being used to save power
 */
void disable_unused_circuits()
{
  power_twi_disable();
  ACSR = _BV(ACD);  // Disable analog comparator (60uA)
}

/*
 * Goes into specified sleep mode. Be sure to configure a wake-up
 * condition, otherwise only a power reset can wake the chip up.
 */
static void __sleep(uint8_t mode)
{
  set_sleep_mode(mode);
  // disable brown-out to save power and sleep
  cli();
  sleep_enable();
  sleep_bod_disable();
  // Make sure we can wake up
  sei();
  sleep_cpu();
  sleep_disable();
}

/*
 * Sleeps in low power mode until Timer 2 overflows.
 * Provides option to reduce clock speed to allow longer sleep time.
 *
 * Sleep time with clock_down:
 * 8 * 1024 * 255 * 1000 / F_CPU ms (584ms @3.58MHz, 104ms @20MHz)
 *
 * Sleep time without clock_down:
 * 1024 * 255 * 1000 / F_CPU ms (73ms @3.58MHz, 13ms @20MHz)
 *
 * SLEEP_MODE_IDLE:
 * Draws about 0.3mA @ 3.58MHz @ 4V, and wakes up immediately
 * 1.0mA avg (measured) when sleep_until_timer is called in a loop.
 *
 * SLEEP_MODE_PWR_SAVE:
 * Draws about 1uA @ 3.58MHz @ 4V, but takes 4ms + 1024clk to wake up
 * 0.1mA avg (measured) when sleep_until_timer is called in a loop.
 */
void sleep_until_timer(uint8_t mode, bool clock_down)
{
  TCCR2B = 0; // Stop timer while we setup
  TCNT2 = 0; // Set timer value to 0
  TIMSK2 |= _BV(TOIE2); // Enable Overflow interrupt

  // Preserve original power settings
  uint8_t power_reduction = PRR;

  // Disable additional circuits to save power
  power_adc_disable();

  // Clock down if requested
  if (clock_down) {
    clock_prescale_set(clock_div_8);
  }

  // Clock = Fcpu/1024 = 3496Hz @ 3.58MHz
  TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20);

  __sleep(mode);

  // Clock back to normal
  clock_prescale_set(clock_div_1);

  // Restore previous power settings
  PRR = power_reduction;
}

/*
 * Sleep AVR in lowest power state. If you want to wake up, make
 * sure to set a wakeup or reset condition beforehand.
 * Note that watch dog timer can awake, so disable beforehand if applicable.
 */
void sleep_forever()
{
  // Preserve original power settings
  uint8_t power_reduction = PRR;
  PRR = 0xFF;  // turn off all peripherals

  __sleep(SLEEP_MODE_PWR_DOWN);

  // Restore previous power settings
  PRR = power_reduction;
}

/*
 * Configures AVR to wake up when PCINT1 goes low.
 */
void wakeup_on_external_interrupt(void)
{
  // configure PB1 as input.
  DDRB &= ~_BV(PB1);
  // PB1 pull-up.
  PORTB |= _BV(PB1);

  // Enable level change on PB1 (PCINT1) to trigger PCINT0
  PCICR |= _BV(PCIE0);
  PCMSK0 |= _BV(PCINT1);
}

/*
 * Triggers a hard reset (via WDT) when the voltage on pin PD3 changes.
 * By wiring this pin to ext power, this will reset the device when ext.
 * power is plugged in our out. This is needed to wake up a battery device
 * from low battery shut-off.
 */
void reset_on_power_change(void) {
  // Configure PD3 (PCINT19) as Input
  DDRD &= ~_BV(PD3);
  // No Pull-up
  PORTD &= ~_BV(PD3);

  // Enable PCINT19 pin level change to trigger PCINT2
  PCMSK2 |= _BV(PCINT19);
  PCICR |= _BV(PCIE2);
}

/*
 * Disables automatic reboot on ext. power change.
 */
void disable_reset_on_power_change(void) {
  PCMSK2 &= ~_BV(PCINT19);
}

EMPTY_INTERRUPT(TIMER2_OVF_vect)

EMPTY_INTERRUPT(PCINT0_vect)

__attribute__((naked)) ISR(PCINT2_vect)
{
  // TODO: Do not count this as a WDT reset as it is intentional

  // Force a cold reset by triggering watchdog timer
  // http://support.atmel.no/bin/customer.exe?=&action=viewKbEntry&id=21
  wdt_enable(WDTO_15MS);
  while(1);
}
