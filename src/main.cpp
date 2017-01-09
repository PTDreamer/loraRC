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

#include <RH_RF22.h>
#include <RHSoftwareSPI.h>
#include "softspidriver.h"
#include "configComms.h"
//#include "ppm_driver.h"
#include "ppm_out_driver.h"

//RHSoftwareSPI spi(RHGenericSPI::Frequency16MHz);
SoftwareSPIDriver spi(RHGenericSPI::Frequency1MHz);
RH_RF22 rf22(nSel_pin, IRQ_pin, spi);
Scanner scan(&rf22);
ConfigComms cc(&rf22, &scan);
bool goConfig = false;
//uint16_t ppmv[8];
//PPMDriver ppm(PIN3, 8);
PPM_OutDriver *ppm;
uint16_t ppp[8];

int serial_putchar(char c, FILE* f) {
    if (c == '\n') serial_putchar('\r', f);
    return Serial.write(c) == 1? 0 : 1;
}

FILE serial_stdout;

void setup()
{
  ppp[0] = 110;
  ppp[1] = 110;
  ppp[2] = 110;
  ppp[3] = 110;
  ppp[4] = 100;
  ppp[5] = 100;
  ppp[6] = 100;
  ppp[7] = 100;
  Serial.begin(115200);
  ppm = new PPM_OutDriver(11 , 8);
  ppm->setPPMValues(ppp);
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
}
int x = 0;
// Function that printf and related will use to print


void loop()
{
  ppp[0] = x;
  ppm->setPPMValues(ppp);
  ++x;
  if(x == 1024)
    x = 0;
  for(int q = 0; q < 8; ++q) {
    uint16_t s;
    uint16_t ss;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    s = (ppm->ICR[q]);
    ss = ppm->COMP[q];
    }
    printf("[%d] ICR:%u COMP:%u\n", q, s,ss);
  }
  delay(1000);
  /*if(x) {
    digitalWrite(9, true);
    x = false;
  }
  else {
    digitalWrite(9, false);
    x = true;
  }
  delay(100);*/
//  _scanResult res = scan.continueScan();
  //if(res.frequency) {
    //Serial.println(res.frequency);
//    Serial.println(res.rssi);
//  }
  if(goConfig) {
      cc.handleConfigComms();
  }
  /*
  //Serial.println("Sending to rf22_server");
  // Send a message to rf22_server
  uint8_t data[] = "Hello World!";
  rf22.send(data, sizeof(data));

  rf22.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf22.waitAvailableTimeout(500))
  {
    // Should be a reply message for us now
    if (rf22.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf22_server running?");
  }
  delay(400);*/
}
