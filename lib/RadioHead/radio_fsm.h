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

#include "../../src/fifo.h"
#define TRANSMIT_BUFFER_SIZE  64
#define SIZE_OF_METADATA      2
#define TRANSMIT_BUFFER_DATA_SIZE TRANSMIT_BUFFER_SIZE - SIZE_OF_METADATA
#define NUMBER_OF_HOP_CHANNELS  4
class RH_RF22JB;

class RadioFSM {
public:
  RadioFSM(Fifo *fifo);
  enum fsm_events {
   EVENT_PACKET_RECEIVED,
   EVENT_PACKET_SENT,
   EVENT_TIMER_EXPIRY,
   EVENT_AUTO,

   EVENT_NUM_EVENTS	/* Must be last */
  };
  enum packet_type {TLM_ONLY, PPM_TLM, FAILSAFE_SET, FAILSAFE};
struct radio_stats{
  uint32_t sentOK;
  uint32_t sentNOK;
  uint32_t receivedOK;
  uint32_t receivedNOK;
};
enum fsm_states {
	STATE_FSM_FAULT = 0,	/* Must be zero so undefined transitions land here */
  STATE_INIT,
  STATE_RECEIVING_PACKET,
  STATE_SENDING_PACKET,
  STATE_RESET,
  STATE_HOP,
  STATE_PARSE_RECEIVE,
	STATE_NUM_STATES	/* Must be last */
};

  struct fsm_context {
	enum fsm_states curr_state;

	/* FSM timer */
	bool fsm_timer_enabled;
	unsigned long fsm_timer_remaining_us;

  uint8_t usedBytes;
  bool lastReceivedSeq;
  bool lastSentSeq;
  bool lastPacketAcked;
  packet_type lastPacketType;
  uint8_t nextHOPChannel;
  uint8_t nextHOPChannelUnAcked;
  radio_stats stats[NUMBER_OF_HOP_CHANNELS];
  uint8_t currentHOPChannel;
};
  virtual void sent();
  virtual void received();
  virtual void validPreambleReceived() {};
  virtual void handle();
  virtual void fsm_init();
  void setRadio(RH_RF22JB *radio);

  virtual void go_fsm_transmit();
  virtual void go_fsm_reset();
  virtual void go_fsm_receive();
  virtual void go_fsm_hop();
  virtual void go_fsm_parse_receive() {printf("parent SHIT\n");};
  virtual void go_fsm_fault();
protected:
  RH_RF22JB *m_radio;
  volatile bool hasReceived;
  volatile bool hasSent;
  Fifo *serialFifo;
  struct {
  uint8_t type : 3;
  uint8_t txSeq : 1;
  uint8_t rxSeq : 1;
  uint8_t : 0;
  uint8_t nextHOPChannel : 8;
  uint8_t dataBuffer[TRANSMIT_BUFFER_DATA_SIZE];
} radio_packet;
  float getChannelRSSI(uint8_t channel);
  volatile fsm_context context;
  bool fsm_timer_expired_p();
  void fsm_timer_add_ticks(unsigned long elapsed_us);
  void fsm_timer_cancel();
  void fsm_timer_start(unsigned long timer_duration_us);
  void fsm_process_auto();
  void fsm_inject_event(enum fsm_events event);
  enum RadioFSM::fsm_states fsm_get_state();
  void fsm_setup_entry(fsm_states state, void (RadioFSM::*fn)());
  void fsm_setup_next_state(fsm_states state, fsm_events event, fsm_states nextState);
  struct fsm_transition {
    void (RadioFSM::*entry_fn) ();
    volatile enum fsm_states next_state[EVENT_NUM_EVENTS];
  };
  volatile struct fsm_transition fsm_transitions[STATE_NUM_STATES];
};
#endif
