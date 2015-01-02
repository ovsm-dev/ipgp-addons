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


#include <ipgp/gui/map/drawables/indicator.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/util.h>
#include <QFontMetrics>
#include <QFont>
#include <QRect>
#include <QPainter>
#include <QPointF>
#include <QSize>
#include <math.h>

namespace IPGP {
namespace Gui {
namespace Map {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Indicator::draw(QPainter& painter, const QPointF& middleTile,
                     const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF coords;
	coord2screen(geoPosition(), coords, middleTile, zoom);

	// Cross section will not get drawn if the POI isn't visible :]
	if ( !viewport.contains(coords.toPoint()) )
		return;

	setScreenPosition(coords);

	painter.save();
	painter.setPen(pen());
	painter.setBrush(brush());
	painter.setOpacity(opacity());
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());

	QFont font;
	QFontMetrics fm(font);

	int width = fm.width(toolTip()) + (10 * _rectSizeFactor);
	int height = fm.height() * _rectSizeFactor;
	QRect r = QRect(0, 0, width, height);

	QPixmap pix(width + 1, height + 1);
	pix.fill(Qt::transparent);

	QPainter pixPaint(&pix);
	pixPaint.setBrush(brush());
	pixPaint.drawRoundRect(r, 5, 5);
	pixPaint.setPen(pen());
	pixPaint.setOpacity(opacity());

	QString elidedText = fm.elidedText(toolTip(), _textElideMode, r.width());
	pixPaint.drawText(r, _textAlignement, elidedText);

	QPoint p1;
	QPoint p2;
	QPixmap pix2(17, 17);
	pix2.fill(Qt::transparent);
	QPainter pix2Paint(&pix2);
	pix2Paint.setBrush(brush());
	pix2Paint.setOpacity(opacity());

	switch ( _position ) {
		case Map::MiddleRight: {
			static const QPoint tpoints[4] = {
			                                   QPoint(0, 8),
			                                   QPoint(8, 0),
			                                   QPoint(8, 16),
			                                   QPoint(0, 8)
			};

			pix2Paint.drawPolygon(tpoints, 4, Qt::WindingFill);

			p1 = QPoint(pix2.width() / 2, -pix.height() / 3);
			p2 = QPoint(1, -pix2.height() / 2);
		}
		break;

		case Map::MiddleLeft: {
			static const QPoint tpoints[4] = {
			                                   QPoint(0, 0),
			                                   QPoint(8, 8),
			                                   QPoint(0, 16),
			                                   QPoint(0, 0)
			};


			pix2Paint.drawPolygon(tpoints, 4, Qt::WindingFill);

			p1 = QPoint(-(pix2.width() / 2 + pix.width()), -pix.height() / 3);
			p2 = QPoint(-pix2.width() / 2, -pix2.height() / 2);
		}
		break;

		case Map::TopMiddle: {
			static const QPoint tpoints[4] = {
			                                   QPoint(0, 0),
			                                   QPoint(16, 0),
			                                   QPoint(8, 8),
			                                   QPoint(0, 0)
			};


			pix2Paint.drawPolygon(tpoints, 4, Qt::WindingFill);

			p1 = QPoint(-(pix.width() / 2), -(pix.height() + pix2.height() / 2));
			p2 = QPoint(-(pix2.width() / 2), -pix2.height() / 2);
		}
		break;


		case Map::BottomMiddle: {
			static const QPoint tpoints[4] = {
			                                   QPoint(8, 0),
			                                   QPoint(16, 8),
			                                   QPoint(0, 8),
			                                   QPoint(8, 0)
			};


			pix2Paint.drawPolygon(tpoints, 4, Qt::WindingFill);

			p1 = QPoint(-(pix.width() / 2), pix2.height() / 2);
			p2 = QPoint(-(pix2.width() / 2), 1);
		}
		break;

		default:
			break;
	}

	painter.drawPixmap(coords + p1, pix);
	painter.drawPixmap(coords + p2, pix2);


	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Qt::Alignment& Indicator::textAlignement() const {
	return _textAlignement;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Indicator::setTextAlignement(const Qt::Alignment& textAlignement) {
	_textAlignement = textAlignement;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::TextElideMode Indicator::textElideMode() const {
	return _textElideMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Indicator::setTextElideMode(Qt::TextElideMode textElideMode) {
	_textElideMode = textElideMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Map::ElementPosition Indicator::position() const {
	return _position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Indicator::setPosition(Map::ElementPosition position) {
	_position = position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qint32& Indicator::rectSizeFactor() const {
	return _rectSizeFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Indicator::setRectSizeFactor(const qint32& rectSizeFactor) {
	_rectSizeFactor = rectSizeFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}


