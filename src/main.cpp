// rf22_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF22 class. RH_RF22 class does not provide for addressing or
// reliability, so you should only use RH_RF22 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf22_server
// Tested on Duemilanove, Uno with Sparkfun RFM22 wireless shield
// Tested on Flymaple with sparkfun RFM22 wireless shield
// Tested on ChiKit Uno32 with sparkfun RFM22 wireless shield
//413 a 453mhz
#define UART_RX0_BUFFER_SIZE 256
#define UART_TX0_BUFFER_SIZE 256
#define SDO_pin A0
#define SDI_pin A1
#define SCLK_pin A2
#define IRQ_pin 2
#define nSel_pin 4
#define IRQ_interrupt 0

//#define SDO_pin 12 //pb4
//#define SDI_pin 11 //pb3
//#define SCLK_pin 13 //pb5

#define RSSI_out_pin 5

#define PPM_out 3

//#define IRQ_pin 2 //pd2
//#define nSel_pin 4 //pd4
//#define IRQ_interrupt 0

#include <RH_RF22JB.h>
#include <RHSoftwareSPI.h>
#include "softspidriver.h"
#include "configComms.h"
#include "ppm_in_driver.h"
#include "ppm_out_driver.h"
#include "txFSM.h"

settings mySettings;
SoftwareSPIDriver spi(RHGenericSPI::Frequency1MHz);
RH_RF22JB rf22(nSel_pin, IRQ_pin, spi);
PPMDriver ppm(PIN3, 8);
txFSM fsm(&ppm);
Scanner scan(&rf22);
ConfigComms cc(&rf22, &scan);
bool goConfig = false;
//uint16_t ppmv[8];
//PPMDriver ppm(PIN3, 8);
//PPM_OutDriver *ppm;
uint16_t ppp[8];
uint8_t pwm[8] = {12, 255,255,255,255,255,255,255};
int serial_putchar(char c, FILE* f) {
    if (c == '\n') serial_putchar('\r', f);
    return Serial.write(c) == 1? 0 : 1;
}

FILE serial_stdout;

void setup()
{
  ppp[0] = 0;
  ppp[1] = 110;
  ppp[2] = 110;
  ppp[3] = 110;
  ppp[4] = 100;
  ppp[5] = 100;
  ppp[6] = 100;
  ppp[7] = 100;
  Serial.begin(115200);
  //ppm = new PPM_OutDriver(11 , 8, pwm);
  //ppm->setPPMValues(ppp);
//  pinMode(nSel_pin, OUTPUT);
  //pinMode(IRQ_pin, INPUT);
  //pinMode(nSel_pin, OUTPUT);
  //digitalWrite(nSel_pin, HIGH);

//  digitalWrite(nSel_pin, HIGH);
  spi.setPins(SDO_pin, SDI_pin, SCLK_pin);
  spi.begin();
  const char appStr[] = {'l','o','r','a','A','p','p'};
  Serial.begin(115200);
  unsigned long timeNow = millis();
  int index = 0;
  while((millis() - timeNow < 100) && !goConfig) {
    while (Serial.available() && !goConfig) {
      if(Serial.read() == appStr[index]) {
        ++index;
        if(index == sizeof(appStr)/sizeof(char)) {
          for(int x = 0; x < 10; ++x)
            Serial.write("loraFw");
          goConfig = true;
          Serial.flush();
        }
      }
      else {
        index = 0;
      }
    }
  }
  //scan.initScan(430, 450, 0.1f, 10);
  //if (!rf22.init())
    //Serial.println("init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  sei();
  // Set up stdout
fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
stdout = &serial_stdout;
rf22.setRadioFSM(&fsm);
if (!rf22.init())
  Serial.println("init failed");
rf22.setFrequency(413.0);
rf22.setFHStepSize(25);
rf22.setRadioID(mySettings.address);
rf22.setTxPower(mySettings.power);
}

int x = 0;

void loop()
{
  uint8_t data[] = "And hello back to you";
  rf22.send(data, sizeof(data));
  unsigned long m = micros();
  Serial.println("SEND");
  Serial.println(m);
  delay(3000);
  if (rf22.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf22.recv(buf, &len))
    {
//      RF22::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf22.lastRssi(), DEC);

      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf22.send(data, sizeof(data));
      rf22.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  if(goConfig) {
      cc.handleConfigComms();
  }

}
