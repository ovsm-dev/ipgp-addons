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

#include <ipgp/gui/map/widgets/originwidget.h>
#include <ipgp/gui/map/geometries/line.h>
#include <ipgp/gui/map/geometries/circle.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/drawables/arrival.h>
#include <ipgp/gui/map/drawables/station.h>
#include <ipgp/gui/map/drawables/city.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/geo/geo.h>
#include <ipgp/core/math/math.h>

#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/seismology/regions.h>

#include <QListIterator>
#include <QtGui>

#define DECIMALSSHOWN 4


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math::Geo;

using namespace IPGP;
using namespace IPGP::Core;
using namespace IPGP::Core::Geo;
using namespace IPGP::Core::Math;
using namespace IPGP::Gui;
using namespace IPGP::Gui::Misc;


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginWidget::OriginWidget(const std::vector<std::string>& names,
                           const std::vector<std::string>& paths,
                           const double& lon, const double& lat,
                           QWidget* parent, Qt::WFlags f,
                           const bool& paintOrphan) :
		MapWidget(names, paths, parent, lon, lat, Map::DEFAULT_TILE_PATTERN, f),
		_epicenter(NULL), _paintOrphanOrigin(paintOrphan) {

	setWindowFlags(f);
	resize(400, 400);

	connect(this, SIGNAL(elementClicked(const QString&)), this, SLOT(mapClicked(const QString&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginWidget::~OriginWidget() {

	_epicenter = NULL;
	//! No Origin pointer has to be deleted here, this operation shall be
	//! done by the instance providing the pointer.
	//! Also, each layer will delete its children, so no Epicenter nor Arrivals
	//! deletion here either^^
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginWidget::setDatabase(DatabaseQuery* query) {
	_query = query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginWidget::setOrigin(Origin* org) {

	if ( !_query )
		return;

	EventPtr event = _query->getEvent(org->publicID());

	if ( !event ) {
		if ( !_paintOrphanOrigin )
			return;
		log(Client::LM_WARNING, QString("%1 is an orphan but has received a painting request")
		        .arg(org->publicID().c_str()));
	}

	MagnitudePtr mag = NULL;
	if ( event ) {
		mag = Magnitude::Find(event->preferredMagnitudeID());

		if ( !mag )
			mag = Magnitude::Cast(_query->getObject(Magnitude::TypeInfo(),
			    event->preferredMagnitudeID()));
	}

	clear();

	bool hasDepth = false;
	double depth = .0;
	try {
		depth = org->depth().value();
		hasDepth = true;
	} catch ( ... ) {}

	bool hasLonUncertainty = false;
	double lonUncertainty = .0;
	try {
		lonUncertainty = org->longitude().uncertainty();
		hasLonUncertainty = true;
	} catch ( ... ) {}

	bool hasLatUncertainty = false;
	double latUncertainty = .0;
	try {
		latUncertainty = org->latitude().uncertainty();
		hasLatUncertainty = true;
	} catch ( ... ) {}

	bool hasDepthUncertainty = false;
	double depthUncertainty = .0;
	try {
		depthUncertainty = org->depth().uncertainty();
		hasDepthUncertainty = true;
	} catch ( ... ) {}

	if ( org->magnitudeCount() == 0 )
		_query->loadMagnitudes(org);

	if ( org->arrivalCount() == 0 )
		_query->loadArrivals(org);


	QString dist = QString("-");
	try {
		if ( scheme().distanceInKM() )
			dist = QString("%1km").arg(QString::number(deg2km(
			    org->quality().minimumDistance()), 'f', scheme().distancePrecision()));
		else
			dist = QString::fromUtf8("%1°").arg(QString::number(
			    org->quality().minimumDistance(), 'f', scheme().distancePrecision()));
	} catch ( ... ) {}

	QString azimuth = QString("-");
	try {
		azimuth = QString::number(org->quality().azimuthalGap(), 'f', scheme().azimuthPrecision());
	} catch ( ... ) {}

	QString rms = QString("-");
	try {
		rms = QString::number(org->quality().standardError(), 'f', scheme().rmsPrecision());
	} catch ( ... ) {}

	QString magType = QString("");
	bool hasMagnitude = false;
	double magnitude;
	if ( mag ) {
		try {
			magnitude = mag->magnitude().value();
			hasMagnitude = true;
		} catch ( ... ) {}
		magType = mag->type().c_str();
	}

	QString author = "-";
	try {
		author = org->creationInfo().author().c_str();
	} catch ( ... ) {}

	Map::Epicenter* epicenter = new Map::Epicenter;
	epicenter->setGeoPosition(QPointF(org->longitude().value(), org->latitude().value()));

	if ( hasLatUncertainty && hasLonUncertainty )
		epicenter->setPositionUncertainty(QPointF(lonUncertainty, latUncertainty));

	epicenter->setToolTip(epicenterTooltip(org->publicID().c_str(),
	    org->time().value().toString("%Y-%m-%d %H:%M:%S").c_str(),
	    getStringPosition(org->latitude().value(), Latitude).c_str(),
	    (hasLatUncertainty) ? QString::number(latUncertainty, 'f', scheme().generalPrecision()) : "-",
	    getStringPosition(org->longitude().value(), Longitude).c_str(),
	    (hasLonUncertainty) ? QString::number(lonUncertainty, 'f', scheme().generalPrecision()) : "-",
	    (hasDepth) ? QString::number(depth, 'f', scheme().depthPrecision()) : "-",
	    (hasDepthUncertainty) ? QString::number(depthUncertainty, 'f', scheme().generalPrecision()) : "-",
	    rms, azimuth,
	    (hasMagnitude) ? QString::number(roundDouble(mag->magnitude().value(), 1), 'f', scheme().generalPrecision()) : "",
	    magType, dist, Regions::getRegionName(org->latitude().value(), org->longitude().value()).c_str(),
	    author));

	(hasMagnitude) ? epicenter->setMagnitude(magnitude) : epicenter->setMagnitude(.0);

	epicenter->setAntialiased(true);
	epicenter->setSize(EpicenterDefaultSize);
	(hasDepth) ? epicenter->pen().setColor(getDepthColoration(depth)) :
	        epicenter->pen().setColor(Qt::black);
	epicenter->pen().setWidthF(EpicenterDefaultPenWidth);
	epicenter->uncertaintyPen().setColor(Qt::magenta);
	epicenter->uncertaintyBrush().setColor(Qt::magenta);
	epicenter->uncertaintyBrush().setStyle(Qt::SolidPattern);
	epicenter->setName(org->publicID().c_str());

	if ( foregroundCanvas().addGeometry(epicenter) ) {

		setDefaultGeoPosition(QPointF(org->longitude().value(), org->latitude().value()));
		centerOn(QPointF(org->longitude().value(), org->latitude().value()));

		_epicenter = epicenter;
	}
	else {
		delete epicenter;
		epicenter = NULL;
	}

	if ( !epicenter )
		return;

	for (size_t i = 0; i < org->arrivalCount(); ++i) {

		ArrivalPtr ar = org->arrival(i);

		if ( !ar )
			continue;

		QString staAzimuth = QString("-");
		try {
			staAzimuth = QString::number(ar->azimuth(), 'f', scheme().azimuthPrecision());
		} catch ( ... ) {}

		QString staDistance = QString("-");
		try {
			if ( scheme().distanceInKM() )
				staDistance = QString("%1km").arg(QString::number(
				    deg2km(ar->distance()), 'f', scheme().distancePrecision()));
			else
				staDistance = QString::fromUtf8("%1°").arg(QString::number(
				    ar->distance(), 'f', scheme().distancePrecision()));
		} catch ( ... ) {}

		QString staResiduals = QString("-");
		try {
			staResiduals = QString::number(ar->timeResidual(), 'f', scheme().generalPrecision());
		} catch ( ... ) {}


		PickPtr pick = Pick::Find(ar->pickID());
		if ( !pick ) {
			pick = Pick::Cast(_query->getObject(Pick::TypeInfo(), ar->pickID()));
			if ( !pick )
				continue;
		}

		StationPtr station = _query->getStation(pick->waveformID().networkCode(),
		    pick->waveformID().stationCode(), Time::GMT());

		if ( !station )
			continue;

		QString staElevation = QString("-");
		try {
			staElevation = QString::number(station->elevation(), 'f', scheme().azimuthPrecision());
		} catch ( ... ) {}

		if ( !getMapStation(pick->waveformID().networkCode().c_str(), station->code().c_str()) ) {

			Map::Station* stationGeometry = new Map::Station();
			stationGeometry->setName(station->code().c_str());
			stationGeometry->setNetwork(pick->waveformID().networkCode().c_str());
			stationGeometry->setGeoPosition(QPointF(station->longitude(), station->latitude()));
			stationGeometry->pen().setColor(Qt::white);
			stationGeometry->setToolTip(stationTooltip(station->code().c_str(),
			    station->description().c_str(), getStringPosition(station->latitude(), Latitude).c_str(),
			    getStringPosition(station->longitude(), Longitude).c_str(), staDistance,
			    staElevation, staAzimuth, station->type().c_str()));
			stationGeometry->setSize(StationCircleDefaultSize);

			if ( !addStation(stationGeometry) ) {
				delete stationGeometry;
				stationGeometry = NULL;
			}
		}

		Map::Station* parent = getMapStation(pick->waveformID().networkCode().c_str(),
		    station->code().c_str());

		if ( parent ) {
			if ( !getMapArrival(parent->name(), ar->phase().code().c_str()) ) {

				Map::Arrival* stationArrival = new Map::Arrival();
				stationArrival->setName(ar->pickID().c_str());
				stationArrival->setEpicenter(epicenter);
				stationArrival->setStation(parent);
				stationArrival->setPhaseCode(ar->phase().code().c_str());
				stationArrival->pen().setColor(Qt::white);
				stationArrival->setOpacity(.3);
				stationArrival->setResiduals(staResiduals.toFloat());
				stationArrival->setToolTip(QString::fromUtf8("Station %1 arrival")
				        .arg(station->code().c_str()));

				if ( !addArrival(stationArrival) ) {
					delete stationArrival;
					stationArrival = NULL;
				}
			}
		}
	}


	QListIterator<Map::Arrival*> itA(_arrivals);
	while ( itA.hasNext() ) {
		Map::Arrival* a = itA.next();
		foregroundCanvas().addGeometry(a);
	}

	QListIterator<Map::Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Map::Station* s = itS.next();
		foregroundCanvas().addGeometry(s);
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginWidget::mapClicked(const QString& id) {

	if ( _epicenter->name() == id ) {
		emit epicenterClicked(id.toStdString());
		return;
	}

	QListIterator<Map::Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Map::Station* s = itS.next();
		if ( s->name() == id ) {
			emit stationClicked(id.toStdString());
			return;
		}
	}

	emit cityClicked(id);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginWidget::addCity(Map::City* city, const bool& makeLink) {

	if ( !city ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr city to map"));
		return false;
	}

	if ( makeLink && _epicenter )
		city->linkToParent(_epicenter);

	if ( !foregroundCanvas().addGeometry(city) )
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginWidget::addArrival(Map::Arrival* arrival, const bool& toCanvas) {

	if ( !arrival ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr arrival to map"));
		return false;
	}

	if ( toCanvas ) {
		if ( !foregroundCanvas().addGeometry(arrival) )
			return false;
	}
	else {
		if ( !_arrivals.contains(arrival) )
			_arrivals.append(arrival);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Map::Arrival* OriginWidget::getMapArrival(const QString& station,
                                          const QString& phaseCode) {

	QListIterator<Map::Arrival*> itA(_arrivals);
	while ( itA.hasNext() ) {
		Map::Arrival* a = itA.next();
		if ( a->station()->name() == station && a->phaseCode() == phaseCode )
			return a;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginWidget::addStation(Map::Station* station, const bool& toCanvas) {

	if ( !station ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr station to map"));
		return false;
	}

	if ( toCanvas ) {
		if ( !foregroundCanvas().addGeometry(station) )
			return false;
	}
	else {
		if ( !_stations.contains(station) )
			_stations.append(station);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Map::Station* OriginWidget::getMapStation(const QString& network,
                                          const QString& code) {

	QListIterator<Map::Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Map::Station* s = itS.next();
		if ( s->name() == code && s->network() == network )
			return s;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginWidget::setStationsVisible(const bool& visible) {

	QListIterator<Map::Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Map::Station* s = itS.next();
		s->setVisible(visible);
	}
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginWidget::setArrivalsVisible(const bool& visible) {

	QListIterator<Map::Arrival*> itA(_arrivals);
	while ( itA.hasNext() ) {
		Map::Arrival* a = itA.next();
		a->setVisible(visible);
	}
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginWidget::clear() {

	// Layers within canvas will destroy children geometries pointers
	foregroundCanvas().clearGeometries();

	// Therefore we need only clear the lists
	_arrivals.clear();
	_stations.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}




