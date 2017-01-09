/**
 ******************************************************************************
 * @Filename:	types.h
 * @Project: 	loraRC
 * @Author: 	Jose Barros
 * @Copyright (C) 2017 Jose Barros
 * @Email:  	josemanuelbarros@gmail.com
 *****************************************************************************/
/*
 * @License:
 * This file is part of loraRC.
 * loraRC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * loraRC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with loraRC.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Arduino.h"
//#include <avr/io.h>
//#include <avr/sleep.h>
//#include <avr/eeprom.h>
//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
//#include <avr/sfr_defs.h>

/**
 * Restore processor flags and possible enable of interrupts.
 * Internal clean up function for synchronized block.
 * @param[in] key processor flags.
 */
inline void __unlock(uint8_t* key) __attribute__((always_inline));
inline void __unlock(uint8_t* key)
{
  SREG = *key;
  __asm__ __volatile__("" ::: "memory");
}
inline uint8_t lock() __attribute__((always_inline));
inline uint8_t lock()
{
  uint8_t key = SREG;
  __asm__ __volatile__("cli" ::: "memory");
  return (key);
}
 #define synchronized	for (uint8_t __key __attribute__((__cleanup__(__unlock))) = lock(),	i = 1; i != 0; i--)
