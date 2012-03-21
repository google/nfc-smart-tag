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

#ifndef __EEPROM_DATA_H__
#define __EEPROM_DATA_H__

#include <stdbool.h>
#include <stdint.h>

#define STATION_ID_BYTES 8
#define STATION_KEY_BYTES 16

#define FLAG_FORCED_WDT 0

// Increases counters based on reset status flags.
void eeprom_count_mcusr(uint8_t mcusr);

// Check whether the EEPROM is configured.
bool eeprom_has_station_info(void);


void eeprom_write_station_info(uint8_t station_id[STATION_ID_BYTES],
                               uint8_t station_key[STATION_KEY_BYTES]);
void eeprom_read_station_info(uint8_t station_id[STATION_ID_BYTES],
                              uint8_t station_key[STATION_KEY_BYTES]);
void eeprom_read_station_id(uint8_t station_id[STATION_ID_BYTES]);

// Increment a 32bit counter stored in EEPROM.
void eeprom_increment_counter(uint32_t *ctr);

// Increment counter for serial (USART) failures
void eeprom_increment_usart_fail(void);

// Return number of Power on Resets
uint32_t eeprom_read_number_porf(void);

// return number of Brown-out Resets
uint32_t eeprom_read_number_borf(void);

// Return number of External Resets
uint32_t eeprom_read_number_extrf(void);

// Return number of Watchdog Resets
uint32_t eeprom_read_number_wdrf(void);

// Return number of serial (USART) failures / timeout
uint32_t eeprom_read_number_usart_fail(void);

void eeprom_set_flag(uint8_t bit);
void eeprom_clear_flag(uint8_t bit);
bool eeprom_is_flag_set(uint8_t bit);

#endif /* !__EEPROM_DATA_H__ */
