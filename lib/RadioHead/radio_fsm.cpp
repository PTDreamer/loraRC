#include "radio_fsm.h"
#include "Arduino.h"
void RadioFSM::received() {
  printf("received handler\n");
	fsm_inject_event(EVENT_PACKET_RECEIVED);
}

void RadioFSM::sent() {
  printf("sent handler\n");
  fsm_inject_event(EVENT_PACKET_SENT);
}
