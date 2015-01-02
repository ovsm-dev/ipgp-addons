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


#define APP_SHORTDESC "Interactive tool for managing events/origins from a <br/>"\
						"SeisComP3 database, generate and visualize graphics."
#define APP_FULLDESC "<p>This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.<br/>"\
		    		"It has been co-financed by the European Union and le Ministère de <br/>"\
		    		"l'Ecologie, du Développement Durable, des Transports et du Logement.</p>"
#define APP_VERSION "0.1.0"

#include "advancedeventmanagerview.h"
#include "ui_advancedeventmanagerview.h"
#include "ui_configuredialog.h"

#include <QtGui>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/client/inventory.h>

#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/seismology/regions.h>

#include <ipgp/core/math/math.h>
#include <ipgp/core/geo/geo.h>
#include <ipgp/core/string/string.h>
#include <ipgp/gui/datamodel/mainwindow.h>
#include <ipgp/gui/datamodel/progressindicator/progressindicator.h>
#include <ipgp/gui/datamodel/originplot/originplot.h>
#include <ipgp/gui/datamodel/gutenbergrichter/gutenbergrichterwidget.h>
#include <ipgp/gui/datamodel/hypocentersdrift/hypocentersdriftwidget.h>
#include <ipgp/gui/datamodel/uncertainty/uncertaintywidget.h>
#include <ipgp/gui/datamodel/crosssection/crosssection.h>
#include <ipgp/gui/datamodel/magnitudevariance/magnitudevariancewidget.h>
#include <ipgp/gui/datamodel/eventtypevariance/eventtypevariancewidget.h>
#include <ipgp/gui/datamodel/stream/streamwidget.h>
#include <ipgp/gui/datamodel/particlemotion/particlemotionwidget.h>
#include <ipgp/gui/datamodel/magnitudedensity/magnitudedensity.h>
#include <ipgp/gui/datamodel/eventphaseduration/eventphaseduration.h>
#include <ipgp/gui/datamodel/eventenergy/eventenergy.h>
#include <ipgp/gui/datamodel/wadatidiagram/wadatidiagram.h>
#include <ipgp/gui/math/math.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/widgets/originwidget.h>
#include <ipgp/gui/map/drawables/crosssection.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/drawables/indicator.h>
#include <ipgp/gui/map/drawables/pin.h>
#include <ipgp/gui/opengl/widgets/topographymap.h>
#include <ipgp/gui/opengl/drawables/crosssection.h>

#include <ipgp/gui/3rd-party/qprogressindicator/qprogressindicator.h>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math::Geo;


using namespace IPGP;
using namespace IPGP::Core;
using namespace IPGP::Core::Math;
using namespace IPGP::Core::Geo;
using namespace IPGP::Core::String;
using namespace IPGP::Gui;
using namespace IPGP::Gui::Misc;
using namespace IPGP::Gui::OpenGL;



namespace {

void activateWindow(const QList<QMdiSubWindow*>& list, const QString& name) {
	for (QList<QMdiSubWindow*>::const_iterator it = list.constBegin();
	        it != list.constEnd(); ++it)
		if ( (*it)->objectName() == name ) {
			(*it)->activateWindow();
			(*it)->raise();
		}
}

}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AdvancedEventManagerView::
AdvancedEventManagerView(int& argc, char** argv) :
		IPGP::Gui::Client::Application(argc, argv) {

	setAutoApplyNotifierEnabled(true);
	setInterpretNotifierEnabled(true);

	setLoadInventoryEnabled(true);
	setLoadStationsEnabled(true);

	messageSubscription("LOCATION");
	messageSubscription("CONFIG");
	messageSubscription("EVENT");
	messageSubscription("MAGNITUDE");
	messageSubscription("PICK");

	_ui = NULL;
	_summaryMap = NULL;
	_region = NULL;
	_eventListWidget = NULL;
	_databaseLabel = NULL;
	_pi = NULL;
	_progressIndicator = NULL;
	_configDialog = NULL;
	_configDialogUi = NULL;
	_timer = NULL;
	_globalMap = NULL;
	_topoMap = NULL;
	_streamWidget = NULL;
	_particleMotionWidget = NULL;

	_useNormalizedEpicenterSize = true;
	_selectedObjectID = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AdvancedEventManagerView::~AdvancedEventManagerView() {

	//! We need only delete objects without parents...

	for (PlotList::iterator it = _plotList.begin();
	        it != _plotList.end(); ++it) {
		if ( !(*it) ) continue;
		delete (*it);
	}
	_plotList.clear();
	SEISCOMP_DEBUG("Remaining plot(s) = %d", _plotList.size());

	for (WidgetList::iterator it = _widgetList.begin();
	        it != _widgetList.end(); ++it) {
		if ( !(*it) ) continue;
		delete (*it);
	}
	_widgetList.clear();
	SEISCOMP_DEBUG("Remaining plotting widget(s) = %d", _widgetList.size());

	if ( _region )
	    delete _region;
	_region = NULL;

	if ( _ui )
	    delete _ui;
	_ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::handleDatabaseState() {
	_databaseLabel->setPixmap(databaseIcon());
	_databaseLabel->setToolTip(databaseServer().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::loadingPercentage(const int& value,
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
void AdvancedEventManagerView::setupApplicationMenus() {

	connect(_ui->actionQuit, SIGNAL(triggered()), this, SLOT(exitGui()));
	connect(_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
	connect(_ui->actionShowSummary, SIGNAL(triggered()), this, SLOT(showSummary()));
	connect(_ui->actionShowLog, SIGNAL(triggered()), this, SLOT(showLog()));
	connect(_ui->actionConfigureAdvancedManagerView, SIGNAL(triggered()), _configDialog, SLOT(show()));

	connect(_ui->actionConfigureConnection, SIGNAL(triggered()), this, SLOT(showConnectionDialog()));
	connect(_ui->actionLoadedPlugins, SIGNAL(triggered()), this, SLOT(showPlugins()));
	connect(this, SIGNAL(changeDatabaseIcon()), this, SLOT(handleDatabaseState()));
	connect(this, SIGNAL(databaseChanged(const QString&, const QString&)),
	    this, SLOT(databaseMessage(const QString&, const QString&)));

	connect(_ui->actionResidualAzimuth, SIGNAL(triggered()), this, SLOT(showResidualAzimuthGraph()));
	connect(_ui->actionResidualDistance, SIGNAL(triggered()), this, SLOT(showResidualDistanceGraph()));
	connect(_ui->actionResidualTakeOff, SIGNAL(triggered()), this, SLOT(showResidualTakeOffGraph()));

	connect(_configDialogUi->buttonBox, SIGNAL(accepted()), this, SLOT(configAccepted()));

	connect(_eventListWidget, SIGNAL(rowClicked(const QString&)), this, SLOT(displayObjectInformation(const QString&)));
	connect(_eventListWidget, SIGNAL(viewUpdateRequested()), this, SLOT(eventListViewUpdateRequested()));
	connect(_eventListWidget, SIGNAL(viewUpdated()), this, SLOT(eventListViewUpdated()));
	connect(_eventListWidget, SIGNAL(updateProgress(const int&, const QString&, const QString&)),
	    this, SLOT(loadingPercentage(const int&, const QString&, const QString&)));
	connect(_eventListWidget, SIGNAL(logMessage(const int&, const QString&,const QString&)),
	    this, SLOT(log(const int&, const QString&, const QString&)));
	connect(_eventListWidget, SIGNAL(enableRegionFilterByMouse()), this, SLOT(selectRegionByMouse()));
	connect(this, SIGNAL(itemSelected(const QString&)), _eventListWidget, SLOT(selectItem(const QString&)));
	connect(this, SIGNAL(itemsHighlighted(const QStringList&)), _eventListWidget, SLOT(highlightItems(const QStringList&)));

	connect(_summaryMap, SIGNAL(logMessage(const int&, const QString&, const QString&)),
	    this, SLOT(log(const int&, const QString&, const QString&)));

	connect(_ui->actionGutenbergRichterRelation, SIGNAL(triggered()), this, SLOT(showGutenbergRichterGraph()));
	connect(_ui->actionDrifttoHypocenters, SIGNAL(triggered()), this, SLOT(showHypocenterDriftGraph()));
	connect(_ui->actionEventsMap, SIGNAL(triggered()), this, SLOT(showGlobalMap()));
	connect(_ui->actionEventsTopoMap, SIGNAL(triggered()), this, SLOT(showTopographyMap()));
	connect(_ui->actionCrossSection, SIGNAL(triggered()), this, SLOT(showCrossSectionWidget()));
	connect(_ui->actionMagnitudeVariation, SIGNAL(triggered()), this, SLOT(showMagnigtudeVarationGraph()));
	connect(_ui->actionEventTypeVariation, SIGNAL(triggered()), this, SLOT(showEventTypeVarationGraph()));
	connect(_ui->actionEventPhaseDuration, SIGNAL(triggered()), this, SLOT(showEventPhaseDurationGraph()));
	connect(_ui->actionEventEnergy, SIGNAL(triggered()), this, SLOT(showEventEnergyGraph()));
	connect(_ui->actionWadatiRelation, SIGNAL(triggered()), this, SLOT(showWadatiGraph()));
	connect(_ui->actionUncertainties, SIGNAL(triggered()), this, SLOT(showUncertaintyGraph()));
	connect(_ui->actionStationStream, SIGNAL(triggered()), this, SLOT(showStationStreamGraph()));
	connect(_ui->actionParticleMotion, SIGNAL(triggered()), this, SLOT(showParticleMotionGraph()));
	connect(_ui->pushButton_particleMotion, SIGNAL(clicked()), this, SLOT(showParticleMotionGraph()));
	connect(_ui->actionMagnitudeDensity, SIGNAL(triggered()), this, SLOT(showMagnitudeDensityGraph()));
	connect(_ui->actionDocumentationIndex, SIGNAL(triggered()), this, SLOT(showGeneralDocumentation()));
	connect(_ui->actionDocumentationForScaemv, SIGNAL(triggered()), this, SLOT(showDocumentation()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::setupGraphicalInterface() {

	IPGP::Gui::Client::Application::setupGraphicalInterface();

	_ui = new Ui::AdvancedEventManagerView;
	_ui->setupUi(mainWindow());

#ifdef __APPLE__
	_ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
#else
	_ui->mdiArea->setViewMode(QMdiArea::TabbedView);
#endif

	getMainWindow()->setConfirmOnClose(true);

	showMessage("Setup graphical interface");

	_progressIndicator = new ProgressIndicator(mainWindow(), Qt::Dialog);
	_progressIndicator->setModal(true);
	loadingPercentage(-1, "AdvancedEventManagerView", "");

	_pi = new QProgressIndicator(mainWindow());
	_ui->statusBar->addPermanentWidget(_pi, 2);
	_pi->hide();

	_configDialogUi = new Ui::ConfigureDialog;
	_configDialog = new QDialog(mainWindow());
	_configDialog->setObjectName("AdvancedEventManagerViewConfigureDialog");
	_configDialog->setModal(true);
	_configDialogUi->setupUi(_configDialog);
	_configDialogUi->checkBox_el_advancedOptions->setChecked(_eventListSettings.useAdvancedOptions);
	_configDialogUi->checkBox_el_decorateItems->setChecked(_eventListSettings.decorateItems);
	_configDialogUi->checkBox_el_fetchUnassociated->setChecked(_eventListSettings.fetchUnassociated);
	_configDialogUi->checkBox_el_useSeperateThread->setChecked(_eventListSettings.useMultiThreads);
	_configDialogUi->checkBox_el_useCache->setChecked(_eventListSettings.useCacheEngine);
	_configDialogUi->checkBox_el_fetchUnCom->setChecked(_eventListSettings.fetchUnassociatedOriginComment);
	_configDialogUi->checkBox_el_fetchPrefCom->setChecked(_eventListSettings.fetchPreferredOriginComment);
	_configDialogUi->checkBox_el_fetchSibCom->setChecked(_eventListSettings.fetchSiblingOriginComment);
	QString mags;
	for (size_t i = 0; i < _magnitudes.size(); ++i) {
		mags.append(_magnitudes.at(i).c_str());
		if ( i != _magnitudes.size() - 1 )
		    mags.append(", ");
	}
	_configDialogUi->lineEdit_g_magnitudes->setText(mags);
	_configDialogUi->lineEdit_g_dataSource->setText(globalSettings().streamURI());
	_configDialogUi->comboBox_em_epicentersSizes->setCurrentIndex(static_cast<int>(!_useNormalizedEpicenterSize));
//	        (_useNormalizedEpicenterSize) ? 1 : 0);
	_configDialog->hide();

	_databaseLabel = new QLabel(mainWindow());
	_ui->statusBar->addPermanentWidget(_databaseLabel);

	_region = new Regions;
	std::string region = _region->getRegionName(mapDescriptor().defaultLatitude(),
	    mapDescriptor().defaultLongitude());

	_ui->event_hourLabel->setText(QString("-"));
	_ui->event_latitudeLabel->setText(QString("-"));
	_ui->event_longitudeLabel->setText(QString("-"));
	_ui->event_phasesLabel->setText(QString("-"));
	_ui->event_depthLabel->setText(QString("-"));
	_ui->event_rmsLabel->setText(QString("-"));
	_ui->event_azimuthLabel->setText(QString("-"));
	_ui->event_minDistLabel->setText(QString("-"));
	_ui->event_magnitudeLabel->setText(QString("-"));
	_ui->event_agencyLabel->setText(QString("-"));
	_ui->event_authorLabel->setText(QString("-"));
	_ui->event_methodLabel->setText(QString("-"));
	_ui->event_modelLabel->setText(QString("-"));
	_ui->event_evaluationLabel->setText(QString("-"));
	_ui->dockWidget->setWindowTitle(region.c_str());
	_ui->actionShowSummary->setChecked(true);
	_ui->splitter->setSizes(QList<int>() << 50 << 50);
	_ui->splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	_summaryMap = new OriginWidget(mapDescriptor().names(), mapDescriptor().paths(),
	    mapDescriptor().defaultLongitude(), mapDescriptor().defaultLatitude(),
	    mainWindow(), 0, true);
	_summaryMap->setTilePattern(mapDescriptor().tilePattern().c_str());
	_summaryMap->setMinimumHeight(200);
	_summaryMap->setScheme(scheme());

	QBoxLayout* l = new QVBoxLayout(_ui->frameSummaryMap);
	_ui->frameSummaryMap->setLayout(l);
	l->addWidget(_summaryMap);
	l->setMargin(0);

	_eventListWidget = new EventListWidget(_ui->frameEventList, query(), &_cache);
	_eventListWidget->setLocationProfiles(_locators);
	_eventListWidget->setSettings(_eventListSettings);
	_eventListWidget->setLayoutMargins(0, 0, 0, 0);

	QBoxLayout* l1 = new QVBoxLayout(_ui->frameEventList);
	_ui->frameEventList->setLayout(l1);
	l1->addWidget(_eventListWidget);
	l1->setMargin(0);

	_ui->actionStationStream->setIcon(QIcon(":images/wave24x24.png"));
	_ui->actionStationStream->setIconVisibleInMenu(true);

	_ui->pushButton_particleMotion->setText("");
	_ui->pushButton_particleMotion->setIconSize(QSize(32, 32));
	_ui->pushButton_particleMotion->setIcon(QIcon(":images/axis_3d.png"));
	_ui->pushButton_particleMotion->setToolTip("Show ParticleMotion");

	QString message = QString("%1 days of events loaded from database")
	        .arg(globalSettings().loadEventDB());
	_ui->statusBar->showMessage(message, 6000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::nullifyQObject(QObject* obj) {

	if ( obj == _particleMotionWidget )
	    _particleMotionWidget = NULL;

	if ( obj == _streamWidget )
	    _streamWidget = NULL;

	if ( obj == _globalMap )
	    _globalMap = NULL;

	if ( obj == _topoMap )
	    _topoMap = NULL;

	if ( qobject_cast<OriginPlot*>(obj) ) {
		for (PlotList::iterator it = _plotList.begin();
		        it != _plotList.end(); ++it) {
			if ( !(*it) ) continue;
			if ( (*it) == obj ) {
				_plotList.removeOne((*it));
				SEISCOMP_DEBUG("Deleted plot %s", obj->objectName().toStdString().c_str());
				break;
			}
		}
	}

	if ( qobject_cast<PlottingWidget*>(obj) ) {
		for (WidgetList::iterator it = _widgetList.begin();
		        it != _widgetList.end(); ++it) {
			if ( !(*it) ) continue;
			if ( (*it) == obj ) {
				_widgetList.removeOne((*it));
				SEISCOMP_DEBUG("Deleted plotting widget %s", obj->objectName().toStdString().c_str());
				break;
			}
		}
	}

	QApplication::processEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::configAccepted() {

	log(Gui::Client::LM_INFO, __func__, "Application configuration changed");

	_eventListSettings.decorateItems = _configDialogUi->checkBox_el_decorateItems->isChecked();
	_eventListSettings.fetchUnassociated = _configDialogUi->checkBox_el_fetchUnassociated->isChecked();
	_eventListSettings.useAdvancedOptions = _configDialogUi->checkBox_el_advancedOptions->isChecked();
	_eventListSettings.useMultiThreads = _configDialogUi->checkBox_el_useSeperateThread->isChecked();
	_eventListSettings.useCacheEngine = _configDialogUi->checkBox_el_useCache->isChecked();
	_eventListSettings.fetchPreferredOriginComment = _configDialogUi->checkBox_el_fetchPrefCom->isChecked();
	_eventListSettings.fetchSiblingOriginComment = _configDialogUi->checkBox_el_fetchSibCom->isChecked();
	_eventListSettings.fetchUnassociatedOriginComment = _configDialogUi->checkBox_el_fetchUnCom->isChecked();
	_eventListWidget->setSettings(_eventListSettings);

	QStringList mags = _configDialogUi->lineEdit_g_magnitudes->text().split(",");
	_magnitudes.clear();
	for (int i = 0; i < mags.size(); ++i)
		_magnitudes.push_back(stripWhiteSpace(mags.at(i).toStdString()));

	if ( _configDialogUi->lineEdit_g_dataSource->text() != globalSettings().streamURI() )
	    globalSettings().setStreamURI(_configDialogUi->lineEdit_g_dataSource->text());


	for (WidgetList::const_iterator it = _widgetList.constBegin();
	        it != _widgetList.constEnd(); ++it) {
		if ( !(*it) ) continue;
		(*it)->setMagnitudeTypes(_magnitudes);
	}

	_useNormalizedEpicenterSize = (bool) _configDialogUi->comboBox_em_epicentersSizes->currentIndex();

	if ( _configDialogUi->checkBox_el_clearCache->isChecked() ) {
		if ( _timer ) delete _timer, _timer = NULL;
		_timer = new QTimer(mainWindow());
		connect(_timer, SIGNAL(timeout()), this, SLOT(updateCacheEngine()));
		_timer->start(1000 * 60 * _configDialogUi->spinBox_el_clearCachePeriod->value());
		log(Gui::Client::LM_INFO, __func__, QString("Set cache updater ON. Next run in %1min")
		        .arg(_configDialogUi->spinBox_el_clearCachePeriod->value()));
	}
	else {
		if ( _timer ) {
			_timer->stop();
			delete _timer, _timer = NULL;
			log(Gui::Client::LM_INFO, __func__, "Set cache updater OFF");
		}
	}

	log(Gui::Client::LM_INFO, __func__, "Updated widgets configurations");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::eventListViewUpdateRequested() {
	showWaitingWidget();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::updateCacheEngine() {

	if ( _eventListWidget->widgetStatus() != EventListWidget::ELW_Idle ) {
		log(Gui::Client::LM_ERROR, __func__, "Cache update failed, the event's list is still reading");
		return;
	}

	//! This operation has to be run in another thread, although objects
	//! stored being smart pointers, check and see how this can be done in
	//! a very neat way... c'mon you can do it!!

	_eventListWidget->lockReading();
	QApplication::processEvents();

	SEISCOMP_INFO("A scheduled cache update is starting");
	showWaitingWidget();
	_cache.updateObjects(query());
	hideWaitingWidget();
	SEISCOMP_DEBUG("Scheduled cache update has been performed");
	_eventListWidget->unlockReading();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showWaitingWidget() {
	mainWindow()->setCursor(Qt::WaitCursor);
	_pi->startAnimation();
	_pi->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::hideWaitingWidget() {
	mainWindow()->setCursor(Qt::ArrowCursor);
	_pi->stopAnimation();
	_pi->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::eventListViewUpdated() {

	for (WidgetList::const_iterator it = _widgetList.constBegin();
	        it != _widgetList.constEnd(); ++it) {
		if ( !(*it) ) continue;
		(*it)->setEvents(_eventListWidget->originList());
		(*it)->startBlinking();
	}

	updateEventsMap();

	hideWaitingWidget();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::updateEventsMap() {

	if ( _eventListWidget->originList()->size() == 0 ) return;

	if ( _topoMap )
	    _topoMap->setOrigins(_eventListWidget->originList());

	if ( !_globalMap ) return;

	showWaitingWidget();

	_globalMap->foregroundCanvas().removeGeometries(Map::Geometry::d_Epicenter);


	for (size_t i = 0; i < _eventListWidget->originList()->size(); ++i) {

		OriginPtr org = _eventListWidget->originList()->at(i).first;

		if ( !org ) continue;

		if ( org->magnitudeCount() == 0 )
		    query()->loadMagnitudes(org.get());

		QString depth = QString("-");
		double depthValue = -9999.99;
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
			    scheme().depthPrecision());
		} catch ( ... ) {}

		QString dist = QString("-");
		try {
			if ( scheme().distanceInKM() )
				dist = QString("%1km").arg(QString::number(
				    Seiscomp::Math::Geo::deg2km(org->quality().minimumDistance()),
				    'f', scheme().distancePrecision()));
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
		double magValue;
		bool hasMagnitude = false;

		for (size_t j = 0; j < org->magnitudeCount(); ++j) {

			if ( !org->magnitude(j) ) continue;

			if ( org->magnitude(j)->publicID() ==
			        _eventListWidget->originList()->at(i).second->preferredMagnitudeID() ) {
				try {
					magValue = org->magnitude(j)->magnitude().value();
					magnitude = QString::number(roundDouble(org->magnitude(j)->magnitude().value(), 1),
					    'f', scheme().generalPrecision());
					magType = org->magnitude(j)->type().c_str();
					hasMagnitude = true;
				} catch ( ... ) {}
			}
		}

		QString author = "-";
		try {
			author = org->creationInfo().author().c_str();
		} catch ( ... ) {}
		double latitude, longitude;
		try {
			latitude = org->latitude().value();
		} catch ( ... ) {}
		try {
			longitude = org->longitude().value();
		} catch ( ... ) {}

		Map::Epicenter* epicenter = new Map::Epicenter;
		epicenter->setName(org->publicID().c_str());
		epicenter->setGeoPosition(QPointF(longitude, latitude));
		epicenter->setToolTip(epicenterTooltip(org->publicID().c_str(),
		    org->time().value().toString("%Y-%m-%d %H:%M:%S").c_str(),
		    getStringPosition(latitude, Latitude).c_str(),
		    latU, getStringPosition(longitude, Longitude).c_str(), lonU,
		    depth, depthUncertainty, rms, azimuth, magnitude, magType, dist,
		    Regions::getRegionName(latitude, longitude).c_str(),
		    author));
		epicenter->setSize(EpicenterDefaultSize);

		if ( hasMagnitude ) {

			if ( _useNormalizedEpicenterSize )
				epicenter->setMagnitude(magValue);
			else
				epicenter->setSize(QSizeF(1.5 * magValue, 1.5 * magValue));
			epicenter->setBrush(getDepthColoration(depthValue));
			epicenter->pen().setColor(Qt::black);
		}
		else {
			epicenter->setSize(QSizeF(4., 4.));
			epicenter->pen().setColor(getDepthColoration(depthValue));
			epicenter->setBrush(Qt::transparent);
		}

		if ( !_globalMap->foregroundCanvas().addGeometry(epicenter) ) {
			delete epicenter;
			epicenter = NULL;
		}
	}

	_globalMap->update();

	hideWaitingWidget();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T* AdvancedEventManagerView::initPlottingWidget(const QString& name) {

	T* object = new T(query());
	connect(object, SIGNAL(nullifyQObject(QObject*)), this, SLOT(nullifyQObject(QObject*)));
	connect(object, SIGNAL(elementClicked(const QString&)), this, SLOT(displayObjectInformation(const QString&)));
	connect(object, SIGNAL(elementClicked(const QString&)), this, SIGNAL(itemSelected(const QString&)));
	connect(object, SIGNAL(working()), this, SLOT(showWaitingWidget()));
	connect(object, SIGNAL(idling()), this, SLOT(hideWaitingWidget()));
	connect(object, SIGNAL(loadingPercentage(const int&, const QString&, const QString&)),
	    this, SLOT(loadingPercentage(const int&, const QString&, const QString&)));

	QMdiSubWindow* sub = NULL;
	QList<QMdiSubWindow*> list = _ui->mdiArea->subWindowList();
	for (int i = 0; i < list.size(); i++)
		if ( list.at(i)->windowTitle() == name )
		    sub = list.at(i);

	if ( !sub ) {
		sub = _ui->mdiArea->addSubWindow(object);
		sub->setWindowTitle(name);
		sub->setObjectName(object->objectName());
		sub->setAttribute(Qt::WA_DeleteOnClose);
	}
	sub->show();

	object->setEvents(_eventListWidget->originList());
	object->setMagnitudeTypes(_magnitudes);

	_widgetList.append(object);

	return object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T> T* AdvancedEventManagerView::getPlottingWidget() {

	T* obj = NULL;
	for (WidgetList::iterator it = _widgetList.begin();
	        it != _widgetList.end(); ++it) {
		if ( !(*it) ) continue;
		T* tmpObj = qobject_cast<T*>((*it));
		if ( tmpObj ) {
			obj = tmpObj;
			break;
		}
	}

	if ( !obj ) return NULL;

	::activateWindow(_ui->mdiArea->subWindowList(), obj->objectName());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginPlot* AdvancedEventManagerView::initPlotGraph(const int& type,
                                                    const QString& name) {

	OriginPlot* object = new OriginPlot(NULL, static_cast<OriginPlot::Type>(type));
	connect(object, SIGNAL(nullifyQObject(QObject*)), this, SLOT(nullifyQObject(QObject*)));

	QMdiSubWindow* sub = NULL;
	QList<QMdiSubWindow*> list = _ui->mdiArea->subWindowList();
	for (int i = 0; i < list.size(); i++)
		if ( list.at(i)->windowTitle() == name )
		    sub = list.at(i);

	if ( !sub ) {
		sub = _ui->mdiArea->addSubWindow(object);
		sub->setWindowTitle(name);
		sub->setObjectName(object->objectName());
		sub->setAttribute(Qt::WA_DeleteOnClose);
	}
	sub->resize(QSize(250, 250));
	sub->show();

	_plotList.append(object);

	return object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginPlot* AdvancedEventManagerView::getPlotGraph(const int& type) {

	OriginPlot* plot = NULL;

	for (PlotList::iterator it = _plotList.begin();
	        it != _plotList.end(); ++it) {
		if ( !(*it) ) continue;
		if ( (*it)->type() == type ) {
			plot = (*it);
			break;
		}
	}

	if ( !plot ) return NULL;

	::activateWindow(_ui->mdiArea->subWindowList(), plot->objectName());

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showResidualDistanceGraph() {

	OriginPlot* object = getPlotGraph(OriginPlot::ResidualDistance);
	if ( object ) return;

	object = initPlotGraph(OriginPlot::ResidualDistance, "Residuals/distances");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showResidualAzimuthGraph() {

	OriginPlot* object = getPlotGraph(OriginPlot::ResidualAzimuth);
	if ( object ) return;

	object = initPlotGraph(OriginPlot::ResidualAzimuth, "Residuals/azimuths");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showResidualTakeOffGraph() {

	OriginPlot* object = getPlotGraph(OriginPlot::ResidualTakeOff);
	if ( object ) return;

	object = initPlotGraph(OriginPlot::ResidualTakeOff, "Residuals/TakeOffs");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showUncertaintyGraph() {

	UncertaintyWidget* object = getPlottingWidget<UncertaintyWidget>();
	if ( object ) return;

	object = initPlottingWidget<UncertaintyWidget>("Uncertainties");
	object->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showStationStreamGraph() {

	if ( _streamWidget ) {
		::activateWindow(_ui->mdiArea->subWindowList(), _streamWidget->objectName());
		return;
	}

	_streamWidget = new StreamWidget(query());
	_streamWidget->setObjectName("StreamWidget");

	connect(_streamWidget, SIGNAL(nullifyQObject(QObject*)), this, SLOT(nullifyQObject(QObject*)));
	connect(_streamWidget, SIGNAL(idling()), this, SLOT(hideWaitingWidget()));
	connect(_streamWidget, SIGNAL(working()), this, SLOT(showWaitingWidget()));
	connect(_streamWidget, SIGNAL(loadingPercentage(const int&, const QString&, const QString&)),
	    this, SLOT(loadingPercentage(const int&, const QString&, const QString&)));

	QMdiSubWindow* sub = NULL;
	QList<QMdiSubWindow*> list = _ui->mdiArea->subWindowList();
	for (int i = 0; i < list.size(); i++)
		if ( list.at(i)->windowTitle() == "Station stream" )
		    sub = list.at(i);

	if ( !sub ) {
		sub = _ui->mdiArea->addSubWindow(_streamWidget);
		sub->setWindowTitle("Station stream");
		sub->setAttribute(Qt::WA_DeleteOnClose);
	}
	sub->show();

	_streamWidget->setupInventory();
	_streamWidget->setStreamUrl(globalSettings().streamURI().toStdString());
	_streamWidget->initFilters(globalSettings().streamFilters());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showParticleMotionGraph() {

	if ( _particleMotionWidget ) {
		_particleMotionWidget->activateWindow();
		_particleMotionWidget->raise();
		return;
	}

	_particleMotionWidget = new ParticleMotionWidget(query());
	_particleMotionWidget->setAttribute(Qt::WA_DeleteOnClose, true);
	_particleMotionWidget->setStreamUrl(globalSettings().streamURI().toStdString());
	_particleMotionWidget->initFilters(globalSettings().streamFilters());

	connect(_particleMotionWidget, SIGNAL(idling()), this, SLOT(hideWaitingWidget()));
	connect(_particleMotionWidget, SIGNAL(working()), this, SLOT(showWaitingWidget()));
	connect(mainWindow(), SIGNAL(mainWindowClosing()), _particleMotionWidget, SLOT(close()));
	connect(_particleMotionWidget, SIGNAL(nullifyQObject(QObject*)), this, SLOT(nullifyQObject(QObject*)));

	_particleMotionWidget->show();

	if ( !_selectedObjectID.empty() )
	    _particleMotionWidget->setOrigin(_selectedObjectID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showGlobalMap() {

	if ( _globalMap ) {
		::activateWindow(_ui->mdiArea->subWindowList(), _globalMap->objectName());
		return;
	}

	_globalMap = new MapWidget(mapDescriptor().names(), mapDescriptor().paths(),
	NULL, mapDescriptor().defaultLongitude(), mapDescriptor().defaultLatitude(),
	    mapDescriptor().tilePattern());
	_globalMap->setScheme(scheme());
	_globalMap->setObjectName("EventsMap");

	connect(_globalMap, SIGNAL(nullifyQObject(QObject*)), this, SLOT(nullifyQObject(QObject*)));
	connect(_globalMap, SIGNAL(elementClicked(const QString&)), this, SLOT(displayObjectInformation(const QString&)));
	connect(_globalMap, SIGNAL(elementClicked(const QString&)), _eventListWidget, SLOT(selectItem(const QString&)));
	connect(_globalMap, SIGNAL(regionSelected(const QPointF&, const QPointF&)),
	    _eventListWidget, SLOT(regionSelected(const QPointF&, const QPointF&)));
	_globalMap->setScheme(scheme());

	QPushButton* clearPins = new QPushButton(_globalMap);
	clearPins->setObjectName("clearPinsButton");
	clearPins->setIcon(QIcon(":images/clear.png"));
	clearPins->setToolTip("Remove pins from the map");
	clearPins->setGeometry(0, 0, 32, 32);
	_globalMap->addOptionalWidget(clearPins);
	connect(clearPins, SIGNAL(clicked()), this, SLOT(clearPinsFromMap()));

	QMdiSubWindow* sub = NULL;
	QList<QMdiSubWindow*> list = _ui->mdiArea->subWindowList();
	for (int i = 0; i < list.size(); i++)
		if ( list.at(i)->windowTitle() == "Events Map" )
		    sub = list.at(i);

	if ( !sub ) {
		sub = _ui->mdiArea->addSubWindow(_globalMap);
		sub->setWindowTitle("Events map");
		sub->setObjectName(_globalMap->objectName());
		sub->setAttribute(Qt::WA_DeleteOnClose);
	}
	sub->resize(QSize(300, 250));
	sub->show();

	updateEventsMap();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showTopographyMap() {

	if ( _topoSettings.size() == 0 ) {
		QMessageBox::information(mainWindow(), tr("Missing profile"),
		    QString("It appears that no topography settings have been defined.\n"
			    "Make sure that there is at least one available to ensure 3D drawings."));
		return;
	}

	if ( _topoMap ) {
		_topoMap->activateWindow();
		_topoMap->raise();
		return;
	}

	_topoMap = new TopographyMap(mainWindow());
	_topoMap->setDatabase(query());
	_topoMap->renderer()->setAvailableSettings(_topoSettings);
	_topoMap->renderer()->setActiveSettings(_topoSettings.begin().value());

	if ( QFile::exists(_topoSettings.begin().value().filepath()) ) {
		if ( _topoSettings.begin().value().fileType() == TopographyRendererSettings::XYZ_FILE )
			_topoMap->renderer()->loadXYZ(_topoSettings.begin().value().filepath());
		else if ( _topoSettings.begin().value().fileType() == TopographyRendererSettings::STL_FILE )
		    _topoMap->renderer()->loadSTL(_topoSettings.begin().value().filepath());
		_topoMap->updateInterface();
		_topoMap->setInventoryVisible(true);
	}
	else {
		showErrorBox(QString("Topography file %1 has not been found")
		        .arg(_topoSettings.begin().value().filepath()).toStdString().c_str());
	}

	connect(_topoMap, SIGNAL(nullifyQObject(QObject*)), this, SLOT(nullifyQObject(QObject*)));
	connect(_topoMap, SIGNAL(loadingPercentage(const int&, const QString&, const QString&)),
	    this, SLOT(loadingPercentage(const int&, const QString&, const QString&)));

	QMdiSubWindow* sub = NULL;
	QList<QMdiSubWindow*> list = _ui->mdiArea->subWindowList();
	for (int i = 0; i < list.size(); i++)
		if ( list.at(i)->windowTitle() == "Events Topography Map" )
		    sub = list.at(i);

	if ( !sub ) {
		sub = _ui->mdiArea->addSubWindow(_topoMap);
		sub->setWindowTitle("Events Topography Map");
		sub->setAttribute(Qt::WA_DeleteOnClose);
	}
	sub->resize(QSize(300, 250));
	sub->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showGutenbergRichterGraph() {

	GutenbergRichterWidget* object = getPlottingWidget<GutenbergRichterWidget>();
	if ( object ) return;

	initPlottingWidget<GutenbergRichterWidget>("Gutenberg-Richter Relation");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showHypocenterDriftGraph() {

	HypocentersDriftWidget* object = getPlottingWidget<HypocentersDriftWidget>();
	if ( object ) return;

	initPlottingWidget<HypocentersDriftWidget>("Hypocenters Drift");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showCrossSectionWidget() {

	Gui::CrossSection* object = getPlottingWidget<Gui::CrossSection>();
	if ( object ) return;

	object = initPlottingWidget<Gui::CrossSection>("Cross Section");

	connect(object, SIGNAL(disableCrossSectionPainting()), this, SLOT(removeCrossSectionFromMap()));
	connect(object, SIGNAL(enableCrossSection(const QPointF& , const QPointF& , const qreal&, const qreal&,const qreal&)),
	    this, SLOT(addCrossSectionToMap(const QPointF& , const QPointF&, const qreal&, const qreal& , const qreal& )));
	connect(object, SIGNAL(objectsInSection(const QStringList&)), this, SIGNAL(itemsHighlighted(const QStringList&)));
	connect(object, SIGNAL(idling()), this, SLOT(hideWaitingWidget()));
	connect(object, SIGNAL(working()), this, SLOT(showWaitingWidget()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showMagnitudeDensityGraph() {

	MagnitudeDensity* object = getPlottingWidget<MagnitudeDensity>();
	if ( object ) return;

	initPlottingWidget<MagnitudeDensity>("Magnitude density");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showEventPhaseDurationGraph() {

	EventPhaseDuration* object = getPlottingWidget<EventPhaseDuration>();
	if ( object ) return;

	initPlottingWidget<EventPhaseDuration>("S-P Duration");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showEventEnergyGraph() {

	EventEnergy* object = getPlottingWidget<EventEnergy>();
	if ( object ) return;

	initPlottingWidget<EventEnergy>("Accumulated Energy");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showWadatiGraph() {

	WadatiDiagram* object = getPlottingWidget<WadatiDiagram>();
	if ( object ) return;

	initPlottingWidget<WadatiDiagram>("Wadati Diagram");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showMagnigtudeVarationGraph() {

	MagnitudeVarianceWidget* object = getPlottingWidget<MagnitudeVarianceWidget>();
	if ( object ) return;

	initPlottingWidget<MagnitudeVarianceWidget>("Magnitude Variation");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showEventTypeVarationGraph() {

	EventTypeVarianceWidget* object = getPlottingWidget<EventTypeVarianceWidget>();
	if ( object ) return;

	initPlottingWidget<EventTypeVarianceWidget>("Event Type Variation");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::removeCrossSectionFromMap() {

	if ( _topoMap )
	    _topoMap->setCrossSection(NULL);

	if ( _globalMap )
	    _globalMap->foregroundCanvas().removeGeometries(Map::Geometry::d_CrossSection);

	if ( _eventListWidget )
	    _eventListWidget->highlightItems(QStringList());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::clearPinsFromMap() {
	if ( _globalMap )
	    _globalMap->foregroundCanvas().removeGeometries(Map::Geometry::d_Pin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::addCrossSectionToMap(const QPointF& start,
                                                    const QPointF& end,
                                                    const qreal& depthMin,
                                                    const qreal& depthMax,
                                                    const qreal& trench) {

	if ( _topoMap ) {

		SEISCOMP_DEBUG("Ax: %f Ay: %f", start.x(), start.y());
		SEISCOMP_DEBUG("Bx: %f By: %f", end.x(), end.y());

		OpenGL::CrossSection* c = new OpenGL::CrossSection;
		c->setCubeColor(Qt::yellow);
		c->setDepth(depthMin, depthMax);
		c->setName("default");
		c->setPointA(start.x(), start.y());
		c->setPointB(end.x(), end.y());
		c->setWidth(trench);
		c->computeCube();
		_topoMap->setCrossSection(c);
	}

	if ( !_globalMap )
	    return;

	_globalMap->foregroundCanvas().removeGeometries(Map::Geometry::d_CrossSection);

	Map::CrossSection* cs = new Map::CrossSection;
	cs->setStartPointF(start);
	cs->setEndPointF(end);
	cs->setDepthMin(depthMin);
	cs->setDepthMax(depthMax);
	cs->setTrench(trench);

	if ( !_globalMap->foregroundCanvas().addGeometry(cs) ) {
		delete cs;
		cs = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::selectRegionByMouse() {

	if ( !_globalMap ) {
		if ( QMessageBox::question(mainWindow(), applicationName(),
		    tr("The events map isn't launched yet.\nWould you like to activate this widget now ?"),
		    QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes) != QMessageBox::Yes )
			return;
		else
			showGlobalMap();
	}

	_globalMap->setRubberBandEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AdvancedEventManagerView::run() {

	showMessage("Setup user interface");
	setupGraphicalInterface();
	readApplicationSettings();
	setupApplicationMenus();

	setInterfaceName("Advanced Event Manager View");
	setBinaryName("scaemv");

	setApplicationDescriptor(IPGP::Gui::Client::ApplicationDescriptor(
	    interfaceName().c_str(), QString::fromUtf8(APP_SHORTDESC),
	    QString::fromUtf8(APP_FULLDESC), APP_VERSION));

	showMessage("Loading event database");
	_eventListWidget->setRange(globalSettings().loadEventDB());
	_eventListWidget->readEvents();

	showNormal();

	handleDatabaseState();

	return (bool) !QApplication::exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::
displayObjectInformation(const QString& objectID) {

	if ( !query() ) {
		QMessageBox::critical(mainWindow(), tr("Query error"),
		    tr("No database interface is currently available"));
		return;
	}

	PublicObjectPtr obj = query()->getObject(Event::TypeInfo(), objectID.toStdString());
	EventPtr evt = Event::Cast(obj.get());

	if ( evt ) {
		showOriginSummary(evt->preferredOriginID());
		_ui->statusBar->showMessage(QString("Selected object with ID %1").arg(evt->preferredOriginID().c_str()), 3000);
	}
	else {
		obj = query()->getObject(Origin::TypeInfo(), objectID.toStdString());
		OriginPtr org = Origin::Cast(obj.get());

		if ( org ) {
			showOriginSummary(org);
			_ui->statusBar->showMessage(QString("Selected object with ID %1").arg(org->publicID().c_str()), 3000);
		}
		else
			showErrorBox(QString("Object with id %1 not found in database")
			        .arg(objectID).toStdString().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showOriginSummary(const std::string& publicID) {

	PublicObjectPtr obj = query()->getObject(Origin::TypeInfo(), publicID);
	OriginPtr org = Origin::Cast(obj.get());

	if ( org ) {

		showOriginSummary(org);
		_selectedObjectID = org->publicID();

		if ( _particleMotionWidget )
		    _particleMotionWidget->setOrigin(_selectedObjectID);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showOriginSummary(OriginPtr org) {

	for (PlotList::const_iterator it = _plotList.constBegin();
	        it != _plotList.constEnd(); ++it) {
		if ( !(*it) ) continue;
		(*it)->setOrigin(query(), org.get());
	}

	float lonUncertainty = .0;
	try {
		lonUncertainty = org->longitude().uncertainty();
	} catch ( ... ) {}

	float latUncertainty = .0;
	try {
		latUncertainty = org->latitude().uncertainty();
	} catch ( ... ) {}

	float depthUncertainty = .0;
	try {
		depthUncertainty = org->depth().uncertainty();
	} catch ( ... ) {}

	_ui->dockWidget->setWindowTitle(QString::fromUtf8(_region->getRegionName(
	    org->latitude().value(), org->longitude().value()).c_str()));

	if ( org->magnitudeCount() == 0 )
	    query()->loadMagnitudes(org.get());

	if ( org->arrivalCount() == 0 )
	    query()->loadArrivals(org.get());


	QString diststr = QString("-");
	try {
		diststr = QString::number(deg2km(org->quality().minimumDistance()), 'f', scheme().distancePrecision());
	} catch ( ... ) {}

	QString azistr = QString("-");
	try {
		azistr = QString::number(org->quality().azimuthalGap(), 'f', scheme().azimuthPrecision());
	} catch ( ... ) {}

	QString rmsstr = QString("-");
	try {
		rmsstr = QString::number(org->quality().standardError(), 'f', scheme().rmsPrecision());
	} catch ( ... ) {}


	QString depthstr = QString("-");
	try {
		depthstr = QString::number(org->depth().value(), 'f', scheme().depthPrecision());
	} catch ( ... ) {}

	QString evaluationStatus = QString("-");
	try {
		evaluationStatus = org->evaluationStatus().toString();
	} catch ( ... ) {}

	QString magnitude;
	for (size_t i = 0; i < org->magnitudeCount(); ++i) {
		MagnitudePtr mag = org->magnitude(i);
		if ( !mag ) continue;
		try {
			magnitude.append(QString("%1 %2\n")
			        .arg(QString::number(mag->magnitude().value(), 'f', scheme().generalPrecision()))
			        .arg(mag->type().c_str()));
		} catch ( ... ) {}
	}

	if ( magnitude.isEmpty() )
	    magnitude = QString("-");

	_ui->event_hourLabel->setText(QString::fromStdString(org->time().value().toString("%Y-%m-%d %H:%M:%S")));
	_ui->event_latitudeLabel->setText(QString::number(fabs(org->latitude().value()), 'f', 4));
	_ui->event_latCard->setText(QString::fromUtf8("°%1").arg(getCardLetter(org->latitude().value(), Latitude).c_str()));
	_ui->event_latUncertainty->setText(QString("+/- %1").arg(QString::number(latUncertainty, 'f', 2)));
	_ui->event_longitudeLabel->setText(QString::number(fabs(org->longitude().value()), 'f', 4));
	_ui->event_lonCard->setText(QString::fromUtf8("°%1").arg(getCardLetter(org->longitude().value(), Longitude).c_str()));
	_ui->event_lonUncertainty->setText(QString("+/- %1").arg(QString::number(lonUncertainty, 'f', 2)));
	try {
		_ui->event_phasesLabel->setText(QString("%1 / %2 [%3]")
		        .arg(org->quality().associatedPhaseCount())
		        .arg(org->quality().usedPhaseCount())
		        .arg(org->arrivalCount()));
	} catch ( ... ) {
		_ui->event_phasesLabel->setText("");
	}
	_ui->event_depthLabel->setText(depthstr);
	_ui->event_depthUncertainty->setText(QString("+/- %1").arg(QString::number(depthUncertainty, 'f', 2)));
	_ui->event_rmsLabel->setText(rmsstr);
	_ui->event_azimuthLabel->setText(azistr);
	_ui->event_minDistLabel->setText(diststr);
	_ui->event_agencyLabel->setText(QString::fromUtf8(org->creationInfo().agencyID().c_str()));
	_ui->event_authorLabel->setText(QString::fromUtf8(org->creationInfo().author().c_str()));
	_ui->event_methodLabel->setText(QString::fromUtf8(org->methodID().c_str()));
	_ui->event_modelLabel->setText(QString::fromUtf8(org->earthModelID().c_str()));
	_ui->event_magnitudeLabel->setText(magnitude);
	_ui->event_evaluationLabel->setText(evaluationStatus);

	if ( _globalMap ) {

		_globalMap->foregroundCanvas().removeGeometries(Map::Geometry::d_Pin);
		Map::Pin* pin = new Map::Pin();
		pin->setGeoPosition(QPointF(org->longitude().value(), org->latitude().value()));
		pin->setPix(QPixmap(":images/redpin.png"), QSize(30, 24));
		pin->setLegendEnabled(true);
		pin->setBrush(Qt::black);
		pin->pen().setColor(Qt::white);
		pin->setToolTip(QString("<p style=\"color:white;\">"
			"<b>Origin</b>:&nbsp;%1<br/>"
			"<b>Latitude</b>:&nbsp;%2<br/>"
			"<b>Longitude</b>:&nbsp;%3<br/>"
			"<b>Depth</b>:&nbsp;%4 km</p>")
		        .arg(org->publicID().c_str())
		        .arg(org->latitude().value())
		        .arg(org->longitude().value())
		        .arg(depthstr));
		if ( _globalMap->foregroundCanvas().addGeometry(pin) ) {
			_globalMap->setDefaultGeoPosition(QPointF(org->longitude().value(), org->latitude().value()));
			_globalMap->centerOn(QPointF(org->longitude().value(), org->latitude().value()));
		}
		else {
			delete pin;
			pin = NULL;
		}
	}

	if ( _topoMap )
	    _topoMap->setOrigin(org.get());

	_summaryMap->setDatabase(query());
	_summaryMap->setOrigin(org.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AdvancedEventManagerView::initConfiguration() {

	if ( !IPGP::Gui::Client::Application::initConfiguration() )
	    return false;

	int error = 0;

	// ---------------------------------------------------------------------
	//  GLOBAL parameters
	// ---------------------------------------------------------------------
	try {
		_magnitudes = configGetStrings("aemv.global.availableMagnitudes");
	}
	catch ( ... ) {}

	// ---------------------------------------------------------------------
	//  EventList parameters
	// ---------------------------------------------------------------------
	try {
		_eventListSettings.fetchUnassociated = configGetBool(
		    "aemv.eventList.fetchUnassociatedOrigins");
	}
	catch ( ... ) {}
	try {
		_eventListSettings.fetchPreferredOriginComment = configGetBool(
		    "aemv.eventList.fetchPreferredOriginComment");
	}
	catch ( ... ) {}
	try {
		_eventListSettings.fetchSiblingOriginComment = configGetBool(
		    "aemv.eventList.fetchSiblingOriginComment");
	}
	catch ( ... ) {}
	try {
		_eventListSettings.fetchUnassociatedOriginComment = configGetBool(
		    "aemv.eventList.fetchUnassociatedOriginComment");
	}
	catch ( ... ) {}
	try {
		_eventListSettings.useMultiThreads = configGetBool(
		    "aemv.eventList.useSeparateThread");
	}
	catch ( ... ) {}
	try {
		_eventListSettings.useAdvancedOptions = configGetBool(
		    "aemv.eventList.allowAdvancedOptions");
	}
	catch ( ... ) {}
	try {
		_eventListSettings.decorateItems = configGetBool(
		    "aemv.eventList.decorateItems");
	}
	catch ( ... ) {}
	try {
		_useNormalizedEpicenterSize = configGetBool(
		    "aemv.eventList.useNormalizedEpicenterSize");
	}
	catch ( ... ) {
		_useNormalizedEpicenterSize = false;
	}


	// ---------------------------------------------------------------------
	//  Locators parameters
	// ---------------------------------------------------------------------
	std::vector<std::string> tmpLocators;
	try {
		tmpLocators = configGetStrings("aemv.locators");
	} catch ( ... ) {}

	for (std::vector<std::string>::const_iterator it = tmpLocators.begin();
	        it != tmpLocators.end(); ++it) {

		EarthModelIDs list;
		std::string profiles = "aemv.locator." + *it + ".profiles";

		try {
			list = configGetStrings(profiles);
		} catch ( ... ) {}

		if ( list.empty() ) continue;

		_locators.push_back(Locators(*it, list));
	}


	// ---------------------------------------------------------------------
	//  Topography parameters
	// ---------------------------------------------------------------------
	_topoSettings.clear();
	std::vector<std::string> tmpTopographies;
	try {
		tmpTopographies = configGetStrings("aemv.topographies");
	} catch ( ... ) {}

	for (std::vector<std::string>::const_iterator it = tmpTopographies.begin();
	        it != tmpTopographies.end(); ++it) {

		TopographyRendererSettings set;

		set.setName((*it).c_str());

		// File
		try {
			set.setFilepath(configGetString(Environment::Instance()->absolutePath(
			    "aemv.topography." + (*it) + ".file")).c_str());
		} catch ( ... ) {}
		try {
			QString type = configGetString(Environment::Instance()->absolutePath(
			    "aemv.topography." + (*it) + ".type")).c_str();
			if ( type.contains("XYZ") )
				set.setFileType(TopographyRendererSettings::XYZ_FILE);
			else if ( type.contains("STL") )
			    set.setFileType(TopographyRendererSettings::STL_FILE);
		} catch ( ... ) {
			set.setFileType(TopographyRendererSettings::NONE);
		}

		// Latitudes
		try {
			set.latitude().min = configGetDouble(
			    "aemv.topography." + (*it) + ".latitude.min");
		} catch ( ... ) {
			set.latitude().min = .0;
		}
		try {
			set.latitude().max = configGetDouble(
			    "aemv.topography." + (*it) + ".latitude.max");
		} catch ( ... ) {
			set.latitude().max = .0;
		}
		try {
			set.latitude().convRangeMin = configGetDouble(
			    "aemv.topography." + (*it) + ".latitude.convRangeMin");
		} catch ( ... ) {
			set.latitude().convRangeMin = .0;
		}
		try {
			set.latitude().convRangeMax = configGetDouble(
			    "aemv.topography." + (*it) + ".latitude.convRangeMax");
		} catch ( ... ) {
			set.latitude().convRangeMax = .0;
		}

		// Longitudes
		try {
			set.longitude().min = configGetDouble(
			    "aemv.topography." + (*it) + ".longitude.min");
		} catch ( ... ) {
			set.longitude().min = .0;
		}
		try {
			set.longitude().max = configGetDouble(
			    "aemv.topography." + (*it) + ".longitude.max");
		} catch ( ... ) {
			set.longitude().max = .0;
		}
		try {
			set.longitude().convRangeMin = configGetDouble(
			    "aemv.topography." + (*it) + ".longitude.convRangeMin");
		} catch ( ... ) {
			set.longitude().convRangeMin = .0;
		}
		try {
			set.longitude().convRangeMax = configGetDouble(
			    "aemv.topography." + (*it) + ".longitude.convRangeMax");
		} catch ( ... ) {
			set.longitude().convRangeMax = .0;
		}

		// Elevation
		try {
			set.elevation().min = configGetDouble("aemv.topography." + (*it) + ".elevation.min");
		} catch ( ... ) {
			set.elevation().min = .0;
		}
		try {
			set.elevation().max = configGetDouble("aemv.topography." + (*it) + ".elevation.max");
		} catch ( ... ) {
			set.elevation().max = .0;
		}
		try {
			set.elevation().convRangeMinBelowZero = configGetDouble(
			    "aemv.topography." + (*it) + ".elevation.convRangeMinBelowZero");
		} catch ( ... ) {
			set.elevation().convRangeMinBelowZero = .0;
		}
		try {
			set.elevation().convRangeMaxBelowZero = configGetDouble(
			    "aemv.topography." + (*it) + ".elevation.convRangeMaxBelowZero");
		} catch ( ... ) {
			set.elevation().convRangeMaxBelowZero = .0;
		}
		try {
			set.elevation().convRangeMinAboveZero = configGetDouble(
			    "aemv.topography." + (*it) + ".elevation.convRangeMinAboveZero");
		} catch ( ... ) {
			set.elevation().convRangeMinAboveZero = .0;
		}
		try {
			set.elevation().convRangeMaxAboveZero = configGetDouble(
			    "aemv.topography." + (*it) + ".elevation.convRangeMaxAboveZero");
		} catch ( ... ) {
			set.elevation().convRangeMaxAboveZero = .0;
		}

		_topoSettings.insert((*it).c_str(), set);
	}

	if ( error != 0 ) return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::databaseMessage(const QString& oldDB,
                                               const QString& newDB) {
	_eventListWidget->setDatabase(query());
	_eventListWidget->startBlinking();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedEventManagerView::showSummary() {
	(_ui->actionShowSummary->isChecked()) ?
	        _ui->dockWidget->show() : _ui->dockWidget->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




