/**
 ******************************************************************************
 * @Filename:	pinchangeinterrupt.cpp
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

 #include "pinchangeinterrupt.h"
 // Define symbols for enable/disable pin change interrupts
 #if defined(GIMSK)
 #define PCICR GIMSK
 #endif
 #define PCIEN (_BV(PCIE2) | _BV(PCIE1) | _BV(PCIE0))
 #define bit_mask_set(p,m) ((p) |= (m))
#define bit_mask_clear(p,m) ((p) &= ~(m))

 PinChangeInterrupt* PinChangeInterrupt::s_pin[3] = { NULL };
 uint8_t PinChangeInterrupt::s_state[3] = { 0 };

 PinChangeInterrupt::PinChangeInterrupt(uint8_t pin, InterruptMode mode) :
 m_mode(mode),
 m_next(NULL)
 {
   pin_change_mask = _BV(pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
   m_pcimr = PCIMR_FromPin(pin);
 }

volatile uint8_t* PinChangeInterrupt::PCIMR_FromPin(uint8_t pin)//Pin Change Mask Register
{
  return (pin < 8  ? &PCMSK2 :
   pin < 14 ? &PCMSK0 :
              &PCMSK1);
}

 void PinChangeInterrupt::enable()
 {
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
     *m_pcimr |= pin_change_mask;
     if (m_next == NULL) {
       uint8_t ix = m_pcimr - &PCMSK0;
       m_next = s_pin[ix];
       s_pin[ix] = this;
     }
   }
 }

 void PinChangeInterrupt::disable()
 {
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) *m_pcimr &= ~pin_change_mask;
 }

 void PinChangeInterrupt::begin()
 {
   s_state[0] = PINB;
   s_state[1] = PINC;
   s_state[2] = PIND;

   // Enable the pin change interrupt(s)
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) bit_mask_set(PCICR, PCIEN);
 }

 void PinChangeInterrupt::end()
 {
   // Disable the pin change interrupt(s)
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) bit_mask_clear(PCICR, PCIEN);
 }

 void PinChangeInterrupt::on_interrupt(uint8_t vec, uint8_t mask, uint8_t port)
 {
   uint8_t old_state = s_state[vec];
   uint8_t new_state = port;
   uint8_t changed = (new_state ^ old_state) & mask;

   // Find the interrupt handler for the changed value and check mode
   for (PinChangeInterrupt* pin = s_pin[vec]; pin != NULL; pin = pin->m_next)
   if ((pin->pin_change_mask & changed) && ((pin->m_mode == ON_CHANGE_MODE) ||
        pin->m_mode == ((pin->pin_change_mask & new_state) == 0)))
          pin->on_interrupt();
   // Save the new pin state
   s_state[vec] = new_state;
 }

 #define PCINT_ISR(vec,pin)					\
 ISR(PCINT ## vec ## _vect)					\
 {							\
   PinChangeInterrupt::on_interrupt(vec, PCMSK ## vec, pin);	\
 }

 PCINT_ISR(0, PINB);
 PCINT_ISR(1, PINC);
 PCINT_ISR(2, PIND);
