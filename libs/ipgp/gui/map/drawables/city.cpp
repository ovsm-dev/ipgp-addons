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


#include <ipgp/gui/map/drawables/city.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/math/math.h>
#include <QDebug>
#include <QPointF>
#include <QPen>
#include <QSizeF>
#include <math.h>

namespace IPGP {
namespace Gui {
namespace Map {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void City::draw(QPainter& painter, const QPointF& startTile,
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

	if ( _epicenter ) {

		painter.setPen(brush().color());
		painter.setOpacity(.3);
		QPointF start = Math::intersectPoint(_epicenter->screenPosition(),
		    _epicenter->screenSize().width(), _epicenter->pen().widthF(), coords);
		painter.drawLine(coords, start);
	}

	painter.setOpacity(opacity());
	painter.setPen(pen());

	switch ( _shape ) {
		case City::is_ellipse: {

			painter.drawEllipse(coords, size().width(), size().height());

			//! Draw gradient around if highlight (mouse over)
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
		}
		break;
		case City::is_rectangle:
			painter.drawRect(coords.x(), coords.y(), size().width(), size().height());
		break;
		case City::is_triangle:
			{
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

	QFont font;
	QFontMetrics fm(font);
	int width = fm.width(name());
	int height = fm.height();

	QRect r;
	switch ( _position ) {
		case MiddleLeft:
			r = QRect(coords.x() + 8 - width, coords.y(), width, height);
		break;
		default:
			r = QRect(coords.x() + 8, coords.y(), width, height);
		break;
	}

	QString elidedText = fm.elidedText(name(), Qt::ElideRight, r.width());

	if ( _paintName )
		painter.drawText(r, Qt::AlignCenter, elidedText);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qint32& City::population() const {
	return _population;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void City::setPopulation(const qint32& population) {
	_population = population;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const City::IndicatorShape& City::shape() const {
	return _shape;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void City::setShape(const City::IndicatorShape& shape) {
	_shape = shape;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ElementPosition& City::position() const {
	return _position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void City::setPosition(const ElementPosition& position) {
	_position = position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& City::isPaintName() const {
	return _paintName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void City::setPaintName(const bool& paintName) {
	_paintName = paintName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void City::linkToParent(Epicenter* parent) {
	_epicenter = parent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}
}
}
