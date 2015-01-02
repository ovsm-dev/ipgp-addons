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


#define APP_SHORTDESC "Interactive tool for editing and publishing events catalogs and bulletins."
#define APP_FULLDESC "<p>This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.<br/>"\
		    		"It has been co-financed by the European Union and le Ministère de <br/>"\
		    		"l'Ecologie, du Développement Durable, des Transports et du Logement.</p>"
#define APP_VERSION "0.1.1"

#include "bulletinexporterview.h"

#include "ui_bulletinexporterview.h"
#include "ui_filterbox.h"
#include "ui_exportdialog.h"

#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/utils/files.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/io/archive/xmlarchive.h>

#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/drawables/indicator.h>
#include <ipgp/gui/map/drawables/pin.h>
#include <ipgp/gui/datamodel/progressindicator/progressindicator.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/3rd-party/qprogressindicator/qprogressindicator.h>

#include <ipgp/core/bulletin/bulletin.h>
#include <ipgp/core/datamodel/objectcache.h>
#include <ipgp/core/math/math.h>
#include <ipgp/core/misc/misc.h>
#include <ipgp/core/geo/geo.h>
#include <ipgp/core/string/string.h>

#include <boost/algorithm/string.hpp>

#include <QFile>
#include <QTextStream>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Communication;
using namespace Seiscomp::IO;


using namespace IPGP;
using namespace IPGP::Core;
using namespace IPGP::Core::Misc;
using namespace IPGP::Core::String;
using namespace IPGP::Core::Math;
using namespace IPGP::Core::Geo;
using namespace IPGP::Gui;
using namespace IPGP::Gui::Client;
using namespace IPGP::Gui::Misc;



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BulletinExporterView::BulletinExporterView(int& argc, char** argv) :
		IPGP::Gui::Client::Application(argc, argv) {

	_ui = NULL;
	_filterBox = NULL;
	_filterForm = NULL;
	_info = NULL;
	_databaseLabel = NULL;
	_exportForm = NULL;
	_exportDialog = NULL;
	_map = NULL;
	_mapAction = NULL;
	_progressIndicator = NULL;
	_pi = NULL;
	_eventListWidget = NULL;
	_confirmedCheck = NULL;
	_autoCheck = NULL;
	_manualCheck = NULL;
	_noneCheck = NULL;
	_allCheck = NULL;

	_originCount = 0;
	_eventCount = 0;
	_pickCount = 0;
	_magnitudeCount = 0;

	_showOriginsWithNotType = false;
	_showNotLocatableOrigins = false;
	_showOutOfNetworkOrigins = false;
	_showNotExistingOrigins = false;
	_useOneFilePerBulletin = false;
	_hasQuakeMLSchema = false;
	_setEventAsFinal = false;

	setLoadStationsEnabled(true);
	setMasterMessagingGroup("LOCATION");
	messageSubscription("LOCATION");
	messageSubscription("PICK");
	messageSubscription("EVENT");
	messageSubscription("CONFIG");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BulletinExporterView::~BulletinExporterView() {

	if ( _map )
	    delete _map;
	_map = NULL;

	if ( _exportDialog )
	    delete _exportDialog;
	_exportDialog = NULL;

	if ( _ui )
	    delete _ui;
	_ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::run() {

	showMessage("Setup user interface");
	setupGraphicalInterface();
	readApplicationSettings();
	setupApplicationMenus();

	setInterfaceName("Bulletin Exporter View");
	setBinaryName("scbev");

	setApplicationDescriptor(IPGP::Gui::Client::ApplicationDescriptor(
	    interfaceName().c_str(), QString::fromUtf8(APP_SHORTDESC),
	    QString::fromUtf8(APP_FULLDESC), APP_VERSION));

	handleDatabaseState();

	showMessage("Loading events database");
	_eventListWidget->setRange(globalSettings().loadEventDB());
	_eventListWidget->readEvents();

	showNormal();

	handleHiddenTypes();
	handleSelection();

	return (bool) !QApplication::exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::initConfiguration() {

	if ( !IPGP::Gui::Client::Application::initConfiguration() )
	    return false;

	int error = 0;

	try {
		_author = configGetString("bev.export.author");
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Mandatory bev.export.author not specified");
		error++;
	}

	try {
		_institute = configGetString("bev.export.instituteName");
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Mandatory bev.export.instituteName not specified");
		error++;
	}

	try {
		_instituteTag = configGetString("bev.export.instituteTag");
	}
	catch ( ... ) {
		SEISCOMP_WARNING("Variable export.instituteTag not specified, eventID will be used");
	}

	try {
		_quakemlSchemaFile = configGetString("bev.export.quakeml.schemaFile");
		_hasQuakeMLSchema = true;
	}
	catch ( ... ) {
		SEISCOMP_WARNING("No QuakeML schema file specified, this export format has been disabled");
	}

	try {
		_useOneFilePerBulletin = configGetBool("bev.export.useOneFilePerBulletin");
	} catch ( ... ) {}
	try {
		_showNotExistingOrigins = configGetBool("bev.showNotExistingOrigins");
	} catch ( ... ) {}
	try {
		_showNotLocatableOrigins = configGetBool("bev.showNotLocatableOrigins");
	} catch ( ... ) {}
	try {
		_showOriginsWithNotType = configGetBool("bev.showOriginsWithNoType");
	} catch ( ... ) {}
	try {
		_showOutOfNetworkOrigins = configGetBool("bev.showOutOfNetworkInterestsOrigins");
	} catch ( ... ) {}

	try {
		_tempFolder = configGetString("bev.tempFolder");

		if ( _tempFolder.size() > 0 ) {

			if ( _tempFolder[_tempFolder.size() - 1] != '/' )
			    _tempFolder += '/';

			if ( !Util::pathExists(_tempFolder) ) {
				if ( !Util::createPath(_tempFolder) ) {
					SEISCOMP_WARNING("scbev failed to create path %s", _tempFolder.c_str());
					error++;
				}
			}
		}
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Mandatory bev.tempFolder not specified");
		error++;
	}

	// ---------------------------------------------------------------------
	//  EventList parameters
	// ---------------------------------------------------------------------
	try {
		_eventListSettings.useCacheEngine = configGetBool(
		    "bev.eventList.useCacheEngine");
	} catch ( ... ) {}
	try {
		_eventListSettings.useMultiThreads = configGetBool(
		    "bev.eventList.useSeparateThread");
	} catch ( ... ) {}

	_eventListSettings.showEventColorType = true;
	_eventListSettings.eventsAreCheckable = true;
	_eventListSettings.fetchSiblingOriginComment = false;
	_eventListSettings.fetchUnassociatedOriginComment = false;
	_eventListSettings.fetchUnassociated = false;
	_eventListSettings.useAdvancedOptions = false;
	_eventListSettings.decorateItems = false;

	// ---------------------------------------------------------------------
	//  Locators parameters
	// ---------------------------------------------------------------------
	std::vector<std::string> tmpLocators;
	try {
		tmpLocators = configGetStrings("bev.locators");
	} catch ( ... ) {}
	for (std::vector<std::string>::iterator it = tmpLocators.begin();
	        it != tmpLocators.end(); ++it) {

		EarthModelIDs list;
		std::string profiles = "bev.locator." + *it + ".profiles";

		try {
			list = configGetStrings(profiles);
		} catch ( ... ) {}

		if ( list.empty() ) continue;

		_locators.push_back(Locators(*it, list));
	}

	if ( error != 0 ) return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::setupGraphicalInterface() {

	IPGP::Gui::Client::Application::setupGraphicalInterface();

	_ui = new Ui::BulletinExporterView;
	_ui->setupUi(mainWindow());

	_progressIndicator = new ProgressIndicator(mainWindow());
	_progressIndicator->setModal(true);
	loadingPercentage(-1, "BulletinExporterView", "");

	_filterForm = new QDialog(mainWindow());
	_filterForm->setWindowModality(Qt::ApplicationModal);
	_filterForm->hide();
	_filterBox = new Ui::FilterBox;
	_filterBox->setupUi(_filterForm);

	_filterBox->notExistingCheckBox->setChecked(_showNotExistingOrigins);
	_filterBox->notLocatableCheckBox->setChecked(_showNotLocatableOrigins);
	_filterBox->notSetCheckBox->setChecked(_showOriginsWithNotType);
	_filterBox->outNetworkCheckBox->setChecked(_showOutOfNetworkOrigins);

	connect(_filterBox->notExistingCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideFakeEvents(const bool&)));
	connect(_filterBox->notLocatableCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideUnlocalizedEvents(const bool&)));
	connect(_filterBox->outNetworkCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideExternalEvents(const bool&)));
	connect(_filterBox->notSetCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideNoTypeSetEvents(const bool&)));

	_databaseLabel = new QLabel(mainWindow());
	_ui->statusBar->addPermanentWidget(_databaseLabel);

	_pi = new QProgressIndicator(mainWindow());
	_ui->statusBar->addPermanentWidget(_pi, 1);
	_pi->hide();

	//! Init the map widget
	_map = new MapWidget(mapDescriptor().names(), mapDescriptor().paths(),
	    mainWindow(), mapDescriptor().defaultLongitude(),
	    mapDescriptor().defaultLatitude(), mapDescriptor().tilePattern());
	_map->setScheme(scheme());

	QRect screenGeometry = QApplication::desktop()->screenGeometry();
	int x = (screenGeometry.width() - 500) / 2;
	int y = (screenGeometry.height() - 500) / 2;
	_map->setGeometry(x, y, 500, 500);
	_map->setWindowTitle("Bulletin Exporter Map");
	_map->hide();

	//! @note maintoolbar is set to have icon the size of the objects that show
	//!       texts... trying to increase the value is vain.
	//! @todo Try and use icon buttons instead of basic radio buttons for the
	//!       event type selection, see if they look better than texts...

	QLabel* lbl = new QLabel(tr("Select: "), mainWindow());
	lbl->setObjectName("EvaluationModeLabel");

	_allCheck = new QRadioButton(tr("all"), mainWindow());
	_allCheck->setObjectName("checkBox_all");
	_autoCheck = new QRadioButton(tr("auto"), mainWindow());
	_autoCheck->setObjectName("checkBox_automatic");
	_manualCheck = new QRadioButton(tr("manual"), mainWindow());
	_manualCheck->setObjectName("checkBox_manual");
	_manualCheck->setChecked(true);
	_confirmedCheck = new QRadioButton(tr("confirmed"), mainWindow());
	_confirmedCheck->setObjectName("checkBox_confirmed");
	_noneCheck = new QRadioButton(tr("none"), mainWindow());
	_noneCheck->setObjectName("checkBox_none");
	_mapAction = new QAction(tr("Show map"), mainWindow());
	_mapAction->setObjectName("qaction_map");
	_mapAction->setIcon(QIcon(":images/map.png"));

	connect(_allCheck, SIGNAL(clicked()), this, SLOT(handleSelection()));
	connect(_autoCheck, SIGNAL(clicked()), this, SLOT(handleSelection()));
	connect(_manualCheck, SIGNAL(clicked()), this, SLOT(handleSelection()));
	connect(_confirmedCheck, SIGNAL(clicked()), this, SLOT(handleSelection()));
	connect(_noneCheck, SIGNAL(clicked()), this, SLOT(handleSelection()));
	connect(_mapAction, SIGNAL(triggered()), _map, SLOT(show()));

	_ui->mainToolBar->setObjectName("EvaluationModeSelection");
	_ui->mainToolBar->addWidget(lbl);
	_ui->mainToolBar->addSeparator();
	_ui->mainToolBar->addWidget(_allCheck);
	_ui->mainToolBar->addSeparator();
	_ui->mainToolBar->addWidget(_autoCheck);
	_ui->mainToolBar->addSeparator();
	_ui->mainToolBar->addWidget(_manualCheck);
	_ui->mainToolBar->addSeparator();
	_ui->mainToolBar->addWidget(_confirmedCheck);
	_ui->mainToolBar->addSeparator();
	_ui->mainToolBar->addWidget(_noneCheck);

	QToolBar* t1 = mainWindow()->addToolBar("MapToolbar");
	t1->setObjectName("MapToolbar");
	t1->addAction(_mapAction);

	_eventListWidget = new EventListWidget(0, query(), &_cache);
	_eventListWidget->setLocationProfiles(_locators);
	_eventListWidget->setSettings(_eventListSettings);
	_eventListWidget->setLayoutMargins(0, 0, 0, 0);
	_eventListWidget->setSelectRegionWithMouse(false);

	QLayout* l = new QVBoxLayout(_ui->frameEventList);
	l->addWidget(_eventListWidget);
	_ui->frameEventList->setLayout(l);

	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(handleHiddenTypes()));
	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(handleSelection()));
	connect(_eventListWidget, SIGNAL(viewUpdateRequested()), this, SLOT(showWaitingWidget()));
	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(hideWaitingWidget()));
	connect(_eventListWidget, SIGNAL(rowClicked(const QString&)), this, SLOT(indicateItem(const QString&)));
	connect(_eventListWidget, SIGNAL(updateProgress(const int&, const QString&, const QString&)),
	    this, SLOT(loadingPercentage(const int&, const QString&, const QString&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::setupApplicationMenus() {

	connect(_ui->actionExit, SIGNAL(triggered()), this, SLOT(exitGui()));
	connect(_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
	connect(_ui->actionLoadedPlugins, SIGNAL(triggered()), this, SLOT(showPlugins()));
	connect(_ui->actionExportBulletin, SIGNAL(triggered()), this, SLOT(exportData()));
	connect(_ui->actionConfigureConnection, SIGNAL(triggered()), this, SLOT(showConnectionDialog()));
	connect(_ui->actionExportMap, SIGNAL(triggered()), _map, SLOT(savePicture()));
	connect(_ui->actionConfigureBulletinExporterView, SIGNAL(triggered()), _filterForm, SLOT(show()));
	connect(this, SIGNAL(changeDatabaseIcon()), this, SLOT(handleDatabaseState()));
	connect(this, SIGNAL(databaseChanged(const QString&, const QString&)), this, SLOT(databaseMessage(const QString&, const QString&)));
	connect(_map, SIGNAL(elementClicked(const QString&)), _eventListWidget, SLOT(selectItem(const QString&)));
	connect(_ui->actionDocumentationIndex, SIGNAL(triggered()), this, SLOT(showGeneralDocumentation()));
	connect(_ui->actionDocumentationForScbev, SIGNAL(triggered()), this, SLOT(showDocumentation()));
	connect(_ui->actionShowLog, SIGNAL(triggered()), this, SLOT(showLog()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::writeHypoCatalog(EventPtr event,
                                            OriginPtr origin,
                                            MagnitudePtr mag,
                                            const std::string& id) {

	std::string date = origin->time().value().toString("%Y%m%d");
	std::string hour = origin->time().value().toString("%H%M");
	std::string sec = origin->time().value().toString("%S.%2f");
	std::string lat = decimalToSexagesimal(origin->latitude().value());
	std::string lon = decimalToSexagesimal(origin->longitude().value());

	std::string depth = "";
	try {
		depth = stringify("%7.2f", origin->depth().value());
	} catch ( ... ) {}

	std::string magnitude;
	if ( mag )
		try {
			magnitude = stringify("%5.2f", mag->magnitude().value());
		} catch ( ... ) {}
	else
		magnitude = "";

	std::string dMin = "";
	try {
		if ( Seiscomp::Math::Geo::deg2km(origin->quality().minimumDistance()) > 999.99 )
			dMin = stringify("%5.0f", Seiscomp::Math::Geo::deg2km(origin->quality().minimumDistance()));
		else
			dMin = stringify("%5.1f", Seiscomp::Math::Geo::deg2km(origin->quality().minimumDistance()));
	} catch ( ... ) {}

	// Teleseismic fix: ensure nothing comes and perturb the 5chars rule
	if ( dMin.length() > 5 )
	    dMin = "";

	std::string rms = "";
	try {
		rms = stringify("%5.2f", origin->quality().standardError());
	} catch ( ... ) {}

	// Teleseismic fix: ensure nothing comes and perturb the 5 chars rule
	if ( rms.length() > 5 )
	    rms = "";

	std::string erh = "";
	try {
		erh = stringify("%5.1f", origin->latitude().uncertainty());
	} catch ( ... ) {}

	std::string erz = "";
	try {
		erz = stringify("%5.1f", origin->depth().uncertainty());
	} catch ( ... ) {}

	std::string quality = " ";
	try {
		quality = origin->quality().groundTruthLevel();
	} catch ( ... ) {}

	std::string scode;
	scode = _eventListWidget->getSeismicCode(event->publicID().c_str()).toStdString();

	std::string azigap;
	try {
		azigap = stringify("%4.0f", origin->quality().azimuthalGap());
	} catch ( ... ) {}

	// Date YYYYMMDD col 1-9
	_hypoCatalog.append(date);

	// Hour Minute col 10-14
	_hypoCatalog.append(addWhiteSpace(hour, 5, 0));

	// Seconds col 14-20 F6.2
	_hypoCatalog.append(addWhiteSpace(sec, 6, 0));

	// Latitude col 20-29
	_hypoCatalog.append(addWhiteSpace(lat, 9, 0));

	// Longitude col 30-39
	_hypoCatalog.append(addWhiteSpace(lon, 10, 0));

	// Depth col 39-46
	_hypoCatalog.append(addWhiteSpace(depth, 7, 0));

	// Blank col 46-47
	_hypoCatalog.append(addWhiteSpace("", 1, 0));

	// Magnitude code col 47-48
	if ( mag ) {
		if ( mag->type().length() > 1 ) {
			std::string magLetter = mag->type().substr(1, 1);
			boost::to_upper(magLetter);
			_hypoCatalog.append(addWhiteSpace(magLetter, 1, 0));
		}
	}
	else {
		_hypoCatalog.append(addWhiteSpace("", 1, 0));
	}

	// Magnitude col 48-53
	_hypoCatalog.append(addWhiteSpace(magnitude, 5, 0));

	// Number of P & S times with weights greater than 0.1.
	// becomes here the number of phases col 53-56
	_hypoCatalog.append(addWhiteSpace(toString(origin->arrivalCount()), 3, 0));

	// Azimuthal gap col 56-60
	_hypoCatalog.append(addWhiteSpace(stripWhiteSpace(azigap), 4, 0));

	// Distance to nearest station col 60-65
	_hypoCatalog.append(addWhiteSpace(stripWhiteSpace(dMin), 5, 0));

	// RMS travel time residual col 65-70
	_hypoCatalog.append(addWhiteSpace(stripWhiteSpace(rms), 5, 0));

	// Horizontal error (km) col 70-75
	_hypoCatalog.append(addWhiteSpace(stripWhiteSpace(erh), 5, 0));

	// Vertical error (km) col 75-80
	_hypoCatalog.append(addWhiteSpace(stripWhiteSpace(erz), 5, 0));

	// Remark assigned by analyst (i.e. Q for quarry blast)
	_hypoCatalog.append(addWhiteSpace("", 1, 0));

	// Quality code A-D
	_hypoCatalog.append(addWhiteSpace(quality, 1, 0));

	// Most common data source (i.e. W= earthworm)
	_hypoCatalog.append(addWhiteSpace("", 1, 0));

	// Auxiliary remark from program (i.e. “-“ for depth fixed, etc.)
	_hypoCatalog.append(addWhiteSpace("", 1, 0));

	_hypoCatalog.append(addWhiteSpace(scode, 6, 1));
	_hypoCatalog.append(id);
	_hypoCatalog.append("\n");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::writeHypoBulletin(std::string* str,
                                             EventPtr event, OriginPtr origin) {

	PickList picks;
	int errors = 0;

	std::string scode = "";
	scode = _eventListWidget->getSeismicCode(event->publicID().c_str()).toStdString();
	scode = stripWhiteSpace(scode);

	std::string id = "";
	if ( _instituteTag == "%originID%" )
		id = event->preferredOriginID();
	else if ( _instituteTag == "%eventID%" )
		id = event->publicID();
	else
		id = origin->time().value().toString(_instituteTag.c_str());
//		id = origin->time().value().toString("%Y%m%d") + "_"
//		        + origin->time().value().toString("%H%M") + _instituteTag;

	if ( origin->arrivalCount() == 0 )
	    query()->loadArrivals(origin.get());

	for (size_t i = 0; i < origin->arrivalCount(); ++i) {

		PickPtr pick = Pick::Cast(PublicObjectPtr(query()->getObject(
		    Pick::TypeInfo(), origin->arrival(i)->pickID())));

		if ( pick )
			picks.push_back(pick);
		else {
			SEISCOMP_DEBUG("No pick %s found for arrival of origin %s",
			    origin->arrival(i)->pickID().c_str(), origin->publicID().c_str());
		}
	}

	std::string magnitude;
	MagnitudePtr mag = Magnitude::Cast(PublicObjectPtr(query()->getObject(
	    Magnitude::TypeInfo(), event->preferredMagnitudeID())));
	if ( mag )
	    try {
		    magnitude = "M=" + stringify("%3.1f", mag->magnitude().value());
	    } catch ( ... ) {}


	// Adding line to catalog content
	writeHypoCatalog(event, origin, mag, id);

	bool isPPhase = false;
	bool isSPhase = false;
	bool foundFAS = false;
	double refTime, refTimeSec, prevRefTime, prevRefTimeSec;
//	int refTimeMin, prevRefTimeMin;
	std::string refTimeYear, refTimeMonth, refTimeDay, refTimeHour, refStation;
	std::string prevRefTimeYear, prevRefTimeMonth, prevRefTimeDay,
	        prevRefTimeHour, prevRefStation;

	refTime = refTimeSec = prevRefTime = prevRefTimeSec = .0;
//	refTimeMin = prevRefTimeMin = 0;

	// uncertainty values
	double maxUncertainty = -1, minUncertainty = 100;
	std::string maxWeight = "0";

	for (size_t i = 0; i < picks.size(); ++i) {

		PickPtr p = picks.at(i);

		double ctime = (double) p->time().value();

		if ( refTime == 0 )
		    refTime = ctime;

		if ( p->phaseHint().code().find("P") != std::string::npos ) {

			if ( ctime <= refTime ) {
				refTime = ctime;
				refTimeYear = p->time().value().toString("%Y");
				refTimeMonth = p->time().value().toString("%m");
				refTimeDay = p->time().value().toString("%d");
				refTimeHour = p->time().value().toString("%H");
//				refTimeMin = stringToInt(p->time().value().toString("%M"));
				refTimeSec = stringToDouble(p->time().value().toString("%S.%f"));
				refStation = toString(p->waveformID().stationCode());
				foundFAS = true;

			}
			else {
				prevRefTime = ctime;
				prevRefTimeYear = p->time().value().toString("%Y");
				prevRefTimeMonth = p->time().value().toString("%m");
				prevRefTimeDay = p->time().value().toString("%d");
				prevRefTimeHour = p->time().value().toString("%H");
//				prevRefTimeMin = stringToInt(p->time().value().toString("%M"));
				prevRefTimeSec = stringToDouble(p->time().value().toString("%S.%f"));
				prevRefStation = toString(p->waveformID().stationCode());
			}
		}

		double upper = .0;
		double lower = .0;

		try {
			if ( p->time().upperUncertainty() != .0 )
			    upper = p->time().upperUncertainty();

			if ( p->time().lowerUncertainty() != .0 )
			    lower = p->time().lowerUncertainty();

			if ( (lower + upper) > maxUncertainty )
			    maxUncertainty = lower + upper;

			if ( (lower + upper) < minUncertainty )
			    minUncertainty = lower + upper;
		} catch ( ... ) {}
	}

	if ( foundFAS != true ) {
		refTime = prevRefTime;
		refTimeYear = prevRefTimeYear;
		refTimeMonth = prevRefTimeMonth;
		refTimeDay = prevRefTimeDay;
		refTimeHour = prevRefTimeHour;
//		refTimeMin = prevRefTimeMin;
		refTimeSec = prevRefTimeSec;
		refStation = prevRefStation;
	}

	if ( refStation.compare("") == 0 ) {
		errors++;
		SEISCOMP_DEBUG("%s couldn't identify the reference station of %s",
		    __func__, event->publicID().c_str());
		return;
	}

	int sharedTime = ((int) (refTime / 3600) * 3600);
	std::string oDate = refTimeYear.substr(2, 2) + refTimeMonth + refTimeDay;
	std::string h71PWeight;
	std::string h71SWeight;
	std::vector<std::string> Tstation;
	int line = 0;

	for (size_t i = 0; i < picks.size(); ++i) {

		PickPtr pick = picks.at(i);
		std::string pMinute;
		std::string pSec;
		std::string sSec;
		std::string Ppolarity = "", Spolarity = "";
		std::string stationCode = pick->waveformID().stationCode();
		std::string networkCode = pick->waveformID().networkCode();

		char buffer[10] = "";
		double pmin = .0, psec = .0, ssec = .0;

		// Signal duration
		char fp[6] = "";

		/*
		 * pick->phaseHint().code() = P
		 * we're just gonna look for an eventual S-phase
		 * if there is none, we only add the P-phase
		 * and that's it.
		 */
		if ( pick->phaseHint().code().find("P") != std::string::npos ) {

			bool isIntegrated = false;
			for (size_t x = 0; x < Tstation.size(); ++x)
				if ( Tstation[x] == stripWhiteSpace(stationCode) )
				    isIntegrated = true;

			if ( isIntegrated == false ) {

				AmplitudePtr amp = query()->getAmplitude(pick->publicID(), pick->phaseHint().code());
				if ( amp )
				    try {
					    if ( amp->period().value() != .0 )
					        sprintf(fp, "%04.0f", amp->period().value());
				    } catch ( ... ) {}

				Tstation.push_back(stripWhiteSpace(stationCode));
				isPPhase = true;
				pmin = sharedTime + ((int) (((double) pick->time().value() - sharedTime) / 60)) * 60;
				double newmin = pmin / 60 - (int) (sharedTime / 3600) * 60;
				pMinute = toString((int) newmin);
				psec = getTimeValue(picks, networkCode, stationCode, "P", 0) - pmin;
				sprintf(buffer, "%#02.2f", psec);
				pSec = toString(buffer);
				Ppolarity = getH71PickPolarity(picks, stationCode, "P");

				try {
					h71PWeight = toString(getHypoWeight(picks, origin, networkCode, stationCode, "P", maxUncertainty));
				}
				catch ( ... ) {
					h71PWeight = maxWeight;
				}

				ssec = getTimeValue(picks, networkCode, stationCode, "S", 0) - pmin;

				if ( ssec > 0. ) {

					Spolarity = getH71PickPolarity(picks, stationCode, "S");

					// if ssec > 99.99 then it won't fit into a F5.2 so we convert it into a F5.1
					if ( ssec > 99.99 )
						sprintf(buffer, "%#03.1f", ssec);
					else
						sprintf(buffer, "%#02.2f", ssec);
					sSec = toString(buffer);
					isSPhase = true;
					try {
						h71SWeight = toString(getHypoWeight(picks, origin, networkCode, stationCode, "S", maxUncertainty));
					}
					catch ( ... ) {
						h71SWeight = maxWeight;
					}
				}
			}
		}


		std::string duration = "d=\"";


		//! writing down P-phase with S-phase
		if ( isPPhase == true && isSPhase == true ) {

			_pickCount += 2;

			//! station name //! alphanumeric 4
			*str += addWhiteSpace(stationCode, 4, 1);

			//! description of onset of P-arrival //! alphanumeric 1
			*str += addWhiteSpace("E", 1, 0);

			//! 'P' to denote P-arrival //! alphanumeric 1
			*str += addWhiteSpace("P", 1, 0);

			//! first motion direction of P-arrival //! alphanumeric 1
			*str += addWhiteSpace(Ppolarity, 1, 0);

			//! weight assigned to P-arrival //! float 1.0
			*str += addWhiteSpace(h71PWeight, 1, 0);

			//! blank space between 8-10
			*str += addWhiteSpace("", 1, 0);

			//! year, month and day of P-arrival //! integer 6
			*str += addWhiteSpace(oDate, 6, 0);

			//! hour of P-arrival //! integer 2
			*str += addWhiteSpace(refTimeHour, 2, 0);

			//! minute of P-arrival //! integer 2
			*str += addWhiteSpace(pMinute, 2, 0);

			//! second of P-arrival //! float 5.2
			*str += addWhiteSpace(pSec, 5, 0);

			//! blank space between 24-32
			*str += addWhiteSpace("", 7, 0);

			//! second of S-arrival //! float of 5.2
			*str += addWhiteSpace(sSec, 5, 0);

			//! description of onset S-arrival //! alphanumeric 1
			*str += addWhiteSpace("E", 1, 0);

			//! 'S' to denote S-arrival //! alphanumeric 1
			*str += addWhiteSpace("S", 1, 0);

			//! first motion direction //! alphanumeric 1
			*str += addWhiteSpace(Spolarity, 1, 0);

			//! weight assigned to S-arrival //! float 1.0
			*str += addWhiteSpace(h71SWeight, 1, 0);

			//! maximum peak-to-peak amplitude in mm //! float 4.0
			*str += addWhiteSpace("", 4, 0);

			//! period of the maximum amplitude in sec //! float 3.2
			*str += addWhiteSpace("", 3, 0);

			//! usually not used except as note in next item //! float 4.1
			*str += addWhiteSpace("", 4, 0);

			//! blank space between 54-59
			*str += addWhiteSpace("", 5, 0);

			//! peak-to-peak amplitude of 10 microvolts calibration signal in mm //! float 4.1
			*str += addWhiteSpace("", 4, 0);

			//! remark for this phase card //! alphanumeric 3
			*str += addWhiteSpace("", 3, 0);

			//! time correction in sec //! float 5.2
			*str += addWhiteSpace("", 5, 0);

			//! blank
			*str += addWhiteSpace("", 2, 0);

			// pos 72 / len 4 : Y2000 Coda duration F4.0
			*str += addWhiteSpace(fp, 5, 0);

			*str += addWhiteSpace("", 3, 0); // Last position (76)


			if ( line == 0 ) {
				if ( mag )
					*str += addWhiteSpace(magnitude, 6, 1); // Magnitude
				else
					*str += addWhiteSpace(duration, 6, 1); // Magnitude

				*str += addWhiteSpace(scode, 5, 1); // Seismic code
				*str += addWhiteSpace("", 1, 0); // blank
				*str += id;

				line++;
			}

			*str += "\n";

			isSPhase = false;
			isPPhase = false;
			stationCode = "";

		} // end writing down P-phase with S-phase


		// writing down P-phase without S-phase
		if ( isPPhase == true && isSPhase == false ) {

			_pickCount++;

			//! station name //! alphanumeric 4
			//<< Hypo71::addWhiteSpace(toString(pick->waveformID().stationCode()), 4, 1)
			*str += addWhiteSpace(stationCode, 4, 1);

			//! description of onset of P-arrival //! alphanumeric 1
			*str += addWhiteSpace("E", 1, 0);

			//! 'P' to denote P-arrival //! alphanumeric 1
			*str += addWhiteSpace("P", 1, 0);

			//! first motion direction of P-arrival //! alphanumeric 1
			*str += addWhiteSpace(Ppolarity, 1, 0);

			//! weight assigned to P-arrival //! float 1.0
			*str += addWhiteSpace(h71PWeight, 1, 0);

			//! blank space between 8-10
			*str += addWhiteSpace("", 1, 0);

			//! year, month and day of P-arrival //! integer 6
			*str += addWhiteSpace(oDate, 6, 0);

			//! hour of P-arrival //! integer 2
			*str += addWhiteSpace(refTimeHour, 2, 0);

			//! minute of P-arrival //! integer 2
			*str += addWhiteSpace(pMinute, 2, 0);

			//! second of P-arrival //! float 5.2
			*str += addWhiteSpace(pSec, 5, 0);

			//! blank space between 24-32
			*str += addWhiteSpace("", 7, 0);

			//! second of S-arrival //! float of 5.2
			*str += addWhiteSpace(sSec, 5, 0);

			//! description of onset S-arrival //! alphanumeric 1
			*str += addWhiteSpace("", 1, 0);

			//! 'S' to denote S-arrival //! alphanumeric 1
			*str += addWhiteSpace("", 1, 0);

			//! first motion direction //! alphanumeric 1
			*str += addWhiteSpace("", 1, 0);

			//! weight assigned to S-arrival //! float 1.0
			*str += addWhiteSpace("", 1, 0);

			//! maximum peak-to-peak amplitude in mm //! float 4.0
			*str += addWhiteSpace("", 4, 0);

			//! period of the maximum amplitude in sec //! float 3.2
			*str += addWhiteSpace("", 3, 0);

			//! normally not used except as note in next item //! float 4.1
			*str += addWhiteSpace("", 4, 0);

			//! blank space between 54-59
			*str += addWhiteSpace("", 5, 0);

			//! peak-to-peak amplitude of 10 microvolts calibration signal in mm //! float 4.1
			*str += addWhiteSpace("", 4, 0);

			//! remark for this phase card //! alphanumeric 3
			*str += addWhiteSpace("", 3, 0);

			//! time correction in sec //! float 5.2
			*str += addWhiteSpace("", 5, 0);

			//! blank
			*str += addWhiteSpace("", 2, 0);

			// pos 72 / len 4 : Y2000 Coda duration F4.0
			*str += addWhiteSpace(fp, 5, 0);

			*str += addWhiteSpace("", 3, 0); // Last position (76)


			if ( line == 0 ) {

				if ( mag )
					*str += addWhiteSpace(magnitude, 6, 1);
				else
					*str += addWhiteSpace(duration, 6, 1);

				*str += addWhiteSpace(scode, 5, 1); // Seismic code
				*str += addWhiteSpace("", 1, 0); // blank
				*str += id;

				line++;
			}
			*str += "\n";

			isSPhase = false;
			isPPhase = false;
			stationCode = "";
		} // end writing down P-phase without S-phase

	} // end for loop

	*str += addWhiteSpace("10", 19, 0) + "\n";

	if ( errors != 0 )
	    SEISCOMP_DEBUG("Some errors have been raised while publishing bulletin picks");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::writeBulletinToFile(const std::string& str,
                                               const EXPORT_TYPE& type) {

	bool retCode = false;

	if ( str.compare("") == 0 )
		QMessageBox::critical(mainWindow(), tr("Error"),
		    QString("No data have been generated by export module."));
	else {

		if ( type == eHYPO71 ) {
			QString outCat = _outputFile + QString(".CATALOG.TXT");
			QFile catfile(outCat);
			if ( !catfile.open(QIODevice::WriteOnly) ) {
				return false;
			}
			else {
				catfile.write(_hypoCatalog.c_str());
				catfile.close();
			}
			_outputFile.append(".TXT");
		}
		else if ( type == eGSE )
			_outputFile.append(".gse");
		else if ( type == eIMS )
			_outputFile.append(".ims");
		else if ( type == eQML || type == eSC3ML )
		    _outputFile.append(".xml");

		QFile file(_outputFile);
		if ( !file.open(QIODevice::WriteOnly) ) {
			QMessageBox::critical(mainWindow(), tr("Error"),
			    QString("Couldn't open file %1.\n"
				    "Please make sure the storage unit is writable.").arg(_outputFile));
			return false;
		}
		else {
			file.write(str.c_str());
			file.close();
		}

		retCode = true;

		QMessageBox::information(mainWindow(), tr("Successful export"),
		    QString::fromUtf8("<p>Exported %1 event(s) containing :<br/>"
			    "  * %2 origin(s)<br/> * %3 pick(s)<br/> * %4 magnitude(s)</p>").arg(_eventCount)
		            .arg(_originCount).arg(_pickCount).arg(_magnitudeCount));
	}

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::databaseMessage(const QString& oldDB, const QString& newDB) {

	QMessageBox::information(mainWindow(), "Information",
	    QString("The database connection to %1 has been replaced by %2, "
		    "objects will be updated after closing this window").arg(oldDB)
	            .arg(newDB));

	_eventListWidget->setDatabase(query());
	_eventListWidget->readEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::handleDatabaseState() {
	_databaseLabel->setPixmap(databaseIcon());
	_databaseLabel->setToolTip(databaseServer().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::loadingPercentage(const int& value,
                                             const QString& entity,
                                             const QString& message) {
	if ( value > 0 ) {
		_progressIndicator->show();
		_progressIndicator->setValue(value, entity, message);
	}
	else
		_progressIndicator->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::showWaitingWidget() {
	mainWindow()->setCursor(Qt::WaitCursor);
	_pi->startAnimation();
	_pi->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::hideWaitingWidget() {
	mainWindow()->setCursor(Qt::ArrowCursor);
	_pi->stopAnimation();
	_pi->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void BulletinExporterView::updateMapItems() {

	_map->foregroundCanvas().clearGeometries();

	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 )
	    return;

	for (int i = 0; i < list.size(); ++i) {

		QString publicID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) {
			_map->foregroundCanvas().removeGeometry(publicID);
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

		MagnitudePtr mag;
		mag = Magnitude::Find(event->preferredMagnitudeID());
		if ( !mag )
		    Magnitude::Cast(PublicObjectPtr(query()->getObject(
		        Magnitude::TypeInfo(), event->preferredMagnitudeID())));

		QString magnitude = QString("-");
		QString magType = QString("");

		bool hasMagnitude = false;
		double magValue;
		magValue = .0;
		if ( mag ) {
			try {
				magValue = mag->magnitude().value();
				magnitude = QString::number(roundDouble(mag->magnitude().value(), 1),
				    'f', scheme().generalPrecision());
				hasMagnitude = true;
			} catch ( ... ) {}
			magType = mag->type().c_str();
		}

		QString author = "-";
		try {
			author = origin->creationInfo().author().c_str();
		} catch ( ... ) {}

		Map::Epicenter* epicenter = new Map::Epicenter;
		epicenter->setBrush(getDepthColoration(depthValue));

		epicenter->setToolTip(epicenterTooltip(origin->publicID().c_str(),
		    origin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str(),
		    getStringPosition(origin->latitude().value(), Latitude).c_str(),
		    latU, getStringPosition(origin->longitude().value(), Longitude).c_str(), lonU,
		    depth, depthUncertainty, rms, azimuth, magnitude, magType, dist,
		    Regions::getRegionName(origin->latitude().value(), origin->longitude().value()).c_str(),
		    author));

		if ( hasMagnitude ) {
			epicenter->pen().setColor(Qt::black);
			epicenter->setSize(QSizeF(1.5 * magValue, 1.5 * magValue));
		}
		else {
			epicenter->pen().setColor(getDepthColoration(depthValue));
			epicenter->setSize(QSizeF(4., 4.));
		}
		epicenter->setGeoPosition(QPointF(origin->longitude().value(), origin->latitude().value()));
		epicenter->pen().setWidthF(1.);
		epicenter->setName(event->publicID().c_str());

		if ( !_map->foregroundCanvas().addGeometry(epicenter) ) {
			delete epicenter;
			epicenter = NULL;
		}
	}
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::indicateItem(const QString& publicID) {

	// Show/hide
	updateMapItems();

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

	if ( !origin )
	    return;

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
void BulletinExporterView::hideUnlocalizedEvents(const bool& show) {
	_eventListWidget->hideUnlocalizedEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::hideExternalEvents(const bool& show) {
	_eventListWidget->hideExternalEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::hideNoTypeSetEvents(const bool& show) {
	_eventListWidget->hideNoTypeSetEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::hideFakeEvents(const bool& show) {
	_eventListWidget->hideFakeEvents(show ? false : true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::handleSelection() {

	QList<EventListWidget::EvaluationModeSelection> choice;

	if ( _allCheck->isChecked() )
	    choice << EventListWidget::emsALL;
	if ( _manualCheck->isChecked() )
	    choice << EventListWidget::emsMANUAL;
	if ( _autoCheck->isChecked() )
	    choice << EventListWidget::emsAUTOMATIC;
	if ( _confirmedCheck->isChecked() )
	    choice << EventListWidget::emsCONFIRMED;
	if ( _noneCheck->isChecked() )
	    choice << EventListWidget::emsNONE;

	_eventListWidget->setItemsChecked(choice);

	updateMapItems();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::handleHiddenTypes() {

	hideUnlocalizedEvents(_showNotLocatableOrigins);
	hideExternalEvents(_showOutOfNetworkOrigins);
	hideNoTypeSetEvents(_showOriginsWithNotType);
	hideFakeEvents(_showNotExistingOrigins);

	updateMapItems();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::exportData() {

	_exportDialog = new QDialog(mainWindow());
	_exportDialog->setWindowModality(Qt::ApplicationModal);

	_exportForm = new Ui::Dialog;
	_exportForm->setupUi(_exportDialog);

	connect(_exportForm->gseButton, SIGNAL(clicked()), this, SLOT(exportAsGSE()));
	connect(_exportForm->imsButton, SIGNAL(clicked()), this, SLOT(exportAsIMS()));
	connect(_exportForm->hypoButton, SIGNAL(clicked()), this, SLOT(exportAsHYPO71()));
	connect(_exportForm->quakeButton, SIGNAL(clicked()), this, SLOT(exportAsQUAKEML()));

	_exportDialog->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BulletinExporterView::showOriginWarning() {

	if ( _filterBox->originFinalCheckBox->isChecked() ) {

		QMessageBox::StandardButton q;
		q = QMessageBox::question(mainWindow(),
		    QString::fromUtf8("Warning"),
		    QString::fromUtf8("Option \"set events as FINAL...\" is activated!\n"
			    "Click [Yes] to do so or [NO] to ignore this function"),
		    QMessageBox::Yes | QMessageBox::No);

		if ( q == QMessageBox::Yes )
			_setEventAsFinal = true;
		else
			_setEventAsFinal = false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::exportAsGSE() {

	if ( !getOutputFile() ) return false;

	// objects counters
	_eventCount = 0;
	_originCount = 0;
	_pickCount = 0;
	_magnitudeCount = 0;

	if ( _exportDialog ) {
		_exportDialog->hide();
		delete _exportDialog;
		_exportDialog = NULL;
	}

	showOriginWarning();

	Time now = Time::GMT();
	std::string outputString;
	outputString += "BEGIN GSE2.0\n";
	outputString += "MSG_TYPE DATA\n";
	outputString += "MSG_ID " + now.toString("%Y/%m/%d_%H%M%S") + " " + _author + "\n";
	outputString += "DATA_TYPE BULLETIN GSE2.0\n";


	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 ) {
		SEISCOMP_ERROR("There is no events checked to export");
		return false;
	}

	loadingPercentage(0, applicationName(), "Creating GSE bulletin");

	for (int i = 0; i < list.size(); ++i) {

		QString publicID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) continue;

		EventPtr event = _eventListWidget->getEvent(publicID.toStdString());

		if ( !event ) {
			SEISCOMP_DEBUG("Skipped event %s because it has not been found", event.get()->publicID().c_str());
			continue;
		}

		OriginPtr origin = _eventListWidget->getOrigin(event->preferredOriginID());

		if ( !origin ) {
			SEISCOMP_DEBUG("Skipped event %s, not origin found", event.get()->publicID().c_str());
			continue;
		}

		Bulletin bul(event.get(), query());

		if ( !bul.getObjects() ) {
			log(LM_ERROR, __func__, "Can't get objects to create GSE 2.0 bulletin");
			SEISCOMP_ERROR("Can't get objects to create bulletin");
			return false;
		}

		outputString += bul.getBulletin(IPGP::Core::Bulletin::GSE2_0);

		_eventCount++;
		_originCount++;

		if ( _setEventAsFinal ) {

			Notifier::SetEnabled(true);

			origin->setEvaluationStatus(EvaluationStatus(FINAL));

			NotifierPtr n = new Notifier("EventParameters", OP_UPDATE, origin.get());
			NotifierMessagePtr m = new NotifierMessage;
			m->attach(n.get());
			connection()->send(m.get());

			Notifier::SetEnabled(false);
		}

		try {
			loadingPercentage(percentageOfSomething<int>(list.size(), i),
			    applicationName(), "Writing down GSE bulletin");
		} catch ( ... ) {}
	}

	loadingPercentage(-1, applicationName(), "");

	outputString += "STOP\n";

	return writeBulletinToFile(outputString, eGSE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::exportAsIMS() {

	if ( !getOutputFile() )
	    return false;

	// objects counters
	_eventCount = 0;
	_originCount = 0;
	_pickCount = 0;
	_magnitudeCount = 0;

	if ( _exportDialog ) {
		_exportDialog->hide();
		delete _exportDialog;
		_exportDialog = NULL;
	}

	showOriginWarning();

	Time now = Time::GMT();
	std::string outputString;
	outputString += "BEGIN IMS1.0\n";
	outputString += "MSG_TYPE DATA\n";
	outputString += "MSG_ID " + now.toString("%Y/%m/%d_%H%M%S") + " " + _author + "\n";
	outputString += "BULLETIN (IMS1.0:SHORT FORMAT)\n";
	outputString += "DATA_TYPE BULLETIN IMS1.0:short\n";


	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 ) {
		SEISCOMP_ERROR("There is no events checked to export");
		return false;
	}

	loadingPercentage(0, applicationName(), "Creating IMS bulletin");

	for (int i = 0; i < list.size(); ++i) {

		QString publicID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) continue;

		EventPtr event = _eventListWidget->getEvent(publicID.toStdString());

		if ( !event ) {
			SEISCOMP_DEBUG("Skipped event %s because it has not been found", event.get()->publicID().c_str());
			continue;
		}

		OriginPtr origin = _eventListWidget->getOrigin(event->preferredOriginID());

		if ( !origin ) {
			SEISCOMP_DEBUG("Skipped event %s, not origin found", event.get()->publicID().c_str());
			continue;
		}

		Bulletin bul(event.get(), query());

		if ( !bul.getObjects() ) {
			log(LM_ERROR, __func__, "Can't get objects to create IMS 1.0 bulletin");
			SEISCOMP_ERROR("Can't get objects to create bulletin");
			return false;
		}

		outputString += bul.getBulletin(IPGP::Core::Bulletin::IMS1_0);

		_eventCount++;
		_originCount++;

		if ( _setEventAsFinal ) {

			Notifier::SetEnabled(true);

			origin->setEvaluationStatus(EvaluationStatus(FINAL));

			NotifierPtr n = new Notifier("EventParameters", OP_UPDATE, origin.get());
			NotifierMessagePtr m = new NotifierMessage;
			m->attach(n.get());
			connection()->send(m.get());

			Notifier::SetEnabled(false);
		}

		try {
			loadingPercentage(percentageOfSomething<int>(list.size(), i),
			    applicationName(), "Writing down IMS bulletin");
		} catch ( ... ) {}
	}

	loadingPercentage(-1, applicationName(), "");

	outputString += "STOP\n";

	return writeBulletinToFile(outputString, eIMS);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::exportAsHYPO71() {

	if ( !getOutputFile() ) return false;

	// objects counters
	_eventCount = 0;
	_originCount = 0;
	_pickCount = 0;
	_magnitudeCount = 0;

	if ( _exportDialog ) {
		_exportDialog->hide();
		delete _exportDialog;
		_exportDialog = NULL;
	}

	showOriginWarning();

	_hypoCatalog.clear();
	std::string outputString;

	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 ) {
		SEISCOMP_ERROR("There is no events checked to export");
		return false;
	}

	loadingPercentage(0, applicationName(), "Creating HYPO71 bulletin");

	for (int i = 0; i < list.size(); ++i) {

		QString publicID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) continue;

		EventPtr event = _eventListWidget->getEvent(publicID.toStdString());

		if ( !event ) {
			SEISCOMP_DEBUG("Skipped event %s because it has not been found", event.get()->publicID().c_str());
			continue;
		}

		OriginPtr origin = _eventListWidget->getOrigin(event->preferredOriginID());

		if ( !origin ) {
			SEISCOMP_DEBUG("Skipped event %s, not origin found", event.get()->publicID().c_str());
			continue;
		}

		writeHypoBulletin(&outputString, event, origin);

		_eventCount++;
		_originCount++;

		if ( _setEventAsFinal ) {

			Notifier::SetEnabled(true);

			origin->setEvaluationStatus(EvaluationStatus(FINAL));

			NotifierPtr n = new Notifier("EventParameters", DataModel::OP_UPDATE, origin.get());
			NotifierMessagePtr m = new NotifierMessage;
			m->attach(n.get());
			connection()->send(m.get());

			Notifier::SetEnabled(false);
		}

		try {
			loadingPercentage(percentageOfSomething<int>(list.size(), i),
			    applicationName(), "Writing down HYPO71 bulletin");
		} catch ( ... ) {}
	}

	loadingPercentage(-1, applicationName(), "");

	return writeBulletinToFile(outputString, eHYPO71);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string BulletinExporterView::getEventType(EventType type) const {

	std::string evtType;

	if ( type == EARTHQUAKE )
		evtType = "ke";
	else
	if ( type == INDUCED_EARTHQUAKE )
		evtType = "si";
	else
	if ( type == CHEMICAL_EXPLOSION )
		evtType = "kh";
	else
	if ( type == NUCLEAR_EXPLOSION )
		evtType = "kn";
	else
	if ( type == LANDSLIDE )
		evtType = "ls";
	else
	if ( type == ROCKSLIDE )
		evtType = "kr";
	else
		evtType = "uk";

	return evtType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int BulletinExporterView::getHypoWeight(const PickList& picks,
                                              Seiscomp::DataModel::OriginPtr,
                                              const std::string& networkCode,
                                              const std::string stationCode,
                                              const std::string& phaseCode,
                                              const double& max) {

	int weight = 4;
	double upper = .0, lower = .0;
	std::string pickID;

	for (PickList::const_iterator it = picks.begin();
	        it != picks.end(); ++it) {

		PickPtr pick = *it;

		if ( pick->phaseHint().code().find(phaseCode) == std::string::npos )
		    continue;

		if ( pick->waveformID().networkCode() != networkCode )
		    continue;

		if ( pick->waveformID().stationCode() != stationCode )
		    continue;

		pickID = pick->publicID();
		try {
			upper = pick->time().upperUncertainty();
		} catch ( ... ) {}
		try {
			lower = pick->time().lowerUncertainty();
		} catch ( ... ) {}

		break;
	}

	if ( pickID != "" ) {
		if ( weight != 1.0 )
			weight = 4;
		else
			weight = (int) round((3 / max) * (upper + lower));
	}

	return weight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double BulletinExporterView::getTimeValue(const PickList& picks,
                                                const std::string& networkCode,
                                                const std::string& stationCode,
                                                const std::string& phaseCode,
                                                const size_t& rtype) {

	double time = -1.;
	for (PickList::const_iterator it = picks.begin();
	        it != picks.end(); ++it) {

		PickPtr pick = *it;

		if ( pick->phaseHint().code().find(phaseCode) == std::string::npos )
		    continue;

		if ( pick->waveformID().networkCode() != networkCode )
		    continue;

		if ( pick->waveformID().stationCode() != stationCode )
		    continue;

		switch ( rtype ) {
			case 0:
				time = pick->time().value();
			break;
			case 1:
				time = stringToDouble(pick->time().value().toString("%H"));
			break;
			case 2:
				time = stringToDouble(pick->time().value().toString("%M"));
			break;
			case 3:
				time = stringToDouble(pick->time().value().toString("%S.%f"));
			break;
			default:
				time = pick->time().value();
			break;
		}
	}

	return time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::exportAsQUAKEML() {

	if ( !getOutputFile() ) return false;

	// objects counters
	_eventCount = 0;
	_originCount = 0;
	_pickCount = 0;
	_magnitudeCount = 0;


	if ( _exportDialog ) {
		_exportDialog->hide();
		delete _exportDialog;
		_exportDialog = NULL;
	}

	showOriginWarning();

	std::string outputString;

	EventParametersPtr ep = new EventParameters;

	QVector<QPair<QString, bool> > list = _eventListWidget->eventsCheckStateVector();

	if ( list.size() == 0 ) {
		SEISCOMP_ERROR("There is no events checked to export");
		return false;
	}

	loadingPercentage(0, applicationName(), "Creating QuakeML bulletin");

	QString content;
	for (int i = 0; i < list.size(); ++i) {

		QString publicID = list.at(i).first;
		const bool checked = list.at(i).second;

		if ( !checked ) continue;

		EventPtr event = _eventListWidget->getEvent(publicID.toStdString());

		if ( !event ) {
			SEISCOMP_DEBUG("Skipped event %s because it has not been found", event.get()->publicID().c_str());
			continue;
		}

		OriginPtr origin = _eventListWidget->getOrigin(event->preferredOriginID());

		if ( !origin ) {
			SEISCOMP_DEBUG("Skipped event %s, not origin found", event.get()->publicID().c_str());
			continue;
		}

		if ( _quakemlSchemaFile != "" ) {
			if ( !Util::fileExists(_quakemlSchemaFile) ) {
				log(LM_ERROR, __func__, QString("QuakeML schema file %1 not found")
				        .arg(_quakemlSchemaFile.c_str()));
				QMessageBox::warning(mainWindow(), tr("Error"),
				    QString::fromUtf8("File %1 seems to be missing.\n"
					    "Please ensure it is present and conform.")
				            .arg(_quakemlSchemaFile.c_str()));
				break;
			}
		}
		else {
			QMessageBox::warning(mainWindow(), tr("Erreur"),
			    tr("No QuakeML schema file defined in configuration.\n"
				    "Please specify one."));
			break;
		}

		Bulletin bul(event.get(), query());

		if ( !bul.getObjects() ) {
			SEISCOMP_ERROR("Can't get objects to create bulletin");
			return false;
		}

		// Exporting event as SC3ML in temp file
		std::string outFile = _tempFolder + "event-sc3ml.xml";
		std::ofstream out(outFile.c_str());
		out << bul.getBulletin(IPGP::Core::Bulletin::QUAKEML) << std::endl;
		out.close();

		QString cmd = QString("xalan -in %1 -xsl %2 -out %3event-quakeml.xml -html -indent 4")
		        .arg(outFile.c_str()).arg(_quakemlSchemaFile.c_str())
		        .arg(_tempFolder.c_str());

		QProcess process;
		process.start(cmd);

		log(LM_OK, __func__, "Waiting for converter to produce QuakeML file");

		if ( !process.waitForStarted() ) {
			log(LM_ERROR, __func__, "QuakeML converter process couldn't start");
			return false;
		}

		if ( !process.waitForFinished() ) {
			log(LM_ERROR, __func__, "QuakeML converter process couldn't finish");
			return false;
		}

		std::string inFile = _tempFolder + "event-quakeml.xml";
		std::ifstream file(inFile.c_str());
		std::string line;
		while ( file.good() ) {
			getline(file, line);
			content.append(QString("%1\n").arg(line.c_str()));
		}

		log(LM_OK, __func__, "Read back converted QuakeML file");

		_eventCount++;
		_originCount++;

		if ( _setEventAsFinal ) {

			Notifier::SetEnabled(true);

			origin->setEvaluationStatus(EvaluationStatus(FINAL));

			NotifierPtr n = new Notifier("EventParameters", OP_UPDATE, origin.get());
			NotifierMessagePtr m = new NotifierMessage;
			m->attach(n.get());
			connection()->send(m.get());

			Notifier::SetEnabled(false);
		}

		try {
			loadingPercentage(percentageOfSomething<int>(list.size(), i),
			    applicationName(), "Writing down QuakeML bulletin");
		} catch ( ... ) {}
	}
	loadingPercentage(-1, applicationName(), "");

	return writeBulletinToFile(content.toStdString(), eQML);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BulletinExporterView::getOutputFile() {

	_outputFile = QFileDialog::getSaveFileName(mainWindow(), tr("Save bulletin file"),
	    QDir::currentPath(), tr("Bulletin file (*.TXT)"), 0,
	    QFileDialog::DontUseNativeDialog);

	if ( _outputFile != "" )
		return true;
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




