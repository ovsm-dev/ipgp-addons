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



#define SEISCOMP_COMPONENT COLORMAP_VIEWER


#include <ipgp/gui/datamodel/colormapviewer.h>
#include <seiscomp3/logging/log.h>
#include <ipgp/gui/misc/misc.h>

#include <QtGui>



namespace IPGP {
namespace Gui {

static int const marginFactor = 8;

const char* ColorSPlotType[] = { "Spectogram", "Scalogram" };
const std::vector<const char*> ColorPlotTypeString(ColorSPlotType, ColorSPlotType
        + sizeof(ColorSPlotType) / sizeof(ColorSPlotType[0]));
const size_t positionColorFromEnum(const ColorMapType& e) {
	char* str;
	for (size_t i = 0; i < ColorPlotTypeString.size(); ++i) {
		str = (char*) ColorPlotTypeString.at(i);
		if ( str == ColorSPlotType[e] )
		    return i;
	}
	return -1;
}
const int enumColorFromString(const char* v) {
	for (size_t i = 0; i < ColorPlotTypeString.size(); ++i)
		if ( ColorPlotTypeString.at(i) == v ) return (int) i;
	return -1;
}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ColorMapViewerPlot::ColorMapViewerPlot::Stream::Stream() :
		tracePen(QPen(Misc::defaultTraceColor)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ColorMapViewerPlot::ColorMapViewerPlot(const ColorMapType& t,
                                       QWidget* parent, Qt::WFlags f) :
		QCustomPlot(parent), _r(None), _colorMap(NULL) {

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

	for (size_t i = 0; i < ColorPlotTypeString.size(); ++i)
		_typeBox->insertItem(i, ColorPlotTypeString.at(i));
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

	plotLayout()->clear();
	plotLayout()->setAutoMargins(QCP::msNone);
	plotLayout()->setMargins(QMargins(0, 0, 0, 0));
//	plotLayout()->clear();


	_topAxisRect = new QCPAxisRect(this);
	_bottomAxisRect = new QCPAxisRect(this);

	plotLayout()->addElement(0, 0, _topAxisRect);
	plotLayout()->addElement(1, 0, _bottomAxisRect);

	_topAxisRect->setMinimumSize(50, 70);
	_topAxisRect->setMaximumSize(2048, 70);
	_topAxisRect->setRangeDrag(Qt::Horizontal);
	_topAxisRect->setRangeZoom(Qt::Horizontal);
	_topAxisRect->axis(QCPAxis::atLeft)->setVisible(false);
//	_topAxisRect->axis(QCPAxis::atBottom)->setVisible(false);
	_topAxisRect->axis(QCPAxis::atBottom)->setTicks(false);
	_topAxisRect->axis(QCPAxis::atBottom)->grid()->setPen(QPen(Misc::gridColor));
	_topAxisRect->axis(QCPAxis::atBottom)->setTickLabels(false);
	_topAxisRect->setBackground(Misc::waitingForStreamDataColor);
	_trace = addGraph(_topAxisRect->axis(QCPAxis::atBottom), _topAxisRect->axis(QCPAxis::atLeft));

	_bottomAxisRect->axis(QCPAxis::atBottom)->grid()->setPen(QPen(Misc::gridColor));

	_timeScale = _bottomAxisRect->axis(QCPAxis::atBottom);
//	_timeScale->grid()->setVisible(false);
//	_timeScale->grid()->setSubGridVisible(false);
	_timeScale->setTickLabelType(QCPAxis::ltDateTimeAuto);
	_timeScale->setTickLabelPadding(0);
	_timeScale->setDateTimeFormat("hh:mm:ss");
	_timeScale->setSubTickCount(5);
	_timeScale->setTickLength(0, _timeScale->tickLengthIn() + 3);
	_timeScale->setSubTickLength(0, _timeScale->subTickLengthIn() + 2);

	_topAxisRect->setAutoMargins(QCP::msNone);
	_topAxisRect->setMinimumMargins(QMargins(-10, -10, -10, -10));
	//! Adjust the top rect to fit the bottom rect inner space
	_topAxisRect->setMargins(QMargins(51, 0, 5, -8));

//	_bottomAxisRect->setAutoMargins(QCP::msNone);
	_bottomAxisRect->setMinimumMargins(QMargins(0, 0, 0, 0));
	_bottomAxisRect->setMargins(QMargins(0, 0, 0, 0));


	_colorMap = new QCPColorMap(_bottomAxisRect->axis(QCPAxis::atBottom),
	    _bottomAxisRect->axis(QCPAxis::atLeft));
	addPlottable(_colorMap);
	_colorMap->setGradient(QCPColorGradient::gpPolar);

	_bottomAxisRect->axis(QCPAxis::atTop)->setVisible(true);
	_bottomAxisRect->axis(QCPAxis::atTop)->setTicks(false);
	_bottomAxisRect->axis(QCPAxis::atTop)->setTickLabels(false);
	_bottomAxisRect->axis(QCPAxis::atRight)->setVisible(true);
	_bottomAxisRect->axis(QCPAxis::atRight)->setTicks(false);
	_bottomAxisRect->axis(QCPAxis::atRight)->setTickLabels(false);
	_bottomAxisRect->setBackground(Qt::white);
	_bottomAxisRect->setRangeDrag(Qt::Horizontal);
	_bottomAxisRect->setRangeZoom(Qt::Horizontal);
	_bottomAxisRect->axis(QCPAxis::atLeft)->setLabel("Frequency [Hz]");
	_bottomAxisRect->setupFullAxesBox();
	connect(_bottomAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), _bottomAxisRect->axis(QCPAxis::atTop), SLOT(setRange(QCPRange)));
	connect(_bottomAxisRect->axis(QCPAxis::atLeft), SIGNAL(rangeChanged(QCPRange)), _bottomAxisRect->axis(QCPAxis::atRight), SLOT(setRange(QCPRange)));
	connect(_bottomAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), _topAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
	connect(_topAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), _bottomAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));

	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

	setType(t);
	setFrequencyProfile();
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ColorMapViewerPlot::~ColorMapViewerPlot() {
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::mousePressEvent(QMouseEvent* event) {

	if ( event->button() == Qt::RightButton ) {
		changeRulerState();
		replot();
	}

	QCustomPlot::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::resizeEvent(QResizeEvent* event) {
	_infoLabel->setGeometry(QRect(rect().topRight()
	        - QPoint(_infoLabel->size().width() + 8, -8), _infoLabel->size()));
	QCustomPlot::resizeEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::changeRulerState() {

	int c = (int) _r;
	++c;
	if ( c > 2 )
	    c = 0;
	_r = static_cast<Ruler>(c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::reset() {
	_trace->data()->clear();
	_colorMap->data()->clear();
	_streams.clear();
	_topAxisRect->setBackground(Misc::waitingForStreamDataColor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::addStream(Stream& stream) {

	if ( _streams.find(stream.id) == _streams.end() )
		_streams.insert(stream.id, stream);
	else
		SEISCOMP_DEBUG("Ignored stream %s : already present", stream.id.toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::setType(const ColorMapType& t, const bool& update) {

	_t = t;

	/*
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
	 */

	if ( update ) redraw();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::setFrequencyProfile(const bool& rescaleAuto,
                                             const double& freqRangeMin,
                                             const double& freqRangeMax,
                                             const bool& update) {
	_rescaleFreqRange = rescaleAuto;
	_fmin = freqRangeMin;
	_fmax = freqRangeMax;

	if ( update ) redraw();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::redraw() {

	for (StreamList::const_iterator it = _streams.constBegin();
	        it != _streams.constEnd(); ++it) {

		if ( it.value().data.size() == 0 ) continue;
		DataPointVector points;
		Core::Math::Numbers<double> globalTime;

		for (int i = 0; i < it.value().data.size(); ++i) {

			_topAxisRect->setBackground(Qt::white);
			_trace->setPen(it.value().tracePen);

			Core::Math::Numbers<double> times;

			//! Draw the trace
			double stime = it.value().data.at(i).time;
			for (int j = 0; j < it.value().data.at(i).stack.size(); ++j) {
				times.add(stime);
				globalTime.add(stime);
				_trace->addData(stime, it.value().data.at(i).stack.get(j));
				stime += 1.0 / it.value().data.at(i).fsamp;
			}
			times.add(stime);
			globalTime.add(stime);

			ComplexVector v;
			Seiscomp::Math::fft(v, it.value().data.at(i).stack.size(), it.value().data.at(i).stack.typedData());

			if ( v.size() == 0 ) continue;

			Core::Math::Numbers<double> vamp, vfreq;
			for (size_t k = 0; k < v.size(); ++k) {
				double amp = sqrt(v.at(k).real() * v.at(k).real() + v.at(k).imag() * v.at(k).imag());
				double freq = static_cast<double>(k) * it.value().data.at(i).fsamp
				        / static_cast<double>(v.size());
				vamp.add(amp);
				vfreq.add(freq);
			}

			//! Remap amplitudes in new range
			Core::Math::Numbers<double> rvamp;
			for (size_t t = 0; t < vamp.size(); ++t)
				rvamp.add(Core::Math::remap(vamp.at(t), vamp.min(), vamp.max(), .0, 1.));

			DataPoint p;
			p.startTime = times.min();
			p.endTime = times.max();
			p.frequencies = vfreq;
			p.amplitudes = rvamp;

			points.append(p);
		}

		switch ( _t ) {

			case Spectrogram: {

				SEISCOMP_DEBUG("Points to plot: %d", points.size());
				SEISCOMP_DEBUG("Times calculated: %zu", globalTime.size());

				_colorMap->data()->setSize(points.size(), _fmax * 2);
//				_colorMap->data()->setSize(globalTime.max() - globalTime.min(), _fmax);
				_colorMap->data()->setRange(QCPRange(globalTime.min(), globalTime.max()), QCPRange(_fmin, _fmax));
				_colorMap->setGradient(QCPColorGradient::gpSpectrum);

//				for (int i = 0; i < (int) points.size(); ++i)
//					for (int y = 0; y < (int) (_fmax * 2); ++y)
//						for (size_t j = 0; j < points.at(i).frequencies.size();
//						        ++j)
//							_colorMap->data()->setCell(i, y, points.at(i).amplitudes.at(j));

				for (int i = 0; i < points.size(); ++i) {
					for (size_t j = 0; j < points.at(i).frequencies.size(); ++j)
						_colorMap->data()->setData(points.at(i).startTime,
						    points.at(i).frequencies.at(j), points.at(i).amplitudes.at(j));
				}

				_colorMap->rescaleDataRange();
			}
			break;

			case Scalogram: {

			}
			break;
		}
	}


	rescaleAxes();
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ColorMapViewerPlot::setType(const int& t) {
	ColorMapType e = static_cast<ColorMapType>(t);
	setType(e, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
