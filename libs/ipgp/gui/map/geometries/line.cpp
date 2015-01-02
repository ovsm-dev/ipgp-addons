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


#include <ipgp/gui/map/geometries/line.h>
#include <ipgp/gui/map/util.h>


#include <QPainter>
#include <QRect>
#include <QPointF>
#include <QDebug>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Line::Line(const QPointF& start, const QPointF& end) :
		_start(start), _end(end) {
	setType(Geometry::Line);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Line::~Line() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Line::draw(QPainter& painter, const QPointF& startTile,
                const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF startCoords, endCoords;

	coord2screen(_start, startCoords, startTile, zoom);

	if ( !viewport.contains(startCoords.toPoint()) )
		coord2screen(_start, startCoords, startTile, zoom, true);

	coord2screen(_end, endCoords, startTile, zoom);

	if ( !viewport.contains(endCoords.toPoint()) )
		coord2screen(_end, endCoords, startTile, zoom, true);

	_screenStart = startCoords;
	_screenEnd = endCoords;

	painter.save();
	painter.setPen(pen());
	painter.setBrush(brush());
	painter.setOpacity(opacity());
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());
	painter.drawLine(startCoords, endCoords);

	//! Draw gradient around if highlight (mouse over)
	if ( highlight() ) {

		QLinearGradient linearGrad(startCoords, endCoords);
		linearGrad.setColorAt(0, brush().color());
		linearGrad.setColorAt(0.5, pen().color());
		linearGrad.setColorAt(1, brush().color());
		QRect rect_linear(startCoords.toPoint(), endCoords.toPoint());
		painter.fillRect(rect_linear, linearGrad);
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& Line::end() const {
	return _end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Line::setEnd(const QPointF& end) {
	_end = end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& Line::start() const {
	return _start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Line::setStart(const QPointF& start) {
	_start = start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
