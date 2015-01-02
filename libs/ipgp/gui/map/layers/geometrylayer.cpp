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

#include <ipgp/gui/map/layers/geometrylayer.h>
#include <ipgp/gui/map/tile.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/geometries/line.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/math/math.h>

#include <QPainter>
#include <QString>
#include <QPointF>
#include <QRect>
#include <QPixmap>
#include <QEvent>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <QDebug>

namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeometryLayer::GeometryLayer(const QString& name, const QString& desc,
                             const bool& visible, const bool& antialiasing) :
		Layer(Layer::Layer_Geometry, name, desc, visible, antialiasing) {

	connect(this, SIGNAL(mouseAtPosition(QEvent*)), this, SLOT(mouseOnGeometry(QEvent*)));
	connect(this, SIGNAL(mouseClicked(QMouseEvent*)), this, SLOT(geometryClicked(QMouseEvent*)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeometryLayer::~GeometryLayer() {
	removeGeometries();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::draw(QPainter& painter, const QPointF& startTile,
                         const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	for (int i = 0; i < _geometries.size(); i++)
		if ( _geometries.at(i)->isVisible() )
			_geometries.at(i)->draw(painter, startTile, viewport, zoom);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::mouseMoveEvent(QMouseEvent* event) {

	emit restoreElementsSizes();

	const Geometry* d = geometryAt(event->pos());

	if ( d ) {

		//! Advise the widget that this element should be highlighted,
		//! it will deal with this notice the way it wants to...
		emit highlightElement(d->name());

		if ( !d->toolTip().isEmpty() )
			QToolTip::showText(event->globalPos(), d->toolTip());
		else
			QToolTip::showText(event->globalPos(), d->name());
	}
	else {
		QToolTip::hideText();
		event->ignore();
//		emit restoreElementsSizes();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::removeGeometry(Geometry* geometry) {

	if ( !_geometries.contains(geometry) ) {
		qDebug() << Q_FUNC_INFO << "geometry not in list:"
		         << reinterpret_cast<quintptr>(geometry);
		return;
	}

	delete geometry;
	_geometries.removeOne(geometry);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::removeGeometries() {

	int c = _geometries.size();
	for (int i = c - 1; i >= 0; --i)
		removeGeometry(_geometries.at(i));

//	qDeleteAll(_geometries);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::removeGeometries(const Geometry::Type& type) {

	int c = _geometries.size();
	for (int i = c - 1; i >= 0; --i)
		if ( _geometries.at(i)->type() == type )
			removeGeometry(_geometries.at(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::removeGeometry(const Geometry::Type& type,
                                   const QString& name) {

	int c = _geometries.size();
	for (int i = c - 1; i >= 0; --i)
		if ( _geometries.at(i)->name() == name && _geometries.at(i)->type() == type )
			removeGeometry(_geometries.at(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::removeGeometry(const QString& name) {

	int c = _geometries.size();
	for (int i = c - 1; i >= 0; --i)
		if ( _geometries.at(i)->name().contains(name) )
			removeGeometry(_geometries.at(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeometryLayer::addGeometry(Geometry* drawable) {

	if ( !_geometries.contains(drawable) ) {
		_geometries.append(drawable);
		return true;
	}
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool GeometryLayer::contains(const QString& publicID) const {

	for (int i = 0; i < _geometries.size(); ++i)
		if ( _geometries.at(i)->name() == publicID )
			return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geometry* GeometryLayer::geometryAt(const QPoint& point) {

	for (int i = 0; i < _geometries.size(); i++) {

		switch ( _geometries.at(i)->type() ) {

			case Geometry::Line: {

				Line* line = dynamic_cast<Line*>(_geometries.at(i));

				if ( !line )
					continue;

				QString eq;
				bool found = Math::pointLiesOnLine(point, line->screenStart(), line->screenEnd(), &eq);

				if ( found )
					return _geometries.at(i);
			}
			break;

			case Geometry::d_Epicenter: {

				Epicenter* epicenter = dynamic_cast<Epicenter*>(_geometries.at(i));

				if ( !epicenter )
					continue;

				//! Enlarge path's ellipse so it can contain at least 3 times
				//! the epicenter's screen size (taking the pen width in account)...
				QPainterPath path;
				path.addEllipse(epicenter->screenPosition().x() - epicenter->screenSize().width() * 2,
				    epicenter->screenPosition().y() - epicenter->screenSize().height() * 2,
				    epicenter->screenSize().width() * 3, epicenter->screenSize().height() * 3);

				if ( path.contains(point) )
					return _geometries.at(i);
			}
			break;

			default: {

				//! Over-scanning a lil'bit to ensure tooltips are displayed
				//! when mouse is genuinely at the point which is relative to
				//! the closeness of the converted lat/lon into screen coordinates.
				//! Therefore we use a factor 2 since we've displaced the point
				//! coordinates by its width and height...

				QPainterPath path;
				path.addEllipse(_geometries.at(i)->screenPosition().toPoint().x() - _geometries.at(i)->size().width(),
				    _geometries.at(i)->screenPosition().toPoint().y() - _geometries.at(i)->size().height(),
				    _geometries.at(i)->size().width() * 2, _geometries.at(i)->size().height() * 2);

				if ( path.contains(point) )
					return _geometries.at(i);
			}
			break;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::mouseOnGeometry(QEvent* event) {

	if ( event->type() == QEvent::ToolTip ) {

		QHelpEvent* helpEvent = static_cast<QHelpEvent *>(event);
		const Geometry* d = geometryAt(helpEvent->pos());

		if ( d ) {

			//! Advise the widget that this element should be highlighted,
			//! it will deal with this notice the way it wants to...
			emit highlightElement(d->name());

			if ( !d->toolTip().isEmpty() )
				QToolTip::showText(helpEvent->globalPos(), d->toolTip());
			else
				QToolTip::showText(helpEvent->globalPos(), d->name());
		}
		else {
			QToolTip::hideText();
			event->ignore();
			emit restoreElementsSizes();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeometryLayer::geometryClicked(QMouseEvent* event) {

	const Geometry* d = geometryAt(event->pos());

	if ( d )
		emit elementClicked(d->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Map
} // namespace Gui
} // namespace IPGP
