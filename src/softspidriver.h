/**
 ******************************************************************************
 * @Filename:	softspidriver.h
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

 #ifndef SoftwareSPIDriver_h
 #define SoftwareSPIDriver_h

 #include "../lib/RadioHead/RHGenericSPI.h"

 class SoftwareSPIDriver : public RHGenericSPI
 {
 public:

     /// Constructor
     /// Creates an instance of a bit-banged software SPI interface.
     /// Sets the SPI pins to the defaults of
     /// MISO = 12, MOSI = 11, SCK = 13. If you need other assigments, call setPins() before
     /// calling manager.init() or driver.init().
     /// \param[in] frequency One of RHGenericSPI::Frequency to select the SPI bus frequency. The frequency
     /// is mapped to the closest available bus frequency on the platform. CAUTION: the achieved
     /// frequency will almost certainly be very much slower on most platforms. eg on Arduino Uno, the
     /// the clock rate is likely to be at best around 46kHz.
     /// \param[in] bitOrder Select the SPI bus bit order, one of RHGenericSPI::BitOrderMSBFirst or
     /// RHGenericSPI::BitOrderLSBFirst.
     /// \param[in] dataMode Selects the SPI bus data mode. One of RHGenericSPI::DataMode
     SoftwareSPIDriver(Frequency frequency = Frequency1MHz, BitOrder bitOrder = BitOrderMSBFirst, DataMode dataMode = DataMode0);

     /// Transfer a single octet to and from the SPI interface
     /// \param[in] data The octet to send
     /// \return The octet read from SPI while the data octet was sent.
     uint8_t transfer(uint8_t data);

     /// Initialise the software SPI library
     /// Call this after configuring the SPI interface and before using it to transfer data.
     /// Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
     void begin();

     /// Disables the SPI bus usually, in this case
     /// there is no hardware controller to disable.
     void end();

     /// Sets the pins used by this SoftwareSPIClass instance.
     /// The defaults are: MISO = 12, MOSI = 11, SCK = 13.
     /// \param[in] miso master in slave out pin used
     /// \param[in] mosi master out slave in pin used
     /// \param[in] sck clock pin used
     void setPins(uint8_t miso = 12, uint8_t mosi = 11, uint8_t sck = 13);

 private:

     /// Delay routine for bus timing.
     inline void delayPeriod();
     static volatile uint8_t *pintToPort (uint8_t pin, bool input = false);
     static volatile uint8_t pintToMask (uint8_t pin);
     static volatile uint8_t pintToBit (uint8_t pin);
 private:
     uint8_t _miso;
     uint8_t _mosi;
     uint8_t _sck;
     uint8_t _miso_mask;
     uint8_t _mosi_mask;
     uint8_t _sck_mask;
     uint8_t _miso_bit;
     volatile uint8_t *_miso_port;
     volatile uint8_t *_mosi_port;
     volatile uint8_t *_sck_port;
     uint8_t _delayCounts;
 };

 #endif
