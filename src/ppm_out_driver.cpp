/**
 ******************************************************************************
 * @Filename:	ppm_out_driver.cpp
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

#include "ppm_out_driver.h"

#if (F_CPU == 16000000)
#define PWM_MULTIPLIER 2
#define PPM_PULSELEN   600
#define PWM_DEJITTER   32
#define PPM_FRAMELEN   40000
#define MIN_SYNC 3000
#elif (F_CPU == 8000000)
#define PWM_MULTIPLIER 1
#define PPM_PULSELEN   300
#define PWM_DEJITTER   16
#define PPM_FRAMELEN   20000
#define MIN_SYNC 3000
#else
#error F_CPU not supported
#endif

PPM_OutDriver::PPM_OutDriver(uint8_t pin, uint8_t ppmChannelsToUse, uint8_t *pwm_pins):TimerDriver(),
 ppmChannels(ppmChannelsToUse), ppmCounter(0), nextICR(PPM_FRAMELEN),  ppmSync(PPM_FRAMELEN),
  disablePPM(false), disablePWM(true) {
  if(pwm_pins) {
    disablePWM = false;
    pwmPins = new pin_control[ppmChannelsToUse];
    for(int x = 0; x < ppmChannelsToUse; ++x) {
      if(pwm_pins[x] != 255) {
        pwmPins[x].port = pintToPort(pwm_pins[x]);
        pwmPins[x].mask = _BV(pwm_pins[x] < 8  ? pwm_pins[x] : pwm_pins[x] < 14 ? pwm_pins[x] - 8 : pwm_pins[x] - 14);
      }
      else {
        pwmPins[x].port = NULL;
        pwmPins[x].mask = 0;
      }
    }
  }
  ppm_port = pintToPort(ppm_pin);
  ppm_pin =  _BV(pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
  pinMode(pin, OUTPUT);
  PPM = new uint16_t[ppmChannelsToUse];
  timerConfig cfg;
  useCompB = true;
  useOCPin = true;
  cfg.timerID = TimerDriver::TIMER_1;
  cfg.waveForm = TimerDriver::WAVE_FAST_PWM_ICR;
  cfg.compareA = TimerDriver::COMPARE_NORMAL;
  cfg.compareB = TimerDriver::COMPARE_CLEAR;
  cfg.tclock = TimerDriver::CLOCK_1_8;
  cfg.inputCaptureEdgeSelect = LOW;
  cfg.useInputCaptureNoiseCanceler = false;
  setOutputCompareValueB(65535);
  setOutputCompareValueA(65535);
  if(pin == 9) {//OC1A
    cfg.compareB = TimerDriver::COMPARE_NORMAL;
    cfg.compareA = TimerDriver::COMPARE_CLEAR;
    useCompB = false;
    init(cfg);
    enableInterrupts(TimerDriver::INT_ON_OVERFLOW);
  }
  else if(pin == 10) {//OC1B
    cfg.compareA = TimerDriver::COMPARE_NORMAL;
    cfg.compareB = TimerDriver::COMPARE_CLEAR;
    init(cfg);
    enableInterrupts(TimerDriver::INT_ON_OVERFLOW);
  }
  else {//OC1B
    cfg.compareB = TimerDriver::COMPARE_NORMAL;
    cfg.compareA = TimerDriver::COMPARE_NORMAL;
    useOCPin = false;
    init(cfg);
    enableInterrupts(TimerDriver::INT_ON_OVERFLOW | TimerDriver::INT_ON_COMPARE_B);
  }
}

void PPM_OutDriver::on_compareAinterrupt() {
  *ppm_port &= ~ppm_pin;
}

void PPM_OutDriver::on_compareBinterrupt() {
  *ppm_port &= ~ppm_pin;
}

void PPM_OutDriver::on_overflow_interrupt() {
  if(!useOCPin) {
    *ppm_port |= ppm_pin;
  }
  if (ppmCounter < ppmChannels) {
    setInputCapture(nextICR);
    nextICR = servoBits2Us(PPM[ppmCounter]) * PWM_MULTIPLIER;
    ICR[ppmCounter] = nextICR;
    ppmSync -= nextICR;
    if (ppmSync < (MIN_SYNC * PWM_MULTIPLIER)) {
      ppmSync = MIN_SYNC * PWM_MULTIPLIER;
    }
    if ((disablePPM) || (ppmCounter >= 8)) {
      if(useCompB)
        setOutputCompareValueB(65535); //do not generate a pulse
      else
        setOutputCompareValueA(65535);
      COMP[ppmCounter] = 65535;
    } else {
      if(useCompB)
        setOutputCompareValueB(nextICR - PPM_PULSELEN);//clear
      else
        setOutputCompareValueA(nextICR - PPM_PULSELEN);
      COMP[ppmCounter] = nextICR - PPM_PULSELEN;
    }
    //Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at BOTTOM (non-inverting mode)
    while (getTimerValue() < PWM_DEJITTER);
    if ((!disablePWM) && (ppmCounter > 0)) {
      allPWMChannelsDown();
      PWMChannelsUp(ppmCounter - 1);
    }
    ppmCounter++;
  } else {
    setInputCapture(nextICR);
    nextICR = ppmSync;
    ICR[ppmCounter] = nextICR;
    if (disablePPM) {
      if(useCompB)
        setOutputCompareValueB(65535); //do not generate a pulse
      else
        setOutputCompareValueA(65535);
        COMP[ppmCounter] = 65535;
    }
    else {
      if(useCompB)
        setOutputCompareValueB(nextICR - PPM_PULSELEN);//clear
      else
        setOutputCompareValueA(nextICR - PPM_PULSELEN);
      COMP[ppmCounter] = nextICR - PPM_PULSELEN;

    }
    ppmSync = PPM_FRAMELEN;
    while (getTimerValue() < PWM_DEJITTER);
    if ((!disablePWM) && (ppmCounter > 0)) {
      allPWMChannelsDown();
      PWMChannelsUp(ppmCounter - 1);
    }
    ppmCounter = 0 ;
  }
}

uint16_t PPM_OutDriver::servoBits2Us(uint16_t x)
{
  uint16_t ret;

  if (x < 12) {
    ret = 808 + x * 16;
  } else if (x < 1012) {
    ret = x + 988;
  } else if (x < 1024) {
    ret = 2000 + (x - 1011) * 16;
  } else {
    ret = 2192;
  }

  return ret;
}

volatile uint8_t *PPM_OutDriver::pintToPort (uint8_t pin) {
    return (pin < 8  ? &PORTD : pin < 14 ? &PORTB : &PORTC);
}

void PPM_OutDriver::setPPMValues(uint16_t *value) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    memcpy(PPM, value, sizeof(uint16_t) * ppmChannels);
  }
}

void PPM_OutDriver::allPWMChannelsDown() {
  for(uint8_t x = 0; x < ppmChannels; ++ x) {
    if(pwmPins[x].port) {
      *pwmPins[x].port &= ~pwmPins[x].mask;
    }
  }
}

void PPM_OutDriver::PWMChannelsUp(uint8_t channel) {
  if(pwmPins[channel].port) {
    *pwmPins[channel].port |= pwmPins[channel].mask;
  }
}
