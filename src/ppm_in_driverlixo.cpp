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

#include "ppm_in_driverlixo.h"
#define INVALID 255
lixo::lixo(uint8_t pin):PinChangeInterrupt(pin, PinChangeInterrupt::ON_RISING_MODE) {

}

void lixo::init() {
  begin();
  enable();
}
void lixo::on_interrupt(uint16_t arg) {
  unsigned long m = micros();
  Serial.println("RECEIVED");
  Serial.println(m);
}
