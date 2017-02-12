#ifndef configMessages_h
#define configMessages_h

#if defined(DESKTOP_BUILD)
#define uint32_t quint32
#define uint8_t quint8
#define uint16_t quint16
#else
#include "Arduino.h"
#endif
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#define MAGIC 0xF1
typedef enum _parseState {STATE_IDLE, STATE_MAGIC, STATE_LENGTH, STATE_TYPE, STATE_MESSAGE, STATE_CRC} parseState;
typedef enum _messagesType {TYPE_START_SCANNING, TYPE_SCAN_INIT_RESULT, TYPE_SCAN_UPDATE, TYPE_STOP_SCAN} messagesType;

struct PACK(scanUpdate {
  float frequency;
#if defined(DESKTOP_BUILD)
  uint8_t rssi[SCANS_PER_PACKET];
#endif
});

struct PACK(startScanning {
  float startFrequency;
  float endFrequency;
  float stepSize;
  uint8_t bytesPerPacket;
  uint8_t averagingSample;
});

struct PACK(settings {
  bool isRX;
  uint8_t ppmChannels;
  uint8_t radioChannels[4];
  uint8_t address;
  uint8_t power;
});

struct PACK(stopScanning {

});

struct PACK(scanInitResult {
  uint8_t initResult;
  uint8_t numberOfPackets;
  uint8_t lengthOfLastPacket;
  uint8_t sizeOfPackets;
  float stepSize;
});

union PACK(configMessages {
  startScanning start;
  scanInitResult si;
  scanUpdate su;
  stopScanning stop;
  settings st;
});

typedef struct PACK(_configMessage {
  uint8_t magic;
  uint8_t length;
  uint8_t type;
  uint8_t message[sizeof(configMessages) + sizeof(uint16_t)];
})configMessage;

#endif
