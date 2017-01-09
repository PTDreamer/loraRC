#include "lorarc.h"
#include "ui_lorarc.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include "hwcomms.h"
#include <QApplication>
#include <QMessageBox>
#include "persistence1d.hpp"
#include "qwt_symbol.h"
#include <QLineEdit>

using namespace p1d;

LoraRC::LoraRC(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoraRC),
    hw(this), maxMinTreshold(3), viewMaxima(true), viewMinima(false), hwSettingsNeedUpdating(true), isScanning(false),
    viewPlot(true), isConnected(false)
{
    ui->setupUi(this);
    QLabel *portLabel = new QLabel("Serial Ports:", this);
    ui->mainToolBar->addWidget(portLabel);
    ports = new QComboBox(this);
    ports->setMinimumSize(QSize(100, 0));
    ui->mainToolBar->addWidget(ports);
    populateSerialCombo();
    buttonConnect = new QToolButton(this);
    buttonConnect->setToolTip(QApplication::translate("LoraRC", "Connect", 0));
    QIcon iconConnect;
    iconConnect.addFile(QStringLiteral(":/icons/resources/plug-connect.png"), QSize(), QIcon::Normal, QIcon::Off);
    buttonConnect->setIcon(iconConnect);
    ui->mainToolBar->addWidget(buttonConnect);

    buttonDisconnect = new QToolButton(this);
    buttonConnect->setToolTip(QApplication::translate("LoraRC", "Disconnect", 0));
    QIcon iconDisconnect;
    iconDisconnect.addFile(QStringLiteral(":/icons/resources/plug-disconnect.png"), QSize(), QIcon::Normal, QIcon::Off);
    buttonDisconnect->setIcon(iconDisconnect);
    ui->mainToolBar->addWidget(buttonDisconnect);
    buttonDisconnect->setEnabled(false);

    connectionStatus = new QLabel(this);
    connectionStatus->setPixmap(QPixmap(":/icons/resources/compile-warning.png"));
    connectionStatus->setVisible(true);
    ui->mainToolBar->addWidget(connectionStatus);

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(test()));
    plot = new SpectrumPlot(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(plot);
    ui->groupBox->setLayout(layout);
    connect(&hw, SIGNAL(scanInitReceived(scanInitResult)), this, SLOT(onScanInitReceived(scanInitResult)));
    connect(&hw, SIGNAL(scanUpdateReceived(scanUpdate)), this, SLOT(onScanUpdateReceived(scanUpdate)));
    connect(plot, SIGNAL(selectionChanged(QRectF)), this, SLOT(on_frequencySelectionChanged(QRectF)));
    connect(plot, SIGNAL(newPlotAdded(int)), this, SLOT(on_newPlotAdded(int)));
    connect(this, SIGNAL(plotColorChanged(int,QColor)), plot, SLOT(on_plotColorChanged(int,QColor)));
    connect(this, SIGNAL(plotVisibilityChanged(int,bool)), plot, SLOT(on_plotVisibilityChanged(int,bool)));
    connect(this, SIGNAL(plotNameChanged(int,QString)), plot, SLOT(on_plotNameChanged(int,QString)));
    connect(buttonConnect, SIGNAL(clicked(bool)), this, SLOT(on_pushButtonConnect()));
    connect(buttonDisconnect, SIGNAL(clicked(bool)), this, SLOT(on_pushButtonDisconnect()));
    on_newPlotAdded(0);
    startTimer(1000);
    connectionStatusTimer = new QTimer(this);
    connectionStatusTimer->setSingleShot(true);
    connect(&hw, SIGNAL(packetReceived(bool,int,int)), this, SLOT(on_packetReceived(bool,int,int)));
    connect(connectionStatusTimer, SIGNAL(timeout()), this, SLOT(handleConnectionStatusTimeout()));
    connect(&hw, SIGNAL(errorDisconnect()), this, SLOT(on_errorDisconnect()));
}

LoraRC::~LoraRC()
{
    delete ui;
}

bool LoraRC::getViewMaxima() const
{
    return viewMaxima;
}

void LoraRC::setViewMaxima(bool value)
{
    viewMaxima = value;
}

bool LoraRC::getViewMinima() const
{
    return viewMinima;
}

void LoraRC::setViewMinima(bool value)
{
    viewMinima = value;
}

float LoraRC::getMaxMinTreshold() const
{
    return maxMinTreshold;
}

void LoraRC::setMaxMinTreshold(float value)
{
    maxMinTreshold = value;
}

void LoraRC::populateSerialCombo()
{
    ports->clear();
    foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
        ui->portList->addItem(info.portName());
        ports->addItem(info.portName());
    }
    for(int x = 0; x < ports->count(); ++x) {
        if(ports->itemText(x).contains("USB", Qt::CaseInsensitive)) {
            ports->setCurrentIndex(x);
            break;
        }
    }
}

void LoraRC::test()
{
    ui->textBrowser->append(QString::number(hw.init(ui->portList->currentText())));
}

void LoraRC::on_pushButton_2_clicked()
{
    hw.startScanningNow(413,453, 0.1f,5);
    plot->SetFrequencyScale(413, 453);
}

void LoraRC::onScanInitReceived(scanInitResult res)
{
    if(res.initResult & 1)
        QMessageBox::critical(this, "ERROR", "There was a problem with the module initialization, could not start the scan");
    else if(res.initResult & 2)
        QMessageBox::critical(this, "ERROR", "There was a problem setting the scan frequency, could not start the scan");
    else {
        scanInfo.numberOfScanPackets = res.numberOfPackets;
        scanInfo.sizeOfPackets = res.sizeOfPackets;
        scanInfo.lenghtOfLastPacket = res.lengthOfLastPacket;
        scanInfo.stepSize = res.stepSize;
        scanInfo.currentPacket = 0;
        scanInfo.frequency.reserve((res.numberOfPackets - 1) * res.sizeOfPackets + res.lengthOfLastPacket);
        scanInfo.rssi.reserve((res.numberOfPackets - 1) * res.sizeOfPackets + res.lengthOfLastPacket);
        scanInfo.rssi.fill(-74, (res.numberOfPackets - 1) * res.sizeOfPackets + res.lengthOfLastPacket);
        for(float x = scanInfo.initialFrequency; x < scanInfo.endFrequency; x = x + scanInfo.stepSize) {
            scanInfo.frequency.append(x);
        }
    }
}

void LoraRC::onScanUpdateReceived(scanUpdate res)
{
    bool isLastPacket = (scanInfo.currentPacket == (scanInfo.numberOfScanPackets - 1));
    for(quint64 x = 0; x < (isLastPacket ? scanInfo.lenghtOfLastPacket : scanInfo.sizeOfPackets); ++x) {
        if(scanInfo.currentPacket * scanInfo.sizeOfPackets + x < scanInfo.frequency.length())
            scanInfo.frequency[scanInfo.currentPacket * scanInfo.sizeOfPackets + x] =(res.frequency + x * scanInfo.stepSize);
        float temp = (-120.0f + (((double)(res.rssi[x]))/2.0f));
        if(ui->displayType->currentIndex() == 1) {
            if(temp < scanInfo.rssi[scanInfo.currentPacket * scanInfo.sizeOfPackets + x]) {
                temp = ((temp * (float)(2 - ui->persistenceStrength->value() * 2)) + (scanInfo.rssi[scanInfo.currentPacket * scanInfo.sizeOfPackets + x] * ui->persistenceStrength->value() * 2)) / 2;
            }
        }
        if(scanInfo.currentPacket * scanInfo.sizeOfPackets + x < scanInfo.rssi.length())
            scanInfo.rssi[scanInfo.currentPacket * scanInfo.sizeOfPackets + x] = temp;
    }
    if((viewMaxima || viewMinima) && viewPlot) {
        Persistence1D p;
        std::vector<double>rssi(scanInfo.rssi.toStdVector());
        p.RunPersistence(rssi);
        std::vector< TPairedExtrema > Extrema;
        p.GetPairedExtrema(Extrema, maxMinTreshold);
        foreach (QwtPlotMarker *marker, markers) {
            marker->detach();
            delete marker;
        }
        markers.clear();
        for(std::vector< TPairedExtrema >::iterator it = Extrema.begin(); it != Extrema.end(); it++)
        {
            if(viewMaxima) {
                QwtSymbol *sym=new QwtSymbol(QwtSymbol::Ellipse,QBrush(Qt::yellow),QPen(Qt::yellow),QSize(5,5));
                QwtPlotMarker *mark=new QwtPlotMarker;
                markers.append(mark);
                mark->setSymbol(sym);
                QwtText text(QString("<p><strong>%0dBm</strong><br>%1MHz</p>").arg(scanInfo.rssi[(*it).MaxIndex]).arg(scanInfo.frequency[(*it).MaxIndex]));
                text.setColor(Qt::yellow);
                mark->setLabel(text);
                mark->setLabelAlignment(Qt::AlignBottom | Qt::AlignRight);
                mark->setValue(scanInfo.frequency[(*it).MaxIndex], scanInfo.rssi[(*it).MaxIndex]);
                mark->attach(plot);
            }
            if(viewMinima) {
                QwtSymbol *sym = new QwtSymbol(QwtSymbol::Ellipse,QBrush(Qt::red),QPen(Qt::red),QSize(5,5));
                QwtPlotMarker *mark = new QwtPlotMarker;
                markers.append(mark);
                mark->setSymbol(sym);
                QwtText text(QString("<p><strong>%0dBm</strong><br>%1MHz</p>").arg(scanInfo.rssi[(*it).MaxIndex]).arg(scanInfo.frequency[(*it).MaxIndex]));
                text.setColor(Qt::red);
                mark->setLabel(text);
                mark->setLabelAlignment(Qt::AlignBottom | Qt::AlignRight);
                mark->setValue(scanInfo.frequency[(*it).MinIndex], scanInfo.rssi[(*it).MinIndex]);
                mark->attach(plot);
            }
        }
    }
    plot->SetData(scanInfo.frequency, scanInfo.rssi);
    if(isLastPacket) {
        scanInfo.currentPacket = 0;

    }
    else {
        ++scanInfo.currentPacket;
    }
}


void LoraRC::on_showLocalMax_toggled(bool checked)
{
    viewMaxima = checked;
    removeAllMarkers();
}

void LoraRC::on_showLocalMin_toggled(bool checked)
{
    viewMinima = checked;
    removeAllMarkers();
}

void LoraRC::removeAllMarkers()
{
    foreach (QwtPlotMarker *marker, markers) {
        marker->detach();
        delete marker;
    }
    markers.clear();
}

void LoraRC::on_minMaxTreshold_valueChanged(double arg1)
{
    maxMinTreshold = arg1;
    removeAllMarkers();
}

void LoraRC::on_initialFreq_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    hwSettingsNeedUpdating = true;
}

void LoraRC::on_finalFreq_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    hwSettingsNeedUpdating = true;
}

void LoraRC::on_freqStepKHz_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    hwSettingsNeedUpdating = true;
}

void LoraRC::on_averegingSamples_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    hwSettingsNeedUpdating = true;
}

void LoraRC::timerEvent(QTimerEvent *event)
{
    bool found = false;
    if(isConnected) {
        foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
            if(info.portName() == ports->currentText()) {
                found = true;
                break;
            }
        }
        if(!found) {
            on_errorDisconnect();
        }
    }
    if(!isConnected) {
       populateSerialCombo();
    }

    Q_UNUSED(event)
    if(hwSettingsNeedUpdating && isScanning && isConnected) {
        scanInfo.initialFrequency = ui->initialFreq->value();
        scanInfo.endFrequency = ui->finalFreq->value();
        hw.startScanningNow(ui->initialFreq->value(), ui->finalFreq->value(), (float)(ui->freqStepKHz->value()) / 1000.0f, ui->averegingSamples->value());
        qDebug() << QString("Requested init freq %0 final %1 step %2").arg(ui->initialFreq->value()).arg(ui->finalFreq->value()).arg(ui->freqStepKHz->value() / 1000.0f);
        plot->SetFrequencyScale(ui->initialFreq->value(), ui->finalFreq->value());
        hwSettingsNeedUpdating = false;
    }
}

void LoraRC::on_tabWidget_currentChanged(int index)
{
    if(index == 1) {
        hwSettingsNeedUpdating = true;
        isScanning = true;
    } else {
        hw.stopScanningNow();
        isScanning = false;
    }
}

void LoraRC::on_checkBox_toggled(bool checked)
{
    if(checked) {
        plot->setAxisAutoScale(QwtPlot::yLeft);
    }

    else {
        plot->setAxisScale( QwtPlot::yLeft, -120, 20);
    }
}

void LoraRC::on_frequencySelectionChanged(QRectF selection)
{
    if(selection.width() / (ui->freqStepKHz->value() / 1000.0f) > 2) {
        ui->initialFreq->setValue(selection.left());
        ui->finalFreq->setValue(selection.right());
    }
}

void LoraRC::on_copyPlot_clicked()
{
    plot->copyPlot();
}

void LoraRC::on_newPlotAdded(int plotNumber)
{
    static QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->gridLayout->removeItem(spacer);
    QString plotTitle;
    QColor plotColor;
    int y = 1 + 2 * plotNumber;
    if(plotNumber == 0) {
        plotTitle = QString("Current");
        plotColor = Qt::green;
        QLabel *le = new QLabel(ui->scrollAreaWidgetContents_2);
        le->setText(plotTitle);
        ui->gridLayout->addWidget(le, y, 0, 1, 1);
        le->setProperty("plotnumber", plotNumber);
    } else {
        plotTitle = QString("New plot %0").arg(plotNumber);
        plotColor = Qt::gray;
        QLineEdit *le = new QLineEdit(ui->scrollAreaWidgetContents_2);
        le->setText(plotTitle);
        ui->gridLayout->addWidget(le, y, 0, 1, 1);
        connect(le, SIGNAL(textChanged(QString)), this, SLOT(handleChangePlotName()));
        le->setProperty("plotnumber", plotNumber);
    }

    QPushButton *pb = new QPushButton(ui->scrollAreaWidgetContents_2);
    ui->gridLayout->addWidget(pb, y, 1, 1, 1);
    QPalette pal = pb->palette();
    pal.setColor(QPalette::Button, plotColor);
    pb->setAutoFillBackground(true);
    pb->setPalette(pal);
    pb->setMaximumSize(QSize(50, 20));
    pb->update();
    QCheckBox *cb = new QCheckBox(ui->scrollAreaWidgetContents_2);
    cb->setChecked(true);
    ui->gridLayout->addWidget(cb, y, 2, 1, 1);
    cb->setProperty("plotnumber", plotNumber);
    pb->setProperty("plotnumber", plotNumber);
    connect(pb, SIGNAL(clicked(bool)), this, SLOT(handleChangePlotColor()));
    connect(cb, SIGNAL(toggled(bool)), this, SLOT(handleChangePlotVisibility()));
    emit plotColorChanged(plotNumber, plotColor);
    emit plotNameChanged(plotNumber, plotTitle);
    if(plotNumber) {
        QFrame *line = new QFrame(ui->scrollAreaWidgetContents_2);
        line->setFrameShape((QFrame::HLine));
        line->setFrameShadow(QFrame::Sunken);
        ui->gridLayout->addWidget(line, y - 1, 0, 1, 3);
    }
    ui->gridLayout->addItem(spacer, y + 1, 1);
}

void LoraRC::handleChangePlotColor()
{
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    QPalette pal = b->palette();
    QColor currentColor = pal.color(QPalette::Button);
    QColor newColor = QColorDialog::getColor(currentColor, this);
    pal.setColor(QPalette::Button, newColor);
    b->setPalette(pal);
    emit plotColorChanged(b->property("plotnumber").toInt(), newColor);
}

void LoraRC::handleChangePlotVisibility()
{
    QCheckBox *c = qobject_cast<QCheckBox *>(sender());
    if(c->property("plotnumber").toInt() == 0) {
        viewPlot = c->isChecked();
        if(!viewPlot) {
            removeAllMarkers();
        }
    }
    emit plotVisibilityChanged(c->property("plotnumber").toInt(), c->isChecked());
}

void LoraRC::handleChangePlotName()
{
    QLineEdit *e = qobject_cast<QLineEdit *>(sender());
    emit plotNameChanged(e->property("plotnumber").toInt(), e->text());
}

void LoraRC::on_copyPlotAndMarkers_clicked()
{
    plot->copyPlot(markers);
}

void LoraRC::on_actionSettings_toggled(bool arg1)
{
    ui->settingsGroupBox->setVisible(arg1);
}

void LoraRC::on_pushButtonConnect()
{
    buttonConnect->setEnabled(false);
    bool success = hw.init(ports->currentText());
    if(success) {
        isConnected = true;
        buttonConnect->setEnabled(false);
        buttonDisconnect->setEnabled(true);
        connectionStatus->setVisible(true);
        connectionStatus->setPixmap(QPixmap(":/icons/resources/compile-warning.png"));
        hwSettingsNeedUpdating = true;
        ui->statusBar->showMessage("Connected", 2000);
    }
    else {
        buttonConnect->setEnabled(true);
        ui->statusBar->showMessage(hw.getLastError(), 2000);
    }
}

void LoraRC::on_pushButtonDisconnect()
{
    connectionStatus->setVisible(false);
    hw.close();
    isConnected = false;
    buttonConnect->setEnabled(true);
    buttonDisconnect->setEnabled(false);
    ui->statusBar->showMessage("Disconnected", 2000);
}

void LoraRC::on_packetReceived(bool error, int totalPacketsReceived, int totalErrors)
{
    if(error) {
        connectionStatus->setPixmap(QPixmap(":/icons/resources/compile-error.png"));
        connectionStatus->update();
        connectionStatusTimer->start(100);
    }
    else {
        connectionStatus->setPixmap(QPixmap(":/icons/resources/compile.png"));
        connectionStatus->update();
        connectionStatusTimer->start(2000);
    }
    connectionStatus->setToolTip(QString("Total Received Packets:%0, total errors %1").arg(totalPacketsReceived).arg(totalErrors));

}

void LoraRC::handleConnectionStatusTimeout()
{
    connectionStatus->setPixmap(QPixmap(":/icons/resources/compile-warning.png"));
    connectionStatus->update();
}

void LoraRC::on_errorDisconnect()
{
    ui->statusBar->showMessage("Serial error, disconnecting!", 5000);
    on_pushButtonDisconnect();
}
