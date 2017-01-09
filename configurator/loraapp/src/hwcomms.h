#ifndef HWCOMMS_H
#define HWCOMMS_H

#include <QtSerialPort/QtSerialPort>
#include <QEventLoop>
#include <QTimer>
#include <QObject>
#include <QVector>
#define SCANS_PER_PACKET 100
#include "configMessages.h"
#include "checksum.h"

class hwComms : public QObject
{
    Q_OBJECT
public:
    hwComms(QObject *parent);
    bool init(QString portName);
    void close();
    QString getLastError() const;
    void setLastError(const QString &value);
    void stopScanningNow();
    void startScanningNow(float startFreq, float endFreq, float stepSize, quint8 averaging);
private:
    QSerialPort serialPort;
    QEventLoop delayLoop;
    QTimer delayTimer;
    void delayMs(int value);
    QString lastError;
    void calculateCRC(configMessage *message);
    void sendMessage(configMessage *message);
    configMessage txMsg;
    configMessage receiveMessage;
    parseState currentParseState;
    void processScanInitResult(scanInitResult result);
    void processReceivedMessage(configMessage message);
    void processScanUpdate(scanUpdate result);
    int totalPacketsReceived;
    int totalErrors;
private slots:
    void serialReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
signals:
    void scanInitReceived(scanInitResult res);
    void scanUpdateReceived(scanUpdate res);
    void packetReceived(bool error, int totalPacketsReceived, int totalErrors);
    void errorDisconnect();
};

#endif // HWCOMMS_H
