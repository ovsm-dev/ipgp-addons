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


#include <ipgp/gui/map/decorators/griddecorator.h>
#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/map/config.h>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QPointF>
#include <QRect>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GridDecorator::GridDecorator(MapWidget* map, const bool& visible,
                             const bool& antialiasing) :
		MapDecorator(map), _antialiased(antialiasing), _visible(visible),
		_autoAdjust(true), _autoAdjustValue(500) {

	_pen.setColor(Qt::white);
#ifdef __APPLE__
	_pen.setStyle(Qt::DashLine);
#else
	_pen.setStyle(Qt::DotLine);
#endif
	_font.setPointSize(9);

	_graticules << 70 << 60 << 60 << 30 << 15 << 10 << 4 << 2 << 1 << 1 << 1 << 1 << 1 << 1 << 1;
	_moreGraticules << 60 << 40 << 20 << 20 << 10 << 4 << 2 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GridDecorator::~GridDecorator() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridDecorator::paintEvent(QPainter& painter) {

	if ( !_visible )
		return;

	painter.save();
	painter.setPen(_pen);
	painter.setFont(_font);

	QList<int> graticules = _graticules;

	if ( _autoAdjust )
		if ( _target->rect().width() < _autoAdjustValue
		    || _target->rect().height() < _autoAdjustValue )
			graticules = _moreGraticules;

	QPointF point;
	QPointF coords;

	for (int meridian = -180; meridian < 180;
	        meridian += graticules.at(_target->zoomLevel())) {

		point = QPointF(.0, meridian);

		// Latitudes
		if ( coord2screen(point, coords, _target->centralCoordinates(), _target->zoomLevel()) ) {

			if ( coords.x() != 0 )
				painter.drawLine(0, coords.y(), _target->rect().width(), coords.y());

			if ( meridian < 0 ) {
				QString text = QString("%1 S");
				painter.drawText(GRATICULE_TEXT_OFFSET, coords.y() + 15, text.arg(abs(meridian)));
			}
			else if ( meridian == 0 ) {
				_font.setBold(true);
				_font.setItalic(true);
				painter.setFont(_font);
				painter.drawText(GRATICULE_TEXT_OFFSET, coords.y() + 15, "0");
				_font.setBold(false);
				_font.setItalic(false);
				painter.setFont(_font);
			}
			else if ( meridian > 0 ) {
				QString text = QString("%1 N");
				painter.drawText(GRATICULE_TEXT_OFFSET, coords.y() + 15, text.arg(meridian));
			}
		}

		// Longitudes
		point = QPointF(meridian, .0);

		if ( coord2screen(point, coords, _target->centralCoordinates(), _target->zoomLevel()) ) {

			painter.drawLine(coords.x(), _target->rect().height(), coords.x(), 0);
			if ( meridian < 0 ) {
				QString text = QString("%1 W");
				painter.drawText(coords.x() + GRATICULE_TEXT_OFFSET, 15, text.arg(abs(meridian)));
			}
			else if ( meridian == 0 ) {
				_font.setBold(true);
				_font.setItalic(true);
				painter.setFont(_font);
				painter.drawText(coords.x() + GRATICULE_TEXT_OFFSET, 15, "0");
				_font.setBold(false);
				_font.setItalic(false);
				painter.setFont(_font);
			}
			else if ( meridian > 0 ) {
				QString text = QString("%1 E");
				painter.drawText(coords.x() + GRATICULE_TEXT_OFFSET, 15, text.arg(meridian));
			}
		}
	}
	painter.restore();

	MapDecorator::paintEvent(painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridDecorator::setGraticules(const QList<int>& graticules) {
	_graticules = graticules;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridDecorator::setMoreGraticules(const QList<int>& moreGraticules) {
	_moreGraticules = moreGraticules;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridDecorator::setMoreGraticulesEnabled(const bool& enabled,
                                             const int& value) {
	_autoAdjust = enabled;
	_autoAdjustValue = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridDecorator::setVisible(const bool& visible) {
	_visible = visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& GridDecorator::isVisible() const {
	return _visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
