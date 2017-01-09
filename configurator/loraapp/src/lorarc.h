#ifndef LORARC_H
#define LORARC_H

#include <QMainWindow>
#include "hwcomms.h"
#include "spectrumplot.h"
#include <QColorDialog>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>

namespace Ui {
class LoraRC;
}

class LoraRC : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoraRC(QWidget *parent = 0);
    ~LoraRC();

    bool getViewMaxima() const;
    void setViewMaxima(bool value);

    bool getViewMinima() const;
    void setViewMinima(bool value);

    float getMaxMinTreshold() const;
    void setMaxMinTreshold(float value);

private:
    Ui::LoraRC *ui;
    hwComms hw;
    SpectrumPlot *plot;
    struct {
        quint64 numberOfScanPackets;
        int sizeOfPackets;
        int lenghtOfLastPacket;
        quint64 currentPacket;
        float stepSize;
        QVector<double> frequency;
        QVector<double> rssi;
        float initialFrequency;
        float endFrequency;
    }scanInfo;
    bool viewMaxima;
    bool viewMinima;
    bool viewPlot;
    float maxMinTreshold;
    QList<QwtPlotMarker*> markers;
    bool hwSettingsNeedUpdating;
    bool isScanning;
    QComboBox *ports;
    QToolButton *buttonConnect;
    QToolButton *buttonDisconnect;
    bool isConnected;
    QLabel *connectionStatus;
    QTimer *connectionStatusTimer;
    void populateSerialCombo();
private slots:
    void test();
    void on_pushButton_2_clicked();
    void onScanInitReceived(scanInitResult);
    void onScanUpdateReceived(scanUpdate);
    void on_showLocalMax_toggled(bool checked);
    void on_showLocalMin_toggled(bool checked);
    void removeAllMarkers();
    void on_minMaxTreshold_valueChanged(double arg1);
    void on_initialFreq_valueChanged(double arg1);
    void on_finalFreq_valueChanged(double arg1);
    void on_freqStepKHz_valueChanged(int arg1);
    void on_averegingSamples_valueChanged(int arg1);
    void on_tabWidget_currentChanged(int index);
    void on_checkBox_toggled(bool checked);
    void on_frequencySelectionChanged(QRectF selection);
    void on_copyPlot_clicked();
    void on_newPlotAdded(int plotNumber);
    void handleChangePlotColor();
    void handleChangePlotVisibility();
    void handleChangePlotName();
    void on_copyPlotAndMarkers_clicked();
    void on_actionSettings_toggled(bool arg1);
    void on_pushButtonConnect();
    void on_pushButtonDisconnect();
    void on_packetReceived(bool error, int totalPacketsReceived, int totalErrors);
    void handleConnectionStatusTimeout();
    void on_errorDisconnect();
signals:
    void plotColorChanged(int plotNumber, QColor color);
    void plotVisibilityChanged(int plotNumber, bool visible);
    void plotNameChanged(int plotNumber, QString name);
protected:
    void timerEvent(QTimerEvent *event);
};

#endif // LORARC_H
