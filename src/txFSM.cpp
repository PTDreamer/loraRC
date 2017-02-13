/**
 ******************************************************************************
 * @Filename:	txFSM.cpp
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

#include "txFSM.h"
#include "ppm_in_driver.h"
#include "RH_RF22JB.h"

#define MAX_FSM_SEND_TIME 0 // TODO

txFSM::txFSM(PPMDriver *ppm, Fifo *fifo): m_ppm(ppm), serialFifo(fifo) {
	printf("FSM CONSTRUCTOR\n");
	context.fsm_timer_remaining_us = 0;
	context.fsm_timer_enabled = false;
	fsm_setup();
}

void txFSM::setRadio(RH_RF22JB *radio) {
	m_radio = radio;
}

void txFSM::fsm_setup() {
	printf("FSM SETUP\n");
	fsm_setup_entry			(STATE_FSM_FAULT, &txFSM::go_fsm_fault);
	fsm_setup_next_state(STATE_FSM_FAULT, EVENT_TIMER_EXPIRY, STATE_INIT);

	fsm_setup_entry			(STATE_INIT, NULL);
	fsm_setup_next_state(STATE_INIT, EVENT_AUTO, STATE_SENDING_PACKET);

	fsm_setup_entry			(STATE_SENDING_PACKET, &txFSM::go_fsm_transmit);
	fsm_setup_next_state(STATE_SENDING_PACKET, EVENT_TIMER_EXPIRY, STATE_RESET);
	fsm_setup_next_state(STATE_SENDING_PACKET, EVENT_PACKET_SENT, STATE_RECEIVING_PACKET);

	fsm_setup_entry			(STATE_RESET, &txFSM::go_fsm_reset);
	fsm_setup_next_state(STATE_RESET, EVENT_AUTO, STATE_SENDING_PACKET);

	fsm_setup_entry			(STATE_RECEIVING_PACKET, &txFSM::go_fsm_receive);
	fsm_setup_next_state(STATE_RECEIVING_PACKET, EVENT_TIMER_EXPIRY, STATE_HOP);
	fsm_setup_next_state(STATE_RECEIVING_PACKET, EVENT_PACKET_RECEIVED, STATE_PARSE_RECEIVE);

	fsm_setup_entry			(STATE_PARSE_RECEIVE, &txFSM::go_fsm_parse_receive);
	fsm_setup_next_state(STATE_PARSE_RECEIVE, EVENT_AUTO, STATE_HOP);

	fsm_setup_entry			(STATE_HOP, &txFSM::go_fsm_hop);
	fsm_setup_next_state(STATE_HOP, EVENT_AUTO, STATE_SENDING_PACKET);
}

float txFSM::getChannelRSSI(uint8_t channel) {
	if(context.stats[channel].sentNOK == 0 || context.stats[channel].receivedNOK == 0)
		return 0xFFFF;
	float rssi_tx = context.stats[channel].sentOK / context.stats[channel].sentNOK;
	float rssi_rx = context.stats[channel].receivedOK / context.stats[channel].receivedNOK;
	return (rssi_tx + rssi_rx) / 2;
}
void txFSM::go_fsm_hop() {
		if(!context.lastPacketAcked) {
		if(getChannelRSSI(context.nextHOPChannel) > getChannelRSSI(context.currentHOPChannel)) {
			//hop to nextHOPChannel
			float better = 0;
			uint8_t bestChannel = 0;
			for(uint8_t i = 0; i < NUMBER_OF_HOP_CHANNELS; ++ i) {
				if(getChannelRSSI(i) > better) {
					bestChannel = i;
					better = getChannelRSSI(i);
				}
			}
			context.nextHOPChannelUnAcked = bestChannel;
		}
	}
}

void txFSM::go_fsm_parse_receive() {
	uint8_t len = sizeof(radio_packet);
	m_radio->recv((uint8_t*)&radio_packet, &len);

	if(radio_packet.type != TLM_ONLY) {
		//ALARM
	}
	if(radio_packet.rxSeq == ~context.lastReceivedSeq) {
		++context.stats[context.currentHOPChannel].receivedOK;
	//	Serial.write(radio_packet.dataBuffer, len - SIZE_OF_METADATA);
	}
	else {
		++context.stats[context.currentHOPChannel].receivedNOK;
	}
	if(radio_packet.txSeq == ~context.lastSentSeq) {
		context.lastPacketAcked = true;
		context.nextHOPChannel = radio_packet.nextHOPChannel;
		serialFifo->confirmPending();
		++context.stats[context.currentHOPChannel].sentOK;
	}
	else {
		++context.stats[context.currentHOPChannel].sentNOK;
	}
	context.lastReceivedSeq = radio_packet.rxSeq;
}

void txFSM::go_fsm_receive() {
	fsm_timer_start(MAX_FSM_SEND_TIME);
}

void txFSM::go_fsm_reset() {
		m_radio->reset();
		//add configuration
}

void txFSM::go_fsm_transmit() {
	printf("transmiting\n");
	return;
	PPMDriver::status s = m_ppm->getStatus();
	uint8_t usedBytes = 0;
	uint8_t serialBytes = 0;
	if(s.timeout) {
		radio_packet.type = FAILSAFE;
		usedBytes += serialFifo->pendingPop(radio_packet.dataBuffer, TRANSMIT_BUFFER_DATA_SIZE - usedBytes);
	}
	else if(s.newPPM || ((context.lastPacketType == PPM_TLM) && !context.lastPacketAcked)) {
		usedBytes += m_ppm->packChannels(radio_packet.dataBuffer);
		serialBytes = serialFifo->pendingPop(radio_packet.dataBuffer, TRANSMIT_BUFFER_DATA_SIZE - usedBytes);
		radio_packet.type = PPM_TLM;
		usedBytes += serialBytes;
	}
	else {
		serialBytes = serialFifo->pendingPop(radio_packet.dataBuffer, TRANSMIT_BUFFER_DATA_SIZE - usedBytes);
		radio_packet.type = TLM_ONLY;
		usedBytes += serialBytes;
	}
	context.usedBytes = usedBytes;
	if(context.lastPacketAcked) {
		radio_packet.txSeq = ~context.lastSentSeq;
		context.lastSentSeq = radio_packet.txSeq;
	}
	else {
		radio_packet.txSeq = context.lastSentSeq;
	}
	context.lastPacketAcked = false;
	context.lastPacketType = (packet_type)radio_packet.type;
	radio_packet.nextHOPChannel = context.nextHOPChannelUnAcked;
	radio_packet.rxSeq = ~context.lastReceivedSeq;
	m_radio->send((uint8_t*)&radio_packet, usedBytes);
	//fsm_timer_start(sendTimeout(usedBytes));
}

void txFSM::fsm_setup_entry(fsm_states state, void (txFSM::*fn)()) {
	fsm_transitions[state].entry_fn = fn;
//	memset(fsm_transitions[state].next_state, STATE_FSM_FAULT, EVENT_NUM_EVENTS + 1);  TODO FIND why memset not working
	for(int x = 0; x < EVENT_NUM_EVENTS; ++x) {
		fsm_transitions[state].next_state[x] = STATE_FSM_FAULT;
		printf("fsm_transitions for state %d %d=%d\n", state, x, fsm_transitions[state].next_state[x]);
	}
}

void txFSM::fsm_setup_next_state(fsm_states state, fsm_events event, fsm_states nextState) {
	fsm_transitions[state].next_state[event] = nextState;
}
enum txFSM::fsm_states txFSM::fsm_get_state()
{
	return context.curr_state;
}

void txFSM::fsm_timer_start(unsigned long timer_duration_us)
{
	context.fsm_timer_remaining_us = timer_duration_us;
	context.fsm_timer_enabled = true;
}

void txFSM::fsm_timer_cancel()
{
	context.fsm_timer_enabled = false;
}

void txFSM::fsm_timer_add_ticks(unsigned long elapsed_us)
{
	if (context.fsm_timer_enabled) {
		if (elapsed_us >= context.fsm_timer_remaining_us) {
			/* Timer has expired */
			context.fsm_timer_remaining_us = 0;
		} else {
			/* Timer is still running, account for the elapsed time */
			context.fsm_timer_remaining_us -= elapsed_us;
		}
	}
}

bool txFSM::fsm_timer_expired_p()
{
	if ((context.fsm_timer_enabled) && (context.fsm_timer_remaining_us == 0))
		return true;

	return false;
}

void txFSM::go_fsm_fault()
{
	fsm_timer_start(10);
//	led_pwm_config(&context->leds, 2500, 100, 2500, 100);
}

void txFSM::fsm_process_auto()
{
	while (fsm_transitions[context.curr_state].next_state[EVENT_AUTO]) {
		context.curr_state = fsm_transitions[context.curr_state].next_state[EVENT_AUTO];

		/* Call the entry function (if any) for the next state. */
		if (fsm_transitions[context.curr_state].entry_fn) {
			(this->*fsm_transitions[context.curr_state].entry_fn)();
		}
	}
}

void txFSM::validPreambleReceived() {
	unsigned long m = micros();
	PORTD |= digitalPinToBitMask(3);
	while (micros() - m < 1000) {
		/* code */
	}
	PORTD &= ~digitalPinToBitMask(3);
	Serial.println("PREAMBLE");
	Serial.println(m);
	hasSent = true;
}

void txFSM::fsm_inject_event(enum fsm_events event)
{
	/*
	 * Move to the next state
	 *
	 * This is done prior to calling the new state's entry function to
	 * guarantee that the entry function never depends on the previous
	 * state.  This way, it cannot ever know what the previous state was.
	 */
	context.curr_state = fsm_transitions[context.curr_state].next_state[event];

	/* Call the entry function (if any) for the next state. */
	if (fsm_transitions[context.curr_state].entry_fn) {
		(this->*fsm_transitions[context.curr_state].entry_fn)();
	}

	/* Process any AUTO transitions in the FSM */
	fsm_process_auto();
}

void txFSM::fsm_init()
{
	context.curr_state = STATE_INIT;
	fsm_process_auto();
}

void txFSM::handle() {
static unsigned long prev_ticks = micros();
unsigned long elapsed_ticks = micros() - prev_ticks;
	/* Run the fsm timer */
	if (elapsed_ticks) {
		fsm_timer_add_ticks(elapsed_ticks);
			if (fsm_timer_expired_p() == true) {
				/* Timer has expired, inject an expiry event */
			fsm_inject_event(EVENT_TIMER_EXPIRY);
		}
			/* pulse the LEDs */
		//	led_pwm_add_ticks(&bl_fsm_context.leds, elapsed_ticks);
	//		led_pwm_update_leds(&bl_fsm_context.leds);
			prev_ticks += elapsed_ticks;
		}
	}

unsigned long txFSM::sendTimeout(uint8_t sentBytes) {
	return 0;
}
