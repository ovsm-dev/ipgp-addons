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


#include <ipgp/gui/map/drawables/station.h>
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
void Station::draw(QPainter& painter, const QPointF& startTile,
                   const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF coords;
	coord2screen(geoPosition(), coords, startTile, zoom);

	if ( !viewport.contains(coords.toPoint()) )
		coord2screen(geoPosition(), coords, startTile, zoom, true);

	setScreenPosition(coords);

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());
	painter.setBrush(brush());
	painter.setOpacity(opacity());
	painter.setPen(pen());

	//! Draw gradient if highlight before any other drawing, this will make
	//! the drawable pop out of the scene ;)
	if ( highlight() ) {

		qreal factor = 2.;
		QRadialGradient radialGrad(coords, size().width() * factor);
		radialGrad.setColorAt(0, brush().color());
		radialGrad.setColorAt(0.5, pen().color());
		radialGrad.setColorAt(1, Qt::transparent);
		QRect rect_radial(coords.x() - size().width() * factor, coords.y() - size().width() * factor,
		    coords.x() + size().width() * factor, coords.y() + size().width() * factor);
		painter.fillRect(rect_radial, radialGrad);
	}

	switch ( _shape ) {
		case Station::ss_ellipse: {
			painter.drawEllipse(coords, size().width() / 2, size().height() / 2);
		}
		break;

		case Station::ss_rectangle:
			painter.drawRect(coords.x(), coords.y(), size().width(), size().height());
		break;

		case Station::ss_triangle: {

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
			tmp.setOpacity(1);
			tmp.setPen(pen());
			tmp.drawPolygon(tpoints, 4, Qt::WindingFill);

			painter.drawPixmap(coords.x() - (size().width() / 2),
			    coords.y() - (size().height() / 2),
			    size().width(), size().height(), pix);
		}
		break;
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& Station::isAutoSelectColor() const {
	return _autoSelectColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setAutoSelectColor(const bool& autoSelectColor) {
	_autoSelectColor = autoSelectColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& Station::locationCode() const {
	return _locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setLocationCode(const QString& locationCode) {
	_locationCode = locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& Station::network() const {
	return _network;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setNetwork(const QString& network) {
	_network = network;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Station::StationShape& Station::shape() const {
	return _shape;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::setShape(const Station::StationShape& shape) {
	_shape = shape;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}


