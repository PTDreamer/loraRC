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
#include "fifo.h"
#define TRANSMIT_BUFFER_SIZE  64
#define SIZE_OF_METADATA      2
#define TRANSMIT_BUFFER_DATA_SIZE TRANSMIT_BUFFER_SIZE - SIZE_OF_METADATA
#define NUMBER_OF_HOP_CHANNELS  4
class PPM_OutDriver;
class RH_RF22JB;

class rxFSM : public RadioFSM {
public:
  rxFSM(PPM_OutDriver *ppm, Fifo *fifo);
  void handle();
  void received();
  void sent();
  void validPreambleReceived();
  void setRadio(RH_RF22JB *radio);
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
enum fsm_events {
 EVENT_PACKET_RECEIVED,
 EVENT_PACKET_SENT,
 EVENT_TIMER_EXPIRY,
 EVENT_AUTO,

 BL_EVENT_NUM_EVENTS	/* Must be last */
};
struct fsm_transition {

  void (rxFSM::*entry_fn) ();
	enum fsm_states next_state[BL_EVENT_NUM_EVENTS];
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
	/* LED state */
	//struct led_pwm_state leds;
};
private:
  void fsm_process_auto();
  void fsm_init();
  void fsm_inject_event(enum fsm_events event);
  void go_fsm_fault();
  void go_fsm_transmit();
  void go_fsm_reset();
  void go_fsm_receive();
  void go_fsm_hop();
  void go_fsm_parse_receive();
  bool fsm_timer_expired_p();
  void fsm_timer_add_ticks(unsigned long elapsed_us);
  void fsm_timer_cancel();
  void fsm_timer_start(unsigned long timer_duration_us);
  enum rxFSM::fsm_states fsm_get_state();
  struct fsm_transition fsm_transitions[STATE_NUM_STATES];
  fsm_context context;
  void fsm_setup_entry(fsm_states state, void (rxFSM::*fn)());
  void fsm_setup_next_state(fsm_states state, fsm_events event, fsm_states nextState);
  void fsm_setup();
  volatile bool hasReceived;
  volatile bool hasSent;
  PPM_OutDriver *m_ppm;
  RH_RF22JB *m_radio;
  Fifo *serialFifo;
  unsigned long sendTimeout(uint8_t);
  struct {
    uint8_t type : 3;
    uint8_t txSeq : 1;
    uint8_t rxSeq : 1;
    uint8_t : 0;
    uint8_t nextHOPChannel : 8;
    uint8_t dataBuffer[TRANSMIT_BUFFER_DATA_SIZE];
  } radio_packet;
  float getChannelRSSI(uint8_t channel);

};
#endif
