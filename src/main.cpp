#if (ISTRANSMITER)
#define SDO_pin 9
#define SDI_pin 8
#define SCLK_pin 7
#define IRQ_pin 2
#define nSel_pin 4
#define IRQ_interrupt 0
#else
#define SDO_pin A0
#define SDI_pin A1
#define SCLK_pin A2
#define IRQ_pin 2
#define nSel_pin 4
#define IRQ_interrupt 0
#endif

#define RSSI_out_pin 5

#define PPM_out 3

#include <RH_RF22JB.h>

#include "softspidriver.h"
#include "configComms.h"
#if (ISTRANSMITER)
#include "ppm_in_driver.h"
#include "txFSM.h"
#else
#include "ppm_out_driver.h"
#include "rxFSM.h"
#endif
#include "fifo.h"
#include "utils.h"
#include <avr/wdt.h>
Fifo fifo(10);
settings mySettings;
SoftwareSPIDriver spi(RHGenericSPI::Frequency1MHz);
RH_RF22JB rf22(nSel_pin, IRQ_pin, spi);

RadioFSM *fsm;

bool goConfig = false;

void setup()
{
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
  Utils::printfInit();
  Serial.write("RESTART");

MEM_REPORT;
  #if (ISTRANSMITER)
  PPMDriver *ppm = new PPMDriver(PIN3, 8);
  fsm = new txFSM(ppm, &fifo);
  #else
  PPM_OutDriver *ppm = new PPM_OutDriver(PIN3, 8);
  fsm = new rxFSM(ppm, &fifo);
  #endif

  rf22.setRadioFSM(fsm);
  if (!rf22.init())
    Serial.println("init failed");
  rf22.setFrequency(413.0);
  rf22.setFHStepSize(25);

  Serial.println("loop");
}
int x = 1;
unsigned long mil = millis();

void loop()
{
  fsm->fsm_init();
/*
  uint8_t data[] = "And hello back to you";
  String str = String(x);
  char charBuffer[50];
  str.toCharArray(data,4);
//  memcpy(data, charBuffer, 50);
  printf("%d;", x);
  rf22.send(data, 50);
  rf22.waitPacketSent();*/
  uint32_t lastCount = 0;

  while (true) {
#if (ISTRANSMITER == 0)
  fsm->handle();
  if(true  && (millis() - mil > 1000)) {
  //  Utils::handlePrintDelayed();
    printf("OK=%lu %lu %lu %lu\n",fsm->context.stats[0].receivedOK,fsm->context.stats[1].receivedOK,fsm->context.stats[2].receivedOK,fsm->context.stats[3].receivedOK);
    printf("NOK=%lu %lu %lu %lu\n",fsm->context.stats[0].receivedNOK,fsm->context.stats[1].receivedNOK,fsm->context.stats[2].receivedNOK,fsm->context.stats[3].receivedNOK);
    printf("SYNC:%d %d\n",fsm->context.isInSync, fsm->context.numberOfRxTimeouts);
    ++x;
    Serial.println(fsm->context.curr_state);
    mil = millis();
  }
}
#else
fsm->handle();
if(millis() - mil > 5000) {
  //Utils::handlePrintDelayed();
  printf("OK=%lu %lu %lu %lu\n",fsm->context.stats[0].receivedOK,fsm->context.stats[1].receivedOK,fsm->context.stats[2].receivedOK,fsm->context.stats[3].receivedOK);
  printf("NOK=%lu %lu %lu %lu\n",fsm->context.stats[0].receivedNOK,fsm->context.stats[1].receivedNOK,fsm->context.stats[2].receivedNOK,fsm->context.stats[3].receivedNOK);
//  if(fsm->context.stats[0].receivedOK == lastCount)
  //  fsm->context.debug = true;
  lastCount = fsm->context.stats[0].receivedOK;
  mil = millis();
}
}
#endif
#if 0


if(millis() - mil > 1000) {
  printf("ok\n");
  mil = millis();
char data[] = "And hello back to you";
String str = String(x);
str.toCharArray(data,4);
printf("%d;", x);
rf22.send((uint8_t*)data, 10);
rf22.waitPacketSent();
}

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
