/**
 ******************************************************************************
 * @Filename:	fifo.h
 * @Project: 	loraRC
 * @Author: 	Jose Barros <jose>
 * @Copyright (C) 2017 Jose Barros <jose>
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

class Fifo {
  Fifo(uint8_t size);
public:
  bool push(uint8_t value);
  uint8_t push(volatile uint8_t *values, uint8_t size);
  uint8_t pop(bool &ok, bool pending = false);
  uint8_t pop(volatile uint8_t *values, uint8_t size);
  uint8_t pendingPop(volatile uint8_t *values, uint8_t size);
  void confirmPending();
private:
  volatile uint8_t m_size;
  volatile uint8_t *buffer;
  volatile uint8_t buffer_head;
  volatile uint8_t buffer_tail;
  volatile uint8_t pending_head;
};
