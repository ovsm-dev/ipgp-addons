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

#include <ipgp/gui/map/canvases/foregroundcanvas.h>
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
ForegroundCanvas::ForegroundCanvas() {

	connect(this, SIGNAL(mouseAtPosition(QEvent*)), &_geometryLayer, SIGNAL(mouseAtPosition(QEvent*)));
	connect(this, SIGNAL(clickOnCanvas(QMouseEvent*)), &_geometryLayer, SIGNAL(mouseClicked(QMouseEvent*)));
	connect(&_geometryLayer, SIGNAL(elementClicked(const QString&)), this, SIGNAL(elementClicked(const QString&)));
	connect(&_geometryLayer, SIGNAL(highlightElement(const QString&)), this, SLOT(highlightGeometry(const QString&)));
	connect(&_geometryLayer, SIGNAL(restoreElementsSizes()), this, SLOT(restoreGeometriesSizes()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ForegroundCanvas::~ForegroundCanvas() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::draw(QPainter& painter, const QPointF& startTile,
                            const QRect& viewport, const int& zoom) {

	_cityLayer.draw(painter, startTile, viewport, zoom);
	_geometryLayer.draw(painter, startTile, viewport, zoom);

	QListIterator<Layer*> it(_layers);
	while ( it.hasNext() ) {
		Layer* l = it.next();
		if ( l->isVisible() )
			l->draw(painter, startTile, viewport, zoom);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::mouseMoveEvent(QMouseEvent* event) {
		_geometryLayer.mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::clearGeometries() {

	_geometryLayer.removeGeometries();
	emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ForegroundCanvas::addGeometry(Geometry* geometry) {
	return _geometryLayer.addGeometry(geometry);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ForegroundCanvas::addGeometries(const QList<Geometry*>& list) {

	size_t count = 0;
	for (int i = 0; i < list.size(); i++)
		if ( _geometryLayer.addGeometry(list.at(i)) )
			++count;

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::removeGeometry(const QString& name,
                                      const bool& update) {

	_geometryLayer.removeGeometry(name);

	if ( update )
		emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::removeGeometry(const Geometry::Type& type,
                                      const QString& name, const bool& update) {

	_geometryLayer.removeGeometry(type, name);

	if ( update )
		emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::removeGeometries(const Geometry::Type& type,
                                        const bool& update) {

	_geometryLayer.removeGeometries(type);

	if ( update )
		emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool
ForegroundCanvas::containsGeometry(const QString& publicID) const {
	return _geometryLayer.contains(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::addCities(const std::vector<Seiscomp::Math::Geo::CityD>& list) {

	QList<QString> types;

	for (size_t i = 0; i < list.size(); ++i) {

		if ( !types.contains(list.at(i).category().c_str()) )
			types.append(list.at(i).category().c_str());

//		if ( list.at(i).category() == "B" ) {
//			City* city = new City();
//			city->setName(QString::fromUtf8(list.at(i).name().c_str()));
//			city->setPopulation(list.at(i).population());
//			city->setGeoPosition(QPointF(list.at(i).longitude(), list.at(i).latitude()));
//			city->setSize(QSizeF(6.,6.));
//			_cityLayer.addCity(city);
//		}
	}

	for (int i = 0; i < types.size(); i++)
		qDebug() << "Type " << types.at(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::highlightGeometry(const QString& publicID) {

	for (int i = 0; i < _geometryLayer.geometries().size(); ++i)
		if ( _geometryLayer.geometries().at(i)->name() == publicID )
			_geometryLayer.geometries().at(i)->setHighlighted(true);

	emit updateRequested();
//	emit repaintRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ForegroundCanvas::restoreGeometriesSizes() {

	for (int i = 0; i < _geometryLayer.geometries().size(); ++i)
		_geometryLayer.geometries().at(i)->setHighlighted(false);

	emit updateRequested();
//	emit repaintRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Map
} // namespace Gui
} // namespace IPGP
