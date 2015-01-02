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


#include <ipgp/gui/map/decorators/scaledecorator.h>
#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/map/config.h>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QPointF>
#include <QRect>


static const int TEXTPADDING = 10;
static const int GENERALPADDING = 10;


namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ScaleDecorator::ScaleDecorator(MapWidget* map, const bool& visible,
                               const bool& antialiasing) :
		MapDecorator(map), _antialiased(antialiasing), _visible(visible),
		_position(BottomRight) {

	_pen.setColor(Qt::black);
	_font.setPointSize(9);
	_distances << 5000000 << 2000000 << 1000000 << 1000000 << 1000000 << 100000
	           << 100000 << 50000 << 50000 << 10000 << 10000 << 10000 << 1000
	           << 1000 << 500 << 200 << 100 << 50 << 25;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ScaleDecorator::~ScaleDecorator() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ScaleDecorator::paintEvent(QPainter& painter) {

	if ( !_visible )
		return;

	if ( _target->zoomLevel() >= 0 && _distances.size() > _target->zoomLevel() ) {

		QFontMetrics fm(_font);

		double line = _distances.at(_target->zoomLevel()) / pow(2, 18 - _target->zoomLevel()) / .597164;
		QString distance;
		if ( _distances.at(_target->zoomLevel()) >= 1000 )
			distance = QVariant(_distances.at(_target->zoomLevel()) / 1000).toString() + " km";
		else
			distance = QVariant(_distances.at(_target->zoomLevel())).toString() + " m";

		int boxWidth = line + fm.width(distance) + TEXTPADDING * 2;
		int boxHeight = fm.height() + TEXTPADDING / 2;

		int x, y;
		if ( _position == BottomLeft ) {
			x = GENERALPADDING;
			y = _target->size().height() - boxHeight - GENERALPADDING;
		}
		else if ( _position == BottomRight ) {
			x = _target->size().width() - boxWidth - GENERALPADDING;
			y = _target->size().height() - boxHeight - GENERALPADDING;
		}
		else if ( _position == TopLeft ) {
			x = y = GENERALPADDING;
		}
		else if ( _position == TopRight ) {
			x = _target->size().width() - boxWidth - GENERALPADDING;
			y = GENERALPADDING;
		}

		QRect r = QRect(QPoint(0, 0), QSize(boxWidth, boxHeight));
		QPoint p1(TEXTPADDING / 2, boxHeight / 2);
		QPoint p2(line + TEXTPADDING / 2, boxHeight / 2);

		painter.save();
		painter.setPen(_pen);
		painter.setFont(_font);

		// Get to the new start position
		painter.translate(QPoint(x, y));

		// Draw background
		painter.fillRect(r, QColor(255, 255, 255, 200));
		painter.drawRect(r);

		painter.setPen(_pen);

		// Draw horizontal bar
		painter.drawLine(p1, p2);

		// Draw vertical bars
		painter.drawLine(TEXTPADDING / 2, boxHeight / 3, TEXTPADDING / 2, boxHeight - boxHeight / 3);
		painter.drawLine(line + TEXTPADDING / 2, boxHeight / 3, line + TEXTPADDING / 2, boxHeight - boxHeight / 3);

		// Draw text
		painter.drawText(QPoint(line + TEXTPADDING, boxHeight - boxHeight / 3), distance);
		painter.restore();
	}

	MapDecorator::paintEvent(painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ScaleDecorator::setPosition(const Position& position) {
	_position = position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ScaleDecorator::Position& ScaleDecorator::position() const {
	return _position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ScaleDecorator::setVisible(const bool& visible) {
	_visible = visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& ScaleDecorator::isVisible() const {
	return _visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
