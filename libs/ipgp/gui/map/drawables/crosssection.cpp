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


#include <ipgp/gui/map/drawables/crosssection.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/map/layer.h>
#include <ipgp/gui/misc/misc.h>
#include <QDebug>
#include <QRectF>
#include <QPolygon>
#include <QPixmap>
#include <math.h>


using namespace IPGP::Gui::Misc;


namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection::CrossSection() :
		_trenchkm(0), _depthMinkm(0), _depthMaxkm(0), _startPointF(.0, .0),
		_endPointF(.0, .0), _startPoint(0, 0), _endPoint(0, 0), _distancekm(.0),
		_textA("A"), _textB("B"), _shadow(false) {

	setType(Geometry::d_CrossSection);

	_startEndPen.setColor(Qt::cyan);
	_startEndPen.setWidth(2);
	_startEndPen.setStyle(Qt::DashLine);

	_trenchPen.setWidth(2);
	_trenchPen.setColor(QColor(255, 0, 255));
#ifdef __APPLE__
	_trenchPen.setStyle(Qt::DashLine);
#else
	_trenchPen.setStyle(Qt::DotLine);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::draw(QPainter& painter, const QPointF& startTile,
                        const QRect& viewport, const int& zoom) {

	painter.save();

	QPointF start, end;

	if ( !coord2screen(startPointF(), start, startTile, zoom) )
		return;

	if ( !coord2screen(endPointF(), end, startTile, zoom) )
		return;

	if ( !viewport.contains(start.toPoint()) )
		if ( !coord2screen(startPointF(), start, startTile, zoom, true) )
			return;

	if ( !viewport.contains(end.toPoint()) )
		if ( !coord2screen(endPointF(), end, startTile, zoom, true) )
			return;

	double screenTrench = trench() / WGS84CELLSIZE / pow(2, 18 - zoom) * 1000;

	painter.setPen(_startEndPen);
	painter.drawLine(start, end);
	painter.setPen(pen());

	QFont font;
	font.setBold(true);

	QFontMetrics fm(font);

	// A text reference
	int width = fm.width(_textA) + 6;
	int height = fm.height() + 2;
	QRect r = QRect(0, 0, width, height);

	QPixmap pixA(width + 1, height + 1);
	pixA.fill(Qt::transparent);
	QPainter pixAPaint(&pixA);
	pixAPaint.setBrush(Qt::white);
	pixAPaint.drawRoundRect(r, 5, 5);
	pixAPaint.setPen(pen());
	pixAPaint.setFont(font);
	pixAPaint.setOpacity(opacity());
	pixAPaint.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter,
	    fm.elidedText(_textA, Qt::ElideRight, r.width()));

	QPointF a = start - QPointF(width, height / 2);

	// Draw a background for A
//	QPoint ablur = QPoint(start.x() - width, start.y() - height);
//	qreal factor = 2.;
//	QRadialGradient radialGrad(start, width * factor);
//	radialGrad.setColorAt(0, Qt::white);
//	radialGrad.setColorAt(0.5, pen().color());
//	radialGrad.setColorAt(1, Qt::transparent);
//	QRect rect_radial = QRect(ablur, QSize(width * factor, height * factor));
//	painter.fillRect(rect_radial, radialGrad);

	if ( _shadow )
		drawShadowedPixmap(&painter, pixA, a.x(), a.y());
	else
		painter.drawPixmap(a, pixA);

	// B text reference
	width = fm.width(_textB) + 6;
	height = fm.height() + 2;
	r = QRect(0, 0, width, height);

	QPixmap pixB(width + 1, height + 1);
	pixB.fill(Qt::transparent);
	QPainter pixBPaint(&pixB);
	pixBPaint.setBrush(brush());
	pixBPaint.drawRoundRect(r, 5, 5);
	pixBPaint.setPen(pen());
	pixBPaint.setFont(font);
	pixBPaint.setOpacity(opacity());
	pixBPaint.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter,
	    fm.elidedText(_textB, Qt::ElideRight, r.width()));

	QPointF b = end + QPointF(width / 2, -height / 2);
	if ( _shadow )
		drawShadowedPixmap(&painter, pixB, b.x(), b.y());
	else
		painter.drawPixmap(b, pixB);

	painter.setPen(_trenchPen);
	painter.drawLine(start.x(), start.y() + screenTrench, end.x(), end.y() + screenTrench);
	painter.drawLine(start.x(), start.y() - screenTrench, end.x(), end.y() - screenTrench);
	painter.drawLine(start.x(), start.y() + screenTrench, start.x(), start.y() - screenTrench);
	painter.drawLine(end.x(), end.y() + screenTrench, end.x(), end.y() - screenTrench);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setTrench(const qint32& trench) {
	_trenchkm = trench;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setDepthMin(const qint32& depth) {
	_depthMinkm = depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setDepthMax(const qint32& depth) {
	_depthMaxkm = depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setStartPoint(const QPoint& startPoint) {
	_startPoint = startPoint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setEndPoint(const QPoint& endPoint) {
	_endPoint = endPoint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setStartPointF(const QPointF& startPoint) {
	_startPointF = startPoint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setEndPointF(const QPointF& endPoint) {
	_endPointF = endPoint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setDistance(const qreal& dist) {
	_distancekm = dist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qint32& CrossSection::trench() const {
	return _trenchkm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qint32& CrossSection::depthMin() const {
	return _depthMinkm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qint32& CrossSection::depthMax() const {
	return _depthMaxkm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& CrossSection::startPointF() const {
	return _startPointF;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& CrossSection::endPointF() const {
	return _endPointF;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPoint& CrossSection::startPoint() const {
	return _startPoint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPoint& CrossSection::endPoint() const {
	return _endPoint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal& CrossSection::distance() const {
	return _distancekm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setTextA(const QString& s) {
	_textA = s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& CrossSection::textA() const {
	return _textA;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setTextB(const QString& s) {
	_textB = s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& CrossSection::textB() const {
	return _textB;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setShadowEnabled(const bool& enabled) {
	_shadow = enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& CrossSection::shadowEnabled() const {
	return _shadow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
