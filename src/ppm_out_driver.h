/**
 ******************************************************************************
 * @Filename:	ppm_out_driver.h
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

#ifndef PPM_OutDriver_h
#define PP_OutDriver_h

#include "timersdriver.h"

class PPM_OutDriver:public TimerDriver {
public:
  PPM_OutDriver(uint8_t ppm_pin, uint8_t ppmChannelsToUse, uint8_t *pwm_pins = NULL);
  virtual void on_overflow_interrupt();
  virtual void on_compareAinterrupt();
  virtual void on_compareBinterrupt();
  virtual void on_input_capture_interrupt() {};
  void setPPMValues(uint16_t *values);
  uint16_t *COMP;
  uint16_t *ICR;
private:
  struct pin_control {
    volatile uint8_t *port;
    uint8_t mask;
  };
  pin_control *pwmPins;
  uint8_t ppmChannels;
  uint8_t ppmCounter;
  volatile uint16_t nextICR;
  volatile uint8_t *pintToPort (uint8_t pin);
  volatile uint8_t *ppm_port;
  volatile uint8_t ppm_pin;
  inline uint16_t servoBits2Us(uint16_t x);
  uint16_t *PPM;
  uint16_t ppmSync;
  bool disablePPM;
  bool disablePWM;
  bool useCompB;
  bool useOCPin;
  void allPWMChannelsDown();
  void PWMChannelsUp(uint8_t channel);
};
#endif
