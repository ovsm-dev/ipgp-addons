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

#include <QRubberBand>
#include <ipgp/gui/datamodel/squarrezoomplot.h>


namespace IPGP {
namespace Gui {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SquarreZoomPlot::SquarreZoomPlot(QWidget* parent, Qt::WFlags f) :
		QCustomPlot(parent), _rubberBand(new QRubberBand(QRubberBand::Rectangle, this)),
		_zoomMode(true) {

	setWindowFlags(f);

	QPalette pal;
	pal.setBrush(QPalette::Highlight, QBrush(Qt::blue));
	_rubberBand->setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SquarreZoomPlot::~SquarreZoomPlot() {
	//! Only delete object without parent
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SquarreZoomPlot::setZoomMode(bool mode) {
	_zoomMode = mode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& SquarreZoomPlot::zoomModeEnabled() const {
	return _zoomMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SquarreZoomPlot::mousePressEvent(QMouseEvent* event) {

	if ( _zoomMode ) {
		if ( event->button() == Qt::RightButton ) {
			_origin = event->pos();
			_xRange = xAxis->range();
			_yRange = yAxis->range();
			_rubberBand->setGeometry(QRect(_origin, QSize()));
			_rubberBand->show();
		}
		else if ( event->button() == Qt::MidButton )
			restoreOriginalRange();
	}

	QCustomPlot::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SquarreZoomPlot::mouseMoveEvent(QMouseEvent* event) {

	if ( _rubberBand->isVisible() )
		_rubberBand->setGeometry(QRect(_origin, event->pos()).normalized());

	QCustomPlot::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SquarreZoomPlot::mouseReleaseEvent(QMouseEvent* event) {

	if ( _rubberBand->isVisible() ) {
		const QRect& zoomRect = _rubberBand->geometry();
		if ( zoomRect.width() != 0 && zoomRect.height() != 0 ) {

			int xp1, yp1, xp2, yp2;
			zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);

			double x1 = xAxis->pixelToCoord(xp1);
			double x2 = xAxis->pixelToCoord(xp2);
			double y1 = yAxis->pixelToCoord(yp1);
			double y2 = yAxis->pixelToCoord(yp2);

			xAxis->setRange(x1, x2);
			yAxis->setRange(y1, y2);

			_rubberBand->hide();
			replot();
		}
	}

	QCustomPlot::mouseReleaseEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SquarreZoomPlot::restoreOriginalRange() {

	xAxis->setRange(_xRange);
	yAxis->setRange(_yRange);
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
