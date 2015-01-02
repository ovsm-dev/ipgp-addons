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

#define SEISCOMP_COMPONENT IPGP_RECORDENTITY


#include <ipgp/gui/datamodel/recordentity.h>
#include <ipgp/gui/datamodel/recordmanager.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/math/math.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/math/fft.h>


using namespace Seiscomp;
using namespace Seiscomp::Core;


namespace {

struct ColorMapEntity {
		IPGP::Core::Math::Numbers<double> times, vamp, vfreq;
		double fsamp;
};
typedef QVector<ColorMapEntity> ColorMapStack;

typedef QMap<double, double> TimeData;
typedef QVector<double> DoubleVector;

struct TraceData {
		IPGP::Core::Math::Numbers<double> times;
		DoubleVector values;
};

TraceData getTraceData(const double& start, const double& end, TimeData* d) {
	TraceData td;
	if ( d ) {
		for (TimeData::const_iterator it = d->constBegin();
		        it != d->constEnd(); ++it)
			if ( it.key() >= start && it.key() < end ) {
				td.times.add(it.key());
				td.values << it.value();
			}
	}
	return td;
}

}




namespace IPGP {
namespace Gui {

typedef std::vector<Seiscomp::Math::Complex> ComplexArray;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordEntity::RecordEntity(QCustomPlot* p, const QString& n, const Type& t) :
		_rect(NULL), _bottomAxis(NULL), _topAxis(NULL),
		_leftAxis(NULL), _rightAxis(NULL), _trace(NULL), _colorMap(NULL),
		_label(NULL), _filter(NULL), _visibleAmpMin(.0), _visibleAmpMax(.0),
		_isValid(false), _isSelected(false), _cp(Unknown), _type(t) {

	setPlot(p);
	setName(n);

	if ( p && !p->parent() )
	    SEISCOMP_WARNING("Record %s ptr has been created without parent",
	        (n.isEmpty()) ? QString::number(reinterpret_cast<quintptr>(p)).toStdString().c_str() : n.toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordEntity::~RecordEntity() {
	// This entity's object have the same parent as the plot used. One needs
	// to make sure that when
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::destroy() throw (RecordEntityException) {

	if ( !_comp )
	    throw RecordEntityException("This record isn't initialized");

	_comp->removePlottable(_colorMap);

	delete _comp;

	_comp = NULL;
	_colorMap = NULL;
	_bottomAxis = NULL;
	_trace = NULL;
	_leftAxis = NULL;
	_rect = NULL;
	_rightAxis = NULL;
	_gaps.clear();

	setValid(false);
	setSelected(false);

	SEISCOMP_DEBUG("RecordEntity %s destroyed", _name.toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::reset(const QString& newName) throw (RecordEntityException) {

	if ( !_comp )
	    throw RecordEntityException(QString("%1: This entity isn't initialized").arg(_name));

	setName(newName);
	setValid(false);
	setSelected(false);
	_trace->clearData();
	_rect->setBackground(Misc::waitingForStreamDataColor);

	if ( _colorMap ) _colorMap->data()->clear();

	for (int i = 0; i < _gaps.size(); ++i)
		_comp->removeItem(_gaps.at(i));
	_gaps.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::rescaleItems(const bool& replot) {
	rescaleGaps();
	if ( replot ) _comp->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::rescaleItems(const double& min, const double& max,
                                const bool& replot) {
	rescaleGaps(min, max);
	if ( replot ) _comp->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::setName(const QString& n) throw (RecordEntityException) {

	if ( n.isEmpty() )
	    throw RecordEntityException(QString("%1: Empty QString passed as argument").arg(_name));

	_name = n;
	_label->setText(_name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::rescale(const bool& replot) {

	QVector<double> range;
	for (QCPDataMap::iterator itm = trace()->data()->begin();
	        itm != trace()->data()->end(); ++itm)
		if ( itm.value().key > bottomAxis()->range().lower
		        && itm.value().key < bottomAxis()->range().upper )
		    range.append(itm.value().value);
	setVisibleAmpMax(Math::getMax(range));
	setVisibleAmpMin(Math::getMin(range));

	_trace->valueAxis()->setRange(Math::getMin(range), Math::getMax(range));
//	_trace->rescaleValueAxis();
	rescaleGaps(Math::getMin(range), Math::getMax(range));

	if ( replot ) _comp->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::setPlot(QCustomPlot* p) throw (RecordEntityException) {

	if ( !p )
	    throw RecordEntityException(QString("%1: NULL QCustomPlot ptr passed as argument").arg(_name));

	_comp = p;
	_trace = _comp->addGraph();

	QFont font = _comp->font();
	font.setPointSize(9);
	font.setBold(true);

	_label = new QCPItemText(_comp);
	_label->setClipAxisRect(_comp->axisRect(0));
	_label->setClipToAxisRect(true);
	_label->position->setAxisRect(_comp->axisRect(0));
	_label->position->setType(QCPItemPosition::ptAbsolute);
	_label->position->setCoords(QPointF(1., 11.));
	_label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	_label->setColor(Qt::black);
	_label->setBrush(Qt::white);
	_label->setPen(QPen(Qt::black));
	_label->setText(_name);
	_label->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_label->setFont(font);
	_label->setPadding(QMargins(2, 2, 2, 2));
	_comp->addItem(_label);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::
setRectAxis(QCPAxisRect* r) throw (RecordEntityException) {

	if ( !r )
	    throw RecordEntityException(QString("%1: NULL QCPAxisRect ptr passed as argument").arg(_name));

	_rect = r;
	_bottomAxis = _rect->axis(QCPAxis::atBottom);
	_leftAxis = _rect->axis(QCPAxis::atLeft);
	_topAxis = _rect->axis(QCPAxis::atTop);
	_rightAxis = _rect->axis(QCPAxis::atRight);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::
setSequence(RecordSequence* seq, const bool& update) throw (RecordEntityException) {

	if ( !seq )
	    throw RecordEntityException(QString("%1: NULL Seiscomp::RecordSequence ptr passed as argument").arg(_name));

	_trace->clearData();

	for (int i = 0; i < _gaps.size(); ++i)
		_comp->removeItem(_gaps.at(i));
	_gaps.clear();

	drawTrace(seq);

	_trace->rescaleValueAxis();

	if ( update ) _comp->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::
appendSequence(RecordSequence* seq, const bool& update, const qreal& buffer) throw (RecordEntityException) {

	if ( !seq )
	    throw RecordEntityException(QString("%1: NULL Seiscomp::RecordSequence ptr passed as argument").arg(_name));

	drawTrace(seq);

	_trace->rescaleValueAxis();

	RecordCPtr rec = seq->at(0);
	if ( rec && buffer > .0 ) {
		double time = (double) rec->startTime() - buffer;
		_trace->removeDataBefore(time);
		for (int i = 0; i < _gaps.size(); ++i) {
			QCPItemRect* g = _gaps.at(i);
			if ( g->bottomRight->coords().x() < time ) {
				_comp->removeItem(g);
				_gaps.remove(i);
			}
		}
	}

	if ( update ) _comp->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::drawTrace(RecordSequence* seq) {

	if ( seq->size() > 0 )
	    (_isSelected) ? _rect->setBackground(QColor(130, 187, 231, 100)) :
	            _rect->setBackground(Qt::white);

	//! Draw simple sensor trace
	if ( _type == Numeric ) {

		for (size_t i = 0; i < seq->size(); ++i) {

			RecordCPtr rec = seq->at(i);

			if ( !rec ) continue;

			double stime = rec->startTime();
			double delta = 1. / rec->samplingFrequency();

			ArrayPtr tmp_ar;
			const DoubleArray* data = DoubleArray::ConstCast(rec->data());
			if ( !data ) {
				tmp_ar = rec->data()->copy(Array::DOUBLE);
				data = DoubleArray::ConstCast(tmp_ar);
				if ( !data ) continue;
			}

			DoubleArray* data2 = DoubleArray::Cast(data->clone());

			if ( _filter ) {
				_filter->setStartTime(rec->startTime());
				_filter->setSamplingFrequency(rec->samplingFrequency());
				_filter->setStreamID(rec->networkCode(), rec->stationCode(),
				    rec->locationCode(), rec->channelCode());
				try {
					_filter->apply(*data2);
				}
				catch ( std::exception& e ) {
					SEISCOMP_ERROR("%s", e.what());
				}
			}

			for (int j = 0; j < data2->size(); ++j) {
				_trace->addData(stime, data2->get(j));
				stime += TimeSpan(delta);
			}
		}
	}

	//! Draw spectrogram
	else if ( _type == Frequency ) {

		QCPColorGradient gradient;
		gradient.clearColorStops();
		gradient.setColorInterpolation(QCPColorGradient::ciRGB);
		gradient.setColorStopAt(1, QColor(132, 0, 0));
		gradient.setColorStopAt(.8, QColor(255, 255, 0));
		gradient.setColorStopAt(.5, QColor(0, 255, 0));
		gradient.setColorStopAt(0, QColor(0, 0, 155));

		RecordManager rm;
		rm.setComputingResolution(QuarterOfSecond);
		rm.feed(seq);
		rm.prepareFFT();
		rm.computeFFT();

		RecordManager::EntityVector v = rm.entities();

		/*
		 double cr = rm.getComputingResolution();
		 for (int i = 0; i < v.size(); ++i) {

		 QCPColorMap* map = new QCPColorMap(_comp->xAxis, _comp->yAxis);
		 _comp->addPlottable(map);
		 map->setGradient(gradient);
		 map->data()->setSize(1, 10);
		 map->data()->setRange(QCPRange(v.at(i).time, v.at(i).time + cr), QCPRange(.0, 10.));

		 for (size_t j = 0; j < v.at(i).freqs.size(); ++j)
		 map->data()->setData(v.at(i).time, v.at(i).freqs.at(j), v.at(i).amps.at(j));

		 map->rescaleDataRange();

		 }
		 */


		if ( _colorMap ) {
			_comp->removePlottable(_colorMap);
			_colorMap = NULL;
		}

		if ( v.size() != 0 ) {

			double minTime, maxTime;
			size_t maxCells = 0;
			for (int i = 0; i < v.size(); ++i) {
				if ( i == 0 ) minTime = maxTime = v.at(i).time;
				if ( v.at(i).time > maxTime )
				    maxTime = v.at(i).time;
				if ( v.at(i).time < minTime )
				    minTime = v.at(i).time;
				if ( v.at(i).amps.size() > maxCells )
				    maxCells = v.at(i).amps.size();
			}

			_colorMap = new QCPColorMap(_comp->xAxis, _comp->yAxis);
			_comp->addPlottable(_colorMap);

			_colorMap->data()->setSize(v.size(), 5);
//			_colorMap->data()->setRange(QCPRange(seq->timeWindow().startTime(), seq->timeWindow().endTime()), QCPRange(.0, 10.));
			_colorMap->data()->setRange(QCPRange(minTime, maxTime), QCPRange(.0, 10.));
			_colorMap->setGradient(gradient);

			for (int i = 0; i < v.size(); ++i)
				for (size_t j = 0; j < v.at(i).freqs.size(); ++j)
					_colorMap->data()->setData(v.at(i).time, v.at(i).freqs.at(j), v.at(i).amps.at(j));

			_colorMap->rescaleDataRange();
			leftAxis()->setRange(.0, 10.);
		}


		rm.clear();
	}

	//! Draw the sequence's gaps
	if ( seq->gaps().size() > 0 ) {
		for (size_t g = 0; g < seq->gaps().size(); ++g) {
			QCPItemRect* rect = new QCPItemRect(_comp);
			rect->setClipAxisRect(_rect);
			rect->setClipToAxisRect(true);
			rect->setPen(QPen(Misc::gapColor));
			rect->setBrush(Misc::gapColor);
			rect->topLeft->setType(QCPItemPosition::ptPlotCoords);
			rect->topLeft->setAxes(_bottomAxis, _leftAxis);
			rect->topLeft->setCoords((double) seq->gaps().at(g).startTime(),
			    _trace->valueAxis()->range().upper);
			rect->bottomRight->setType(QCPItemPosition::ptPlotCoords);
			rect->bottomRight->setAxes(_bottomAxis, _leftAxis);
			rect->bottomRight->setCoords((double) seq->gaps().at(g).endTime(),
			    _trace->valueAxis()->range().lower);
			_comp->addItem(rect);
			_gaps.append(rect);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::rescaleGaps() {

	for (int i = 0; i < _gaps.size(); ++i) {
		_gaps.at(i)->topLeft->setCoords(_gaps.at(i)->topLeft->coords().x(),
		    _trace->valueAxis()->range().upper);
		_gaps.at(i)->bottomRight->setCoords(_gaps.at(i)->bottomRight->coords().x(),
		    _trace->valueAxis()->range().lower);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordEntity::rescaleGaps(const double& min, const double& max) {

	for (int i = 0; i < _gaps.size(); ++i) {
		_gaps.at(i)->topLeft->setCoords(_gaps.at(i)->topLeft->coords().x(), max);
		_gaps.at(i)->bottomRight->setCoords(_gaps.at(i)->bottomRight->coords().x(), min);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}// namespace Gui
} // namespace IPGP
