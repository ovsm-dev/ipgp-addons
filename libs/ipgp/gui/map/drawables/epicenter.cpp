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


#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/util.h>
#include <QDebug>
#include <QPointF>
#include <QPen>
#include <math.h>

namespace IPGP {
namespace Gui {
namespace Map {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::draw(QPainter& painter, const QPointF& startTile,
                     const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	QPointF coords;
	coord2screen(geoPosition(), coords, startTile, zoom);

	if ( !viewport.contains(coords.toPoint()) )
		coord2screen(geoPosition(), coords, startTile, zoom, true);

	setScreenPosition(coords);

	//! Conversion of kilometers uncertainties values into pixels
	qreal latU = positionUncertainty().y() / WGS84CELLSIZE / pow(2, 18 - zoom) * 1000;
	qreal lonU = positionUncertainty().x() / WGS84CELLSIZE / pow(2, 18 - zoom) * 1000;

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, isAntialiased());

	//! Ensure that at least one value is true and prevent drawing useless uncertainties
	if ( lonU != .0 || latU != .0 ) {
		painter.setOpacity(uncertaintyOpacity());
		painter.setPen(uncertaintyPen());
		painter.setBrush(uncertaintyBrush());
		painter.drawEllipse(coords, lonU, latU);
	}

	qreal wsize, hsize;
	wsize = hsize = (4.9 * (magnitude() - 1.2)) / 2.;

	//! Ensure that visible size lays within Qt plotting capabilities
	if ( wsize < 2. )
		wsize = hsize = 2.;

	_screenSize = QSizeF(wsize, hsize);

	//! Draw gradient if highlight before any other drawing, this will make
	//! the drawable pop out of the scene ;)
	if ( highlight() ) {

#ifdef __APPLE__
		//! Epicenters gradient seems small and irrelevant, make it splashy
		qreal factor = 2.;
		QRadialGradient radialGrad(coords, size().width() * factor);
		radialGrad.setColorAt(0, brush().color());
		radialGrad.setColorAt(0.5, pen().color());
		radialGrad.setColorAt(1, Qt::transparent);
		QRect rect_radial(coords.x() - size().width() * factor, coords.y() - size().width() * factor,
				coords.x() + size().width() * factor, coords.y() + size().width() * factor);
		painter.fillRect(rect_radial, radialGrad);
#else
		qreal factor = 2.;
		QRadialGradient radialGrad(coords, wsize * factor);
		radialGrad.setColorAt(0, brush().color());
		radialGrad.setColorAt(0.5, pen().color());
		radialGrad.setColorAt(1, Qt::transparent);
		QRect rect_radial(coords.x() - wsize * factor, coords.y() - wsize * factor,
		    coords.x() + wsize * factor, coords.y() + wsize * factor);
		painter.fillRect(rect_radial, radialGrad);
#endif

	}

	painter.setOpacity(opacity());
	painter.setPen(pen());
	painter.setBrush(brush());
	painter.drawEllipse(coords, wsize, hsize);
	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::setPositionUncertainty(const QPointF& positionUncertainty) {
	this->_positionUncertainty = positionUncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& Epicenter::positionUncertainty() const {
	return _positionUncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::setDepth(const qreal& depth) {
	this->_depth = depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal& Epicenter::depth() const {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::setMagnitude(const qreal& mag) {
	this->_mag = mag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal& Epicenter::magnitude() const {
	return _mag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QBrush& Epicenter::uncertaintyBrush() {
	return _uncertaintyBrush;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::setUncertaintyBrush(const QBrush& uncertaintyBrush) {
	_uncertaintyBrush = uncertaintyBrush;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPen& Epicenter::uncertaintyPen() {
	return _uncertaintyPen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::setUncertaintyPen(const QPen& uncertaintyPen) {
	_uncertaintyPen = uncertaintyPen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal& Epicenter::uncertaintyOpacity() const {
	return _uncertaintyOpacity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Epicenter::setUncertaintyOpacity(const qreal& uncertaintyOpacity) {
	_uncertaintyOpacity = uncertaintyOpacity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}
}
}
