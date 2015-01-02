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


#include <ipgp/gui/map/geometries/star.h>
#include <ipgp/gui/map/util.h>


#include <QPainter>
#include <QRect>
#include <QPointF>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Star::Star() {
	setType(Geometry::Star);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Star::~Star() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Star::draw(QPainter& painter, const QPointF& startTile,
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

	qreal width = size().width();
	qreal height = size().height();

	static const QPointF star[11] = {
	                                  QPoint(width / 2., 0.), // A
	                                  QPoint(width / 1.45, height / 2.5), // A'
	                                  QPoint(width, height / 2.5), // B
	                                  QPoint((width / 4) * 3, height / 3. + height / 4.), // B'
	                                  QPoint(width / 1.1, height / 1.1), // C
	                                  QPoint(width / 2., (height / 4) * 3), // C'
	                                  QPoint(width / 8, height / 1.1), // D
	                                  QPoint(width / 4, height / 3. + height / 4.), // D'
	                                  QPoint(.0, height / 2.5), // E
	                                  QPoint(width / 2.65, height / 2.5), // E'
	                                  QPoint(width / 2., 0.) // A
	};

	QPixmap pix = QPixmap(size().width() + 1, size().height() + 1);
	pix.fill(Qt::transparent);

	QPainter tmp(&pix);
	tmp.setBrush(brush());
	tmp.setPen(pen());
	tmp.setOpacity(opacity());
	tmp.drawPolygon(star, 11, Qt::WindingFill);

	painter.drawPixmap(coords.x() - (size().width() / 2),
	    coords.y() - (size().height() / 2),
	    size().width(), size().height(), pix);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
