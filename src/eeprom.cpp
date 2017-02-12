/**
 ******************************************************************************
 * @Filename:	eemprom.cpp
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

# include "eeprom.h"

Eeprom::Eeprom() {

}

settings Eeprom::getSettings() {
  settings set;
  set.radioChannels[0] = 0;
  set.radioChannels[1] = 1;
  set.radioChannels[2] = 2;
  set.radioChannels[3] = 3;
  set.power = 0x07;
  return set;
}
