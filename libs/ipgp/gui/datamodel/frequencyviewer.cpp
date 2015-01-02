/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/



#define SEISCOMP_COMPONENT FREQUENCY_VIEWER


#include <ipgp/gui/datamodel/frequencyviewer.h>
#include <seiscomp3/logging/log.h>
#include <ipgp/core/math/math.h>
#include <ipgp/gui/misc/misc.h>

#include <QtGui>



namespace IPGP {
namespace Gui {

static int const marginFactor = 8;

const char* SPlotType[] = { "FreqAmpCounts", "FreqAmpdB", "FreqPhase" };
const std::vector<const char*> PlotTypeString(SPlotType, SPlotType
        + sizeof(SPlotType) / sizeof(SPlotType[0]));
const size_t positionFromEnum(const EPlotType& e) {
	char* str;
	for (size_t i = 0; i < PlotTypeString.size(); ++i) {
		str = (char*) PlotTypeString.at(i);
		if ( str == SPlotType[e] )
		    return i;
	}
	return -1;
}
const int enumFromString(const char* v) {
	for (size_t i = 0; i < PlotTypeString.size(); ++i)
		if ( PlotTypeString.at(i) == v ) return (int) i;
	return -1;
}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyViewerPlot::FrequencyViewerPlot::Stream::Stream() :
		fsamp(100.), tracePen(QPen(Misc::defaultTraceColor)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyViewerPlot::FrequencyViewerPlot(const EPlotType& t,
                                         QWidget* parent, Qt::WFlags f) :
		QCustomPlot(parent), _r(None) {

	QPen mpen;
	mpen.setWidthF(2.);
	mpen.setColor(Qt::darkRed);

	_marker = new QCPItemLine(this);
	_marker->setAntialiased(false);
	_marker->setHead(QCPLineEnding::esSpikeArrow);
	_marker->setTail(QCPLineEnding::esDisc);
	_marker->setPen(mpen);
	_marker->setClipAxisRect(axisRect());
	_marker->setClipToAxisRect(true);
	_marker->start->setType(QCPItemPosition::ptPlotCoords);
	_marker->start->setCoords(.0, .0);
	_marker->end->setType(QCPItemPosition::ptPlotCoords);
	_marker->end->setCoords(.0, .0);
	_marker->setVisible(false);
	addItem(_marker);

	QFont font = this->font();
	font.setPointSize(8);
	font.setBold(true);

	_typeBox = new QComboBox(this);
	_typeBox->setCursor(QCursor(Qt::PointingHandCursor));
	_typeBox->setAutoFillBackground(false);
	_typeBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	_typeBox->setGeometry(QRect(QPoint(5, 5), _typeBox->size()));

	for (size_t i = 0; i < PlotTypeString.size(); ++i)
		_typeBox->insertItem(i, PlotTypeString.at(i));
	_typeBox->setCurrentIndex(0);
	connect(_typeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setType(const int&)));
	_typeBox->hide();

	_infoLabel = new QLabel(this);
	_infoLabel->setText("");
	_infoLabel->setFrameShape(QFrame::Box);
	_infoLabel->setFrameShadow(QFrame::Raised);
	_infoLabel->setFont(font);

	QPalette palette = _infoLabel->palette();
	palette.setColor(_infoLabel->backgroundRole(), QColor("#CDFBCD"));
	_infoLabel->setAutoFillBackground(true);
	_infoLabel->setPalette(palette);
	_infoLabel->hide();

	setWindowFlags(f);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMinimumHeight(45);
	resize(100, 100);
	setBackground(Qt::transparent);
	plotLayout()->setAutoMargins(QCP::msNone);
	plotLayout()->setMargins(QMargins(0, 0, 0, 0));
	axisRect(0)->setMinimumMargins(QMargins(0, 0, 0, 0));

	xAxis->grid()->setVisible(true);
	xAxis2->setVisible(true);
	xAxis2->setTicks(false);
	xAxis2->setTickLabels(false);
//	yAxis->grid()->setSubGridVisible(false);
	yAxis2->setVisible(true);
	yAxis2->setTicks(false);
	yAxis2->setTickLabels(false);

//	xAxis->setNumberPrecision(0);
	xAxis->setSubTickCount(10);

//	xAxis->grid()->setSubGridVisible(true);
//	yAxis->setNumberPrecision(0);
	yAxis->setSubTickCount(10);


	yAxis->grid()->setSubGridVisible(true);
	xAxis->setScaleType(QCPAxis::stLogarithmic);
	xAxis->setScaleLogBase(10);
//	xAxis->setScaleLogBase(2);
	yAxis->setScaleType(QCPAxis::stLogarithmic);
	yAxis->setScaleLogBase(10);
//	yAxis->setScaleLogBase(2);

	axisRect()->setBackground(Qt::white);
	axisRect()->setRangeDrag(Qt::Horizontal);
	axisRect()->setRangeZoom(Qt::Horizontal);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	xAxis->setLabel("Frequency [Hz]");
	yAxis->setLabel("Amplitude");
	axisRect()->setupFullAxesBox();
	connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
	connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

	setType(t);
	setFrequencyProfile();
	setAmplitudeProfile();
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyViewerPlot::~FrequencyViewerPlot() {
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::mousePressEvent(QMouseEvent* event) {

	if ( event->button() == Qt::RightButton ) {
		changeRulerState();
		replot();
	}

	QCustomPlot::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::mouseMoveEvent(QMouseEvent* event) {

	(axisRect(0)->rect().contains(event->pos())) ?
	        setCursor(Qt::PointingHandCursor) : setCursor(Qt::ArrowCursor);

	(QRect(QPoint(5, 5), _typeBox->size()).contains(event->pos())) ?
	        _typeBox->show() : _typeBox->hide();

	if ( _r == None ) {
		_marker->setVisible(false);
		_infoLabel->hide();
		emit markerIndication("");
	}
	else {

		const qreal y = yAxis->pixelToCoord(event->posF().y());
		const qreal x = xAxis->pixelToCoord(event->posF().x());

		if ( _r == Vertical ) {

			_marker->start->setCoords(xAxis->range().minRange, y);
			_marker->end->setCoords(xAxis->range().maxRange, y);
			_marker->setVisible(true);
			_infoLabel->setText(QString("%1 %2").arg(
			    _type == "Counts" ? QString::number(y, 'f', 0) :
			            QString::number(y, 'f', 2)).arg(_type));
			_infoLabel->show();
			emit markerIndication(QString("%1: %2").arg(_type).arg(
			    _type == "Counts" ? QString::number(y, 'f', 0) :
			            QString::number(y, 'f', 2)));
		}
		else if ( _r == Horizontal ) {
			_marker->start->setCoords(x, yAxis->range().minRange);
			_marker->end->setCoords(x, yAxis->range().maxRange);
			_marker->setVisible(true);
			_infoLabel->setText(QString("%1 Hz").arg(QString::number(x, 'f', 2)));
			_infoLabel->show();
			emit markerIndication(QString("Freq.: %1 Hz").arg(QString::number(x, 'f', 2)));
		}

		QRect r = _infoLabel->fontMetrics().boundingRect(_infoLabel->text());
		_infoLabel->resize(QSize(r.width() + marginFactor, r.height() + marginFactor));
		_infoLabel->setGeometry(QRect(rect().topRight()
		        - QPoint(_infoLabel->size().width() + marginFactor, -marginFactor),
		    _infoLabel->size()));
	}

	//! Do this to make sure the damn marker is hidden or displayed properly
	replot();

	QCustomPlot::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::resizeEvent(QResizeEvent* event) {
	_infoLabel->setGeometry(QRect(rect().topRight()
	        - QPoint(_infoLabel->size().width() + 8, -8), _infoLabel->size()));
	QCustomPlot::resizeEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::changeRulerState() {

	int c = (int) _r;
	++c;
	if ( c > 2 )
	    c = 0;
	_r = static_cast<Ruler>(c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::reset() {
	clearPlottables();
	_streams.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::addStream(Stream& stream) {

	/*
	 The first bin in the FFT is DC (0 Hz), the second bin is 'Fs / N', where 'Fs'
	 is the sample rate and 'N' is the size of the FFT. The next bin is '2 * Fs / N'.
	 To express this in general terms, the 'nth' bin is 'n * Fs / N'.

	 So if the component sample rate, Fs is say 1KHz(1000Hz) and the FFT size,
	 N is 1024 samples, then the FFT output bins are at:

	 0:   0 * 1000 / 1024 =     0.0 Hz
	 1:   1 * 1000 / 1024 =     .97 Hz
	 2:   2 * 1000 / 1024 =    1.95 Hz
	 3:   3 * 1000 / 1024 =    2.92 Hz
	 4: ...
	 5: ...
	 ...
	 511: 511 * 1000 / 1024 = 499.0 Hz
	 Note that for a real input signal (imaginary parts all zero) the second half
	 of the FFT (bins from N / 2 + 1 to N - 1) contain no useful additional
	 information (they have complex conjugate symmetry with the first N / 2 - 1 bins).
	 The last useful bin (for practical applications) is at N / 2 - 1, which
	 corresponds to 499.0 Hz in the above example. The bin at N / 2 represents
	 energy at the Nyquist frequency, i.e. Fs / 2 ( = 500 Hz in this example),
	 but this is in general not of any practical use, since anti-aliasing filters
	 will typically attenuate any signals at and above Fs / 2.
	 */

	Seiscomp::Math::fft(stream.fftRes, stream.data.size(), stream.data.typedData());

#ifdef DRAWINGDEBUG
	SEISCOMP_DEBUG("Number of samples in FFT: %d", stream.data.size());
	SEISCOMP_DEBUG("Number of complexes in FFT: %zu", stream.fftRes.size());
	SEISCOMP_DEBUG("FFT fsamp: %.2fHz", stream.fsamp);
#endif

	_streams.insert(stream.id, stream);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::setType(const EPlotType& t, const bool& update) {

	_t = t;

	switch ( _t ) {
		case FreqAmpCounts:
			_type = "Counts";
			yAxis->setLabel("Amplitude [Counts]");
		break;
		case FreqAmpdB:
			_type = "dB";
			yAxis->setLabel("Amplitude [dB]");
		break;
		case FreqPhase:
			_type = "Phase";
			yAxis->setLabel("Phase");
		break;
	}

	if ( update ) redraw();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::setFrequencyProfile(const bool& rescaleAuto,
                                              const double& freqRangeMin,
                                              const double& freqRangeMax,
                                              const bool& update) {
	_rescaleFrequencyRange = rescaleAuto;
	_freqRangeMin = freqRangeMin;
	_freqRangeMax = freqRangeMax;

	if ( update ) redraw();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::setAmplitudeProfile(const bool& rescaleAuto,
                                              const double& ampRangeMin,
                                              const double& ampRangeMax,
                                              const bool& update) {
	_rescaleAmplitudeRange = rescaleAuto;
	_ampRangeMin = ampRangeMin;
	_ampRangeMax = ampRangeMax;

	if ( update ) redraw();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::redraw() {

	clearPlottables();

	for (StreamList::const_iterator it = _streams.constBegin();
	        it != _streams.constEnd(); ++it) {

		if ( it.value().fftRes.size() == 0 ) continue;

		QCPGraph* g = addGraph();
		g->setPen(it.value().tracePen);

		//! @note The sc3 fft function already outputs half of the complex +1 bins
		size_t maxSize = it.value().fftRes.size();

		Core::Math::Numbers<double> vamp, vampdB, vfreq, vphase;
		for (size_t i = 0; i < maxSize; ++i) {

			double amp = sqrt(it.value().fftRes[i].real() * it.value().fftRes[i].real()
			        + it.value().fftRes[i].imag() * it.value().fftRes[i].imag());
			double ampdB = 20. * log10(amp);
			double phase = atan(it.value().fftRes[i].real() / it.value().fftRes[i].imag());
			double freq = static_cast<double>(i) * it.value().fsamp
			        / static_cast<double>(it.value().fftRes.size());

			vamp.add(amp);
			vampdB.add(ampdB);
			vfreq.add(freq);
			vphase.add(phase);
		}


		switch ( _t ) {

			case FreqAmpCounts: {
				// Draw spectrum of freq/amplitude in Hz/stream unit (m/s or m/s2)

				for (size_t i = 0; i < maxSize; ++i) {
					double freq = vfreq.at(i);
					double amp = vamp.at(i);
					if ( _rescaleAmplitudeRange )
					    amp = Core::Math::remap<double>(amp, vamp.min(), vamp.max(), _ampRangeMin, _ampRangeMax);
					if ( _rescaleFrequencyRange && (freq < _freqRangeMin || freq > _freqRangeMax) )
					    continue;
					g->addData(freq, amp);
				}
			}
			break;

			case FreqAmpdB: {
				// Draw spectrum of freq/amplitudeDB in Hz/decibels
				for (size_t i = 0; i < maxSize; ++i) {
					double freq = vfreq.at(i);
					double ampdB = vampdB.at(i);

					// Why this here?
//					if ( _rescaleAmplitudeRange )
//					    ampdB = Core::Math::remap<double>(ampdB, vampdB.min(), vampdB.max(),
//					        10. * log10(_ampRangeMin), 10. * log10(_ampRangeMax));

					if ( _rescaleFrequencyRange && (freq < _freqRangeMin || freq > _freqRangeMax) )
					    continue;

					g->addData(freq, ampdB);
				}
			}
			break;

			case FreqPhase: {
				// Draw spectrum of freq/phase in Hz/
				for (size_t i = 0; i < maxSize; ++i) {
					double freq = vfreq.at(i);
					if ( _rescaleFrequencyRange && (freq < _freqRangeMin || freq > _freqRangeMax) )
					    continue;
					g->addData(freq, vphase.at(i));
				}
			}
			break;
		}
	}

	rescaleAxes();
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FrequencyViewerPlot::setType(const int& t) {
	EPlotType e = static_cast<EPlotType>(t);
	setType(e, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
