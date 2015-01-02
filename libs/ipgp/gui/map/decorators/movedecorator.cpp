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


#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/decorators/movedecorator.h>
#include <ipgp/gui/map/config.h>

#include <QMouseEvent>
#include <QPainter>
#include <stdio.h>
#include <math.h>
#include <QDebug>
#include <QPen>


namespace IPGP {
namespace Gui {
namespace Map {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MoveDecorator::MoveDecorator(MapWidget* map) :
		MapDecorator(map), _oldX(0), _oldY(0), _dragging(false), _enabled(true),
		_kineticEnabled(false), _refX(0), _refY(0), _offsetX(0), _offsetY(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::mousePressEvent(QMouseEvent* event) {

	if ( _enabled )
	    if ( event->buttons() & Qt::LeftButton ) {
		    _oldX = event->x();
		    _oldY = event->y();
		    _dragging = true;
		    _target->setCursor(Qt::DragMoveCursor);

		    _refX = event->pos().x();
		    _refY = event->pos().y();
	    }

	MapDecorator::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::mouseReleaseEvent(QMouseEvent* event) {

	if ( _enabled )
	    if ( (event->buttons() & Qt::LeftButton) == 0 ) {

		    _dragging = false;
		    _target->setCursor(Qt::ArrowCursor);

		    _refY = event->y();
		    _refX = event->x();
	    }

	MapDecorator::mouseReleaseEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::mouseMoveEvent(QMouseEvent* event) {

	if ( _enabled ) {

		if ( _dragging ) {

			int deltaX = -event->x() + _oldX;
			int deltaY = -event->y() + _oldY;

			_refX = event->x();
			_refY = event->y();

			_oldX = event->x();
			_oldY = event->y();

			double x;
			double y;
			_target->screen2tile(deltaX, deltaY, x, y);

			double lmin, lmax;

			switch ( _target->zoomLevel() ) {
				case 3:
					lmin = -1;
					lmax = 5.7;
				break;
				case 4:
					lmin = -1;
					lmax = 13.7;
				break;
				case 5:
					lmin = -1;
					lmax = 29.7;
				break;
				case 6:
					lmin = -1;
					lmax = 61.7;
				break;
				case 7:
					lmin = -1;
					lmax = 125.7;
				break;
				case 8:
					lmin = -1;
					lmax = 252.7;
				break;
				case 9:
					lmin = -1;
					lmax = 504.7;
				break;
				case 10:
					lmin = -1;
					lmax = 1008.7;
				break;
				case 11:
					lmin = -1;
					lmax = 2016.7;
				break;
				case 12:
					lmin = -1;
					lmax = 4032.7;
				break;
				case 13:
					lmin = -1;
					lmax = 8064.7;
				break;
				case 14:
					lmin = -1;
					lmax = 16128.7;
				break;
			}

			if ( y > lmin && y < lmax )
			    _target->setTileLocation(x, y);

		}
		else
			MapDecorator::mouseMoveEvent(event);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::mouseDoubleClickEvent(QMouseEvent* event) {
	if ( _enabled )
	    if ( event->buttons() & Qt::LeftButton )
	        emit changePositionTo(event->posF());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::setEnabled(const bool& enabled) {
	_enabled = enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::setKineticEnabled(const bool& enabled) {
	_kineticEnabled = enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::changeLongitudePosition(const qreal& value) {
	_offsetX = value;
	dragMap(_offsetX, _offsetY);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MoveDecorator::dragMap(const qreal&xpos, const qreal& ypos) {

	if ( _kineticEnabled ) {
		double x;
		double y;
		_target->screen2tile(xpos, ypos, x, y);
		_target->setTileLocation(x, y);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace Map
} // namespace Gui
} // namespace IPGP
