/**
 ******************************************************************************
 * @Filename:	fifo.cpp
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

#include "fifo.h"

Fifo::Fifo(uint8_t size) {
  buffer = new volatile uint8_t[size];
  m_size = size;
  buffer_head = 0;
  buffer_tail = 0;
  pending_head = 0;
}

bool Fifo::push(uint8_t value) {
  if((buffer_tail + 1) % m_size != buffer_head) {
    buffer[buffer_tail] = value;
    buffer_tail = (buffer_tail + 1) % m_size;
    return true;
  }
  else
    return false;
}

uint8_t Fifo::pop(bool &ok, bool pending) {
  uint8_t ret = 0;
  if(pending) {
    if(buffer_tail != pending_head) {
      ret = buffer[pending_head];
      pending_head = (pending_head +1) % m_size;
      ok = true;
      return ret;
    }
    else {
      ok = false;
      return 0;
    }
  }
  else {
    if(buffer_tail != buffer_head) {
      ret = buffer[buffer_head];
      buffer_head = (buffer_head +1) % m_size;
      ok = true;
      return ret;
    }
    else {
      ok = false;
      return 0;
    }
  }
}

uint8_t Fifo::pop(volatile uint8_t *values, uint8_t size) {
  bool ok = true;
  uint8_t index = 0;
  while(ok && (index < size)) {
    values[index] = pop(ok);
    ++index;
  }
  return index - 1;
}

uint8_t Fifo::pendingPop(volatile uint8_t *values, uint8_t size) {
  bool ok = true;
  uint8_t index = 0;
  pending_head = buffer_head;
  while(ok && (index < size)) {
    values[index] = pop(ok, true);
    ++index;
  }
  return index - 1;
}

void Fifo::confirmPending() {
  buffer_head = pending_head;
}

uint8_t Fifo::push(volatile uint8_t *values, uint8_t size) {
  bool ok = true;
  uint8_t index = 0;
  while(ok && (index < size)) {
    ok = push(values[index]);
    ++index;
  }
  return index - 1;
}
