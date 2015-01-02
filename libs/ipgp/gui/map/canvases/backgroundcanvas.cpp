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

#include <ipgp/gui/map/canvases/backgroundcanvas.h>
#include <ipgp/gui/map/drawables/city.h>
#include <QPainter>
#include <QRect>
#include <QPointF>
#include <QListIterator>
#include <QDebug>
#include <QMouseEvent>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BackgroundCanvas::BackgroundCanvas() :
		_isDirty(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BackgroundCanvas::~BackgroundCanvas() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BackgroundCanvas::updateSettings() {

	_tileLayer.setMapSettings(mapSettings());

	QListIterator<Layer*> it(_layers);
	while ( it.hasNext() ) {
		Layer* l = it.next();
		if ( l->isVisible() )
			l->setMapSettings(mapSettings());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BackgroundCanvas::draw(QPainter& painter, const QPointF& startTile,
                            const QRect& viewport, const int& zoom) {

	_tileLayer.draw(painter, startTile, viewport, zoom);
	_plateBoundaryLayer.draw(painter, startTile, viewport, zoom);

	QListIterator<Layer*> it(_layers);
	while ( it.hasNext() ) {
		Layer* l = it.next();
		l->setDirty(_isDirty);
		if ( l->isVisible() )
			l->draw(painter, startTile, viewport, zoom);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BackgroundCanvas::setMapSettings(const MapSettings& settings) {
	_settings = settings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MapSettings& BackgroundCanvas::mapSettings() const {
	return _settings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BackgroundCanvas::setDirty(const bool& dirty) {

	_tileLayer.setDirty(dirty);
	_plateBoundaryLayer.setDirty(dirty);

	QListIterator<Layer*> it(_layers);
	while ( it.hasNext() ) {
		Layer* l = it.next();
		l->setDirty(dirty);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Map
} // namespace Gui
} // namespace IPGP
