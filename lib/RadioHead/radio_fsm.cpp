/**
 ******************************************************************************
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

#include "radio_fsm.h"
#include "Arduino.h"
RadioFSM::RadioFSM(Fifo *fifo): serialFifo(fifo) {

}
void RadioFSM::received() {
  hasReceived = true;
  printf("received handler\n");
	fsm_inject_event(EVENT_PACKET_RECEIVED);
}

void RadioFSM::sent() {
  hasSent = true;
  printf("sent handler\n");
  fsm_inject_event(EVENT_PACKET_SENT);
}
void RadioFSM::setRadio(RH_RF22JB *radio) {
	m_radio = radio;
}

float RadioFSM::getChannelRSSI(uint8_t channel) {
	if(context.stats[channel].sentNOK == 0 || context.stats[channel].receivedNOK == 0)
		return 0xFFFF;
	float rssi_tx = context.stats[channel].sentOK / context.stats[channel].sentNOK;
	float rssi_rx = context.stats[channel].receivedOK / context.stats[channel].receivedNOK;
	return (rssi_tx + rssi_rx) / 2;
}

void RadioFSM::fsm_timer_start(unsigned long timer_duration_us)
{
	context.fsm_timer_remaining_us = timer_duration_us;
	context.fsm_timer_enabled = true;
}

void RadioFSM::fsm_timer_cancel()
{
	context.fsm_timer_enabled = false;
}

void RadioFSM::fsm_timer_add_ticks(unsigned long elapsed_us)
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

bool RadioFSM::fsm_timer_expired_p()
{
	if ((context.fsm_timer_enabled) && (context.fsm_timer_remaining_us == 0))
		return true;

	return false;
}

void RadioFSM::fsm_process_auto()
{
	while (fsm_transitions[context.curr_state].next_state[EVENT_AUTO]) {
		printf("current %d\n", context.curr_state);
		context.curr_state = fsm_transitions[context.curr_state].next_state[EVENT_AUTO];
		printf("next %d\n", context.curr_state);
		/* Call the entry function (if any) for the next state. */
		if (fsm_transitions[context.curr_state].entry_fn) {
			(this->*fsm_transitions[context.curr_state].entry_fn)();
		}
	}
}

void RadioFSM::fsm_inject_event(enum fsm_events event)
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

void RadioFSM::fsm_init()
{
	context.curr_state = STATE_INIT;
	fsm_process_auto();
}

void RadioFSM::fsm_setup_entry(fsm_states state, void (RadioFSM::*fn)()) {
	fsm_transitions[state].entry_fn = fn;
//	memset(fsm_transitions[state].next_state, STATE_FSM_FAULT, EVENT_NUM_EVENTS + 1);  TODO FIND why memset not working
	for(int x = 0; x < EVENT_NUM_EVENTS; ++x) {
		fsm_transitions[state].next_state[x] = STATE_FSM_FAULT;
		printf("fsm_transitions for state %d %d=%d\n", state, x, fsm_transitions[state].next_state[x]);
	}
}

void RadioFSM::fsm_setup_next_state(fsm_states state, fsm_events event, fsm_states nextState) {
	fsm_transitions[state].next_state[event] = nextState;
}
enum rxFSM::fsm_states rxFSM::fsm_get_state()
{
	return context.curr_state;
}

void RadioFSM::go_fsm_fault()
{
	fsm_timer_start(10);
	printf("fsm FAULT\n");
//	led_pwm_config(&context->leds, 2500, 100, 2500, 100);
}