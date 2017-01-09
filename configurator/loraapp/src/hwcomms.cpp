#include "hwcomms.h"
#include <QDebug>
#include <QApplication>

#define MAX_CON_RETRIES 3

hwComms::hwComms(QObject *parent): QObject(parent), totalPacketsReceived(0), totalErrors(0)
{
    connect(&delayTimer, SIGNAL(timeout()), &delayLoop, SLOT(quit()));
    delayTimer.setSingleShot(true);
    qDebug() << sizeof(startScanning);
    currentParseState = STATE_IDLE;
}

bool hwComms::init(QString portName)
{
    serialPort.setPortName(portName);
    serialPort.setBaudRate(QSerialPort::Baud115200);
    if(!serialPort.open(QIODevice::ReadWrite)) {
        setLastError("Could not open port " + serialPort.portName());
        return false;
    }
    for(int x = 0; x < MAX_CON_RETRIES; ++x) {
        serialPort.setDataTerminalReady(false);
        delayMs(10);
        serialPort.setDataTerminalReady(true);
        QTimer timeOutTimer;
        timeOutTimer.setSingleShot(true);
        timeOutTimer.start(1000);
        bool stop = false;
        QByteArray receiveArray;
        while(timeOutTimer.isActive() && !stop) {
            serialPort.write("loraApp");
            delayMs(1);
            if(serialPort.bytesAvailable()) {
                receiveArray.append(serialPort.readAll());
                if(receiveArray.contains("loraFw"))
                    stop = true;
            }
            QApplication::processEvents();
        }
        delayTimer.stop();
        if(stop) {
            connect(&serialPort, SIGNAL(readyRead()), this, SLOT(serialReadyRead()));
            return true;
        }
    }
    setLastError("Could not start conversation with the firmware");
    serialPort.close();
    return false;
}

void hwComms::close()
{
    disconnect(&serialPort, SIGNAL(readyRead()), this, SLOT(serialReadyRead()));
    serialPort.setDataTerminalReady(false);
    delayMs(10);
    serialPort.setDataTerminalReady(true);
    serialPort.close();
}

void hwComms::delayMs(int value) {
    delayTimer.start(value);
    delayLoop.exec();
}

QString hwComms::getLastError() const
{
    return lastError;
}

void hwComms::setLastError(const QString &value)
{
    lastError = value;
}

void hwComms::stopScanningNow() {
    stopScanning ss;
    txMsg.length = sizeof(stopScanning);
    txMsg.type = TYPE_STOP_SCAN;
    memcpy(txMsg.message, &ss, sizeof(stopScanning));
    calculateCRC(&txMsg);
    sendMessage(&txMsg);
}

void hwComms::startScanningNow(float startFreq, float endFreq, float stepSize, quint8 averaging) {
    startScanning ss;
    ss.startFrequency = startFreq;
    ss.endFrequency = endFreq;
    ss.stepSize = stepSize;
    ss.averagingSample = averaging;
    ss.bytesPerPacket = SCANS_PER_PACKET;
    txMsg.length = sizeof(startScanning);
    txMsg.type = TYPE_START_SCANNING;
    memcpy(txMsg.message, &ss, sizeof(startScanning));
    sendMessage(&txMsg);
}

void hwComms::calculateCRC(configMessage *message) {
    uint16_t crc;
    crc_init(&crc);
    crc_accumulate(message->length, &crc);
    crc_accumulate(message->type, &crc);
    for(int x = 0; x < message->length; ++x) {
        crc_accumulate(*(message->message + x), &crc);
    }
    *((uint16_t*)(message->message + message->length)) = crc;
}

void hwComms::processScanInitResult(scanInitResult result) {
    qDebug()<< "scaninitres" << result.initResult;
    qDebug()<< "scaninit lenght of last packet" << result.lengthOfLastPacket;
    qDebug()<< "scaninit number of packets" << result.numberOfPackets;
    qDebug()<< "scaninit size of packets" << result.sizeOfPackets;
    qDebug()<< "scaninit step size" << result.stepSize;
}
void hwComms::processScanUpdate(scanUpdate result) {
   // qDebug()<< QString("frequency=%0 rssi=%1").arg(result.frequency).arg(result.rssi);
}

void hwComms::processReceivedMessage(configMessage message) {
    switch (message.type) {
    case TYPE_SCAN_INIT_RESULT:
        scanInitResult ir;
        memcpy(&ir, message.message, sizeof(scanInitResult));
        processScanInitResult(ir);
        emit scanInitReceived(ir);
        qDebug() << "Received scan init " << ir.initResult;
        break;
    case TYPE_SCAN_UPDATE:
        scanUpdate su;
        memcpy(&su, message.message, sizeof(scanUpdate));
        processScanUpdate(su);
        emit scanUpdateReceived(su);
      //  qDebug() << "Received scan update " << su.frequency << su.rssi[0];
        break;
    default:
        break;
    }
}

void hwComms::sendMessage(configMessage *message) {
    message->magic = MAGIC;
    calculateCRC(message);
    for(int x = 0; x < message->length + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t); ++ x) {
        qDebug() <<"[" << x << "]=" << *(uint8_t*)(((uint8_t*)(message)) + x);
        delayMs(10);
        serialPort.write((const char*)(((const char*)(message)) + x),1);
    }
    //serialPort.write((const char*)(message), message->length + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t));
}

void hwComms::serialReadyRead()
{
    static uint8_t messagePayloadIndex;
    static uint16_t crc;
    char cc;
    uint8_t c;
    while(serialPort.read(&cc, 1)) {
        c = (uint8_t)(cc);
       // qDebug() << "REC" << QString::number(c) << cc << QString::number(currentParseState);
        switch (currentParseState) {
        case STATE_IDLE:
            if(c == MAGIC) {
                currentParseState = STATE_MAGIC;
                messagePayloadIndex = 0;
                crc_init(&crc);
               // qDebug() << "MAGIC";
            }
            break;
        case STATE_MAGIC:
            if(false) {
                //qDebug() << "LEN OUT OF SPEC " << c;
                currentParseState = STATE_IDLE;
            } else {
                receiveMessage.length = c;
                currentParseState = STATE_LENGTH;
                crc_accumulate(c, &crc);
                //qDebug() << "LEN:" << QString::number(c);
            }
            break;
        case STATE_LENGTH:
            receiveMessage.type = (messagesType)c;
            currentParseState = STATE_TYPE;
            crc_accumulate(c, &crc);
           // qDebug() << "TYPE:" << QString::number(c);
            break;
        case STATE_TYPE:
            *(receiveMessage.message + messagePayloadIndex) = c;
            crc_accumulate(c, &crc);
            ++messagePayloadIndex;
            //qDebug() << "MESSAGE[" << QString::number(messagePayloadIndex - 1) << "]="<<QString::number(c) << " CRC=" << crc;
            if(messagePayloadIndex == receiveMessage.length) {
                currentParseState = STATE_MESSAGE;
              //  qDebug() << "state message";
            }
            break;
        case STATE_MESSAGE:
            if(c == (crc & 0x00FF)) {
                //qDebug() << "first crc OK";
                currentParseState = STATE_CRC;
            }
            else {
               // qDebug() << "first crc NOK";
                currentParseState = STATE_IDLE;
                ++totalErrors;
                emit packetReceived(true, totalPacketsReceived, totalErrors);
            }
            break;
        case STATE_CRC:
            if(c == (crc >> 8)) {
                currentParseState = STATE_IDLE;
                ++totalPacketsReceived;
                emit packetReceived(false, totalPacketsReceived, totalErrors);
                processReceivedMessage(receiveMessage);
             //   qDebug() << "second crc OK";
            }
            else {
                //qDebug() << "second crc NOK";
                ++totalErrors;
                emit packetReceived(true, totalPacketsReceived, totalErrors);
                currentParseState = STATE_IDLE;
            }
            break;
        }
    }
}

void hwComms::onSerialError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::WriteError || error == QSerialPort::ReadError || error == QSerialPort::ResourceError) {
        emit errorDisconnect();
    }
}
