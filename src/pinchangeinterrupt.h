/**
 ******************************************************************************
 * @Filename:	pinchangeinterrupt.h
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

 #ifndef PIN_CHANGE_INTERRUPT_H
 #define PIN_CHANGE_INTERRUPT_H

 #include "Arduino.h"
 #include "util/atomic.h"

 class PinChangeInterrupt {
 public:
   static void begin();
   static void end();

   enum InterruptMode {
     ON_FALLING_MODE = 0,
     ON_RISING_MODE = 1,
     ON_CHANGE_MODE = 2
   } __attribute__((packed));

   PinChangeInterrupt(uint8_t pin, InterruptMode mode = ON_CHANGE_MODE);

   /**
    * @override{Interrupt::Handler}
    * Enable interrupt pin change detection and interrupt handler.
    * @note atomic
    */
   virtual void enable();

   /**
    * @override{Interrupt::Handler}
    * Disable interrupt pin change detection.
    * @note atomic
    */
   virtual void disable();

   /**
    * @override{Interrupt::Handler}
    * Default interrupt service on pin change interrupt.
    * @param[in] arg argument from interrupt service routine.
    */
   virtual void on_interrupt(uint16_t arg = 0) = 0;
   static void on_interrupt(uint8_t ix, uint8_t mask, uint8_t base);

 private:
   static PinChangeInterrupt* s_pin[3];
   static uint8_t s_state[3];
   static inline volatile uint8_t* PCIMR_FromPin(uint8_t pin)__attribute__((always_inline));
   uint8_t pin_change_mask;
   volatile uint8_t* m_pcimr;
   /** Interrupt Mode. */
   InterruptMode m_mode;

   /** Linked list of pin change interrupt handlers. */
   PinChangeInterrupt* m_next;

   /**
    * Map interrupt source: Check which pin(s) are the source of the
    * pin change interrupt and call the corresponding interrupt handler
    * per pin.
    * @param[in] ix port index.
    * @param[in] mask pin mask.
    * @param[in] base pin number.
    */

 };
 #endif
