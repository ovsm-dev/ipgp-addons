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

#include <ipgp/gui/map/widgets/motionmap.h>
#include <ipgp/gui/map/geometries/line.h>
#include <ipgp/gui/map/geometries/circle.h>
#include <ipgp/gui/map/drawables/epicenter.h>
//#include <ipgp/gui/map/drawables/arrival.h>
#include <ipgp/gui/map/drawables/station.h>
//#include <ipgp/gui/map/drawables/city.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/geo/geo.h>
#include <ipgp/core/math/math.h>

#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/databasequery.h>
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

namespace Map {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnimatedCanvas::AnimatedCanvas() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnimatedCanvas::~AnimatedCanvas() {
	clearGeometries();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnimatedCanvas::draw(QPainter& painter, const QPointF& centralCoords,
                          const QRect& viewport, const int& zoomLevel) {
	for (int i = 0; i < _geometries.size(); ++i)
		_geometries.at(i)->draw(painter, centralCoords, viewport, zoomLevel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnimatedCanvas::clearGeometries() {
	for (int i = 0; i < _geometries.size(); ++i) {
		delete _geometries[i];
		_geometries[i] = NULL;
	}

	for (int j = 0; j < _geometries.size(); ++j)
		if ( !_geometries[j] )
			_geometries.removeAt(j);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AnimatedCanvas::addGeometry(Geometry* g) throw (GeneralException) {

	if ( !g )
		throw GeneralException("Cannot add NULL ptr Geometry object to canvas");

	if ( !_geometries.contains(g) ) {
		_geometries.append(g);
		return true;
	}
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AnimatedCanvas::addGeometries(const QList<Geometry*>& list) {
	int count = 0;
	for (int i = 0; i < list.size(); i++)
		if ( addGeometry(list.at(i)) )
			++count;

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnimatedCanvas::removeGeometry(const QString& publicID) {
	for (int i = 0; i < _geometries.size(); ++i)
		if ( _geometries.at(i)->name() == publicID ) {
			delete _geometries[i];
			_geometries[i] = NULL;
		}

	for (int j = 0; j < _geometries.size(); ++j)
		if ( !_geometries[j] )
			_geometries.removeAt(j);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnimatedCanvas::removeGeometry(const Geometry::Type& type,
                                    const QString& publicID) {
	for (int i = 0; i < _geometries.size(); ++i)
		if ( _geometries.at(i)->type() == type )
			if ( _geometries.at(i)->name() == publicID || publicID.isEmpty() ) {
				delete _geometries[i];
				_geometries[i] = NULL;
			}

	for (int j = 0; j < _geometries.size(); ++j)
		if ( !_geometries[j] )
			_geometries.removeAt(j);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnimatedCanvas::removeGeometries(const Geometry::Type& type) {
	removeGeometry(type, "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Geometry*
AnimatedCanvas::geometry(const QString& publicID) throw (GeneralException) {

	if ( publicID.isEmpty() )
		throw GeneralException("Object publicID is empty");

	for (int i = 0; i < _geometries.size(); ++i) {
		if ( _geometries.at(i)->name() == publicID ) {
			qDebug() << "Found occurrence of object " << publicID;
			return _geometries[i];
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Map



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MotionMap::MotionMap(const MapDescriptor& md, QWidget* parent, Qt::WFlags f) :
		MapWidget(md.names(), md.paths(), parent, md.defaultLongitude(), md.defaultLatitude()),
		_query(NULL), _origin(NULL), _epicenter(NULL), _timer(new QTimer(this)),
		_animationTime(-1.), _animationRate(50) {

	setWindowFlags(f);
	resize(400, 400);

	connect(this, SIGNAL(elementClicked(const QString&)), this, SLOT(mapClicked(const QString&)));
	connect(_timer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MotionMap::~MotionMap() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::paintEvent(QPaintEvent* event) {

	MapWidget::paintEvent(event);
	QPainter painter(this);
	_ac.draw(painter, centralCoordinates(), rect(), zoomLevel());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::setDatabase(DatabaseQuery* query) {
	_query = query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::setOrigin(Origin* org) {

	if ( !_query ) {
		log(Client::LM_WARNING, QString("It is mandatory to set a valid database query interface first"));
		return;
	}

	if ( !org ) {
		log(Client::LM_ERROR, QString("NULL ptr Seiscomp::DataModel::Origin object cannot be managed"));
		return;
	}
	_origin = org;

	EventPtr event = _query->getEvent(org->publicID());

	if ( !event ) {
		log(Client::LM_WARNING, QString("%1 is an orphan but has received a painting request")
		        .arg(org->publicID().c_str()));
		return;
	}

	MagnitudePtr mag = NULL;
	if ( event ) {
		mag = Magnitude::Find(event->preferredMagnitudeID());

		if ( !mag )
			mag = Magnitude::Cast(_query->getObject(Magnitude::TypeInfo(),
			    event->preferredMagnitudeID()));
	}

	clear();

	double depthValue = -9999.99;
	QString depth = QString("-");
	try {
		depthValue = org->depth().value();
		depth = QString::number(org->depth().value(), 'f', scheme().depthPrecision());
	} catch ( ... ) {}

	double lonUncertainty = -1.;
	try {
		lonUncertainty = org->longitude().uncertainty();
	} catch ( ... ) {}

	QString lonU = QString("-");
	if ( lonUncertainty != -1. )
		lonU = QString::number(lonUncertainty, 'f', scheme().generalPrecision());

	double latUncertainty = -1.;
	try {
		latUncertainty = org->latitude().uncertainty();
	} catch ( ... ) {}

	QString latU = QString("-");
	if ( latUncertainty != -1. )
		latU = QString::number(latUncertainty, 'f', scheme().generalPrecision());

	QString depthUncertainty = QString("-");
	try {
		depthUncertainty = QString::number(org->depth().uncertainty(), 'f',
		    scheme().generalPrecision());
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

	QString magnitude = QString("-");
	QString magType = QString("");
	if ( mag ) {
		try {
			magnitude = QString::number(roundDouble(mag->magnitude().value(), 1),
			    'f', scheme().generalPrecision());
		} catch ( ... ) {}
		magType = mag->type().c_str();
	}

	QString author = "-";
	try {
		author = org->creationInfo().author().c_str();
	} catch ( ... ) {}

	Map::Epicenter* epicenter = new Map::Epicenter();
	epicenter->setGeoPosition(QPointF(org->longitude().value(), org->latitude().value()));
	if ( latUncertainty != -1 && lonUncertainty != -1 )
		epicenter->setPositionUncertainty(QPointF(lonUncertainty, latUncertainty));
	epicenter->setToolTip(epicenterTooltip(org->publicID().c_str(),
	    org->time().value().toString("%Y-%m-%d %H:%M:%S").c_str(),
	    getStringPosition(org->latitude().value(), Latitude).c_str(),
	    latU, getStringPosition(org->longitude().value(), Longitude).c_str(), lonU,
	    depth, depthUncertainty, rms, azimuth, magnitude, magType, dist,
	    Regions::getRegionName(org->latitude().value(), org->longitude().value()).c_str(),
	    author));

	if ( magnitude != "-" && magnitude.toDouble() != .0 )
		epicenter->setMagnitude(magnitude.toDouble());

	epicenter->setAntialiased(true);
	epicenter->setSize(EpicenterDefaultSize);
	if ( depthValue != -9999.99 )
		epicenter->pen().setColor(getDepthColoration(depthValue));
	else
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

		Map::Circle* wave = new Map::Circle;
		wave->setName(org->publicID().c_str());
		wave->setBrush(Qt::transparent);
		wave->pen().setWidth(4);
		wave->pen().setColor(QColor("#DDF300"));
		wave->setGeoPosition(QPointF(org->longitude().value(), org->latitude().value()));
		wave->setSize(QSizeF(.1, .1));
		wave->setVisible(false);
		wave->setAntialiased(true);
		if ( !_ac.addGeometry(wave) )
			qDebug("Geometry already exist");

//		Map::Line* ruler = new Map::Line;
//		ruler->setName("ruler");
//		ruler->setStart()

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
//			stationGeometry->pen().setColor(Qt::white);
			stationGeometry->setBrush(Qt::transparent);
			stationGeometry->setToolTip(stationTooltip(station->code().c_str(),
			    station->description().c_str(), getStringPosition(station->latitude(), Latitude).c_str(),
			    getStringPosition(station->longitude(), Longitude).c_str(), staDistance,
			    staElevation, staAzimuth, station->type().c_str()));
			stationGeometry->setShape(Map::Station::ss_triangle);
			stationGeometry->setSize(StationTriangleDefaultSize);

			if ( !addStation(stationGeometry) ) {
				delete stationGeometry;
				stationGeometry = NULL;
			}
		}
	}

	QListIterator<Map::Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Map::Station* s = itS.next();
		foregroundCanvas().addGeometry(s);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::mapClicked(const QString& id) {

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
void MotionMap::updateAnimation() {

	if ( !_origin )
		return;

	Map::Geometry* g = _ac.geometry(_origin->publicID().c_str());

	if ( !g ) {
		qDebug("circle not found");
		return;
	}

	QSizeF vsize = g->size();

	if ( vsize.height() < 400. && vsize.width() < 400. )
		g->setSize(vsize + QSizeF(.5, .5));
	else
		g->setSize(QSizeF(.1, .1));

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MotionMap::addStation(Map::Station* station, const bool& toCanvas) {

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
Map::Station* MotionMap::getMapStation(const QString& network,
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
void MotionMap::setStationsVisible(const bool& visible) {

	QListIterator<Map::Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Map::Station* s = itS.next();
		s->setVisible(visible);
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::clear() {

	// Layers within canvas will destroy children geometries objects
	foregroundCanvas().clearGeometries();

	// Therefore we need only clear the lists
	_stations.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::startAnimation() {

	if ( !_origin )
		return;

	_timer->start(_animationRate);

	Map::Geometry* g = _ac.geometry(_origin->publicID().c_str());
	if ( g )
		g->setVisible(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::pauseAnimation() {
	_timer->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MotionMap::stopAnimation() {

	if ( !_origin )
		return;

	_timer->stop();

	Map::Geometry* g = _ac.geometry(_origin->publicID().c_str());
	if ( g ) {
		g->setSize(QSizeF(.1, .1));
		g->setVisible(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP




