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


#include <ipgp/gui/map/geometries/circle.h>
#include <ipgp/gui/map/util.h>

#include <QPainter>
#include <QRect>
#include <QPointF>
#include <QDebug>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Circle::Circle(const Map::ElementPosition& position) :
		_position(position) {
	setType(Geometry::Circle);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Circle::~Circle() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Circle::draw(QPainter& painter, const QPointF& startTile,
                  const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF coords;

	coord2screen(geoPosition(), coords, startTile, zoom);

	if ( !viewport.contains(coords.toPoint()) )
		coord2screen(geoPosition(), coords, startTile, zoom, true);

	setScreenPosition(coords);

	qreal yu = size().height() / WGS84CELLSIZE / pow(2, 18 - zoom) * 1000;
	qreal xu = size().width() / WGS84CELLSIZE / pow(2, 18 - zoom) * 1000;

	painter.save();
	painter.setPen(pen());
	painter.setBrush(brush());
	painter.setOpacity(opacity());
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());

	if ( isSizeInKm() )
		painter.drawEllipse(coords, xu, yu);
	else
		painter.drawEllipse(coords, size().width(), size().height());

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
