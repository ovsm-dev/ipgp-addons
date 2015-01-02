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


#include <ipgp/gui/map/geometries/triangle.h>
#include <ipgp/gui/map/util.h>


#include <QPainter>
#include <QRect>
#include <QPointF>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Triangle::Triangle() {
	setType(Geometry::Triangle);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Triangle::~Triangle() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Triangle::draw(QPainter& painter, const QPointF& startTile,
                    const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF coords;

	coord2screen(geoPosition(), coords, startTile, zoom);

	if ( !viewport.contains(coords.toPoint()) )
		coord2screen(geoPosition(), coords, startTile, zoom, true);

	setScreenPosition(coords);

	painter.save();
	painter.setPen(pen());
	painter.setBrush(brush());
	painter.setOpacity(opacity());
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());

	static const QPoint tpoints[4] = {
	                                   QPoint(size().width() / 2, 1),
	                                   QPoint(size().width() - 1, size().width() - 1),
	                                   QPoint(1, size().width() - 1),
	                                   QPoint(size().width() / 2, 1)
	};

	QPixmap pix = QPixmap(size().width() + 1, size().height() + 1);
	pix.fill(Qt::transparent);

	QPainter tmp(&pix);
	tmp.setBrush(brush());
	tmp.setPen(pen());
	tmp.setOpacity(opacity());
	tmp.drawPolygon(tpoints, 4, Qt::WindingFill);

	painter.drawPixmap(coords.x() - (size().width() / 2),
	    coords.y() - (size().height() / 2),
	    size().width(), size().height(), pix);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
