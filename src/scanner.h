#ifndef scanner_h
#define scanner_h

#include "RH_RF22JB.h"

enum freqScanInitResultCodes {FAILED_INIT = 1, FAILED_FREQUENCY_SET = 2};
typedef struct _scanResult {
  float frequency;
  int rssi;
}scanResult;

class Scanner {
public:
  Scanner(RH_RF22JB *radio);
  uint8_t initScan(float initialFrequency, float finalFrequency, float stepSize, uint8_t averageRounds);

  scanResult continueScan();
private:
  float _initialFrequency;
  float _finalFrequency;
  float _stepSize;
  uint8_t _averageRounds;
  uint8_t currentAverageRound;
  float currentFrequency;
  RH_RF22JB *_radio;
};

#endif
