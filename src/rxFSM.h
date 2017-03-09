/**
 ******************************************************************************
 * @Filename:	rxFSM.h
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

#ifndef rxFSM_h
#define  rxFSM_h
#include "Arduino.h"
#include "radio_fsm.h"

class PPM_OutDriver;
class RH_RF22JB;

class rxFSM : public RadioFSM {
public:
  rxFSM(PPM_OutDriver *ppm, Fifo *fifo);
private:
  void go_fsm_transmit();
  void go_fsm_reset();
  void go_fsm_receive();
  void go_fsm_hop();
  void go_fsm_parse_receive();
  void go_fsm_receive_timeout();
  void go_fsm_preamble_timeout();
  void fsm_setup();
  void go_fsm_preamble();
  PPM_OutDriver *m_ppm;
  unsigned long sendTimeout(uint8_t);
  unsigned long temp;
  unsigned long temp2;
};
#endif
