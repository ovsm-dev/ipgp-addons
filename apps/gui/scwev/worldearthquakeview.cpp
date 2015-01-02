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

#define APP_SHORTDESC "Interactive tool for visualizing earthquakes around the world."
#define APP_FULLDESC "<p>This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.<br/>"\
		    		"It has been co-financed by the European Union and le Ministère de <br/>"\
		    		"l'Ecologie, du Développement Durable, des Transports et du Logement.</p>"
#define APP_VERSION "0.1.1"

#include "ui_worldearthquakeview.h"
#include "ui_configurationdialog.h"
#include "worldearthquakeview.h"
#include <ipgp/gui/datamodel/usgsfeed/dataloadworker.h>
#include <ipgp/gui/datamodel/usgsfeed/quakeevent.h>

#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/drawables/station.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/drawables/pin.h>
#include <ipgp/gui/datamodel/qcustomitems.hpp>
#include <ipgp/gui/3rd-party/qprogressindicator/qprogressindicator.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/geo/geo.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/math/geo.h>


#include <QtGui>
#include <QtWebKit/QWebView>

using namespace Seiscomp;
using namespace Seiscomp::DataModel;

using namespace IPGP;
using namespace IPGP::Core;
using namespace IPGP::Core::Geo;
using namespace IPGP::Core::Math;
using namespace IPGP::Gui;
using namespace IPGP::Gui::Client;
using namespace IPGP::Gui::Misc;



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WorldEarthquakeView::WorldEarthquakeView(int& argc, char** argv) :
		IPGP::Gui::Client::Application(argc, argv) {

	_map = NULL;
	_databaseLabel = NULL;
	_configDialog = NULL;
	_configDialogUi = NULL;
	_customMenu = NULL;
	_worker = NULL;
	_ui = NULL;
	_timer = NULL;
	_clearPins = NULL;
	_pi = NULL;
	_eventListWidget = NULL;

	_refreshDelay = 60;

	_autoRefresh = true;
	_exitRequested = false;
	_showOriginsWithNotType = false;
	_showNotLocatableOrigins = false;
	_showOutOfNetworkOrigins = false;
	_showNotExistingOrigins = false;
	_showInventoryComponents = false;
	_useOwnWebViewer = false;

	messageSubscription("EVENT");
	messageSubscription("LOCATION");
	messageSubscription("CONFIG");
	messageSubscription("MAGNITUDE");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WorldEarthquakeView::~WorldEarthquakeView() {

	_exitRequested = true;

	if ( _worker ) {

		SEISCOMP_DEBUG("Stopping ATOM parser");
		_worker->cancelUpdate();
		_worker->requestExit();
		SEISCOMP_INFO("Waiting for ATOM parser destruction");
		delete _worker;
		_worker = NULL;
		SEISCOMP_INFO("ATOM parser destroyed");
	}

	_worker = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::addOrigin() {

	QModelIndex index = _ui->tableWidget_events->currentIndex();
	QVariant data = index.data(QuakeEvent::DataRole);
	QuakeEvent event = data.value<QuakeEvent>();

	Notifier::SetEnabled(true);
	EventParametersPtr ep = new EventParameters;

	Time ot;
	ot.set(event.when().date().year(), event.when().date().month(),
	    event.when().date().day(), event.when().time().hour(),
	    event.when().time().minute(), event.when().time().second(),
	    event.when().time().msec());

	CreationInfo ci;
	ci.setCreationTime(Time::GMT());
	ci.setAuthor(_defaultAuthor);
	ci.setAgencyID(_defaultAgency);

	OriginPtr origin = Origin::Create();
	if ( _configDialogUi->checkBox_useSourceID->isChecked() )
	    origin->setPublicID(event.id().toStdString());
	origin->setCreationInfo(ci);
	origin->setEarthModelID(_defaultEarthModelID);
	origin->setMethodID(_defaultMethodID);
	origin->setEvaluationMode(EvaluationMode(MANUAL));
	origin->setTime(ot);
	origin->setLatitude(RealQuantity(event.position().x(), None, None, None, None));
	origin->setLongitude(RealQuantity(event.position().y(), None, None, None, None));
	origin->setDepth(RealQuantity(fabs(event.elevation() / 1000), None, None, None, None));

	ep->add(origin.get());
	NotifierMessagePtr msg1 = Notifier::GetMessage(true);
	connection()->send("LOCATION", msg1.get());

	MagnitudePtr mag = Magnitude::Create();
	mag->setType("ML");
	mag->setOriginID(origin->publicID());
	mag->setMagnitude(RealQuantity(event.magnitude()));
	mag->setMethodID("mean");
	mag->setCreationInfo(ci);
	origin->add(mag.get());

	NotifierMessagePtr msg2 = Notifier::GetMessage(true);
	connection()->send("LOCATION", msg2.get());
	Notifier::SetEnabled(false);

	SEISCOMP_NOTICE("Origin with id %s processed and sent to messaging system",
	    origin->publicID().c_str());

	QMessageBox::information(mainWindow(), tr("New origin"),
	    QString("Origin with id %1 successfully processed.")
	            .arg(origin->publicID().c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::handleDatabaseState() {

	_databaseLabel->setPixmap(databaseIcon());
	_databaseLabel->setToolTip(databaseServer().c_str());

	_ui->label_agencyID->setText(QString(" %1").arg(agencyID().c_str()));

	paintStations();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WorldEarthquakeView::initConfiguration() {

	if ( !IPGP::Gui::Client::Application::initConfiguration() ) return false;

	bool retCode = true;

	try {
		_showInventoryComponents = configGetBool("wev.inventoryVisible");
	} catch ( ... ) {}

	try {
		_defaultAgency = configGetString("wev.import.agency");
	}
	catch ( ... ) {
		_defaultAgency = agencyID();
	}

	try {
		_defaultAuthor = configGetString("wev.import.author");
	}
	catch ( ... ) {
		_defaultAuthor = "scwev";
	}

	try {
		_defaultEarthModelID = configGetString("wev.import.earthModelID");
	}
	catch ( ... ) {
		_defaultEarthModelID = "Unknown Earth Model";
	}

	try {
		_defaultMethodID = configGetString("wev.import.methodID");
	}
	catch ( ... ) {
		_defaultMethodID = "Unknown Method";
	}

	try {
		_autoRefresh = configGetBool("wev.refresh.enable");
	} catch ( ... ) {}

	try {
		_refreshDelay = configGetInt("wev.refresh.delay");
	} catch ( ... ) {}

	try {
		_feed = configGetString("wev.rss.feed.USGS");
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Could read 'wev.rss.USGS' from configuration file");
		retCode = false;
	}

	try {
		_otherFeed = configGetString("wev.rss.feed.other");
	} catch ( ... ) {}

	try {
		std::string fp = configGetString("wev.rss.feedPeriod");
		FeedPeriod t;
		if ( t.fromString(fp) )
		    _feedPeriod = t;
	} catch ( ... ) {
		_feedPeriod = Past30Days;
	}

	try {
		std::string eft = configGetString("wev.rss.feedType");
		EarthquakeFeedType t;
		if ( t.fromString(eft) )
		    _feedType = t;
	} catch ( ... ) {
		_feedType = All;
	}

	try {
		_useOwnWebViewer = configGetBool("wev.useOwnWebViewer");
	} catch ( ... ) {}

	// ---------------------------------------------------------------------
	//  EventList parameters
	// ---------------------------------------------------------------------
	try {
		_eventListSettings.useCacheEngine = configGetBool(
		    "wev.eventList.useCacheEngine");
	} catch ( ... ) {}
	try {
		_eventListSettings.useMultiThreads = configGetBool(
		    "wev.eventList.useSeparateThread");
	} catch ( ... ) {}

	_eventListSettings.showEventColorType = true;
	_eventListSettings.eventsAreCheckable = true;
	_eventListSettings.fetchSiblingOriginComment = false;
	_eventListSettings.fetchUnassociatedOriginComment = false;
	_eventListSettings.fetchUnassociated = false;
	_eventListSettings.useAdvancedOptions = false;
	_eventListSettings.decorateItems = false;

	try {
		_showNotExistingOrigins = configGetBool("wev.eventList.showNotExistingOrigins");
	} catch ( ... ) {}
	try {
		_showNotLocatableOrigins = configGetBool("wev.eventList.showNotLocatableOrigins");
	} catch ( ... ) {}
	try {
		_showOriginsWithNotType = configGetBool("wev.eventList.showOriginsWithNoType");
	} catch ( ... ) {}
	try {
		_showOutOfNetworkOrigins = configGetBool("wev.eventList.showOutOfNetworkInterestsOrigins");
	} catch ( ... ) {}

	// ---------------------------------------------------------------------
	//  Locators parameters
	// ---------------------------------------------------------------------
	std::vector<std::string> tmpLocators;
	try {
		tmpLocators = configGetStrings("wev.locators");
	} catch ( ... ) {}
	for (std::vector<std::string>::iterator it = tmpLocators.begin();
	        it != tmpLocators.end(); ++it) {

		EarthModelIDs list;
		std::string profiles = "wev.locator." + *it + ".profiles";

		try {
			list = configGetStrings(profiles);
		} catch ( ... ) {}

		if ( list.empty() ) continue;

		_locators.push_back(Locators(*it, list));
	}

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::eventSelected(QTableWidgetItem* item) {

	if ( !item ) {
		QMessageBox::information(mainWindow(), tr("Item error"), QString("nullptr object received."));
		return;
	}

	_map->foregroundCanvas().removeGeometries(Map::Geometry::d_Pin);

	QVariant data = item->data(QuakeEvent::DataRole);
	QuakeEvent event = data.value<QuakeEvent>();

//	_pagerURL = event.pagerURL();
//	_shakemapURL = event.shakemapURL();
//	_dyfiURL = event.dyfiURL();

	_ui->mapLabel->setText(Regions::getRegionName(event.position().y(), event.position().x()).c_str());

	Map::Pin* pin = new Map::Pin;
	pin->setGeoPosition(event.position());
	pin->setPix(QPixmap(":images/redpin.png"), QSize(30, 24));
	pin->setLegendEnabled(true);
	pin->setBrush(Qt::black);
	pin->pen().setColor(Qt::white);
	pin->setToolTip(QString("<p style=\"color:white;\">"
		"<b>Origin</b>:&nbsp;%1<br/>"
		"<b>Time</b>:&nbsp;%2<br/>"
		"<b>Lat.</b>:&nbsp;%3<br/>"
		"<b>Lon.</b>:&nbsp;%4<br/>"
		"<b>Region</b>:&nbsp;%5<br/>"
		"<b>Depth</b>:&nbsp;%6 km<br/>"
		"<b>Mag.</b>:&nbsp;%7</p>")
	        .arg(event.id())
	        .arg(event.when().toString(Qt::TextDate))
	        .arg(getStringPosition(event.position().y(), Latitude).c_str())
	        .arg(getStringPosition(event.position().x(), Longitude).c_str())
	        .arg(Regions::getRegionName(event.position().y(), event.position().x()).c_str())
	        .arg(fabs(event.elevation() / 1000))
	        .arg(event.magnitude()));

	if ( _map->foregroundCanvas().addGeometry(pin) ) {
		_map->setDefaultGeoPosition(event.position());
		_map->centerOn(event.position());
	}
	else {
		delete pin;
		pin = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::pinEventListItem(const QString& publicID) {

	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 || publicID.isEmpty() ) return;

	OriginPtr origin;
	for (int i = 0; i < list.size(); ++i) {

		QString selectedID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) continue;
		if ( publicID != selectedID ) continue;

		EventPtr event = _eventListWidget->getEvent(publicID.toStdString());

		if ( event )
			origin = _eventListWidget->getOrigin(event->preferredOriginID());
		else
			origin = _eventListWidget->getOrigin(publicID.toStdString());

		break;
	}

	if ( !origin ) return;

	showEventMap();

	QString depthstr = QString("-");
	try {
		depthstr = QString::number(origin->depth().value(), 'f', scheme().depthPrecision());
	} catch ( ... ) {}

	_map->foregroundCanvas().removeGeometries(Map::Geometry::d_Pin);

	Map::Pin* pin = new Map::Pin;
	pin->setGeoPosition(QPointF(origin->longitude().value(), origin->latitude().value()));
	pin->setPix(QPixmap(":images/redpin.png"), QSize(30, 24));
	pin->setLegendEnabled(true);
	pin->setBrush(Qt::black);
	pin->pen().setColor(Qt::white);
	pin->setToolTip(QString("<p style=\"color:white;\">"
		"<b>Origin</b>:&nbsp;%1<br/>"
		"<b>Date</b>:&nbsp;%2<br/>"
		"<b>Latitude</b>:&nbsp;%3<br/>"
		"<b>Longitude</b>:&nbsp;%4<br/>"
		"<b>Depth</b>:&nbsp;%5 km</p>")
	        .arg(origin->publicID().c_str())
	        .arg(origin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str())
	        .arg(origin->latitude().value())
	        .arg(origin->longitude().value())
	        .arg(depthstr));
	pin->setName(origin->publicID().c_str());

	if ( _map->foregroundCanvas().addGeometry(pin) ) {
		_map->setDefaultGeoPosition(QPointF(origin->longitude().value(), origin->latitude().value()));
		_map->centerOn(QPointF(origin->longitude().value(), origin->latitude().value()));
	}
	else {
		delete pin;
		pin = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showEventMap() {
	_ui->tabWidget->setCurrentIndex(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showEventList() {
	_ui->tabWidget->setCurrentIndex(1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::feedPeriodChanged() {

	if ( !_worker ) return;

//	showWaitingWidget();
	_worker->cancelUpdate();
	_worker->requestExit();
	_worker->setUrl(updateFeedURL());
	_worker->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::feedEarthquakeTypeChanged() {
	feedPeriodChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::tableContextMenu(const QPoint& widgetXY) {

	Q_UNUSED(widgetXY);

	QVariant data = _ui->tableWidget_events->currentItem()->data(QuakeEvent::DataRole);
	QuakeEvent event = data.value<QuakeEvent>();

	QMenu menu(mainWindow());
	_customMenu->setEnabled(
	    (!_ui->tableWidget_events->rowCount()) ? false : true);

	if ( !event.pagerURL().isEmpty() ) {
		menu.addAction("&See Pager", this, SLOT(showPager()));
		_pagerURL = event.pagerURL();
	}

	if ( !event.dyfiURL().isEmpty() ) {
		menu.addAction("&See DYFI", this, SLOT(showDYFI()));
		_dyfiURL = event.dyfiURL();
	}

	if ( !event.shakemapURL().isEmpty() ) {
		menu.addAction("&See Shakemap", this, SLOT(showShakemap()));
		_shakemapURL = event.shakemapURL();
	}

	if ( _configDialogUi->enableCommitCheckBox->isChecked() ) {
		menu.addSeparator();
		menu.addAction(_customMenu);
	}

	menu.exec(QCursor::pos());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::updateError(const QString& error) {
	if ( !_exitRequested )
	    QMessageBox::information(mainWindow(), "Update error", error);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::loadWebPage(const QString& type,
                                      const QString& url) {

	//! Prevent creating an already existing window... Though if the window
	//! is hidden (closed) we need only show it...
	bool hasWindow = false;
	for (int i = 0; i < _webpages.size(); ++i) {

		if ( !_webpages[i] )
		    continue;

		if ( _webpages.at(i)->url().toString() == url ) {
			(_webpages.at(i)->isVisible()) ?
			        _webpages.at(i)->raise() : _webpages.at(i)->show();
			hasWindow = true;
		}
	}

	if ( !hasWindow ) {
		QWebView* wv = new QWebView;
		wv->setWindowTitle(QString("%1 - %2").arg(type).arg(url));
		wv->setGeometry(mainWindow()->pos().x(), mainWindow()->pos().y(), 400, 400);
		wv->load(QUrl(url));
		_webpages.append(wv);
		wv->show();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showDYFI() {
	if ( _useOwnWebViewer )
		loadWebPage("DYFI", _dyfiURL);
	else
		QDesktopServices::openUrl(QUrl(_dyfiURL));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showPager() {
	if ( _useOwnWebViewer )
		loadWebPage("Pager", _pagerURL);
	else
		QDesktopServices::openUrl(QUrl(_pagerURL));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showShakemap() {
	if ( _useOwnWebViewer )
		loadWebPage("Shakemap", _shakemapURL);
	else
		QDesktopServices::openUrl(QUrl(_shakemapURL));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::closeWebpages() {

	for (int i = 0; i < _webpages.size(); ++i) {

		if ( !_webpages[i] ) continue;

		_webpages.at(i)->close();
		delete _webpages[i];
		_webpages[i] = NULL;
	}
	_webpages.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void WorldEarthquakeView::populateFromLocalDatabase() {

	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 ) return;

	_map->foregroundCanvas().removeGeometry("{DATABASE}", false);

	_ui->label_agencyEntries->setText("-");

	size_t count = 0;
	for (int i = 0; i < list.size(); ++i) {

		QString publicID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) {
			_map->foregroundCanvas().removeGeometry(publicID, false);
			continue;
		}

		EventPtr event = _eventListWidget->getEvent(publicID.toStdString());

		if ( !event ) continue;

		OriginPtr origin = _eventListWidget->getOrigin(event->preferredOriginID());

		if ( !origin ) continue;

		double depthValue = -9999.99;
		QString depth = QString("-");
		try {
			depthValue = origin->depth().value();
			depth = QString::number(origin->depth().value(), 'f', scheme().depthPrecision());
		} catch ( ... ) {}

		double lonUncertainty = -1.;
		try {
			lonUncertainty = origin->longitude().uncertainty();
		} catch ( ... ) {}

		QString lonU = QString("-");
		if ( lonUncertainty != -1. )
		    lonU = QString::number(lonUncertainty, 'f', scheme().generalPrecision());

		double latUncertainty = -1.;
		try {
			latUncertainty = origin->latitude().uncertainty();
		} catch ( ... ) {}

		QString latU = QString("-");
		if ( latUncertainty != -1. )
		    latU = QString::number(latUncertainty, 'f', scheme().generalPrecision());

		QString depthUncertainty = QString("-");
		try {
			depthUncertainty = QString::number(origin->depth().uncertainty(), 'f',
			    scheme().depthPrecision());
		} catch ( ... ) {}

		QString dist = QString("-");
		try {
			if ( scheme().distanceInKM() )
				dist = QString("%1km").arg(QString::number(
				    Seiscomp::Math::Geo::deg2km(origin->quality().minimumDistance()),
				    'f', scheme().distancePrecision()));
			else
				dist = QString::fromUtf8("%1°").arg(QString::number(
				    origin->quality().minimumDistance(), 'f', scheme().distancePrecision()));
		} catch ( ... ) {}

		QString azimuth = QString("-");
		try {
			azimuth = QString::number(origin->quality().azimuthalGap(), 'f', scheme().azimuthPrecision());
		} catch ( ... ) {}

		QString rms = QString("-");
		try {
			rms = QString::number(origin->quality().standardError(), 'f', scheme().rmsPrecision());
		} catch ( ... ) {}

		MagnitudePtr mag = Magnitude::Cast(PublicObjectPtr(query()->getObject(
		    Magnitude::TypeInfo(), event->preferredMagnitudeID())));
		QString magnitude = QString("-");
		QString magType = QString("");
		float magValue = -1.;
		if ( mag ) {
			try {
				magValue = mag->magnitude().value();
				magnitude = QString::number(roundDouble(mag->magnitude().value(), 1),
				    'f', scheme().generalPrecision());
			} catch ( ... ) {}
			magType = mag->type().c_str();
		}

		QString author = "-";
		try {
			author = origin->creationInfo().author().c_str();
		} catch ( ... ) {}

		Map::Epicenter* epicenter = new Map::Epicenter;
		epicenter->setBrush(Qt::transparent);
		epicenter->pen().setColor(getDepthColoration(depthValue));
		epicenter->pen().setWidthF(EpicenterDefaultPenWidth);
		epicenter->setToolTip(epicenterTooltip(origin->publicID().c_str(),
		    origin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str(),
		    getStringPosition(origin->latitude().value(), Latitude).c_str(),
		    latU, getStringPosition(origin->longitude().value(), Longitude).c_str(), lonU,
		    depth, depthUncertainty, rms, azimuth, magnitude, magType, dist,
		    Regions::getRegionName(origin->latitude().value(), origin->longitude().value()).c_str(),
		    author));
		epicenter->setSize(EpicenterDefaultSize);
		(magValue > -1.) ? epicenter->setMagnitude(magValue) : epicenter->setSize(QSizeF(4., 4.));
		epicenter->setGeoPosition(QPointF(origin->longitude().value(), origin->latitude().value()));
		epicenter->setName(QString("{DATABASE} %1").arg(event->publicID().c_str()));

		if ( !_map->foregroundCanvas().addGeometry(epicenter) ) {
			delete epicenter;
			epicenter = NULL;
		}

		++count;
	}

	_ui->label_agencyEntries->setText(QString("%1").arg(count));

	log(Gui::Client::LM_INFO, __func__, QString("Populated map with %1 database event(s)").arg(count));
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::populateFromWorker() {

	if ( _worker->quakeEventList().size() == 0 ) return;

	showWaitingWidget();

	_map->foregroundCanvas().removeGeometry("{FEED}", false);

	_ui->tableWidget_events->clearSelection();
//	_ui->tableWidget_events->setRowCount(0);
	_ui->tableWidget_events->model()->removeRows(0, _ui->tableWidget_events->rowCount());

	_ui->label_usgsEntries->setText("-");
	QApplication::processEvents();

	size_t count = 0;
	for (int i = 0; i < _worker->quakeEventList().size(); ++i) {

		QuakeEvent event = _worker->quakeEventList().at(i);

		if ( event.isEmpty() || event.elevation() == .0 || event.magnitude() == .0 )
		    continue;

		if ( event.position().y() < -90. || event.position().y() > 90. )
		    continue;

		if ( event.position().x() < -180. || event.position().x() > 180. )
		    continue;

		QVariant var;
		var.setValue(event);

		QFont boldFont;
		boldFont.setBold(true);

		int idx = _ui->tableWidget_events->rowCount();
		_ui->tableWidget_events->insertRow(idx);

		QString tooltip = QString("<p style=\"color:white;\">"
			"<b>ID</b>:&nbsp;%1<br/>"
			"<b>Date</b>:&nbsp;%2<br/>"
			"<b>Pager</b>:&nbsp;%3<br/>"
			"<b>Shakemap</b>:&nbsp;%4<br/>"
			"<b>DYFI</b>:&nbsp;%5</p>")
		        .arg(event.id()).arg(event.when().toString("yyyy-MM-dd hh:mm:ss"))
		        .arg(event.pagerURL()).arg(event.shakemapURL()).arg(event.dyfiURL());

		QTableWidgetItem* date = new QTableWidgetItem(event.when().toString("yyyy-MM-dd hh:mm:ss"));
		date->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		_ui->tableWidget_events->setItem(idx, 0, date);

		QTableWidgetItem* mag = new QTableWidgetItem(QString::number(event.magnitude(), 'f', 1));
		mag->setFont(boldFont);
		mag->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		mag->setForeground(getMagnitudeColoration(event.magnitude()));
		_ui->tableWidget_events->setItem(idx, 1, mag);

		QTableWidgetItem* lat = new QTableWidgetItem(getStringPosition(event.position().y(), Latitude).c_str());
		lat->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_ui->tableWidget_events->setItem(idx, 2, lat);

		QTableWidgetItem* lon = new QTableWidgetItem(getStringPosition(event.position().x(), Longitude).c_str());
		lon->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_ui->tableWidget_events->setItem(idx, 3, lon);

		NumberTableItem* depth;
		if ( scheme().distanceInKM() )
			depth = new NumberTableItem(QString("%1 km").arg(QString::number(fabs(
			    event.elevation() / 1000), 'f', scheme().depthPrecision())));
		else
			depth = new NumberTableItem(QString::fromUtf8("%1 °").arg(QString::number(
			    Seiscomp::Math::Geo::km2deg(fabs(event.elevation() / 1000)), 'f', scheme().depthPrecision())));
		depth->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		depth->setToolTip(tooltip);
		_ui->tableWidget_events->setItem(idx, 4, depth);

		_ui->tableWidget_events->setItem(idx, 5, new QTableWidgetItem(
		    Regions::getRegionName(event.position().y(), event.position().x()).c_str()));
		_ui->tableWidget_events->setItem(idx, 6, new QTableWidgetItem(event.id()));

		for (int i = 0; i < 7; ++i) {
			_ui->tableWidget_events->item(idx, i)->setData(QuakeEvent::DataRole, var);
			_ui->tableWidget_events->item(idx, i)->setToolTip(tooltip);
		}

		_ui->tableWidget_events->resizeColumnsToContents();
		_ui->tableWidget_events->resizeRowsToContents();

		Map::Epicenter* epicenter = new Map::Epicenter;
		epicenter->setBrush(Qt::transparent);
		epicenter->pen().setColor(getDepthColoration(abs(event.elevation() / 1000)));
		epicenter->setGeoPosition(event.position());
		epicenter->pen().setWidthF(EpicenterDefaultPenWidth);
		epicenter->setToolTip(epicenterTooltip(event.id(),
		    event.when().toString(Qt::TextDate),
		    getStringPosition(event.position().y(), Latitude).c_str(), "",
		    getStringPosition(event.position().x(), Longitude).c_str(), "",
		    QString::number(fabs(event.elevation() / 1000)), "", "", "", "",
		    QString::number(event.magnitude()), "-",
		    Regions::getRegionName(event.position().y(), event.position().x()).c_str(),
		    "-"));
		epicenter->setMagnitude(event.magnitude());
		epicenter->setSize(QSizeF(4., 4.));
		epicenter->setName(QString("{FEED} %1").arg(event.id()));

		if ( !_map->foregroundCanvas().addGeometry(epicenter) ) {
			delete epicenter;
			epicenter = NULL;
		}

		++count;
	}

	hideWaitingWidget();

	_ui->label_usgsEntries->setText(QString("%1").arg(count));

	log(Gui::Client::LM_INFO, __func__, QString("Populated map with %1 feed event(s)").arg(count));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::paintStations() {

	_map->foregroundCanvas().removeGeometries(Map::Geometry::d_Station);

	if ( !_ui->actionShowStations->isChecked() ) return;

	DatabaseReader* reader = new DatabaseReader(this->database());
	Inventory* inv = new Inventory();
	reader->loadNetworks(inv);

	int count = 0;
	for (size_t i = 0; i < inv->networkCount(); ++i) {
		NetworkPtr network = inv->network(i);
		reader->load(network.get());

		for (size_t j = 0; j < network->stationCount(); ++j) {
			StationPtr station = network->station(j);

			QString staElevation = QString("-");
			try {
				staElevation = QString::number(station->elevation(), 'f', scheme().generalPrecision());
			} catch ( ... ) {}

			Map::Station* sta = new Map::Station;
			sta->setName(station->publicID().c_str());
			sta->setGeoPosition(QPointF(station->longitude(), station->latitude()));
			sta->setSize(QSizeF(12., 12.));
			sta->setBrush(Qt::black);
			sta->pen().setColor(Qt::red);
			sta->setToolTip(stationTooltip(station->code().c_str(),
			    station->description().c_str(), getStringPosition(station->latitude(), Latitude).c_str(),
			    getStringPosition(station->longitude(), Longitude).c_str(), "-",
			    staElevation, "-", station->type().c_str()));
			sta->setShape(Map::Station::ss_triangle);
			++count;
			if ( !_map->foregroundCanvas().addGeometry(sta) ) {
				delete sta;
				sta = NULL;
				--count;
			}
		}
	}

	log(Gui::Client::LM_INFO, __func__, QString("Populated map with %1 station(s) from inventory").arg(count));

	delete inv;
	delete reader;
	inv = NULL;
	reader = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::clearPins() {
	_map->foregroundCanvas().removeGeometries(Map::Geometry::d_Pin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::requestDataUpdate() {

	// Update events from RSS feed
	_worker->update();

	// Update local event from database
	_eventListWidget->setRange(_eventListWidget->range().first, QDateTime::currentDateTimeUtc());
	_eventListWidget->readEvents();
	populateFromLocalDatabase();

	_ui->statusBar->showMessage("A scheduled update has been performed.", 3000);
	log(Gui::Client::LM_INFO, __func__, "Scheduled feed update has been performed");
	SEISCOMP_INFO("A scheduled update has been performed.");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WorldEarthquakeView::run() {

	showMessage("Setup user interface");
	setupGraphicalInterface();
	readApplicationSettings();
	setupApplicationMenus();

	setInterfaceName("World Earthquake View");
	setBinaryName("scwev");

	setApplicationDescriptor(IPGP::Gui::Client::ApplicationDescriptor(
	    interfaceName().c_str(), QString::fromUtf8(APP_SHORTDESC),
	    QString::fromUtf8(APP_FULLDESC), APP_VERSION));

	handleDatabaseState();

	showMessage("Loading events database");
	_eventListWidget->setRange(globalSettings().loadEventDB());
	_eventListWidget->readEvents();

	showNormal();

	return (bool) !QApplication::exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::handleSelection() {

	QList<EventListWidget::EvaluationModeSelection> choice;

//	if ( _allCheck->isChecked() )
//		choice << EventListWidget::emsALL;
//	if ( _manualCheck->isChecked() )
	choice << EventListWidget::emsMANUAL;
//	if ( _autoCheck->isChecked() )
//		choice << EventListWidget::emsAUTOMATIC;
//	if ( _confirmedCheck->isChecked() )
//		choice << EventListWidget::emsCONFIRMED;
//	if ( _noneCheck->isChecked() )
//		choice << EventListWidget::emsNONE;

	_eventListWidget->setItemsChecked(choice);

	populateFromLocalDatabase();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::handleHiddenTypes() {

	hideUnlocalizedEvents(_configDialogUi->notLocatableCheckBox->isChecked());
	hideExternalEvents(_configDialogUi->outNetworkCheckBox->isChecked());
	hideNoTypeSetEvents(_configDialogUi->notSetCheckBox->isChecked());
	hideFakeEvents(_configDialogUi->notExistingCheckBox->isChecked());

	populateFromLocalDatabase();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::hideUnlocalizedEvents(const bool& show) {
	_eventListWidget->hideUnlocalizedEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::hideExternalEvents(const bool& show) {
	_eventListWidget->hideExternalEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::hideNoTypeSetEvents(const bool& show) {
	_eventListWidget->hideNoTypeSetEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::hideFakeEvents(const bool& show) {
	_eventListWidget->hideFakeEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::databaseMessage(const QString& oldDB,
                                          const QString& newDB) {

	QMessageBox::information(mainWindow(), "Information",
	    QString("The database connection to %1 has been replaced by %2, "
		    "objects will be updated after closing this window").arg(oldDB)
	            .arg(newDB));

	log(Gui::Client::LM_INFO, __func__, QString("The database connection to %1 has been replaced by %2, "
		"objects will be updated after closing this window").arg(oldDB)
	        .arg(newDB));

	_eventListWidget->setDatabase(query());
	_eventListWidget->readEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::settingsChanged() {

	_refreshDelay = _configDialogUi->delay->value();
	_timer->setInterval(_refreshDelay * 1000);

	if ( _configDialogUi->refreshCheckBox->isChecked() ) {
		_timer->start();
		SEISCOMP_INFO("Set auto-update ON from configuration dialog");
		log(Gui::Client::LM_INFO, __func__, "Set auto-update ON from configuration dialog");
	}
	else {
		_timer->stop();
		SEISCOMP_INFO("Set auto-update OFF from configuration dialog");
		log(Gui::Client::LM_INFO, __func__, "Set auto-update OFF from configuration dialog");
	}

	//! Emulate a user interaction to dispose of the proper feed's URL
	feedPeriodChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::setupApplicationMenus() {

	connect(_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
	connect(_ui->actionConfigureConnection, SIGNAL(triggered()), this, SLOT(showConnectionDialog()));
	connect(_ui->actionConfigureWorldEarthquakeView, SIGNAL(triggered()), _configDialog, SLOT(open()));
	connect(_ui->actionLoadedPlugins, SIGNAL(triggered()), this, SLOT(showPlugins()));
	connect(_ui->actionToggleFullScreen, SIGNAL(triggered()), this, SLOT(handleFullScreen()));
	connect(_ui->actionQuit, SIGNAL(triggered()), this, SLOT(exitGui()));
	connect(_ui->actionShowEventList, SIGNAL(triggered()), this, SLOT(showEventList()));
	connect(_ui->actionShowMap, SIGNAL(triggered()), this, SLOT(showEventMap()));
	connect(_ui->actionShowStations, SIGNAL(triggered()), this, SLOT(paintStations()));
	connect(_ui->actionShowLog, SIGNAL(triggered()), this, SLOT(showLog()));

	connect(_ui->tableWidget_events, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
	    this, SLOT(eventSelected(QTableWidgetItem*)));

	connect(_ui->tableWidget_events, SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(tableContextMenu(const QPoint&)));

	connect(_customMenu, SIGNAL(triggered()), this, SLOT(addOrigin()));
	connect(this, SIGNAL(databaseChanged(const QString&, const QString&)),
	    this, SLOT(databaseMessage(const QString&, const QString&)));
	connect(this, SIGNAL(changeDatabaseIcon()), this, SLOT(handleDatabaseState()));
	connect(_timer, SIGNAL(timeout()), this, SLOT(requestDataUpdate()));
	connect(_configDialogUi->refreshCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showHideMenu(int)));
	connect(_configDialog, SIGNAL(accepted()), this, SLOT(settingsChanged()));

	connect(_map, SIGNAL(elementClicked(const QString&)), this, SLOT(mapElementClicked(const QString&)));

	connect(_worker, SIGNAL(updated()), this, SLOT(populateFromWorker()));
//	connect(_worker, SIGNAL(clearModel()), this, SLOT(clearModel()));
	connect(_worker, SIGNAL(updateError(const QString&)), this, SLOT(updateError(const QString&)));

	connect(_ui->actionDocumentationIndex, SIGNAL(triggered()), this, SLOT(showGeneralDocumentation()));
	connect(_ui->actionDocumentationForScwev, SIGNAL(triggered()), this, SLOT(showDocumentation()));

	connect(_ui->radioButton_earth1, SIGNAL(clicked()), this, SLOT(feedEarthquakeTypeChanged()));
	connect(_ui->radioButton_earth25, SIGNAL(clicked()), this, SLOT(feedEarthquakeTypeChanged()));
	connect(_ui->radioButton_earth45, SIGNAL(clicked()), this, SLOT(feedEarthquakeTypeChanged()));
	connect(_ui->radioButton_earthAll, SIGNAL(clicked()), this, SLOT(feedEarthquakeTypeChanged()));
	connect(_ui->radioButton_earthSignificant, SIGNAL(clicked()), this, SLOT(feedEarthquakeTypeChanged()));

	connect(_ui->radioButton_feedPast1, SIGNAL(clicked()), this, SLOT(feedPeriodChanged()));
	connect(_ui->radioButton_feedPast30, SIGNAL(clicked()), this, SLOT(feedPeriodChanged()));
	connect(_ui->radioButton_feedPast7, SIGNAL(clicked()), this, SLOT(feedPeriodChanged()));
	connect(_ui->radioButton_feedPastHour, SIGNAL(clicked()), this, SLOT(feedPeriodChanged()));

	connect(mainWindow(), SIGNAL(mainWindowClosing()), this, SLOT(closeWebpages()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showWaitingWidget() {
	mainWindow()->setCursor(Qt::WaitCursor);
	_pi->startAnimation();
	_pi->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::hideWaitingWidget() {
	mainWindow()->setCursor(Qt::ArrowCursor);
	_pi->stopAnimation();
	_pi->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::setupGraphicalInterface() {

	IPGP::Gui::Client::Application::setupGraphicalInterface();

	_ui = new Ui::WorldEarthquakeView;
	_ui->setupUi(mainWindow());

	_map = new MapWidget(mapDescriptor().names(), mapDescriptor().paths(),
	    mainWindow(), mapDescriptor().defaultLongitude(),
	    mapDescriptor().defaultLatitude(), mapDescriptor().tilePattern());

	QBoxLayout* lm = new QVBoxLayout(_ui->frameMap);
	_ui->frameMap->setLayout(lm);
	lm->addWidget(_map);
	lm->setMargin(0);

	_pi = new QProgressIndicator(mainWindow());
	_ui->statusBar->addPermanentWidget(_pi, 1);
	_pi->hide();

	_clearPins = new QPushButton();
	_clearPins->setObjectName("clearPinsButton");
	_clearPins->setIcon(QIcon(":images/clear.png"));
	_clearPins->setToolTip("Remove pins from the map");
	_clearPins->setGeometry(0, 0, 32, 32);
	_map->addOptionalWidget(_clearPins);
	connect(_clearPins, SIGNAL(clicked()), this, SLOT(clearPins()));

	_eventListWidget = new EventListWidget(_ui->frameEventList, query());
	_eventListWidget->setLayoutMargins(0, 0, 0, 0);
	_eventListWidget->setSelectRegionWithMouse(false);
	_eventListWidget->setLocationProfiles(_locators);
	_eventListWidget->setSettings(_eventListSettings);
	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(handleHiddenTypes()));
	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(handleSelection()));
	connect(_eventListWidget, SIGNAL(viewUpdateRequested()), this, SLOT(showWaitingWidget()));
	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(hideWaitingWidget()));
	connect(_eventListWidget, SIGNAL(rowDoubleClicked(const QString&)), this, SLOT(pinEventListItem(const QString&)));
	connect(_eventListWidget, SIGNAL(rowClicked(const QString&)), this, SLOT(updateItemStatus(const QString&)));

	QBoxLayout* lev = new QVBoxLayout(_ui->frameEventList);
	_ui->frameEventList->setLayout(lev);
	lev->addWidget(_eventListWidget);
	lev->setMargin(0);

	std::string suffix;
	switch ( _feedPeriod ) {
		case PastHour:
			suffix = "_hour.atom";
			_ui->radioButton_feedPastHour->setChecked(true);
		break;
		case PastDay:
			suffix = "_day.atom";
			_ui->radioButton_feedPast1->setChecked(true);
		break;
		case Past7Days:
			suffix = "_week.atom";
			_ui->radioButton_feedPast7->setChecked(true);
		break;
		case Past30Days:
			suffix = "_month.atom";
			_ui->radioButton_feedPast30->setChecked(true);
		break;
		default:
			suffix = "_month.atom";
			_ui->radioButton_feedPast30->setChecked(true);
		break;
	}

	std::string prefix;
	switch ( _feedType ) {
		case Significant:
			prefix = "significant";
			_ui->radioButton_earthSignificant->setChecked(true);
		break;
		case M4_5:
			prefix = "4.5";
			_ui->radioButton_earth45->setChecked(true);
		break;
		case M2_5:
			prefix = "2.5";
			_ui->radioButton_earth25->setChecked(true);
		break;
		case M1_0:
			prefix = "1.0";
			_ui->radioButton_earth1->setChecked(true);
		break;
		case All:
			prefix = "all";
			_ui->radioButton_earthAll->setChecked(true);
		break;
		default:
			prefix = "all";
			_ui->radioButton_earthAll->setChecked(true);
		break;
	}

	std::string composedFeed;
	if ( QString(_feed.c_str()).contains("usgs") ) {
		composedFeed = _feed + prefix + suffix;
	}
	else {
		composedFeed = _otherFeed;
	}

	log(Gui::Client::LM_INFO, __func__, "Initializing ATOM parser with feed...");
	log(Gui::Client::LM_INFO, __func__, QString("URL: %1").arg(composedFeed.c_str()));

	_configDialogUi = new Ui::ConfigurationDialog;
	_configDialog = new QDialog(mainWindow());
	_configDialogUi->setupUi(_configDialog);

	_configDialogUi->notExistingCheckBox->setChecked(_showNotExistingOrigins);
	_configDialogUi->notLocatableCheckBox->setChecked(_showNotLocatableOrigins);
	_configDialogUi->notSetCheckBox->setChecked(_showOriginsWithNotType);
	_configDialogUi->outNetworkCheckBox->setChecked(_showOutOfNetworkOrigins);

	connect(_configDialogUi->notExistingCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideFakeEvents(const bool&)));
	connect(_configDialogUi->notLocatableCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideUnlocalizedEvents(const bool&)));
	connect(_configDialogUi->outNetworkCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideExternalEvents(const bool&)));
	connect(_configDialogUi->notSetCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideNoTypeSetEvents(const bool&)));

	_configDialogUi->rssLink->setText(_feed.c_str());
	_configDialogUi->delay->setValue(_refreshDelay);

	if ( _autoRefresh ) {
		_configDialogUi->refreshCheckBox->setChecked(true);
		_configDialogUi->label_2->setEnabled(true);
		_configDialogUi->label_3->setEnabled(true);
		_configDialogUi->delay->setEnabled(true);
	}
	else {
		_configDialogUi->refreshCheckBox->setChecked(false);
		_configDialogUi->label_3->setEnabled(false);
		_configDialogUi->label_2->setEnabled(false);
		_configDialogUi->delay->setEnabled(false);
	}

	_ui->label_usgsEntries->setText("-");
	_ui->label_agencyEntries->setText("-");

	_ui->tableWidget_events->setColumnCount(7);
	_ui->tableWidget_events->setHorizontalHeaderLabels(
	    QStringList() << "Time(OT)" << "M" << "Lat" << "Lon"
	                  << "Depth" << "Region" << "ID");
	_ui->tableWidget_events->verticalHeader()->setVisible(false);
	_ui->tableWidget_events->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	_ui->tableWidget_events->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->tableWidget_events->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidget_events->setSelectionMode(QAbstractItemView::SingleSelection);
	_ui->tableWidget_events->resizeColumnsToContents();
	_ui->tableWidget_events->resizeRowsToContents();
	_ui->tableWidget_events->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_ui->tableWidget_events->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidget_events->setAlternatingRowColors(false);
	_ui->tableWidget_events->setContextMenuPolicy(Qt::CustomContextMenu);

	_customMenu = new QAction(tr("&Commit"), _ui->tableWidget_events);

	_databaseLabel = new QLabel(mainWindow());
	_ui->statusBar->addPermanentWidget(_databaseLabel);

	_worker = new DataLoadWorker(composedFeed.c_str(), mainWindow());
	_worker->update();

	_timer = new QTimer(mainWindow());
	_timer->setInterval(_refreshDelay * 1000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::mapElementClicked(const QString& id) {

	//! First check if the object is from the FEED
	for (int i = 0; i < _ui->tableWidget_events->rowCount(); ++i) {

		QVariant data = _ui->tableWidget_events->item(i, 0)->data(QuakeEvent::DataRole);
		QuakeEvent event = data.value<QuakeEvent>();

		//! Remember, id contains either {FEED} or {DATABASE}, we need only
		//! verify that the current event's id is contained into the string
		if ( id.contains(event.id()) ) {
			_ui->tableWidget_events->clearSelection();
			QModelIndex idx = _ui->tableWidget_events->model()->index(i, 0);
			_ui->tableWidget_events->setCurrentIndex(idx);
			return;
		}
	}

	//! If we're here, this means the object isn't from the FEED and therefore
	//! it may be from the DATABASE, otherwise it may be a station...
	if ( id.contains("{DATABASE}") or id.contains("{FEED}") ) {
		QStringList l = id.split(' ');
		if ( l.size() > 0 ) {
			_ui->statusBar->showMessage(QString("Object with publicID %1 has been selected in events tab")
			        .arg(l.at(1)), 3000);
			_eventListWidget->selectItem(l.at(1));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString WorldEarthquakeView::updateFeedURL() {

	QString composedFeed = _configDialogUi->rssLink->text();

	QString suffix;
	if ( _ui->radioButton_feedPastHour->isChecked() )
		suffix = "_hour.atom";
	else if ( _ui->radioButton_feedPast1->isChecked() )
		suffix = "_day.atom";
	else if ( _ui->radioButton_feedPast7->isChecked() )
		suffix = "_week.atom";
	else if ( _ui->radioButton_feedPast30->isChecked() )
		suffix = "_month.atom";
	else
		suffix = "_month.atom";

	QString prefix;
	if ( _ui->radioButton_earthSignificant->isChecked() )
		prefix = "significant";
	else if ( _ui->radioButton_earth45->isChecked() )
		prefix = "4.5";
	else if ( _ui->radioButton_earth25->isChecked() )
		prefix = "2.5";
	else if ( _ui->radioButton_earth1->isChecked() )
		prefix = "1.0";
	else if ( _ui->radioButton_earthAll->isChecked() )
		prefix = "all";
	else
		prefix = "all";

	if ( composedFeed.contains("usgs") ) {
		composedFeed += prefix + suffix;
	}

	log(Gui::Client::LM_INFO, __func__, "ATOM parser new feed...");
	log(Gui::Client::LM_INFO, __func__, QString("URL: %1").arg(composedFeed));

	return composedFeed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void WorldEarthquakeView::updateItemStatus(const QString& item) {
	Q_UNUSED(item);
	populateFromLocalDatabase();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WorldEarthquakeView::showHideMenu(int) {

	if ( _configDialogUi->refreshCheckBox->isChecked() ) {
		_configDialogUi->label_2->setEnabled(true);
		_configDialogUi->label_3->setEnabled(true);
		_configDialogUi->delay->setEnabled(true);
	}
	else {
		_configDialogUi->label_3->setEnabled(false);
		_configDialogUi->label_2->setEnabled(false);
		_configDialogUi->delay->setEnabled(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



