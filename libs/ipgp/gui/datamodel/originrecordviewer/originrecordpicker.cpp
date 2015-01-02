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

#include <ipgp/gui/datamodel/originrecordviewer/originrecordpicker.h>
#include <ipgp/gui/datamodel/originrecordviewer/originrecordviewer.h>
#include <ipgp/gui/misc/misc.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/core/datetime.h>
#include <QtGui>


using namespace Seiscomp::DataModel;
using namespace Seiscomp::Core;

namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordPicker::Marker::Marker() :
		_time(-1.), _pen(QPen()), _font(QFont()),
		_type(mtUnknown), _polarity(Unknown), _ssp(QPointF()),
		_sep(QPointF()) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordPicker::Marker&
OriginRecordPicker::Marker::operator=(const OriginRecordPicker::Marker& m) {

	_time = m._time;
	_pen = m._pen;
	_font = m._font;
	_tooltip = m._tooltip;
	_code = m._code;
	_id = m._id;
	_type = m._type;
	_polarity = m._polarity;
	_ssp = m._ssp;
	_sep = m._sep;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordPicker::OriginRecordPicker(OriginRecordViewer* viewer,
                                       QWidget* parent, Qt::WFlags f) :
		QWidget(parent, f), _viewer(viewer), _mouseMarkerVisible(false),
		_refreshMethod(Default) {

	setMouseTracking(true);
	setAttribute(Qt::WA_TranslucentBackground);
	setAutoFillBackground(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordPicker::~OriginRecordPicker() {
	clearOrigins();
	clearPicks();
	_components.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::leaveEvent(QEvent* event) {

	_highLightedObjectID = "";
	_refreshMethod = Default;
	update();
	QWidget::leaveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::mouseMoveEvent(QMouseEvent* event) {

	_currentRegion.setBottomRight(event->globalPos());

	Marker* m = markerAt(event->posF());
	if ( m ) {
		_highLightedObjectID = m->ID();
		_refreshMethod = Default;
		update();
		QToolTip::showText(event->globalPos(), m->tooltip());
	}
	else {
		_highLightedObjectID = "";
		_refreshMethod = Default;
		update();
		QToolTip::hideText();
	}

	if ( _mouseMarkerVisible ) {
		_mousePosition = event->posF();
		_refreshMethod = Default;

//		for (int i = 0; i < _components.size(); ++i)
//			(_components.at(i).rect().contains(event->pos())) ?
//			        _components[i].setSelected(true) : _components[i].setSelected(false);
//		drawComponents();

		update();
	}

	QWidget::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::mouseDoubleClickEvent(QMouseEvent* event) {

	if ( _mouseMarkerVisible ) {
		addPick(event->posF());
		deactivatePickingMode();
	}

	QWidget::mouseDoubleClickEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::paintEvent(QPaintEvent* event) {

	if ( !_viewer ) return;

	QPainter painter(this);

	if ( _refreshMethod & Markers )
	    drawPicks(&painter), drawOrigins(&painter);

	if ( _refreshMethod & Tooltips )
	    drawTooltips(&painter);

	if ( _refreshMethod & RecordInformation )
	    drawRecordInformation(&painter);

	if ( _mouseMarkerVisible )
	    drawPicker(&painter);

	painter.drawPixmap(0, 0, _buffer);

	QWidget::paintEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordPicker::Component&
OriginRecordPicker::component(const QString& name) throw (PickerException) {
	for (int i = 0; i < _components.size(); ++i)
		if ( _components.at(i).name() == name )
		    return _components[i];
	throw PickerException(QString("Component %1 not found").arg(name).toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::updateComponents() {
	drawComponents();
	_refreshMethod = Default;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool OriginRecordPicker::hasPick(const QString& name) {

	for (MarkerMap::const_iterator it = _markers.constBegin();
	        it != _markers.constEnd(); ++it)
		if ( it.key() == name ) return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<OriginRecordPicker::Marker> OriginRecordPicker::picks() {

	QList<Marker> list;
	for (MarkerMap::iterator it = _markers.begin(); it != _markers.end(); ++it)
		if ( it.value()->type() == mtPick ) {
			Marker m = *it.value();
			list.append(m);
		}
	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::addPicks(const QList<OriginRecordPicker::Marker>& list) {

	for (int i = 0; i < list.size(); ++i)
		if ( _markers.find(list.at(i).ID()) == _markers.end() ) {
			Marker* m = new Marker;
			m->setID(list.at(i).ID());
			m->setCode(list.at(i).code());
			m->setFont(list.at(i).font());
			m->setPen(list.at(i).pen());
			m->setPolarity(list.at(i).polarity());
			m->setTime(list.at(i).time());
			m->setTooltip(list.at(i).tooltip());
			m->setType(list.at(i).type());
			_markers.insert(list.at(i).ID(), m);
		}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordPicker::Marker*
OriginRecordPicker::markerAt(const QPointF& point) {

	//! Markers should be detected with an horizontal margin of 4 pixels
	for (MarkerMap::iterator it = _markers.begin();
	        it != _markers.end(); ++it) {

		QRectF r(QPointF(it.value()->screenStartPoint().x() - 4., 0.),
		    QPointF(it.value()->screenEndPoint().x() + 4., it.value()->screenEndPoint().y()));

		if ( r.contains(point) )
		    return it.value();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::removeMarkers(const QStringList& keys) {
	for (int i = 0; i < keys.size(); ++i)
		_markers.remove(keys.at(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::drawPicker(QPainter* painter) {

	QFont font;
	font.setBold(true);
	QFontMetrics fm(font);
	painter->setFont(font);

	//! TODO ;)
	//! Check and see if this hack works fine with computers which clock is
	//! is set to operate on GMT time... Here on one with local time, it
	//! wouldn't appear correctly by taking the time difference properly...
	//! Answer: ah it works just fine like that ;)
	Time t = Time(_viewer->timeLine()->ruler()->pixelToCoord(_mousePosition.x())).toLocalTime();
	QString time = QString::fromStdString(t.toString("%Y-%m-%d %H:%M:%S.%2f"));

	const int wt = fm.width(_currentMarkerName) + _settings.textMargin;
	const int w = fm.width(time) + 6;
	const int h = fm.height() + 6;
	QRect r(this->width() - w - 1, 0, w, h);
	painter->save();

	QPen pen;
	for (int i = 0; i < _components.size(); ++i)
		if ( _components.at(i).rect().contains(_mousePosition.toPoint()) ) {
			pen.setStyle(Qt::DashLine);
			painter->setPen(pen);
			painter->drawRect(_components.at(i).rect());
		}

	pen.setStyle(Qt::SolidLine);
	painter->setPen(pen);
	painter->drawLine(QPoint(_mousePosition.x(), 0), QPoint(_mousePosition.x(), size().height()));
	painter->drawText(QPoint(_mousePosition.x() - wt, 15), _currentMarkerName);
	painter->setBrush(QColor("#CDFBCD"));
	painter->drawRect(r);
	painter->drawText(r, time, QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));
	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::drawPicks(QPainter* painter) {

	painter->save();

	const double y = this->height() - (this->height() / _components.size()) / 2;

	for (MarkerMap::iterator it = _markers.begin();
	        it != _markers.end(); ++it) {

		if ( it.value()->type() != mtPick ) continue;

		QFont font = it.value()->font();
		QFontMetrics fm(font);

		const double x = _viewer->timeLine()->ruler()->coordToPixel(it.value()->time());
		it.value()->setScreenStartPoint(QPointF(x, 0.));
		it.value()->setScreenEndPoint(QPointF(x, y));
		painter->setPen(it.value()->pen());
		painter->drawLine(it.value()->screenStartPoint(), it.value()->screenEndPoint());
		painter->setFont(it.value()->font());
		painter->drawText(QRect(x + _settings.textMargin, 0, fm.width(it.value()->code()), fm.height()),
		    it.value()->code(), QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));

		if ( it.value()->polarity() != Unknown ) {

			const int aw = 14;
			const int ah = aw * 2;
			QPolygon poly;
			if ( it.value()->polarity() == Positive ) {
				poly.append(QPoint(aw / 2, 1));
				poly.append(QPoint(aw - 1, ah - 1));
				poly.append(QPoint(1, ah - 1));
				poly.append(QPoint(aw / 2, 1));
			}
			else if ( it.value()->polarity() == Negative ) {
				poly.append(QPoint(1, 1));
				poly.append(QPoint(aw - 1, 1));
				poly.append(QPoint(aw / 2, ah - 1));
				poly.append(QPoint(1, 1));
			}

			QPixmap arrow = QPixmap(aw, ah);
			arrow.fill(Qt::transparent);

			QPainter p(&arrow);
			p.save();
			p.setBrush(it.value()->pen().color());
			p.setPen(it.value()->pen());
			p.drawPolygon(poly, Qt::WindingFill);
			p.restore();
			painter->drawPixmap(it.value()->screenStartPoint().x() - aw / 2,
			    fm.height(), aw, ah, arrow);

//			const int asize = 15;
//			QPolygon poly;
//			if ( it.value()->polarity() == Positive ) {
//				poly.append(QPoint(asize / 2, 1));
//				poly.append(QPoint(asize - 1, asize - 1));
//				poly.append(QPoint(1, asize - 1));
//				poly.append(QPoint(asize / 2, 1));
//			}
//			else if ( it.value()->polarity() == Negative ) {
//				poly.append(QPoint(1, 1));
//				poly.append(QPoint(asize - 1, 1));
//				poly.append(QPoint(asize / 2, asize - 1));
//				poly.append(QPoint(1, 1));
//			}

//			QPixmap arrow = QPixmap(asize, asize);
//			arrow.fill(Qt::transparent);
//
//			QPainter p(&arrow);
//			p.save();
//			p.setBrush(it.value()->pen().color());
//			p.setPen(it.value()->pen());
//			p.drawPolygon(poly, Qt::WindingFill);
//			p.restore();
//			painter->drawPixmap(it.value()->screenStartPoint().x() - asize / 2,
//			    fm.height(), asize, asize * 2, arrow);
		}
	}

	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::drawOrigins(QPainter* painter) {

	if ( !_refreshMethod & Markers ) return;

	painter->save();

	for (MarkerMap::iterator it = _markers.begin();
	        it != _markers.end(); ++it) {

		if ( it.value()->type() != mtOrigin ) continue;

		const double x = _viewer->timeLine()->ruler()->coordToPixel(it.value()->time());
		it.value()->setScreenStartPoint(QPointF(x, 0.));
		it.value()->setScreenEndPoint(QPointF(x, this->height()));
		painter->setPen(it.value()->pen());
		painter->drawLine(it.value()->screenStartPoint(), it.value()->screenEndPoint());
	}

	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::drawTooltips(QPainter* painter) {

	painter->save();

	for (MarkerMap::iterator it = _markers.begin();
	        it != _markers.end(); ++it) {

		if ( it.value()->ID() == _highLightedObjectID ) {

			QRectF r(QPointF(it.value()->screenStartPoint().x() - 4., 0.),
			    QPointF(it.value()->screenEndPoint().x() + 4., it.value()->screenEndPoint().y()));

			if ( it.value()->type() == mtPick ) {
				QPen pen;
				pen.setStyle(Qt::DotLine);
				painter->setPen(pen);
				painter->drawRect(r);
			}
			else if ( it.value()->type() == mtOrigin ) {
				QLinearGradient grad(it.value()->screenStartPoint(), it.value()->screenEndPoint());
				grad.setColorAt(.0, QColor(255, 6, 6, 50));
				grad.setColorAt(.5, Qt::transparent);
				grad.setColorAt(1., QColor(255, 6, 6, 50));
				painter->fillRect(r, grad);
			}
		}
	}

	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::drawComponents() {

	_buffer = QPixmap(this->size());
	_buffer.fill(Qt::transparent);

//	const int height = this->height() / _components.size();
//	qDebug() << "Widget size: " << this->height();

	QFont font;
	font.setBold(true);
	QFontMetrics fm(font);
	QPen pen;
	QPainter painter(&_buffer);
	painter.save();
	painter.setFont(font);

	int offset = 0;
	for (int i = 0; i < _components.size(); ++i) {

		int trueHeight = _viewer->recordEntity(_components.at(i).name()).plot()->height();
		if ( i != 0 ) offset += trueHeight;
		if ( _components.at(i).isFrequency() ) continue;

		QRect crect(0, (i == 0) ? 0 : offset, this->width() - 1, trueHeight - 1);
		_components[i].setRect(crect);

		if ( _components.at(i).isSelected() ) {
			pen.setStyle(Qt::DashLine);
			painter.setBrush(Qt::transparent);
			painter.setPen(pen);
			painter.drawRect(crect);
		}
		const int w = fm.width(_components.at(i).label()) + 10;
		const int h = fm.height() + 6;
		QRect r(0, (i == 0) ? 0 : offset, w, h);
		pen.setStyle(Qt::SolidLine);
		painter.setPen(pen);
		painter.setBrush(Qt::white);
		painter.drawRect(r);
		painter.drawText(r, _components.at(i).label(), QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::drawRecordInformation(QPainter* painter) {

//	const int height = this->height() / _components.size();

	QFont font = this->font();
	QFontMetrics fm(font);

	painter->save();
	painter->setFont(font);

	int offset = 0;
	for (int i = 0; i < _components.size(); ++i) {

		int trueHeight = _viewer->recordEntity(_components.at(i).name()).plot()->height();
		offset += trueHeight;
//		if ( i != 0 ) offset += trueHeight;
		if ( _components.at(i).isFrequency() ) continue;

//		trueHeight += offset;
//		int idx = i + 1;
		//! Minimum amplitude value
		int w = fm.width(QString::number(_components.at(i).amplitudeMin(), 'f', 1));
		int h = fm.height();
//		QRectF r(2, trueHeight * idx - h, w, h);
		QRectF r(2, offset - h, w, h);
		painter->drawText(r, QString::number(_components.at(i).amplitudeMin(), 'f', 1),
		    QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));

		//! Maximum amplitude value
		w = fm.width(QString::number(_components.at(i).amplitudeMax(), 'f', 1));
		h = fm.height();
//		r = QRectF(2, trueHeight * idx - (h * 1.8), w, h);
		r = QRectF(2, offset - (h * 1.8), w, h);
		painter->drawText(r, QString::number(_components.at(i).amplitudeMax(), 'f', 1),
		    QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));
	}

	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::addPick(const QPointF& pos) {

	MarkerMap::iterator it = _markers.find(_currentMarkerName);

	const double time = _viewer->timeLine()->ruler()->pixelToCoord(pos.x());

	if ( it == _markers.end() ) {

		QFont font;
		font.setBold(true);

		Marker* m = new Marker;
		m->setTooltip(QString("%1 pick created at %2").arg(_currentMarkerName)
		        .arg(Time(time).toLocalTime().toString("%y-%m-%d %H:%M:%S.%2f").c_str()));
		m->setTime(time);
		m->setCode(_currentMarkerName);
		m->setID(_currentMarkerName);
		m->setPen(QPen(Qt::darkBlue));
		m->setFont(font);
		m->setType(mtPick);
		_markers.insert(_currentMarkerName, m);
	}
	else
		it.value()->setTime(time);

	emit newPick(time, _currentMarkerName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::reset() {

	clearOrigins();
	clearPicks();

	for (int i = 0; i < _components.size(); ++i)
		(i == 0) ? _components[i].setLabel("Z") :
		        _components[i].setLabel(QString::number(i));

	drawComponents();

	_refreshMethod = Default;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::nativeReset() {

	clearOrigins();
	clearPicks();

	_components.clear();

	drawComponents();

	_refreshMethod = Default;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::timeLineUpdated() {

//	qDebug() << "Tickvector size: " << _viewer->timeLine()->ruler()->tickVector().size();
//	qDebug() << "Tickvector step: " << _viewer->timeLine()->ruler()->tickStep();
//	qDebug() << "SubTick count: " << _viewer->timeLine()->ruler()->subTickCount();
//
//	const double w = _viewer->timeLine()->ruler()->tickVector().size() * 10;
//	const double h = 30.;
//
//	_pickingCursor = QPixmap(w, h);
//	_pickingCursor.fill(Qt::transparent);
//
//	QPainter painter(&_pickingCursor);
//	painter.save();
//
//	//! Middle lines
//	painter.drawLine(0, h / 2, w, h / 2);
//	painter.drawLine(w / 2, 0, w / 2, h);
//	painter.restore();

	for (int i = 0; i < _components.size(); ++i) {
		try {
			_components[i].setAmplitudeMin(_viewer->recordEntity(_components.at(i).name()).visibleAmpMin());
			_components[i].setAmplitudeMax(_viewer->recordEntity(_components.at(i).name()).visibleAmpMax());
		} catch ( ... ) {}
	}

	_refreshMethod = Default;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::addPick(Pick* p, Arrival* a) {

	if ( !p || !a ) return;

	if ( _markers.find(p->publicID().c_str()) != _markers.end() )
	    return;

	QPen pen(_settings.pickAutoColor);
	QString type = QString("Automatic");
	try {
		if ( p->evaluationMode() == MANUAL )
		    pen.setColor(_settings.pickManualColor), type = QString("Manual");
	} catch ( ... ) {}

	QString weight = "-";
	try {
		weight = QString::number(a->weight());
	} catch ( ... ) {}

	QString tr = "-";
	try {
		tr = QString::number(a->timeResidual());
	} catch ( ... ) {}

	QString takeOff = "-";
	try {
		takeOff = QString::number(a->takeOffAngle());
	} catch ( ... ) {}

	MarkerPolarity pol = Unknown;
	try {
		if ( p->polarity() == POSITIVE )
			pol = Positive;
		else if ( p->polarity() == POSITIVE )
		    pol = Negative;
	} catch ( ... ) {}

	QString tooltip = QString::fromUtf8("%1 %2 pick created by %3 at %4\n"
		"Weight: %5\nTime residual: %6s\nTake off: %7°")
	        .arg(type).arg(a->phase().code().c_str()).arg(p->creationInfo().author().c_str())
	        .arg(p->time().value().toString("%Y-%m-%d %H:%M:%S").c_str())
	        .arg(weight).arg(tr).arg(takeOff);

	QFont font;
	font.setBold(true);

	Marker* m = new Marker;
	m->setTooltip(tooltip);
	m->setTime(p->time().value());
	m->setCode(a->phase().code().c_str());
	m->setID(p->publicID().c_str());
	m->setPen(pen);
	m->setFont(font);
	m->setPolarity(pol);
	m->setType(mtPick);

	_markers.insert(p->publicID().c_str(), m);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::clearPicks() {

	QStringList itms;
	MarkerMap::iterator it = _markers.begin();
	for (; it != _markers.end(); ++it) {
		if ( it.value()->type() == mtPick ) {
			itms << it.key();
			delete it.value();
			it.value() = NULL;
		}
	}

	removeMarkers(itms);
	_refreshMethod = Markers;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::addOrigin(Origin* o) {

	if ( !o ) return;

	if ( _markers.find(o->publicID().c_str()) != _markers.end() )
	    return;

	QPen pen(_settings.originColor);
	pen.setWidthF(.8);
	QString type = QString("Automatic");
	try {
		if ( o->evaluationMode() == MANUAL )
		    type = QString("Manual");
	} catch ( ... ) {}

	QString tooltip = QString("%1 origin created by %2 at %3")
	        .arg(type).arg(o->creationInfo().author().c_str())
	        .arg(o->time().value().toString("%Y-%m-%d %H:%M:%S").c_str());

	Marker* m = new Marker;
	m->setTooltip(tooltip);
	m->setID(o->publicID().c_str());
	m->setTime(o->time().value());
	m->setPen(pen);
	m->setType(mtOrigin);

	_markers.insert(o->publicID().c_str(), m);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::clearOrigins() {

	QStringList itms;
	MarkerMap::iterator it = _markers.begin();
	for (; it != _markers.end(); ++it) {
		if ( it.value()->type() == mtOrigin ) {
			itms << it.key();
			delete it.value();
			it.value() = NULL;
		}
	}

	removeMarkers(itms);
	_refreshMethod = Markers;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::addComponent(const QString& name,
                                      const QString& label,
                                      const bool& isFrequency) {

	for (int i = 0; i < _components.size(); ++i)
		if ( _components.at(i).name() == name )
		    return;

	Component c;
	c.setName(name);
	c.setLabel(label);
	c.setFrequency(isFrequency);

	_components.append(c);
	drawComponents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::removeComponent(const QString& name) {

	int id = -1;
	for (int i = 0; i < _components.size(); ++i)
		if ( _components.at(i).name() == name )
		    id = i;

	if ( id != -1 )
	    _components.removeAt(id);

	drawComponents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::resizeRequested(const QSize& size) {

	resize(size);
	drawComponents();
	_refreshMethod = Default;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::activatePickingMode(const QString& name) {

	_currentMarkerName = name;
	_mouseMarkerVisible = true;
	_refreshMethod = Default;
	setCursor(Qt::CrossCursor);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordPicker::deactivatePickingMode() {

	_mouseMarkerVisible = false;
	_currentMarkerName = "";
	_refreshMethod = Default;
	unsetCursor();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
