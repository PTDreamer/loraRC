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
#if PTYPE==1
#define SDO_pin A0
#define SDI_pin A1
#define SCLK_pin A2
#define IRQ_pin 2
#define nSel_pin 4
#define IRQ_interrupt 0
#else
#define SDO_pin 9
#define SDI_pin 8
#define SCLK_pin 7
#define IRQ_pin 2
#define nSel_pin 4
#define IRQ_interrupt 0
#endif

#define RSSI_out_pin 5

#define PPM_out 3

#include <RH_RF22JB.h>
#include <RH_RF22.h>
#include <RHSoftwareSPI.h>
#include "softspidriver.h"
#include "configComms.h"
#if (PTYPE == 0)
#include "ppm_in_driver.h"
#include "txFSM.h"
#else
#include "ppm_out_driver.h"
#include "rxFSM.h"
#endif
#include "fifo.h"
#include "utils.h"

Fifo fifo(10);
settings mySettings;
SoftwareSPIDriver spi(RHGenericSPI::Frequency1MHz);
RH_RF22JB rf22(nSel_pin, IRQ_pin, spi);

RadioFSM *fsm_pointer;

bool goConfig = false;


//FILE serial_stdout;

void setup()
{
  spi.setPins(SDO_pin, SDI_pin, SCLK_pin);
  spi.begin();
  const char appStr[] = {'l','o','r','a','A','p','p'};
  Serial.begin(115200);
  Serial.println("START");

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
//fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
//stdout = &serial_stdout;
Utils::printfInit();

#if PTYPE==0
PPMDriver ppm(PIN3, 8);
txFSM fsm(&ppm, &fifo);
#else
PPM_OutDriver ppm(PIN3, 8);
rxFSM fsm(&ppm, &fifo);
#endif
fsm_pointer = &fsm;
#if PTYPE == 0
rf22.setRadioFSM(&fsm);
#else
rf22.setRadioFSM(&fsm);
#endif
if (!rf22.init())
  Serial.println("init failed");
rf22.setFrequency(413.0);
rf22.setFHStepSize(25);
fsm_pointer->fsm_init();

}
int x = 1;
unsigned long mil = millis();

void loop()
{
  delay(1000);
#if PTYPE == 1
  fifo.push(0);
  //fifo.push(1);
  //fifo.push(2);
  //fifo.push(3);
  fsm_pointer->handle();
  if(millis() - mil > 1000) {
    printf("ok\n");
    mil = millis();

  }
#else
if(millis() - mil > 1000) {
  printf("ok\n");
  mil = millis();
uint8_t data[] = "And hello back to you";
String str = String(x);
char charBuffer[50];
str.toCharArray(data,4);
//  memcpy(data, charBuffer, 50);
printf("%d;", x);
rf22.send(data, 10);
rf22.waitPacketSent();
}
#endif
#if 0
#if (PTYPE==0)
    uint8_t data[] = "And hello back to you";
    String str = String(x);
    char charBuffer[50];
    str.toCharArray(data,4);
  //  memcpy(data, charBuffer, 50);
    printf("%d;", x);
    rf22.send(data, 10);
    rf22.waitPacketSent();
    ++x;
delay(500);
#else
uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);
if (rf22.recv(buf, &len))
{
  Serial.print("got reply: ");
  Serial.println((char*)buf);
//  digitalWrite(PIN3, 0);
  delay(100);
//  digitalWrite(PIN3, 1);

}
#endif
#endif

}
