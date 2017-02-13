/**
 ******************************************************************************
 * @Filename:	radio_fsm.h
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
#ifndef RADIO_FSM_H
#define RADIO_FSM_H
class RadioFSM {
public:
  enum fsm_events {
   EVENT_PACKET_RECEIVED,
   EVENT_PACKET_SENT,
   EVENT_TIMER_EXPIRY,
   EVENT_AUTO,

   EVENT_NUM_EVENTS	/* Must be last */
  };
  virtual void sent();
  virtual void received();
  virtual void validPreambleReceived() = 0;
  virtual void handle() = 0;
  virtual void fsm_init() = 0;
  virtual void fsm_inject_event(enum fsm_events event);
};
#endif
