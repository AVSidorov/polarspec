/*****************************************************************************
 * Qwt Polar Examples - Copyright (C) 2008   Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "Plot.h"

#include <cmath>
#include <QwtRasterData>
#include <QwtPolarPanner>
#include <QwtPolarMagnifier>
#include <QwtPolarGrid>
#include <QwtPolarSpectrogram>
#include <QwtPolarRenderer>
#include <QwtPolarPicker>
#include <QwtPickerMachine>
#include <QwtScaleDiv>
#include <QwtRoundScaleDraw>

#include <QFileDialog>
#include <QPen>
#include <QLocale>
#include "Point.h"
#include "InterpolatorCollection.h"


#define AN 10

namespace
{
    class Picker : public QwtPolarPicker
    {
      public:
        Picker( QwtPolarCanvas* canvas )
            : QwtPolarPicker( canvas )
        {
            setStateMachine( new QwtPickerDragPointMachine() );
            setRubberBand( QwtPicker::NoRubberBand );
            setTrackerMode( ActiveOnly );
        }

        virtual QwtText trackerTextPolar( const QwtPointPolar& pos ) const QWT_OVERRIDE
        {
            QColor bg( Qt::white );
            bg.setAlpha( 200 );

            QwtText text = QwtPolarPicker::trackerTextPolar( pos );
            text.setBackgroundBrush( QBrush( bg ) );
            return text;
        }
    };

    // Pointless synthetic data, showing something nice

    class SpectrogramData : public QwtRasterData
    {
      public:
        SpectrogramData(): m_collection(std::make_shared<InterpolatorCollection>())
        {
            m_interval[Qt::ZAxis].setInterval( 0.0, AN==0 ? 1.0 : 2.0 );
            m_collection->addPoint(Point(0,0, 1, 1));
            for(int z = 1; z<=90; z++){
                for(int a = 0; a<360; a++){
                    auto zenith = static_cast<double>(z)/180*M_PI;
                    auto azimuth = static_cast<double>(a)/180*M_PI;
                    auto value = cos(zenith)*(1-sin(AN*azimuth));
                    m_collection->addPoint(Point(azimuth,zenith, 1., value));
                }
            }
            m_collection->makeTriangulation();
        }

        virtual double value( double azimuth, double radius ) const QWT_OVERRIDE
        {
//            const double c = 0.842;
//            const double x = radius / 10.0 * 3.0 - 1.5;
//            const double y = azimuth / ( 2 * M_PI ) * 3.0 - 1.5;
//
//            const double v1 = qwtSqr( x ) + ( y - c ) * ( y + c );
//            const double v2 = 2 * x * ( y + c );
//
//            const double v = 1.0 / ( qwtSqr( v1 ) + qwtSqr( v2 ) );
//            return v;
              auto func = m_collection.get();
              auto value = (*func)(Point(azimuth/180*M_PI,radius/180*M_PI));
            return value;
        }

        virtual QwtInterval interval( Qt::Axis axis ) const QWT_OVERRIDE
        {
            return m_interval[ axis ];
        }

      private:
        QwtInterval m_interval[3];
        std::shared_ptr<InterpolatorCollection> m_collection;
    };

    class AzimuthScaleDraw : public QwtRoundScaleDraw
    {
      public:
        virtual QwtText label( double value ) const QWT_OVERRIDE
        {
            if ( qFuzzyCompare( fmod( value, 360 ), 0.0 ) )
            {
                return QString( "0" );
            }

            if ( qFuzzyCompare( fmod( value, 45), 0.0 ) )
            {
                QString text;
                if ( !qFuzzyCompare( value, 1 ) )
                {
                    text += QLocale().toString( value / 1 );
                    text += " ";
                }
                text += "<FONT face=Arial size=4>&#176;</FONT>";
                return text;
            }

            return QwtRoundScaleDraw::label( value );
        }
    };

    class Grid : public QwtPolarGrid
    {
      public:
        Grid()
        {
            setPen( QPen( Qt::white ) );

            for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
            {
                showGrid( scaleId );
                showMinorGrid( scaleId );

                QPen minorPen( Qt::gray );
                setMinorGridPen( scaleId, minorPen );
            }

            setAxisPen( QwtPolar::AxisAzimuth, QPen( Qt::black ) );
            setAzimuthScaleDraw( new AzimuthScaleDraw() );
            showAxis( QwtPolar::AxisAzimuth, true );
            showAxis( QwtPolar::AxisLeft, false );
            showAxis( QwtPolar::AxisRight, true );
            showAxis( QwtPolar::AxisTop, false );
            showAxis( QwtPolar::AxisBottom, false );
            showGrid( QwtPolar::Azimuth, true );
            showGrid( QwtPolar::Radius, true );
        }
    };
}

Plot::Plot( QWidget* parent )
    : QwtPolarPlot( parent )
{
    setAutoReplot( false );
    setPlotBackground( Qt::darkBlue );

    // scales
    //setScale( QwtPolar::Azimuth, 0.0, 2 * M_PI, M_PI_4 );
    setScale( QwtPolar::Azimuth, 0.0, 360, 45 );
    setScaleMaxMinor( QwtPolar::Azimuth, 2 );


    setScale( QwtPolar::Radius, 0.0, 90.0 );
    setScaleMaxMinor( QwtPolar::Radius, 2 );

    // grids
    m_grid = new Grid();
    m_grid->attach( this );

    // spectrogram

    m_spectrogram = new QwtPolarSpectrogram();
    m_spectrogram->setPaintAttribute(
        QwtPolarSpectrogram::ApproximatedAtan, true );
    m_spectrogram->setRenderThreadCount( 0 ); // use multi threading
    m_spectrogram->setData( new SpectrogramData() );
    m_spectrogram->attach( this );

    m_spectrogram->setZ( 1.0 );
    m_grid->setZ( 2.0 );

    Picker* picker = new Picker( canvas() );
    picker->setMousePattern( QwtEventPattern::MouseSelect1, Qt::RightButton );

    QwtPolarMagnifier* magnifier = new QwtPolarMagnifier( canvas() );
    magnifier->setMouseButton( Qt::RightButton, Qt::ShiftModifier );

    new QwtPolarPanner( canvas() );
}

QwtPolarSpectrogram* Plot::spectrogram()
{
    return m_spectrogram;
}

void Plot::rotate()
{
    const double interval = 15.0; // degrees

    double origin = azimuthOrigin() / M_PI * 180.0;
    origin = qRound( origin / interval ) * interval + interval;

    setAzimuthOrigin( origin / 180.0 * M_PI );
    replot();
}

void Plot::mirror()
{
    const double a1 = scaleDiv( QwtPolar::Azimuth )->upperBound();
    const double a2 = scaleDiv( QwtPolar::Azimuth )->lowerBound();

    setScale( QwtPolar::Azimuth, a1, a2, qAbs( a2 - a1 ) / 8.0 );
    replot();
}

void Plot::exportDocument()
{
    QwtPolarRenderer renderer;
    renderer.exportTo( this, "spectrogram.pdf", QSizeF( 200, 200 ), 300 );
}

void Plot::showGrid( bool on )
{
    m_grid->setVisible( on );
    replot();
}

#include "moc_Plot.cpp"
