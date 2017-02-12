/**
 ******************************************************************************
 * @Filename:	ppm_driver.cpp
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

#include "ppm_in_driver.h"
#include "utils.h"
#define INVALID 255
#define PPM_TIMEOUT 1000
PPMDriver::PPMDriver(uint8_t pin, uint8_t maxPpmChannels):PinChangeInterrupt(pin, PinChangeInterrupt::ON_RISING_MODE) {
  ppmAge = 0;
  ppmCounter = INVALID;
  ppmChannels = maxPpmChannels;
  ppmValues = new uint16_t[maxPpmChannels];
  previousPpmValues = new uint16_t[maxPpmChannels];
  newPPM = false;
}

void PPMDriver::init() {
  TCCR1A = 0;//normal mode
  TCCR1B = _BV(CS11);// clk I/O /8 (From prescaler)
  TIMSK1 = 0;//no interrupts
  TCNT1 = 0;
  begin();
  enable();
}
void PPMDriver::readPPM(uint16_t *buffer) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    newPPM = false;
    memcpy(buffer, ppmValues, ppmChannels);
    memcpy(previousPpmValues, ppmValues, ppmChannels);
  }
}

PPMDriver::status PPMDriver::getStatus() {
  status s;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    s.newPPM = newPPM;
    if(Utils::getmsSince(ppmAge) > PPM_TIMEOUT) {
      s.timeout = true;
    }
    else
      s.timeout = false;
  }
  return s;
}

void PPMDriver::on_interrupt(uint16_t arg) {
  uint16_t pulseWidth;
  pulseWidth = TCNT1; // read the timer1 value
  TCNT1 = 0; // reset the timer1 value for next
  processPulse(pulseWidth);
}

void PPMDriver::processPulse(uint16_t pulse)
{
  pulse >>= 1; // clock runs at 2MHz = 0.5uS per tick, this makes it 1uS per tick
  if (pulse > 2500) {      // Verify if this is the sync pulse (2.5ms)
    if (ppmCounter != INVALID) {
      ppmAge = millis();// brand new PPM data received
      for(int x = 0; x < ppmChannels; ++ x) {
        if(ppmValues[x] != previousPpmValues[x]) {
          newPPM = true;
        }
      }
    }
    ppmCounter = 0;
  } else if ((pulse > 700) && (ppmCounter < ppmChannels)) {
    ppmValues[ppmCounter++] = servoUs2Bits(pulse);   // Store measured pulse length (converted)
  } else {
    ppmCounter = INVALID;
  }
}
  // conversion between microseconds 800-2200 and value 0-1023
  // 808-1000 == 0 - 11     (16us per step)
  // 1000-1999 == 12 - 1011 ( 1us per step)
  // 2000-2192 == 1012-1023 (16us per step)

  uint16_t PPMDriver::servoUs2Bits(uint16_t x)
  {
    uint16_t ret;
    if (x < 800) {
      ret = 0;
    } else if (x < 1000) {
      ret = (x - 799) / 16;
    } else if (x < 2000) {
      ret = (x - 988);
    } else if (x < 2200) {
      ret = (x - 1992) / 16 + 1011;
    } else {
      ret = 1023;
    }
    return ret;
  }

uint8_t PPMDriver::packChannels(volatile uint8_t *p)
{
  uint8_t i;
  uint8_t t = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    newPPM = false;
    volatile uint16_t *PPM = ppmValues;
    for (i = 0; i < ppmChannels; i++) { // 4ch packed in 5 bytes
      if((i & 0x04) == 0x04) {
        ++t;
      }
      p[5 * t] |= ((PPM[i] >> 8) & 3);
      p[5 * t] = p[5 * t] << 2;
      p[i + t + 1] = (PPM[i] & 0xff);
    }
  }
  return i + t + 1;
}
