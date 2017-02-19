/**
 ******************************************************************************
 * @Filename:	rxFSM.cpp
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

#include "rxFSM.h"
#include "ppm_out_driver.h"
#include "RH_RF22JB.h"

#define MAX_FSM_SEND_TIME 10000000 // TODO

rxFSM::rxFSM(PPM_OutDriver *ppm, Fifo *fifo) : RadioFSM(fifo), m_ppm(ppm) {

	fsm_setup();
	temp = 0;
	temp2 = 0;
}

void rxFSM::fsm_setup() {
	fsm_setup_entry			(STATE_FSM_FAULT, &RadioFSM::go_fsm_fault);
	fsm_setup_next_state(STATE_FSM_FAULT, EVENT_TIMER_EXPIRY, STATE_INIT);

	fsm_setup_entry			(STATE_INIT, NULL);
	fsm_setup_next_state(STATE_INIT, EVENT_AUTO, STATE_RECEIVING_PACKET);

	fsm_setup_entry			(STATE_SENDING_PACKET, &RadioFSM::go_fsm_transmit);
	fsm_setup_next_state(STATE_SENDING_PACKET, EVENT_TIMER_EXPIRY, STATE_RESET);
	fsm_setup_next_state(STATE_SENDING_PACKET, EVENT_PACKET_SENT, STATE_RECEIVING_PACKET);

	fsm_setup_entry			(STATE_RESET, &RadioFSM::go_fsm_reset);
	fsm_setup_next_state(STATE_RESET, EVENT_AUTO, STATE_RECEIVING_PACKET);

	fsm_setup_entry			(STATE_RECEIVED_PREAMBLE, &RadioFSM::go_fsm_preamble);
	fsm_setup_next_state(STATE_RECEIVED_PREAMBLE, EVENT_TIMER_EXPIRY, STATE_SENDING_PACKET);
	fsm_setup_next_state(STATE_RECEIVED_PREAMBLE, EVENT_PACKET_RECEIVED, STATE_PARSE_RECEIVE);

	fsm_setup_entry			(STATE_RECEIVING_PACKET, &RadioFSM::go_fsm_receive);
	fsm_setup_next_state(STATE_RECEIVING_PACKET, EVENT_TIMER_EXPIRY, STATE_HOP);
	fsm_setup_next_state(STATE_RECEIVING_PACKET, EVENT_PREAMBLE_RECEIVED, STATE_RECEIVED_PREAMBLE);

	fsm_setup_entry			(STATE_PARSE_RECEIVE, &RadioFSM::go_fsm_parse_receive);
	fsm_setup_next_state(STATE_PARSE_RECEIVE, EVENT_AUTO, STATE_SENDING_PACKET);

	fsm_setup_entry			(STATE_HOP, &RadioFSM::go_fsm_hop);
	fsm_setup_next_state(STATE_HOP, EVENT_AUTO, STATE_RECEIVING_PACKET);
}
void rxFSM::go_fsm_preamble() {
	unsigned long zzz = micros() - temp;
	if(zzz > temp2)
		temp2 = zzz;
	Utils::printDelayed("Preamble ");
	Utils::printDelayed(String(temp2));
	fsm_timer_start(1000000);
}
void rxFSM::go_fsm_hop() {
		Utils::printDelayed("HOP ");
		++context.currentHOPChannel;
		if(context.currentHOPChannel == NUMBER_OF_HOP_CHANNELS)
		context.currentHOPChannel = 0;
		m_radio->setFHChannel(context.hopChannels[context.currentHOPChannel]);
		Utils::printDelayed(String(context.currentHOPChannel));
		return;
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

void rxFSM::go_fsm_parse_receive() {
	Utils::printDelayed("parse ");
	Utils::printDelayed(String(context.currentHOPChannel));
	return;
	uint8_t len = sizeof(radio_packet);
	m_radio->recv((uint8_t*)&radio_packet, &len);
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

void rxFSM::go_fsm_receive() {
	Utils::printDelayed("Receive ");
	temp = micros();
	m_radio->setModeRx();
	//fsm_timer_start(MAX_FSM_SEND_TIME);
	fsm_timer_start(7500);
}

void rxFSM::go_fsm_reset() {
		m_radio->reset();
}

void rxFSM::go_fsm_transmit() {
	m_radio->setFHChannel(0);
	Utils::printDelayed("transmit ");
	uint8_t usedBytes = SIZE_OF_METADATA;
	uint8_t serialBytes = 0;
 {
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

unsigned long rxFSM::sendTimeout(uint8_t sentBytes) {
	return 0;
}
