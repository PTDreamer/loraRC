#include "spectrumplot.h"
#include <qwt_plot_legenditem.h>
#include <qevent.h>
#include <qwt_symbol.h>
#include <qwt_curve_fitter.h>

class LegendItem: public QwtPlotLegendItem
{
public:
    LegendItem()
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );
        QColor color( Qt::white );
        setTextPen( color );
        setBorderPen( color );
        QColor c( Qt::gray );
        c.setAlpha( 200 );
        setBackgroundBrush( c );
    }
};

class Canvas: public QwtPlotCanvas
{
public:
    Canvas( QwtPlot *plot = NULL ):
        QwtPlotCanvas( plot )
    {
        // The backing store is important, when working with widget
        // overlays ( f.e rubberbands for zooming ).
        // Here we don't have them and the internal
        // backing store of QWidget is good enough.

        setPaintAttribute( QwtPlotCanvas::BackingStore, false );
        setBorderRadius( 10 );

        if ( QwtPainter::isX11GraphicsSystem() )
        {
#if QT_VERSION < 0x050000
            // Even if not liked by the Qt development, Qt::WA_PaintOutsidePaintEvent
            // works on X11. This has a nice effect on the performance.

            setAttribute( Qt::WA_PaintOutsidePaintEvent, true );
#endif

            // Disabling the backing store of Qt improves the performance
            // for the direct painter even more, but the canvas becomes
            // a native window of the window system, receiving paint events
            // for resize and expose operations. Those might be expensive
            // when there are many points and the backing store of
            // the canvas is disabled. So in this application
            // we better don't disable both backing stores.

            if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) )
            {
                setAttribute( Qt::WA_PaintOnScreen, true );
                setAttribute( Qt::WA_NoSystemBackground, true );
            }
        }

        setupPalette();
    }

private:
    void setupPalette()
    {
        QPalette pal = palette();

#if QT_VERSION >= 0x040400
        QLinearGradient gradient;
        gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
        gradient.setColorAt( 0.0, QColor( 0, 49, 110 ) );
        gradient.setColorAt( 1.0, QColor( 0, 87, 174 ) );

        pal.setBrush( QPalette::Window, QBrush( gradient ) );
#else
        pal.setBrush( QPalette::Window, QBrush( color ) );
#endif

        // QPalette::WindowText is used for the curve color
        pal.setColor( QPalette::WindowText, Qt::green );

        setPalette( pal );
    }
};

SpectrumPlot::SpectrumPlot(QWidget *parent) :
    QwtPlot(parent)
{
    setAutoReplot( false );
    setCanvas( new Canvas() );

    //plotLayout()->setAlignCanvasToScales( true );

    setTitle("Spectrum Analyzer");
    setAxisTitle( QwtPlot::xBottom, "Frequency (MHz)" );
    setAxisTitle( QwtPlot::yLeft, "Amplitude (dB)" );
    setAxisScale( QwtPlot::yLeft, -120, 20);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach( this );

    d_legendItem = new LegendItem();
    d_legendItem->attach( this );
    d_legendItem->setMaxColumns( 1 );
    d_legendItem->setAlignment( Qt::AlignRight | Qt::AlignTop);
    d_legendItem->setBackgroundMode(QwtPlotLegendItem::LegendBackground);
    d_legendItem->setBorderRadius( 8 );
    d_legendItem->setMargin( 4 );
    d_legendItem->setSpacing( 2 );
    d_legendItem->setItemMargin( 0 );
    QFont font = d_legendItem->font();
    font.setPointSize( 8 );
    d_legendItem->setFont( font );

    d_curve = new QwtPlotCurve();
    d_curve->setTitle("Current");
    compositePlot plot;
    plot.curve = d_curve;
    curveList.append(plot);
    d_curve->setStyle( QwtPlotCurve::Lines );
    d_curve->setPen( canvas()->palette().color( QPalette::WindowText ) );
    d_curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    d_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
    d_curve->attach( this );
    d_curve->setStyle(QwtPlotCurve::Lines);
    d_curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
    QwtSplineCurveFitter *fitter = new QwtSplineCurveFitter();
    fitter->setFitMode(QwtSplineCurveFitter::Auto);
    fitter->setSplineSize(500);
    d_curve->setCurveFitter(fitter);

    d_curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    setAxisScale( QwtPlot::xBottom, 413.0, 453.0 );
    setAxisAutoScale(QwtPlot::xBottom, true);
    clickPicker = new HorizontalPicker(canvas());
    clickPicker->setTrackerMode( QwtPicker::ActiveOnly );
    clickPicker->setStateMachine( new QwtPickerDragRectMachine() );
    clickPicker->setRubberBand( QwtPlotPicker::VLineRubberBand );

    plotTracePicker = new PlotTracePicker(canvas(), d_curve);
    plotTracePicker->setTrackerMode( QwtPicker::AlwaysOn );
    plotTracePicker->setStateMachine( new QwtPickerTrackerMachine() );
    connect(plotTracePicker, SIGNAL(doubleClick()), this, SLOT(on_doubleClick()));
    connect(clickPicker, SIGNAL(selected(QRectF)), this, SIGNAL(selectionChanged(QRectF)));
    connect(clickPicker, SIGNAL(activated(bool)), this, SLOT(disablePlotTracer(bool)));
    QwtPlotMagnifier *mag = new QwtPlotMagnifier(canvas());
    mag->setAxisEnabled(QwtPlot::yLeft, true);
    mag->setAxisEnabled(QwtPlot::xBottom, false);
    mag->setMouseButton(Qt::NoButton);
    mag->setEnabled(true);
    QwtPlotPanner *pan = new QwtPlotPanner(canvas());
    pan->setAxisEnabled(QwtPlot::yLeft, true);
    pan->setAxisEnabled(QwtPlot::xBottom, false);
    pan->setMouseButton(Qt::LeftButton, Qt::ControlModifier);
}



void SpectrumPlot::SetFrequencyScale(double start, double end) {
    setAxisScale(QwtPlot::xBottom, start, end);
}

QwtPlotCurve *SpectrumPlot::getD_curve() const
{
    return d_curve;
}

void SpectrumPlot::setD_curve(QwtPlotCurve *value)
{
    d_curve = value;
}

void SpectrumPlot::copyPlot(QList<QwtPlotMarker*> markers)
{
    if(!d_curve->data()->size())
        return;
    QwtPlotCurve *newCurve = new QwtPlotCurve();
    newCurve->setSamples((((QwtPointArrayData*)(d_curve->data()))->xData()), (((QwtPointArrayData*)(d_curve->data()))->yData()));
    newCurve->attach(this);
    compositePlot plot;
    plot.curve = newCurve;
    if(!markers.isEmpty()) {
        foreach (QwtPlotMarker *marker, markers) {
            QwtSymbol *sym = new QwtSymbol(marker->symbol()->style(), marker->symbol()->brush(), marker->symbol()->pen(), marker->symbol()->size());
            QwtPlotMarker *mark = new QwtPlotMarker;
            plot.markers.append(mark);
            mark->setSymbol(sym);
            QwtText text(marker->label());
            text.setColor(marker->label().color());
            mark->setLabel(text);
            mark->setLabelAlignment(Qt::AlignBottom | Qt::AlignRight);
            mark->setValue(marker->value());
            mark->attach(this);
        }
    }
    curveList.append(plot);
    emit newPlotAdded(curveList.length() -1);

}

void SpectrumPlot::on_plotColorChanged(int plotNumber, QColor color)
{
    curveList.at(plotNumber).curve->setPen(color);
}

void SpectrumPlot::on_plotVisibilityChanged(int plotNumber, bool visible)
{
    if(visible) {
        curveList.at(plotNumber).curve->attach(this);
        foreach (QwtPlotMarker *marker, curveList.at(plotNumber).markers) {
            marker->attach(this);
        }
    } else {
        curveList.at(plotNumber).curve->detach();
        foreach (QwtPlotMarker *marker, curveList.at(plotNumber).markers) {
            marker->detach();
        }
    }
}

void SpectrumPlot::on_plotNameChanged(int plotNumber, QString name)
{
    curveList.at(plotNumber).curve->setTitle(name);
}

void SpectrumPlot::disablePlotTracer(bool active)
{
    plotTracePicker->setEnabled(!active);
}

void SpectrumPlot::on_doubleClick()
{
    this->setAxisScale( QwtPlot::yLeft, -120, 20);
    QRectF rect;
    rect.setLeft(413);
    rect.setRight(453);
    emit selectionChanged(rect);
    replot();
}

void SpectrumPlot::SetData(QVector<double> x, QVector<double> y)
{
    d_curve->setSamples(x, y);
    replot();
    plotTracePicker->update();
}
void HorizontalPicker::drawRubberBand(QPainter *p) const
{
    const QRect pRect = pickArea().boundingRect().toRect();
    QPolygon poly = selection();
    QRect r = QRect(poly.point(0).x(), 2, poly.point(1).x() - poly.point(0).x(), pRect.height() - 2);
    p->fillRect(r, QBrush(QColor(211, 211, 211, 128)));
    p->drawLine(poly.point(0).x(), 2, poly.point(0).x(), pRect.height() - 2);
    p->drawLine(poly.point(1).x(), 2, poly.point(1).x(), pRect.height() - 2);
}

void PlotTracePicker::drawRubberBand(QPainter *p) const
{
    p->setPen(Qt::yellow);
    const QRect pRect = pickArea().boundingRect().toRect();
    double distance = std::numeric_limits<double>::max();;
    size_t closestIndex = 0;
    for(size_t x = 0; x< activeCurve->data()->size(); ++x) {
        if(fabs(invTransform(trackerPosition()).x() - activeCurve->data()->sample(x).x()) < distance)
            distance = fabs(invTransform(trackerPosition()).x() - activeCurve->data()->sample(x).x());
        else {
            closestIndex = x - 1;
            break;
        }
    }
    QPointF closestTruePoint = activeCurve->data()->sample(closestIndex);
    QPoint closestPlotPoint = transform(closestTruePoint);

    p->drawLine(closestPlotPoint.x(), 2, closestPlotPoint.x(), pRect.height() - 2);
    p->drawLine(2, closestPlotPoint.y(), pRect.width() - 2, closestPlotPoint.y());
    p->drawText(closestPlotPoint.x() + 10, trackerPosition().y() - p->fontMetrics().height() / 2, QString("%0dBm").arg(closestTruePoint.y()));
    p->drawText(closestPlotPoint.x() + 10, trackerPosition().y() - 1.5 * p->fontMetrics().height(), QString("%0MHz").arg(closestTruePoint.x()));
}

void PlotTracePicker::drawTracker(QPainter *p) const
{
    Q_UNUSED(p)
}

void PlotTracePicker::update()
{
    updateDisplay();
}

void PlotTracePicker::widgetMouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "DOUBLE CLICK";
    QwtPlotPicker::widgetMouseDoubleClickEvent(event);
    emit doubleClick();
}
