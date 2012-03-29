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
 * Main part of the base station firmware.
 * Initializes the whole system, and goes into a service.
 */

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "eeprom_data.h"
#include "initiator.h"
#include "nfc_url2.h"
#include "melodies.h"
#include "peripheral/battery.h"
#include "peripheral/eeprom.h"
#include "peripheral/lcd.h"
#include "peripheral/led.h"
#include "peripheral/module_power.h"
#include "peripheral/power_down.h"
#include "peripheral/switch.h"
#include "rcs956/rcs956_common.h"
#include "rcs956/rcs956_initiator.h"
#include "rcs956/rcs956_protocol.h"

// Milliseconds delay between polling. Rough time for one main loop.
#define SLEEP_AFTER_TIMEOUT 500

// Converts seconds into number of main loop iterations.
#define SECS2COUNT(x) (int)((x) / (SLEEP_AFTER_TIMEOUT / 1000.0))

// Google Place in Japanese Shift-JIS encoding.
#define PUSH_URL_LABEL "Google\x83\x76\x83\x8c\x83\x43\x83\x58"
#define PUSH_URL_LABEL_ENGLISH "Google Place"
#define WATCHDOG_TIMEOUT WDTO_4S

// How many times to retry after seeing an initiator
#define TARGET_MODE_RETRY 10

// Battery options
#define CHECK_BATT_EVERY_NSECS 3600 /* beep on low battery once / hour */
#define CHECK_BATT_ONCE_AFTER_SECS 2 /* beep on low battery after power up */

// Timing for WITH_BLINK_LED option (indicate device is on)
// 15ms every 5sec: avg power draw 30uA for 10mA LED
#define BLINK_LED_SLEEP_SEC 5
#define BLINK_LED_DURATION_MS 15

#define BLINK_PATTERN_INTERVAL SECS2COUNT(15)
#define SLEEP_AFTER_N_SECS 180 /* turn off after 3 min until PUSH BUTTON */

/*
 * Stop the watchdog timer and track reason for reset in EEPROM.
 *
 * ".init3" ensures that reset_mcusr will be executed before main (since 0-2 is
 * reserved, 3 is used)
 * See Also: http://www.nongnu.org/avr-libc/user-manual/mem_sections.html
 *
 * Note: Disable watchdog timer even if it is not used.
 * See: AVR Manual 10.8 Watchdog Timer.
 */
void reset_mcusr(void) \
         __attribute__((naked)) \
         __attribute__((section(".init3")));
void reset_mcusr(void)
{
  uint8_t mcusr = MCUSR;
  MCUSR = 0;
  wdt_disable();
  eeprom_count_mcusr(mcusr);
}

#ifdef WITH_WATCHDOG
/* Hardware reset if watchdog timer not cleared in decided interval. */
void watchdog_start(void)
{
  wdt_enable(WATCHDOG_TIMEOUT);
}

/* clear the watchdog timer. */
void watchdog_reset(void)
{
  wdt_reset();
}

/* disable watchdog time, mainly to save power */
void watchdog_disable(void)
{
  wdt_disable();
}

#else /* !WITH_WATCHDOG */
#define watchdog_start() ((void)0)
#define watchdog_reset() ((void)0)
#define watchdog_disable() ((void)0)
#endif /* WITH_WATCHDOG */

#ifdef HAS_LCD
static void print_idle(void)
{
  uint8_t station_id[STATION_ID_BYTES];

  eeprom_read_station_id(station_id);
  lcd_puts(0, "Base Station");
  lcd_print_hex(1, station_id, sizeof(station_id));
}
#else /* !HAS_LCD */
#define print_idle()
#endif /* HAS_LCD */
static void sleep_until_melody_completes(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  // In this tight loop we are guaranteed one last interrupt
  // after is_melody_playing() returns false
  while (is_melody_playing()) {
    sleep_mode();
    watchdog_reset();
  }
}

/*
 * Play a song and wait for its completion in idle mode
 */
static void play_song_and_wait(const struct note *song, uint8_t size)
{
  play_melody(song, size);
  sleep_until_melody_completes();
}

static void beep_n_times_and_wait(uint8_t count)
{
  beep_n_times(count);
  blink(1, count << 1);
  sleep_until_melody_completes();
}

static void play_url_push_success_song_and_wait(void)
{
  if (switch_is_on(SW1)) {
    play_song_and_wait(melody_googlenfc001,
                       sizeof(melody_googlenfc001) / sizeof(struct note));
  } else {
    play_song_and_wait(melody_kayac_beep,
                       sizeof(melody_kayac_beep) / sizeof(struct note));
  }
}

#ifndef WITH_TARGET
static void sleep_after_timeout(void)
{
  watchdog_reset();
  rcs956_rf_off();

  // Active sleep if blinking, deep sleep otherwise.
  for (uint8_t i = 0; i < SLEEP_COUNT_CLK_DOWN(SLEEP_AFTER_TIMEOUT); i++) {
    sleep_until_timer(SLEEP_MODE_PWR_SAVE, true);
  }
}
#endif /* ! WITH_TARGET */

// Let voltage settle before first check
static int batt_check_counter = SECS2COUNT(CHECK_BATT_ONCE_AFTER_SECS);

// Beep and blink every once in a while if battery is low
static void low_battery_check(void)
{
  if (--batt_check_counter == 0) {
    adc_init();
    uint8_t voltage = read_voltage();
    adc_disable();
    set_extra_url_data(voltage);
    if (is_battery_low(voltage)) {
      beep_n_times_and_wait(4);
      // The battery_dead threshold should be set high enough to avoid dropping
      // the AVR into BOD when the RF field is on, because the processor may
      // stop with the field on, which drains the battery rapidly.
      if (is_battery_dead(voltage)) {
        // Turn off the NFC module to minimize power consumption
        module_power_down();
        wdt_disable();
        sleep_forever();
      }
    }
    batt_check_counter = SECS2COUNT(CHECK_BATT_EVERY_NSECS);
  }
}

int main(void)
{
  disable_unused_circuits();

  /* Shut off right away if battery is very low. Do not power on NFC module */
  _delay_ms(50);
  adc_init();
  (void)read_voltage();
  // Read one more time in case first reading is corrupted
  uint8_t voltage = read_voltage();
  adc_disable();
  if (is_battery_dead(voltage)) {
    //beep_n_times_and_wait(4);
    //sleep_forever();
  }

  lcd_init();
  print_idle();

  // Initialize and self-test
  module_power_up();
  led_on();
  while (!rcs956_reset()) {};

  if (!eeprom_has_station_info()) {
    beep_n_times_and_wait(3);
    sleep_forever();
  }
  if (is_on_external_power()) {
    play_song_and_wait(melody_start_up_external,
                       sizeof(melody_start_up_external) / sizeof(struct note));
  } else {
    play_song_and_wait(melody_start_up_battery,
                       sizeof(melody_start_up_battery) / sizeof(struct note));
  }
  led_off();
  initiator_set_defaults();
#ifdef HAS_CHARGER
  reset_on_power_change();
#endif /* HAS_CHARGER */
  watchdog_start();

  for (;;) {
    watchdog_reset();
    // initiator exits after polling times out (false) or URL is pushed (true)
    if (initiator(PUSH_URL_LABEL)) {
      lcd_puts(0, "PUSH SLEEP");
      play_url_push_success_song_and_wait();
    }
#ifdef WITH_TARGET
    uint8_t loop;
    // Loop here to not skip watchdog timer
    for (loop = 0; loop < TARGET_MODE_RETRY; loop++) {
      watchdog_reset();
      (void)rcs956_reset();
      enum target_res res = target(PUSH_URL_LABEL_ENGLISH);
      if (res == TGT_COMPLETE) {
        reset_idle();
        led_off();
        play_url_push_success_song_and_wait();
        break;
      } else if (res == TGT_TIMEOUT || res == TGT_ERROR) {
        break;
      } // loop on TGT_RETRY
    }
    led_off();
    (void)rcs956_reset();
    low_battery_check();
#else /* !WITH_TARGET */
    // Check battery level while RF field is still on
    low_battery_check();
    sleep_after_timeout();
#endif /* WITH_TARGET */

    // Reconfigure Felica module if communication timed out,
    // e.g. due to temporary disconnect.
    if (protocol_errno == TIMEOUT) {
      initiator_set_defaults();
      eeprom_increment_usart_fail();
    }
    protocol_errno = SUCCESS;
  }

  /* NOT REACHABLE */
  return 0;
}
