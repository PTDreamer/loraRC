#include "scanner.h"

Scanner::Scanner(RH_RF22JB *radio) {
  _radio = radio;
}
uint8_t Scanner::initScan(float initialFrequency, float finalFrequency, float stepSize, uint8_t averageRounds) {
  uint8_t res = 0;
  _initialFrequency = initialFrequency;
  _finalFrequency = finalFrequency;
  _stepSize = stepSize;
  _averageRounds = averageRounds;
  currentFrequency = _initialFrequency;
  currentAverageRound = 0;
  if(!_radio->init()) {
    res |= FAILED_INIT;
  }
  else {
    RH_RF22JB::ModemConfig mc;
    _radio->getModemConfig(RH_RF22JB::GFSK_Rb125Fd125, &mc);
    if (stepSize < 0.02f) {
      mc.reg_1c = 0x32;   // 10.6kHz
    } else if (stepSize < 0.03f) {
      mc.reg_1c = 0x22;   // 21.0kHz
    } else if (stepSize < 0.04f) {
      mc.reg_1c = 0x26;   // 32.2kHz
    } else if (stepSize < 0.05f) {

    } else if (stepSize < 0.06f) {
      mc.reg_1c = 0x15;   // 56.2kHz
    } else if (stepSize < 0.07f) {
      mc.reg_1c = 0x01;   // 75.2kHz
    } else if (stepSize < 0.07f) {
      mc.reg_1c = 0x03;   // 90.0kHz
    } else {
      mc.reg_1c = 0x05;   // 112.1kHz
    }
    _radio->setModemRegisters(&mc);
    _radio->setModeRx();
    if(!_radio->setFrequency(_initialFrequency) || !_radio->setFrequency(_finalFrequency)) {
      res |= FAILED_FREQUENCY_SET;
    }
    else {
      _radio->setFrequency(_initialFrequency);
    }
  }
  return res;
}
scanResult Scanner::continueScan() {
  static uint32_t accumulator;
  scanResult res;
  if(currentAverageRound == 0) {
    accumulator = 0;
  }
  accumulator += _radio->rssiRead();
  ++currentAverageRound;
  if(currentAverageRound > _averageRounds) {
    res.rssi = accumulator / _averageRounds;
    res.frequency = currentFrequency;
    currentAverageRound = 0;
    currentFrequency += _stepSize;
    if(currentFrequency >= _finalFrequency) {
      currentFrequency = _initialFrequency;
    }
    _radio->setFrequency(currentFrequency);
  } else {
    res.rssi = 0;
    res.frequency = 0;
  }
  return res;
}
