/**
 ******************************************************************************
 * @Filename:	utils.cpp
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


#include "utils.h"

Utils *Utils::mutils = NULL;

Utils::Utils() {
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;
}

void Utils::printfInit() {
  if(mutils == NULL) {
    mutils = new Utils();
  }
}

int Utils::serial_putchar(char c, FILE* f) {
  if (c == '\n') serial_putchar('\r', f);
  return Serial.write(c) == 1 ? 0 : 1;
}

unsigned long Utils::getUsSince(unsigned long value) {
  unsigned long timenow = micros();
  if(timenow < value) {
    return 0xFFFF - value + timenow;
  }
  else return value - timenow;
}

unsigned long Utils::getmsSince(unsigned long value) {
  unsigned long timenow = millis();
  if(timenow < value) {
    return 0xFFFF - value + timenow;
  }
  else return value - timenow;
}
