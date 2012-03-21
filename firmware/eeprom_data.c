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
 * EEPROM management routine.  All the EEPROM access should be written here.
 * EEPROM image is now not only made by compiler but also made by generator
 * program.  Then, EEPROM's format should be predictable and it should be
 * maintained at a single point.
 */

#include <avr/eeprom.h>
#include <avr/io.h>

#include <stdint.h>
#include <string.h>

#include "peripheral/eeprom.h"

#include "eeprom_data.h"

// Use a bit combination that is unlikely to happen by accident
#define CONFIG_MARKER 0xa5

// Data structure stored in EEPROM
typedef struct {
  /* number of usart failures */
  uint32_t number_usart_fail;

  /* station counter (stores number of touches) */
  uint32_t counter;

  /* station is configured if value is CONFIG_MARKER */
  uint8_t has_station_info;

  /* Reset count and reasons */
  uint32_t number_porf;
  uint32_t number_extrf;
  uint32_t number_borf;
  uint32_t number_wdrf;

  /* Station ID & Key */
  uint8_t station_id[STATION_ID_BYTES];
  uint8_t station_key[STATION_KEY_BYTES];

  /* We point the EEAR to here to reduce risk of EEPROM corruption */
  uint8_t unused;

  uint8_t flags;

  /* Add new fields here */
} stats_t;

static stats_t __attribute__((section(".eeprom"))) stats;

/*
 * Increases counters based on reset status flags.
 */
void eeprom_count_mcusr(uint8_t mcusr)
{
  /*
   * MCUSR (MCU status register) shows the reason why reset is caused:
   * PORF  power-on reset flag.
   * EXTRF external reset flag.
   * BORF  brown-out reset flag.
   * WDRF  watch dog system reset flag.
   */
  if ((mcusr & _BV(PORF)) != 0)
    (void)increment_eeprom_uint32(&stats.number_porf);
  else if ((mcusr & _BV(BORF)) != 0)
    // BORF is also set as part of regular Power-on. Ignore that.
    (void)increment_eeprom_uint32(&stats.number_borf);

  if ((mcusr & _BV(EXTRF)) != 0)
    (void)increment_eeprom_uint32(&stats.number_extrf);

  if ((mcusr & _BV(WDRF)) != 0) {
    if (eeprom_is_flag_set(FLAG_FORCED_WDT)) {
      eeprom_clear_flag(FLAG_FORCED_WDT);
    } else {
      (void)increment_eeprom_uint32(&stats.number_wdrf);
    }
  }
}

/*
 * Check whether the EEPROM is configured by reading the config flag byte.
 * After reading, set the EEPROM address register (EEAR) to an unused byte.
 * If a random EEPROM write happens afterwards, it will not corrupt our data.
 */
bool eeprom_has_station_info(void)
{
  uint8_t has_station_info;

  has_station_info = eeprom_read_byte(&stats.has_station_info);
  EEAR = (uint16_t)(&stats.unused);
  return has_station_info == CONFIG_MARKER;
}

void eeprom_write_station_info(uint8_t station_id[STATION_ID_BYTES],
                               uint8_t station_key[STATION_KEY_BYTES])
{
  stats_t new_stats;
  memset(&new_stats, 0x00, sizeof(stats_t));

  new_stats.has_station_info = CONFIG_MARKER;
  memcpy(&new_stats.station_id, station_id, STATION_ID_BYTES);
  memcpy(&new_stats.station_key, station_key, STATION_KEY_BYTES);

  eeprom_write_block(&new_stats, &stats, sizeof(stats));
}

void eeprom_read_station_info(uint8_t station_id[STATION_ID_BYTES],
                              uint8_t station_key[STATION_KEY_BYTES])
{
  eeprom_read_station_id(station_id);
  eeprom_read_block(station_key, stats.station_key, sizeof(stats.station_key));
}

void eeprom_read_station_id(uint8_t station_id[STATION_ID_BYTES])
{
  eeprom_read_block(station_id, stats.station_id, sizeof(stats.station_id));
}

void eeprom_increment_counter(uint32_t *ctr)
{
  *ctr = increment_eeprom_uint32(&stats.counter);
}

void eeprom_increment_usart_fail(void)
{
  (void)increment_eeprom_uint32(&stats.number_usart_fail);
}

uint32_t eeprom_read_number_porf(void)
{
  return eeprom_read_dword(&stats.number_porf);
}

uint32_t eeprom_read_number_borf(void)
{
  return eeprom_read_dword(&stats.number_borf);
}

uint32_t eeprom_read_number_extrf(void)
{
  return eeprom_read_dword(&stats.number_extrf);
}

uint32_t eeprom_read_number_wdrf(void)
{
  return eeprom_read_dword(&stats.number_wdrf);
}

uint32_t eeprom_read_number_usart_fail(void)
{
  return eeprom_read_dword(&stats.number_usart_fail);
}

void eeprom_set_flag(uint8_t bit)
{
 uint8_t flags;
 flags = eeprom_read_byte(&stats.flags);
 flags |= 1 << bit;
 eeprom_write_byte(&stats.flags, flags);
}

void eeprom_clear_flag(uint8_t bit)
{
 uint8_t flags;
 flags = eeprom_read_byte(&stats.flags);
 flags &= ~(1 << bit);
 eeprom_write_byte(&stats.flags, flags);
}

bool eeprom_is_flag_set(uint8_t bit)
{
  return (eeprom_read_byte(&stats.flags) & (1 << bit)) != 0;
}
