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

#include <ipgp/gui/map/canvas.h>
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
Canvas::Canvas() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Canvas::~Canvas() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::mouseMoveEvent(QMouseEvent*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QList<Layer*>& Canvas::layers() const {
	return _layers;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::regradeLayers() {

	QList<Layer*> newList;

	QListIterator<Layer*> itTile(_layers);
	QListIterator<Layer*> itPlate(_layers);
	QListIterator<Layer*> itGeometry(_layers);
	QListIterator<Layer*> itDrawable(_layers);
	QListIterator<Layer*> itLegend(_layers);

	while ( itTile.hasNext() ) {
		Layer* l = itTile.next();
		if ( l->type() == Layer::Layer_Tile )
			newList << l;
	}

	while ( itPlate.hasNext() ) {
		Layer* l = itPlate.next();
		if ( l->type() == Layer::Layer_Plate )
			newList << l;
	}

	while ( itGeometry.hasNext() ) {
		Layer* l = itGeometry.next();
		if ( l->type() == Layer::Layer_Geometry )
			newList << l;
	}

	while ( itDrawable.hasNext() ) {
		Layer* l = itDrawable.next();
		if ( l->type() == Layer::Layer_Drawable )
			newList << l;
	}

	while ( itLegend.hasNext() ) {
		Layer* l = itLegend.next();
		if ( l->type() == Layer::Layer_Legend )
			newList << l;
	}

	_layers = newList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::addLayer(Layer* layer) {

	_layers.append(layer);

	connect(this, SIGNAL(mouseAtPosition(QEvent*)), layer, SIGNAL(mouseAtPosition(QEvent*)));
	connect(this, SIGNAL(clickOnCanvas(QMouseEvent*)), layer, SIGNAL(mouseClicked(QMouseEvent*)));
	connect(layer, SIGNAL(elementClicked(const QString&)), this, SIGNAL(elementClicked(const QString&)));

	regradeLayers();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::removeLayer(Layer* layer) {

	for (int i = 0; i != _layers.size(); ++i)
		if ( layer == _layers.at(i) ) {
			disconnect(layer);
			delete _layers.takeAt(i);
		}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::raiseLayer(Layer* layer) {

	for (int i = 0; i < _layers.size(); i++)
		if ( _layers.at(i) == layer && i != 0 )
			_layers.swap(i, i - 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::lowerLayer(Layer* layer) {

	for (int i = 0; i < _layers.size(); i++)
		if ( _layers.at(i) == layer && i != _layers.size() )
			_layers.swap(i - 1, i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::deleteLayers() {

	for (int i = 0; i < _layers.size(); i++) {
		disconnect(_layers.at(i));
		delete _layers.takeAt(i);
	}

	_layers.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Map
} // namespace Gui
} // namespace IPGP
