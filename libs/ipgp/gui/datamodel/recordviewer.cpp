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



#include <ipgp/gui/datamodel/recordviewer.h>
#include <ipgp/gui/math/math.h>
#include <seiscomp3/core/recordsequence.h>
#include <QtGui>

using namespace Seiscomp::Core;

//! Handle the RecordEntity streams and timeline placement (in pixels)
static const int leftPadding = 1;
static const int rightPadding = 1;

//! Handle the RecordEntity minimum height (in pixels)
static const int recordHeight = 50;

static const double spectrumMinFreq = .1;
static const double spectrumMaxFreq = 10.;

namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeLine::TimeLine(QWidget* parent, Qt::WFlags f) :
		QCustomPlot(parent), _timeScale(NULL), _la(None), _timeMargin(.0) {

	setWindowFlags(f);
	setMaximumHeight(45);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	setBackground(Qt::transparent);

	plotLayout()->setAutoMargins(QCP::msNone);
	plotLayout()->setMargins(QMargins(leftPadding * 2, 0, rightPadding * 2, 1));
	plotLayout()->clear();

	QCPAxisRect* comp = new QCPAxisRect(this);
	comp->setMinimumMargins(QMargins(0, 0, 0, 0));
	comp->setMargins(QMargins(0, 0, 0, 0));
	comp->removeAxis(comp->axis(QCPAxis::atTop));
	comp->removeAxis(comp->axis(QCPAxis::atLeft));
	comp->removeAxis(comp->axis(QCPAxis::atRight));

	plotLayout()->addElement(0, 0, comp);

	_timeScale = comp->axis(QCPAxis::atBottom);
	_timeScale->grid()->setVisible(false);
	_timeScale->grid()->setSubGridVisible(false);
	_timeScale->setTickLabelType(QCPAxis::ltDateTimeAuto);
	_timeScale->setDateTimeSpec(Qt::UTC);
	_timeScale->setTickLabelPadding(0);
	_timeScale->setDateTimeFormat("hh:mm:ss");
	_timeScale->setSubTickCount(5);
	_timeScale->setTickLength(0, comp->axis(QCPAxis::atBottom)->tickLengthIn() + 3);
	_timeScale->setSubTickLength(0, comp->axis(QCPAxis::atBottom)->subTickLengthIn() + 2);
	connect(_timeScale, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(newTimeWindow(const QCPRange&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::wheelEvent(QWheelEvent* event) {

	if ( (_timeScale->range().upper - _timeScale->range().lower) < 6. )
		_timeScale->setDateTimeFormat("hh:mm:ss.zzz");
	else
		_timeScale->setDateTimeFormat("hh:mm:ss");

	_la = laMouseWheel;
	QCustomPlot::wheelEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::mouseMoveEvent(QMouseEvent* event) {
	_la = laMouseMove;
	QCustomPlot::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::mouseReleaseEvent(QMouseEvent* event) {
	_la = laMouseRelease;
	QCustomPlot::mouseReleaseEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::decreaseTimespan(const qreal& time) {
	_la = laKeyPressed;
	setTimeWindow(Time((double) _tw.startTime() + time), Time((double) _tw.endTime() - time));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::increaseTimespan(const qreal& time) {
	_la = laKeyPressed;
	setTimeWindow(Time((double) _tw.startTime() - time), Time((double) _tw.endTime() + time));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::setTimeWindow(const Seiscomp::Core::Time& start,
                             const Seiscomp::Core::Time& end) {
	_tw = TimeWindow(start, end);
	_timeScale->setRange((double) start, (double) end + _timeMargin);
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {
	_tw = tw;
	_timeScale->setRange((double) tw.startTime(), (double) tw.endTime() + _timeMargin);
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeLine::newTimeWindow(const QCPRange& range) {
	_tw = TimeWindow(Time(range.lower), Time(range.upper));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewer::Settings::Settings() :
		streamTraceColor(Misc::defaultTraceColor),
		streamBackgroundColor(Misc::defaultTraceBackgroundColor) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewer::RecordViewer(QWidget* parent, Qt::WFlags f,
                           const NormalizeAmplitudePolicy& ct,
                           const WidgetMode& wm, const Settings& set) :
		QWidget(parent, f), _sc(NULL), _scrollArea(NULL), _streamLayout(NULL),
		_settings(set), _nap(ct), _wm(wm), _ssp(StreamsNotSelectable) {

	setMouseTracking(true);
	setupLayout();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewer::~RecordViewer() {
	if ( _scrollArea ) delete _scrollArea;
	_scrollArea = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::resizeEvent(QResizeEvent* event) {

	if ( _scrollArea ) {
		QSize ws;
		for (RecordEntityMap::iterator it = _recordEntities.begin();
		        it != _recordEntities.end(); ++it) {
			ws += it->second.rect()->size();
		}
		_hSpacer->setVisible(_scrollArea->size().height() < ws.height());
	}

	QWidget::resizeEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::setupLayout() {

	resize(640, 480);

	QGridLayout* gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(0);
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setObjectName("gridLayout");

	_sc = new Container(this);
	_sc->setObjectName("streamContainer");
	_sc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	if ( _wm == NoScroll )
		gridLayout->addWidget(_sc, 0, 0, 1, 1);
	else {
		_scrollArea = new ScrollArea;
		_scrollArea->setWidgetResizable(true);
		_scrollArea->setWidget(_sc);
		_scrollArea->setFrameShadow(QFrame::Plain);
		_scrollArea->setContentsMargins(0, 0, 0, 0);
		gridLayout->addWidget(_scrollArea, 0, 0, 1, 1);
	}

	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->setMargin(0);
	hLayout->setParent(gridLayout);

	_timeLine = new TimeLine(this);
	_timeLine->setObjectName("streamTimeLine");
	hLayout->addWidget(_timeLine);

	_hSpacer = new QWidget(this);
	_hSpacer->setMinimumSize(QSize(0, 0));
	_hSpacer->setMaximumSize(QSize(15, 0));
	_hSpacer->resize(QSize(15, 0));
	hLayout->addWidget(_hSpacer);

	gridLayout->addItem(hLayout, 1, 0, 1, 1);

	connect(_timeLine->ruler(), SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(timeLineRangeChanged(const QCPRange&)));
	connect(_timeLine, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(timeLineReleased(QMouseEvent*)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::setupRecordEntities() throw (RecordEntityException) {

	if ( !_sc )
	    throw RecordEntityException("The layout isn't initialized!");

	if ( _streamLayout ) delete _streamLayout;

	_streamLayout = new QGridLayout(_sc);
	_streamLayout->setSpacing(0);
	_streamLayout->setContentsMargins(0, 0, 0, 0);
	_streamLayout->setObjectName("streamContainerLayout");

	int idx = 0;
	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it, ++idx) {

		it->second.plot()->setObjectName(it->first);
		it->second.plot()->plotLayout()->setAutoMargins(QCP::msNone);
		it->second.plot()->plotLayout()->setMargins(QMargins(leftPadding, 0, rightPadding, -5));
		it->second.plot()->axisRect(0)->setMinimumMargins(QMargins(0, 0, 0, 0));
		it->second.plot()->yAxis->setVisible(false);
		it->second.plot()->xAxis->setTicks(false);
		it->second.plot()->xAxis->grid()->setPen(QPen(Misc::gridColor));
		it->second.plot()->xAxis->setTickLabels(false);
		it->second.plot()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(it->second.plot(), SIGNAL(customContextMenuRequested(QPoint)), this, SIGNAL(contextMenuRequest(QPoint)));
		connect(it->second.plot(), SIGNAL(mousePress(QMouseEvent*)), this, SLOT(streamClicked(QMouseEvent*)));

		it->second.setRectAxis(it->second.plot()->axisRect(0));

		it->second.rect()->setBackground(Misc::waitingForStreamDataColor);
		it->second.trace()->setPen(QPen(_settings.streamTraceColor));
		it->second.trace()->setAntialiased(false);
		it->second.trace()->setAntialiasedFill(false);

		if ( _wm == NoScroll )
		    it->second.label()->setVisible(false);

		if ( it == _recordEntities.begin() )
			it->second.setPosition(RecordEntity::FirstFromTop);
		else if ( it == _recordEntities.end() )
			it->second.setPosition(RecordEntity::Last);
		else
			it->second.setPosition(RecordEntity::Middle);

		it->second.setValid(true);
		if ( it->second.type() == RecordEntity::Frequency )
			it->second.plot()->setMinimumHeight(recordHeight * 3);
		else
			it->second.plot()->setMinimumHeight(recordHeight);

		_streamLayout->addWidget(it->second.plot(), idx, 0, 1, 1);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordEntity& RecordViewer::
recordEntity(const QString& name) throw (RecordEntityException) {

	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it)
		if ( it->first == name )
		    return it->second;

	throw RecordEntityException(QString("RecordEntity %1 not found").arg(name).toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewer::
removeRecordEntity(const QString& name) throw (RecordEntityException) {

	if ( name.isEmpty() )
	    throw RecordEntityException("RecordEntity name passed as argument is empty");

	bool retCode = false;
	int id = -1, i = 0;

	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it, ++i) {
		if ( it->first == name ) {
			_streamLayout->removeWidget(it->second.plot());
			it->second.destroy();
			id = i;
			retCode = true;
		}
	}

	if ( id != -1 )
	    _recordEntities.removeAt(id);

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int RecordViewer::removeAllRecordEntities() {

	int count = 0;

	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it, ++count) {
		_streamLayout->removeWidget(it->second.plot());
		it->second.destroy();
	}

	_recordEntities.clear();

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::decreaseVisibleTimespan(const qreal& time) {
	_timeLine->decreaseTimespan(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::increaseVisibleTimespan(const qreal& time) {
	_timeLine->increaseTimespan(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::moveVisibleRegion(const qreal& time,
                                     const RecordViewer::StreamMovement& direction) {
	switch ( direction ) {
		case Left:
			_timeLine->setTimeWindow(Time((double) _timeLine->timeWindow().startTime() - time),
			    Time((double) _timeLine->timeWindow().endTime() - time));
		break;
		case Right:
			_timeLine->setTimeWindow(Time((double) _timeLine->timeWindow().startTime() + time),
			    Time((double) _timeLine->timeWindow().endTime() + time));
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const float RecordViewer::
minAmplitude(const QString& name) throw (RecordEntityException) {
	return recordEntity(name).visibleAmpMin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const float RecordViewer::
maxAmplitude(const QString& name) throw (RecordEntityException) {
	return recordEntity(name).visibleAmpMax();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::timeLineRangeChanged(const QCPRange& range) {

	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it) {
		it->second.bottomAxis()->setRange(range);

		QVector<double> vrange;
		for (QCPDataMap::iterator itm = it->second.trace()->data()->begin();
		        itm != it->second.trace()->data()->end(); ++itm)
			if ( itm.value().key > it->second.bottomAxis()->range().lower
			        && itm.value().key < it->second.bottomAxis()->range().upper )
			    vrange.append(itm.value().value);

		it->second.setVisibleAmpMax(Math::getMax(vrange));
		it->second.setVisibleAmpMin(Math::getMin(vrange));

		if ( (_nap & onTheFly) || (_timeLine->lastAction() == TimeLine::laMouseWheel)
		        || (_timeLine->lastAction() == TimeLine::laKeyPressed) ) {

			if ( it->second.type() == RecordEntity::Frequency )
				it->second.leftAxis()->setRange(spectrumMinFreq, spectrumMaxFreq);
			else {
				it->second.trace()->valueAxis()->setRange(Math::getMin(vrange), Math::getMax(vrange));
				it->second.rescaleItems();
			}
		}
	}

	emit requestPickerUpdate();

	updatePlots();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::timeLineReleased(QMouseEvent* event) {

	Q_UNUSED(event);

	if ( _timeLine->lastAction() != TimeLine::laMouseRelease ) return;

	if ( _nap & onTheFly ) return;

	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it) {

		QVector<double> range;
		for (QCPDataMap::const_iterator itm = it->second.trace()->data()->begin();
		        itm != it->second.trace()->data()->end(); ++itm)
			if ( itm.value().key > it->second.bottomAxis()->range().lower
			        && itm.value().key < it->second.bottomAxis()->range().upper )
			    range.append(itm.value().value);

		if ( it->second.type() == RecordEntity::Frequency ) {
			it->second.leftAxis()->setRange(spectrumMinFreq, spectrumMaxFreq);
			it->second.rescaleItems(spectrumMinFreq, spectrumMaxFreq);
		}
		else {
			it->second.trace()->valueAxis()->setRange(Math::getMin(range), Math::getMax(range));
			it->second.rescaleItems(Math::getMin(range), Math::getMax(range));
		}
	}

	updatePlots();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::streamClicked(QMouseEvent* event) {

	if ( event->button() != Qt::LeftButton || _ssp != StreamsSelectable )
	    return;

	QObject* o = sender();

	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it) {
		if ( it->second.plot()->objectName() == o->objectName() )
			it->second.setSelected(true);
		else
			it->second.setSelected(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::
addRecordEntity(const QString& name) throw (RecordEntityException) {

	if ( !_sc )
	    throw RecordEntityException("The widget hasn't been initialized in a proper way!\n"
		    "setupLayout() has to be called first...");

	if ( name.isEmpty() )
	    throw RecordEntityException("Can't add a nameless RecordEntity");

	bool found = false;
	for (int i = 0; i < _recordEntities.size(); ++i)
		if ( _recordEntities.at(i).first == name )
		    found = true;

	if ( !found )
	    _recordEntities.append(NamedRecordEntity(name, RecordEntity(new QCustomPlot(_sc), name)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewer::updatePlots() {

	emit working();
	for (RecordEntityMap::iterator it = _recordEntities.begin();
	        it != _recordEntities.end(); ++it)
		it->second.plot()->replot();
	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}// namespace Gui
} // namespace IPGP
