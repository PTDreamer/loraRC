
#ifndef SPECTRUMPLOT_H
#define SPECTRUMPLOT_H

#include <stdint.h>

#include <qwt_plot.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_point_data.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

#define RESULT_LENGTH 1000

class LegendItem;

class HorizontalPicker : public QwtPlotPicker
{

public:
    HorizontalPicker(QWidget *canvas): QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::VLineRubberBand,
                                                                          QwtPicker::ActiveOnly, canvas) {}
    void drawRubberBand(QPainter *p) const;
};
class PlotTracePicker : public QwtPlotPicker
{
    Q_OBJECT
public:
    PlotTracePicker(QWidget *canvas, QwtPlotCurve *curve): QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::PolygonRubberBand,
                                                    QwtPicker::ActiveOnly, canvas), activeCurve(curve) {}
    void drawRubberBand(QPainter *p) const;
    void drawTracker(QPainter *p) const;
    void update();
    void widgetMouseDoubleClickEvent(QMouseEvent *);
private:
    QwtPlotCurve *activeCurve;
signals:
    void doubleClick();
};
class SpectrumPlot : public QwtPlot
{
    Q_OBJECT
public:
    struct compositePlot {
        QwtPlotCurve *curve;
        QList<QwtPlotMarker*> markers;
    };

    explicit SpectrumPlot(QWidget *parent = 0);
    void SetData(QVector<double> x, QVector<double> y);
    void SetFrequencyScale(double start, double end);
    QwtPlotCurve *getD_curve() const;
    void setD_curve(QwtPlotCurve *value);

signals:
    void selectionChanged(QRectF points);
    void newPlotAdded(int plotNumber);
private slots:
public slots:
    void copyPlot(QList<QwtPlotMarker *> markers = QList<QwtPlotMarker *>());
    void on_plotColorChanged(int plotNumber, QColor color);
    void on_plotVisibilityChanged(int plotNumber, bool visible);
    void on_plotNameChanged(int plotNumber, QString name);
    void disablePlotTracer(bool);
    void on_doubleClick();
private:
    QwtPlotCurve *d_curve;
    void setupPalette();
    QList<compositePlot> curveList;
    PlotTracePicker* plotTracePicker;
    QwtPlotPicker* clickPicker;
    LegendItem *d_legendItem;
};

#endif // SPECTRUMPLOT_H
