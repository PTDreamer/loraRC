/**
 ******************************************************************************
 * @Filename:	txFSM.h
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

#ifndef txFSM_h
#define  txFSM_h
#include "Arduino.h"
#include "radio_fsm.h"

class PPMDriver;

class txFSM : public RadioFSM {
public:
  txFSM(PPMDriver *ppm, Fifo *fifo);
private:
  void go_fsm_transmit();
  void go_fsm_reset();
  void go_fsm_receive();
  void go_fsm_hop();
  void go_fsm_parse_receive();
  void txFSM::go_fsm_preamble();
  void fsm_setup();

  PPMDriver *m_ppm;
  unsigned long sendTimeout(uint8_t);
};
#endif
