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


#include <ipgp/gui/map/drawables/arrival.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/math/math.h>
#include <QDebug>
#include <QPointF>
#include <QPen>
#include <QString>


namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::draw(QPainter& painter, const QPointF& startTile,
                   const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	if ( !_epicenter || !_station )
		return;

	QPointF coords;
	coord2screen(_station->geoPosition(), coords, startTile, zoom);

	if ( !viewport.contains(coords.toPoint()) )
		coord2screen(_station->geoPosition(), coords, startTile, zoom, true);

	setScreenPosition(coords);

	QPointF end = Math::intersectPoint(_epicenter->screenPosition(),
	    _epicenter->screenSize().width(), _epicenter->pen().widthF(), coords);

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());

	if ( isAutoSelectColor() ) {
		if ( phaseCode() == "P" ) {
			painter.setBrush(Misc::getResidualsColoration(residuals()));
			_station->setBrush(Misc::getResidualsColoration(residuals()));
		}
		else
			painter.setBrush(Qt::gray);
	}
	else
		painter.setBrush(brush()), _station->setBrush(brush());

	painter.setOpacity(opacity());
	painter.setPen(pen());

//	QPointF diff = end - coords;

//	QPainterPath groupPath;
//	groupPath.moveTo(coords);
//	groupPath.arcTo(coords.x(), coords.y(), end.x(), end.y(), 5 * 16, 5 * 16);
//	groupPath.lineTo(80.0, 80.0);
//	groupPath.lineTo(80.0, 40.0);
//	groupPath.closeSubpath();
//	painter.drawPath(groupPath);

	/*
	 Map::ElementPosition pos = fromAtoB(coords, end);
	 qreal offsetX = .0, offsetY = .0;
	 switch ( pos ) {
	 case Map::TopLeft:
	 offsetX += 1.;
	 offsetY += 1.;
	 break;
	 case Map::TopRight:
	 offsetX -= 1.;
	 offsetY += 1.;
	 break;
	 case Map::BottomLeft:
	 offsetX += 1.;
	 offsetY -= 1.;
	 break;
	 case Map::BottomRight:
	 offsetX -= 1.;
	 offsetY -= 1.;
	 break;
	 case Map::TopMiddle:
	 offsetX = 0.;
	 offsetY -= 1.;
	 break;
	 case Map::BottomMiddle:
	 offsetX = 0.;
	 offsetY += 1.;
	 break;
	 case Map::MiddleLeft:
	 offsetX += 1.;
	 offsetY = 0.;
	 break;
	 case Map::MiddleRight:
	 offsetX -= 1.;
	 offsetY = 0.;
	 break;
	 }

	 bool linkPainted = false;
	 QPointF start = coords;
	 QPointF pitstop;
	 pitstop.setX(coords.x() + offsetX);
	 pitstop.setY(coords.y() + offsetY);
	 while ( linkPainted ) {
	 painter.drawLine(start, pitstop);
	 if ( start == end )
	 linkPainted = true;
	 else {
	 start.setX(start.x() + offsetX);
	 start.setY(start.y() + offsetY);
	 pitstop.setX(pitstop.x() + offsetX);
	 pitstop.setY(pitstop.y() + offsetY);
	 }
	 }

	 QPointF diff = coords - end;

	 painter.drawLine(coords, diff);
	 painter.drawLine(diff,end);
	 */

	painter.drawLine(coords, end);
	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Epicenter* Arrival::epicenter() {
	return _epicenter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setEpicenter(Epicenter* epicenter) {
	_epicenter = epicenter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& Arrival::phaseCode() const {
	return _phaseCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setPhaseCode(const QString& phaseCode) {
	_phaseCode = phaseCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal& Arrival::residuals() const {
	return _residuals;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setResiduals(const qreal& residuals) {
	_residuals = residuals;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Arrival::isAutoSelectColor() const {
	return _autoSelectColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setAutoSelectColor(const bool& autoSelectColor) {
	_autoSelectColor = autoSelectColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station* Arrival::station() {
	return _station;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Arrival::setStation(Station* station) {
	_station = station;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}
