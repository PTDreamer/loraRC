/**
 ******************************************************************************
 * @Filename:	ppm_driver.h
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

#include "pinchangeinterrupt.h"
class PPMDriver:private PinChangeInterrupt {
public:
  PPMDriver(uint8_t pin, uint8_t ppmChannels);
  void readPPM(uint16_t *buffer);
  static inline uint16_t servoUs2Bits(uint16_t x);
  void init();
private:
  virtual void on_interrupt(uint16_t arg = 0);
  volatile uint8_t ppmCounter;
  volatile uint8_t ppmAge;
  volatile uint8_t ppmChannels;
  uint16_t *ppmValues;
  inline void processPulse(uint16_t pulse);
};
