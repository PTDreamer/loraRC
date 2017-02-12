#include "configComms.h"
#include "checksum.h"
ConfigComms::ConfigComms(RH_RF22JB *radio, Scanner *scan) {
    radioHandler = radio;
    currentParseState = STATE_IDLE;
    scanner = scan;
    isScanning = false;
}

void ConfigComms::processReceivedMessage(configMessage msg) {
    switch (msg.type) {
      case TYPE_START_SCANNING:
        //Serial.print("received start scan");
        startScanning ss;
        memcpy(&ss, msg.message, sizeof(startScanning));
        processStarScan(&ss);
        break;
      case TYPE_STOP_SCAN:
        if(isScanning)
          sendScanUpdate(true);
        break;
    }
}

void ConfigComms::processStarScan(startScanning *msg) {
  if(isScanning) {
    sendScanUpdate(true);
  }
  scanInitResult ir;
  transmitMessage.type = TYPE_SCAN_INIT_RESULT;
  transmitMessage.length = sizeof(scanInitResult);
  if(!ir.initResult) {
    ir.initResult = scanner->initScan(msg->startFrequency, msg->endFrequency, msg->stepSize, msg->averagingSample);
    if(!ir.initResult)
      isScanning = true;
    scanInfo.currentPacket = 0;
    scanInfo.currentByte = 0;
    scanInfo.lenghtOfPackets = msg->bytesPerPacket;
    float temp = ((msg->endFrequency - msg->startFrequency) / msg->stepSize);
    scanInfo.numberOfPackets = ceil(temp / scanInfo.lenghtOfPackets);
    scanInfo.sizeOfLastPacket = ceil(temp) - ((scanInfo.numberOfPackets - 1) * scanInfo.lenghtOfPackets);
    ir.numberOfPackets = scanInfo.numberOfPackets;
    ir.lengthOfLastPacket = scanInfo.sizeOfLastPacket;
    ir.sizeOfPackets = msg->bytesPerPacket;
    ir.stepSize = msg->stepSize;
  }
  memcpy(transmitMessage.message, &ir, sizeof(scanInitResult));
  sendMessage(&transmitMessage);
}

void ConfigComms::sendMessage(configMessage *msg) {
  msg->magic = MAGIC;
  uint16_t crc;
  crc_init(&crc);
  crc_accumulate(msg->length, &crc);
  crc_accumulate(msg->type, &crc);
  for(int x = 0; x < msg->length;++x) {
    crc_accumulate(*(msg->message + x), &crc);
  }
  *((uint16_t*)(msg->message + msg->length)) = crc;
  for(uint8_t x = 0; x < msg->length + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t);++x) {
    //Serial.write((const uint8_t *)(msg), sizeof(configMessage));
    Serial.write(*(((uint8_t*)(msg)) + x));
  }
}

void ConfigComms::sendPartialMessage(configMessage *msg, uint8_t data, bool isFinal) {
  static uint16_t crc;
  if(msg) {
    msg->magic = MAGIC;
    crc_init(&crc);
    for(uint8_t x = 0; x < sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(scanUpdate);++x) {//magic lenght type
      Serial.write(*(((uint8_t*)(msg)) + x));
      if(x != 0)
        crc_accumulate(*(((uint8_t*)(msg)) + x), &crc);
    }
  }
  else {
    crc_accumulate(data, &crc);
    Serial.write(data);
  }
  if(isFinal) {
    Serial.write(crc & 0x00FF);
    Serial.write(crc >> 8);
  //  Serial.write(crc && 0xFF);
  }
}

void ConfigComms::sendScanUpdate(bool flush) {
  //Serial.print("SEND SCAN UPDATE");
  bool isLastPacket = (scanInfo.currentPacket == scanInfo.numberOfPackets -1);
  if(flush) {
    if(scanInfo.currentByte != 0) {
      for(int x = scanInfo.currentByte; x < (isLastPacket? scanInfo.sizeOfLastPacket:scanInfo.lenghtOfPackets); ++x) {
        bool isLastByte = (isLastPacket && (scanInfo.currentByte == scanInfo.sizeOfLastPacket - 1)) || (!isLastPacket && (scanInfo.currentByte == scanInfo.lenghtOfPackets - 1));
        sendPartialMessage(NULL, 0, isLastByte);
        ++scanInfo.currentByte;
      }
    }
    isScanning = false;
    return;
  }
  scanResult res = scanner->continueScan();
  scanUpdate su;
  su.frequency = res.frequency;
  bool isLastByte = (isLastPacket && (scanInfo.currentByte == scanInfo.sizeOfLastPacket - 1)) || (!isLastPacket && (scanInfo.currentByte == scanInfo.lenghtOfPackets - 1));
  if(su.frequency) {
    if(scanInfo.currentByte == 0) {
      memcpy(transmitMessage.message, &su, sizeof(scanUpdate));
      transmitMessage.type = TYPE_SCAN_UPDATE;
      transmitMessage.length = isLastPacket? scanInfo.sizeOfLastPacket + sizeof(scanUpdate):scanInfo.lenghtOfPackets + sizeof(scanUpdate);
      sendPartialMessage(&transmitMessage, 0, false);
    }
    sendPartialMessage(NULL, res.rssi, isLastByte);
    if(isLastByte && !isLastPacket) {
      ++scanInfo.currentPacket;
      scanInfo.currentByte = 0;
    }
    else if(isLastPacket && isLastByte) {
      scanInfo.currentPacket = 0;
      scanInfo.currentByte = 0;
    }
    else {
      ++scanInfo.currentByte;
    }
  }
}

void ConfigComms::handleConfigComms() {
if(isScanning) {
  sendScanUpdate();
}
while (Serial.available()) {
  uint8_t c = Serial.read();
  //Serial.write(c);
  static uint8_t messagePayloadIndex;
  static uint16_t crc;
  switch (currentParseState) {
    case STATE_IDLE:
      if(c == MAGIC) {
        currentParseState = STATE_MAGIC;
        messagePayloadIndex = 0;
        crc_init(&crc);
      //  Serial.print("MAGIC");
      }
      break;
    case STATE_MAGIC:
      if(c > sizeof(configMessages)) {
        currentParseState = STATE_IDLE;
      } else {
        receiveMessage.length = c;
        currentParseState = STATE_LENGTH;
        crc_accumulate(c, &crc);
        //Serial.print("length");
      }
      break;
      case STATE_LENGTH:
        receiveMessage.type = (uint8_t)c;
        currentParseState = STATE_TYPE;
        crc_accumulate(c, &crc);
        if(false) {
          isScanning = false;
          while(true) {
          Serial.print("WTF");
          Serial.write(receiveMessage.length);
          Serial.write(receiveMessage.type);
        }
        }
        break;
      case STATE_TYPE:
      *(receiveMessage.message + messagePayloadIndex) = c;
      crc_accumulate(c, &crc);
      //Serial.print("MESSAGE");
      //Serial.print(crc);
      ++messagePayloadIndex;
      if(messagePayloadIndex == receiveMessage.length) {
        currentParseState = STATE_MESSAGE;
      }
      break;
    case STATE_MESSAGE:
      if(c == (crc & 0x00FF)) {
        currentParseState = STATE_CRC;

    //    Serial.print("MESSAGE FIRST CRC OK");
      }
      else {
        Serial.print("MESSAGE WRONG CRC");
        //Serial.print(crc & 0x00FF);
        //Serial.print(crc >> 8);
        currentParseState = STATE_IDLE;
      }
      break;
    case STATE_CRC:
      if(c == (crc >> 8)) {
  //      Serial.print("MESSAGE OK");
        currentParseState = STATE_IDLE;
        processReceivedMessage(receiveMessage);
      }
      else {
        currentParseState = STATE_IDLE;
        Serial.print("MESSAGE WRONG SECOND CRC");
      //  Serial.print(c);
      }
      break;
  }
}
}
