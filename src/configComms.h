#ifndef ConfigComms_h
#define ConfigComms_h

#include "RH_RF22.h"
#include "configMessages.h"
#include "scanner.h"

class ConfigComms {
public:
  ConfigComms(RH_RF22 *radio, Scanner *scan);
  void handleConfigComms();
  void processStarScan(startScanning *msg);
  void processReceivedMessage(configMessage msg);
  void sendMessage(configMessage *msg);
  void sendPartialMessage(configMessage *msg, uint8_t data, bool isFinal);
private:
  struct _scanInfo {
    uint8_t currentPacket;
    uint8_t currentByte;
    uint8_t numberOfPackets;
    uint8_t sizeOfLastPacket;
    uint8_t lenghtOfPackets;
  } scanInfo;
  void sendScanUpdate(bool flush = false);
  RH_RF22 *radioHandler;
  parseState currentParseState;
  configMessage receiveMessage;
  configMessage transmitMessage;
  Scanner *scanner;
  bool isScanning;
};

#endif
