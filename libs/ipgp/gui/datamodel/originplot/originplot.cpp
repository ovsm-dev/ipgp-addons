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

#include <ipgp/gui/datamodel/originplot/originplot.h>
#include <ipgp/gui/client/application.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/math/geo.h>



using namespace Seiscomp;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core::Math;

static double const PLOT_MARGIN = .5;
static double const ITEM_SIZE = 3.;


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginPlot::OriginPlot(QWidget* parent, const Type& type,
                       Qt::WFlags f) :
		SquarreZoomPlot(parent, f) {

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setBackground(Qt::transparent);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	axisRect()->setBackground(Qt::white);
	axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	axisRect()->setMargins(QMargins(0, 0, 0, 0));
	xAxis->grid()->setVisible(true);
	xAxis2->setVisible(true);
	xAxis2->setTicks(false);
	xAxis2->setTickLabels(false);
	yAxis->grid()->setSubGridVisible(false);
	yAxis2->setVisible(true);
	yAxis2->setTicks(false);
	yAxis2->setTickLabels(false);
	yAxis->setLabel("Residuals");

	connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));

	setType(type);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginPlot::~OriginPlot() {
	emit nullifyQObject(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginPlot::setType(const Type& t) {

	_type = t;
	switch ( _type ) {
		case ResidualAzimuth:
			setObjectName("Residuals/Azimuths");
			xAxis->setLabel("Azimuths");
		break;
		case ResidualDistance:
			setObjectName("Residuals/Distances");
			( AppInstance->scheme().distanceInKM()) ?
			        xAxis->setLabel("Distance [km]") : xAxis->setLabel("Distance [deg.]");
		break;
		case ResidualTakeOff:
			setObjectName("Residuals/TakeOffs");
			xAxis->setLabel("Take off angle");
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginPlot::clearAll() {
	clearItems();
	clearGraphs();
	clearPlottables();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginPlot::plottableClicked(QCPAbstractPlottable* plottable,
                                  QMouseEvent* event) {
	Q_UNUSED(event);
	emit elementClicked(plottable->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginPlot::setOrigin(DatabaseQuery* query, Origin* org) {

	if ( !query || !org )
	    return;

	if ( org->arrivalCount() == 0 )
	    query->loadArrivals(org);

	clearGraphs();

	ArrivalList arrivals;

	Numbers<double> residuals;
	Numbers<double> takeOffs;
	Numbers<double> azimuths;
	Numbers<double> distances;
	for (size_t i = 0; i < org->arrivalCount(); ++i) {

		ArrivalPtr ar = org->arrival(i);

		if ( !ar ) continue;

		OriginArrival o;

		try {
			o.residuals = ar->timeResidual();
			o.hasResiduals = true;
			residuals.add(ar->timeResidual());
		} catch ( ... ) {
			o.hasResiduals = false;
			continue;
		}

		try {
			o.azimuth = ar->azimuth();
			o.hasAzimuth = true;
			azimuths.add(ar->azimuth());
		} catch ( ... ) {
			o.hasAzimuth = false;
		}

		try {
			( AppInstance->scheme().distanceInKM()) ?
			        o.distance = Seiscomp::Math::Geo::deg2km(ar->distance()) : o.distance = ar->distance();
			o.hasDistance = true;
			distances.add(o.distance);
		} catch ( ... ) {
			o.hasDistance = false;
		}

		try {
			o.takeOff = ar->takeOffAngle();
			o.hasTakeOff = true;
			takeOffs.add(ar->takeOffAngle());
		} catch ( ... ) {
			o.hasTakeOff = false;
		}

		PickPtr pick = Pick::Find(ar->pickID());
		if ( !pick ) {
			pick = Pick::Cast(query->getObject(Pick::TypeInfo(), ar->pickID()));
			if ( !pick ) continue;
		}

		try {
			if ( pick->evaluationMode() == DataModel::AUTOMATIC )
			    o.isAuto = true;
			if ( pick->evaluationMode() == DataModel::MANUAL )
			    o.isAuto = false;
		} catch ( ... ) {}

		o.phaseCode = ar->phase().code();
		o.networkCode = pick->waveformID().networkCode();
		o.stationCode = pick->waveformID().stationCode();

		arrivals << o;
	}

	//! Pen offset
	double offset = 2.;

	//! Drawn initial shapes contour
	for (ArrivalList::const_iterator it = arrivals.constBegin();
	        it != arrivals.constEnd(); ++it) {

		if ( _type == ResidualAzimuth && !(*it).hasAzimuth )
			continue;
		else if ( _type == ResidualDistance && !(*it).hasDistance )
			continue;
		else if ( _type == ResidualTakeOff && !(*it).hasTakeOff )
		    continue;

		QCPGraph* g = addGraph();
		g->setName((*it).publicID.c_str());

		if ( _type == ResidualAzimuth )
			g->addData((*it).azimuth, (*it).residuals);
		else if ( _type == ResidualDistance )
			g->addData((*it).distance, (*it).residuals);
		else if ( _type == ResidualTakeOff )
		    g->addData((*it).takeOff, (*it).residuals);

		g->setLineStyle(QCPGraph::lsNone);
		g->setPen(QPen());
		g->setScatterStyle(QCPScatterStyle(
		    ((*it).phaseCode.find("S") != std::string::npos) ?
		            QCPScatterStyle::ssSquare : QCPScatterStyle::ssCircle, ITEM_SIZE + offset));
		g->setTooltip(QString("%1.%2-%3").arg((*it).networkCode.c_str())
		        .arg((*it).stationCode.c_str()).arg((*it).phaseCode.c_str()));
		legend->removeItem(legend->itemCount() - 1);
	}


	//! Fill shapes with colors
	for (ArrivalList::const_iterator it = arrivals.constBegin();
	        it != arrivals.constEnd(); ++it) {

		if ( _type == ResidualAzimuth && !(*it).hasAzimuth )
			continue;
		else if ( _type == ResidualDistance && !(*it).hasDistance )
			continue;
		else if ( _type == ResidualTakeOff && !(*it).hasTakeOff )
		    continue;

		QColor c = ((*it).isAuto) ? Qt::red : QColor(0, 160, 0);

		QCPGraph* g = addGraph();
		g->setName((*it).publicID.c_str());

		if ( _type == ResidualAzimuth )
			g->addData((*it).azimuth, (*it).residuals);
		else if ( _type == ResidualDistance )
			g->addData((*it).distance, (*it).residuals);
		else if ( _type == ResidualTakeOff )
		    g->addData((*it).takeOff, (*it).residuals);

		g->setLineStyle(QCPGraph::lsNone);
		g->setPen(c);
		g->setScatterStyle(QCPScatterStyle(
		    ((*it).phaseCode.find("S") != std::string::npos) ?
		            QCPScatterStyle::ssFilledSquare : QCPScatterStyle::ssDisc, c, ITEM_SIZE));
		g->setTooltip(QString("%1.%2-%3").arg((*it).networkCode.c_str())
		        .arg((*it).stationCode.c_str()).arg((*it).phaseCode.c_str()));
		legend->removeItem(legend->itemCount() - 1);
	}

	switch ( _type ) {

		case ResidualDistance:
			try {
				xAxis->setRange(0, distances.max() + PLOT_MARGIN);
				yAxis->setRange(residuals.min() - PLOT_MARGIN, residuals.max() + PLOT_MARGIN);
			} catch ( ... ) {
				rescaleAxes();
			}
		break;

		case ResidualAzimuth:
			try {
				xAxis->setRange(.0, 360.);
				yAxis->setRange(residuals.min() - PLOT_MARGIN, residuals.max() + PLOT_MARGIN);
			} catch ( ... ) {
				rescaleAxes();
			}
		break;

		case ResidualTakeOff:
			try {
				xAxis->setRange(takeOffs.min() - PLOT_MARGIN, takeOffs.max() + PLOT_MARGIN);
				yAxis->setRange(residuals.min() - PLOT_MARGIN, residuals.max() + PLOT_MARGIN);
			} catch ( ... ) {
				rescaleAxes();
			}
		break;

		default:
			break;
	}

	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
