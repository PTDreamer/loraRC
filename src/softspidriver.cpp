/**
 ******************************************************************************
 * @Filename:	softspidriver.cpp
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

 #include "softspidriver.h"
#define NOP __asm__ __volatile__ ("nop");
 SoftwareSPIDriver::SoftwareSPIDriver(Frequency frequency, BitOrder bitOrder, DataMode dataMode)
     :
     RHGenericSPI(frequency, bitOrder, dataMode)
 {
     setPins(12, 11, 13);
 }

 // Caution: on Arduino Uno and many other CPUs, digitalWrite is quite slow, taking about 4us
 // digitalWrite is also slow, taking about 3.5us
 // resulting in very slow SPI bus speeds using this technique, up to about 120us per octet of transfer
 uint8_t SoftwareSPIDriver::transfer(uint8_t data)
{
 uint8_t readData;
 uint8_t writeData;
 uint8_t builtReturn;
 uint8_t mask;
 mask = 0x80;
 builtReturn = 0;
 readData = 0;

 for (uint8_t count=0; count<8; count++) {
   *_sck_port &= ~_sck_mask;
   NOP;
   if (data & mask)
   {
     writeData = HIGH;
   }
   else
   {
     writeData = LOW;
   }
   // CPHA=0, miso/mosi changing state now
   if(writeData)
     *_mosi_port |= _mosi_mask;
   else
     *_mosi_port &= ~_mosi_mask;
   //digitalWrite(_mosi, writeData);
   NOP;
   *_sck_port |= _sck_mask;
   //digitalWrite(_sck, _clockPolarityreadData);
   NOP;
   //NOP;
   //delayPeriod();
   // CPHA=0, miso/mosi stable now
   readData = (*_miso_port & _miso_mask) >> _miso_bit;
   //NOP;
   //delayPeriod();
   mask >>= 1;
   builtReturn |= (readData << (7 - count));
 }
 //digitalWrite(_sck, _clockPolarity);
 return builtReturn;
}

 /// Initialise the SPI library
 void SoftwareSPIDriver::begin()
 {
     digitalWrite(_sck, 1);
 }

 /// Disables the SPI bus usually, in this case
 /// there is no hardware controller to disable.
 void SoftwareSPIDriver::end() { }

 /// Sets the pins used by this SoftwareSPIClass instance.
 /// \param[in] miso master in slave out pin used
 /// \param[in] mosi master out slave in pin used
 /// \param[in] sck clock pin used
 void SoftwareSPIDriver::setPins(uint8_t miso, uint8_t mosi, uint8_t sck)
 {
     _miso = miso;
     _mosi = mosi;
     _sck = sck;
     _miso_port = pintToPort(miso, true);
     _mosi_port = pintToPort(mosi);
     _sck_port = pintToPort(sck);
     _miso_mask = pintToMask(miso);
     _mosi_mask = pintToMask(mosi);
     _sck_mask = pintToMask(sck);
     _miso_bit = pintToBit(miso);
     pinMode(_miso, INPUT);
     digitalWrite(_miso, 1);
     pinMode(_mosi, OUTPUT);
     pinMode(_sck, OUTPUT);
     digitalWrite(_sck, 1);
 }

volatile uint8_t *SoftwareSPIDriver::pintToPort (uint8_t pin, bool input) {
  if(!input) {
    return (pin < 8  ? &PORTD : pin < 14 ? &PORTB : &PORTC);
  } else {
    return (pin < 8  ? &PIND : pin < 14 ? &PINB : &PINC);
  }
}

volatile uint8_t SoftwareSPIDriver::pintToMask (uint8_t pin) {
  return  _BV(pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
}

volatile uint8_t SoftwareSPIDriver::pintToBit (uint8_t pin) {
  return  (pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
}

 void SoftwareSPIDriver::delayPeriod()
 {
     for (uint8_t count = 0; count < _delayCounts; count++)
     {
 	__asm__ __volatile__ ("nop");
     }
 }
