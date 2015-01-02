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


#include <ipgp/gui/map/drawables/pin.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/misc/misc.h>
#include <QFontMetrics>
#include <QFont>
#include <QRect>
#include <QPainter>
#include <QPointF>
#include <QTextDocument>
#include <math.h>


using namespace IPGP::Gui::Misc;


namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::draw(QPainter& painter, const QPointF& middleTile,
               const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF coords;
	coord2screen(geoPosition(), coords, middleTile, zoom);

	// Cross section will not get drawn if the POI isn't visible ;)
	if ( !viewport.contains(coords.toPoint()) )
		return;

	setScreenPosition(coords);

	painter.save();

	QPoint drawingCoords;
	if ( !_pix.isNull() ) {
		switch ( _pinType ) {
			case PT_GoogleDrop:
				// See 'red-pin-hi1.png'...
				drawingCoords.setX(coords.x() - _pixSize.width() / 4);
				drawingCoords.setY(coords.y() - _pixSize.height());
			break;
			default:
				// The pixmap is painted on middle top
				drawingCoords.setX(coords.x() - _pixSize.width() / 2);
				drawingCoords.setY(coords.y() - _pixSize.height());
			break;
		}

		painter.drawPixmap(drawingCoords.x(), drawingCoords.y(), _pixSize.width(),
		    _pixSize.height(), _pix);
	}


	if ( _legendEnabled ) {

		painter.setPen(pen());
		painter.setBrush(brush());
		painter.setOpacity(opacity());
		painter.setRenderHint(QPainter::Antialiasing, isAntialiased());

		QFont font;
		QFontMetrics fm(font);

		QStringList sl = toolTip().split("<br/>");
		int maxChar = 0;
		QString maxString = "";
		for (int i = 0; i < sl.size(); ++i)
			if ( sl.at(i).length() > maxChar )
				maxChar = sl.at(i).length(), maxString = sl.at(i);

		int width = fm.width(maxString);
		width /= 1.7;
		int height = fm.height() * sl.size() + _rectSizeFactor * 2;

		QRect r = QRect(0, 0, width, height);
		QPixmap pix(width + 1, height + 1);
		pix.fill(Qt::transparent);

		QPainter pixPaint(&pix);
		pixPaint.setBrush(brush());
		pixPaint.drawRoundRect(r, 5, 5);
		pixPaint.setPen(pen());
		pixPaint.setOpacity(opacity());

		// Using QTextDocument will allow html text
		QTextDocument doc;
		doc.setUndoRedoEnabled(false);
		doc.setHtml(toolTip());
		doc.setTextWidth(r.width());
		doc.setUseDesignMetrics(true);
		doc.setDefaultTextOption(QTextOption(_textAlignement));
		doc.drawContents(&pixPaint, r);

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

		p1.setX(p1.x() + 6);
		p2.setX(p2.x() + 6);
		p1.setY(p1.y() + 5);
		p2.setY(p2.y() + 5);

		if ( _shadowEnabled ) {
			QPointF a = drawingCoords + p1;
			QPointF b = drawingCoords + p2;
			drawShadowedPixmap(&painter, pix, a.x(), a.y());
			drawShadowedPixmap(&painter, pix2, b.x(), b.y());
		}
		else {
			painter.drawPixmap(drawingCoords + p1, pix);
			painter.drawPixmap(drawingCoords + p2, pix2);
		}
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Qt::Alignment& Pin::textAlignement() const {
	return _textAlignement;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::setTextAlignement(const Qt::Alignment& textAlignement) {
	_textAlignement = textAlignement;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Map::ElementPosition Pin::position() const {
	return _position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::setPosition(Map::ElementPosition position) {
	_position = position;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qint32& Pin::rectSizeFactor() const {
	return _rectSizeFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::setRectSizeFactor(const qint32& rectSizeFactor) {
	_rectSizeFactor = rectSizeFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPixmap& Pin::pix() const {
	return _pix;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::setPix(const QPixmap& pix, const QSize& size) {
	_pix = pix;
	_pixSize = size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& Pin::isLegendEnabled() const {
	return _legendEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::setLegendEnabled(const bool& legendEnabled) {
	_legendEnabled = legendEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& Pin::isShadowEnabled() const {
	return _shadowEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Pin::setShadowEnabled(const bool& enabled) {
	_shadowEnabled = enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}

