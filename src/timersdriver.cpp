/**
 ******************************************************************************
 * @Filename:	timersdriver.cpp
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

#include "timersdriver.h"

                                            // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
const uint8_t WGM_BITS_TIMER_0_2[]  = { 0, 1, 0, 0, 0, 5, 0, 2, 0, 3, 0, 0, 0, 7, 0, 0, 0 };
const uint8_t WGM_BITS_TIMER_1[]  =   { 0, 0, 1, 2, 3,11,10, 4,12, 0, 5, 6, 7,15,14, 9, 8 };
const uint8_t COM_BITS[] = { 0, 1, 2, 3 };
                                          //  0  1  2  3  4  5  6  7  8  9
const uint8_t CS_BITS_TIMER_0_1[]  = { 0, 1, 2, 0, 3, 0, 4, 5, 6, 7};
const uint8_t CS_BITS_TIMER_2[]  =   { 0, 1, 2, 3, 4, 5, 6, 7, 0, 0};

TimerDriver * TimerDriver::s_pin[3] = { NULL };

TimerDriver::TimerDriver():enabledInterrupts(0), m_next(NULL) {

}
void TimerDriver::init(timerConfig cfg) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  if (cfg.timerID == TIMER_0) {
    TCCRA = &TCCR0A;
    TCCRB = &TCCR0B;
    TCNTL = &TCNT0;
    TCNTH = 0;
    OCRAL = &OCR0A;
    OCRAH = 0;
    OCRBL = &OCR0B;
    OCRBH = 0;
    TIMSK = &TIMSK0;
    TIFR = &TIFR0;
    ICRL = 0;
    ICRH = 0;
  }
  else if(cfg.timerID == TIMER_1){
    TCCRA = &TCCR1A;
    TCCRB = &TCCR1B;
    TCNTL = &TCNT1L;
    TCNTH = &TCNT1H;
    OCRAL = &OCR1AL;
    OCRAH = &OCR1AH;
    OCRBL = &OCR1BL;
    OCRBH = &OCR1BH;
    TIMSK = &TIMSK1;
    TIFR = &TIFR1;
    ICRL = &ICR1L;
    ICRH = &ICR1H;
  }
  else {
    TCCRA = &TCCR2A;
    TCCRB = &TCCR2B;
    TCNTL = &TCNT2;
    TCNTH = 0;
    OCRAL = &OCR2A;
    OCRAH = 0;
    OCRBL = &OCR2B;
    OCRBH = 0;
    TIMSK = &TIMSK2;
    TIFR = &TIFR2;
    ICRL = 0;
    ICRH = 0;
  }
  switch (cfg.timerID) {
    case TIMER_0:
    case TIMER_2:
      *TCCRA = (COM_BITS[cfg.compareA] << 6) | (COM_BITS[cfg.compareB] << 4) | (WGM_BITS_TIMER_0_2[cfg.waveForm] & 0x03);
      *TCCRB = ((WGM_BITS_TIMER_0_2[cfg.waveForm] & 0x04) << 1);
    break;
    case TIMER_1:
      TCCR1A = (COM_BITS[cfg.compareA] << 6) | (COM_BITS[cfg.compareB] << 4) | (WGM_BITS_TIMER_1[cfg.waveForm] & 0x03);
      TCCR1B = ((WGM_BITS_TIMER_1[cfg.waveForm] & 0x0C) << 1) | (cfg.useInputCaptureNoiseCanceler << 7) | (cfg.inputCaptureEdgeSelect << 6);
    break;
  }
  switch (cfg.timerID) {
    case TIMER_0:
    case TIMER_1:
        *TCCRB |= CS_BITS_TIMER_0_1[cfg.tclock];
    break;
    case TIMER_2:
        TCCR2B |= CS_BITS_TIMER_2[cfg.tclock];
    break;
  }
  if (m_next == NULL) {
    uint8_t ix = cfg.timerID;
    m_next = s_pin[ix];
    s_pin[ix] = this;
  }
  }
}

void TimerDriver::enableInterrupts(uint8_t interrupts) {
  *TIMSK |= interrupts;
  enabledInterrupts |= interrupts;
}

void TimerDriver::disableInterrupts(uint8_t interrupts) {
  *TIMSK &= ~interrupts;
  enabledInterrupts &= ~interrupts;
}

uint16_t TimerDriver::getOutputCompareValueA() {
  uint16_t ret;
  if(OCRAH)
    ret = OCR1A;
  else
    ret = *OCRAL;
  return ret;
}

uint16_t TimerDriver::getOutputCompareValueB() {
  uint16_t ret;
  if(OCRBH)
    ret = OCR1B;
  else
    ret = *OCRBL;
  return ret;
}

uint16_t TimerDriver::getInputCapture() {
  uint16_t ret;
  ret = ICR1;
  return ret;
}

void TimerDriver::on_compareAinterrupt_priv(uint8_t ix) {
  for (TimerDriver* tim = s_pin[ix]; tim != NULL; tim = tim->m_next) {
    if(tim->enabledInterrupts & INT_ON_COMPARE_A)
      tim->on_compareAinterrupt();
  }
}

void TimerDriver::on_compareBinterrupt_priv(uint8_t ix) {
  for (TimerDriver* tim = s_pin[ix]; tim != NULL; tim = tim->m_next) {
    if(tim->enabledInterrupts & INT_ON_COMPARE_B)
      tim->on_compareBinterrupt();
  }
}

void TimerDriver::on_overflow_interrupt_priv(uint8_t ix) {
  for (TimerDriver* tim = s_pin[ix]; tim != NULL; tim = tim->m_next) {
    if(tim->enabledInterrupts & INT_ON_OVERFLOW)
      tim->on_overflow_interrupt();
  }
}

void TimerDriver::on_input_capture_interrupt_priv(uint8_t ix) {
  for (TimerDriver* tim = s_pin[ix]; tim != NULL; tim = tim->m_next) {
    if(tim->enabledInterrupts & INT_ON_INPUT_CAPTURE)
      tim->on_input_capture_interrupt();
  }
}

#define TIMER_COMP_ISR(timnumber,comp)					\
ISR(TIMER ## timnumber ## _COMP ## comp ## _vect)	{ \
  TimerDriver::on_compare ## comp ## interrupt_priv(timnumber); }
#define TIMER_OVF_ISR(timnumber)					\
ISR(TIMER ## timnumber ## _OVF_vect) { \
  TimerDriver::on_overflow_interrupt_priv(timnumber); }
TIMER_COMP_ISR(0, A)
TIMER_COMP_ISR(0, B)
TIMER_COMP_ISR(1, A)
TIMER_COMP_ISR(1, B)
TIMER_COMP_ISR(2, A)
TIMER_COMP_ISR(2, B)
//TIMER_OVF_ISR(0)//used by arduino
TIMER_OVF_ISR(1)
TIMER_OVF_ISR(2)
ISR(TIMER1_CAPT_vect)
{
  TimerDriver::on_input_capture_interrupt_priv(1);
}
