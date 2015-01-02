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

#define SEISCOMP_COMPONENT IPGP_GL_TOPOMAP

#include <ipgp/gui/opengl/widgets/topographymap.h>
#include <ipgp/gui/opengl/widgets/ui_topographymap.h>
#include <ipgp/gui/opengl/widgets/ui_topographymapdialog.h>
#include <ipgp/gui/opengl/topographyrenderer.h>
#include <ipgp/gui/opengl/topographyfile.h>
#include <ipgp/gui/opengl/camera.h>
#include <ipgp/gui/opengl/triangle.h>
#include <ipgp/gui/opengl/vertex.h>
#include <ipgp/gui/opengl/gl.h>
#include <ipgp/gui/opengl/drawables/arrival.h>
#include <ipgp/gui/opengl/drawables/hypocenter.h>
#include <ipgp/gui/opengl/drawables/station.h>
#include <ipgp/core/math/math.h>
#include <ipgp/core/file/file.h>
#include <ipgp/gui/misc/misc.h>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/inventory.h>

#include <QtGui>



using namespace Seiscomp;
using namespace Seiscomp::Core;


namespace IPGP {
namespace Gui {
namespace OpenGL {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CustomAction::CustomAction(QObject* parent) :
		QAction(parent) {
	connect(this, SIGNAL(triggered(bool)), this, SLOT(updateIcon(const bool&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CustomAction::CustomAction(const QIcon& checked, const QIcon& unchecked, QObject* parent) :
		QAction(parent), _checked(checked), _unchecked(unchecked) {
	connect(this, SIGNAL(triggered(bool)), this, SLOT(updateIcon(const bool&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CustomAction::~CustomAction() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CustomAction::setChecked(const bool& v) {
	(v) ? setIcon(_checked) : setIcon(_unchecked);
	QAction::setChecked(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CustomAction::updateIcon(const bool& v) {
	(v) ? setIcon(_checked) : setIcon(_unchecked);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyMap::TopographyMap(QWidget* parent, Qt::WFlags f) :
		QMainWindow(parent, f) {

	setObjectName("TopographyMap");
	_paintOrphanOrigin = true;
	_inventoryVisible = false;
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyMap::~TopographyMap() {

//	if ( _renderer ) delete _renderer;
	if ( _dialogUi ) delete _dialogUi;
	if ( _ui ) delete _ui;

	emit nullifyQObject(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::init() {

	_ui = new Ui::TopographyMap;
	_ui->setupUi(this);
	_renderer = new TopographyRenderer(this);
	setCentralWidget(_renderer);

	connect(_renderer, SIGNAL(statusMessage(const QString&)), this, SLOT(showStatusMessage(const QString&)));
	connect(_renderer, SIGNAL(currentSettingsChanged()), this, SLOT(rendererSettingsChanged()));

	_ui->mainToolBar->setIconSize(QSize(24, 24));

	_dialogUi = new Ui::TopographyMapDialog;
	_configDialog = new QDialog(this);
	_dialogUi->setupUi(_configDialog);
	_configDialog->hide();

	connect(_dialogUi->comboBoxTopographies, SIGNAL(currentIndexChanged(const QString&)),
	    this, SLOT(changeRendererSettings(const QString&)));
	connect(_dialogUi->buttonBoxDialog, SIGNAL(rejected()), _configDialog, SLOT(reject()));
	connect(_dialogUi->buttonBoxDialog, SIGNAL(accepted()), this, SLOT(configDialogAccepted()));

	_aAxis = new QAction(tr("Axis"), this);
	_aAxis->setIcon(QIcon(":images/axis_3d.png"));
	_aAxis->setToolTip(tr("Sets the renderer axis visible/hidden"));
	_aAxis->setCheckable(true);
	_aAxis->setChecked(_renderer->generalSettings().axisVisible());
	_aAxis->connect(_aAxis, SIGNAL(triggered(bool)), _renderer, SLOT(setAxisVisible(const bool&)));

	_aInfo = new QAction(tr("Info."), this);
	_aInfo->setIcon(QIcon(":images/info.png"));
	_aInfo->setToolTip(tr("Sets the renderer information visible/hidden"));
	_aInfo->setCheckable(true);
	_aInfo->setChecked(_renderer->generalSettings().rendererInfoVisible());
	_aInfo->connect(_aInfo, SIGNAL(triggered(bool)), _renderer, SLOT(setRendererInfoVisible(const bool&)));

	_aGrid = new QAction(tr("Grid"), this);
	_aGrid->setIcon(QIcon(":images/grid.png"));
	_aGrid->setToolTip(tr("Sets the renderer grid visible"));
	_aGrid->setCheckable(true);
	_aGrid->setChecked(_renderer->activeSettings().graticuleVisible());
	_aGrid->connect(_aGrid, SIGNAL(triggered(bool)), _renderer, SLOT(setGraticuleVisible(const bool&)));

	_aBbox = new QAction(this);
	_aBbox->setIcon(QIcon(":images/bbox.png"));
	_aBbox->setToolTip(tr("Sets the bounding box visible/hidden"));
	_aBbox->setCheckable(true);
	_aBbox->setChecked(_renderer->generalSettings().boundingBoxVisible());
	_aBbox->connect(_aBbox, SIGNAL(triggered(bool)), _renderer, SLOT(setBoundingBoxVisible(const bool&)));

	_aLight = new CustomAction(QIcon(":images/lighton.png"), QIcon(":images/lightoff.png"), this);
	_aLight->setToolTip(tr("Sets the light ON/OFF"));
	_aLight->setCheckable(true);
	_aLight->setChecked(_renderer->generalSettings().lightingActivated());
//	if ( _renderer->generalSettings().lightingActivated() ) {
//		_aLight->setIcon(QIcon(":images/lightoff.png"));
//	}
//	else {
//		_aLight->setIcon(QIcon(":images/lighton.png"));
//	}
	_aLight->connect(_aLight, SIGNAL(triggered(bool)), _renderer, SLOT(setLightingActivated(const bool&)));

	_ui->mainToolBar->addAction(_aAxis);
	_ui->mainToolBar->addAction(_aInfo);
	_ui->mainToolBar->addAction(_aGrid);
	_ui->mainToolBar->addAction(_aBbox);
	_ui->mainToolBar->addAction(_aLight);

	QToolBar* t1 = this->addToolBar(tr("Rendering Type"));
	QActionGroup* ag1 = new QActionGroup(this);

	_aPointCloud = new QAction(tr("Point cloud"), this);
	_aPointCloud->setIcon(QIcon(":images/points.png"));
	_aPointCloud->setToolTip(tr("Renderer points clouds"));
	_aPointCloud->setCheckable(true);
	_aPointCloud->connect(_aPointCloud, SIGNAL(triggered(bool)), this, SLOT(renderingTypeChanged(const bool&)));
	ag1->addAction(_aPointCloud);

	_aMesh = new QAction(tr("Mesh"), this);
	_aMesh->setIcon(QIcon(":images/backlines.png"));
	_aMesh->setToolTip(tr("Renderer mesh"));
	_aMesh->setCheckable(true);
	_aMesh->connect(_aMesh, SIGNAL(triggered(bool)), this, SLOT(renderingTypeChanged(const bool&)));
	ag1->addAction(_aMesh);

	_aFilledMesh = new QAction(tr("Filled mesh"), this);
	_aFilledMesh->setIcon(QIcon(":images/flatlines.png"));
	_aFilledMesh->setToolTip(tr("Renderer filled polygons with visible mesh"));
	_aFilledMesh->setCheckable(true);
	_aFilledMesh->connect(_aFilledMesh, SIGNAL(triggered(bool)), this, SLOT(renderingTypeChanged(const bool&)));
	ag1->addAction(_aFilledMesh);

	_aFilled = new QAction(tr("Filled"), this);
	_aFilled->setIcon(QIcon(":images/flat.png"));
	_aFilled->setToolTip(tr("Renderer filled polygons without mesh"));
	_aFilled->setCheckable(true);
	_aFilled->connect(_aFilled, SIGNAL(triggered(bool)), this, SLOT(renderingTypeChanged(const bool&)));
	ag1->addAction(_aFilled);

	switch ( _renderer->renderingType() ) {
		case TopographyRenderer::POINTCLOUD:
			_aPointCloud->setChecked(true);
		break;
		case TopographyRenderer::MESH:
			_aMesh->setChecked(true);
		break;
		case TopographyRenderer::FILLEDMESH:
			_aFilledMesh->setChecked(true);
		break;
		case TopographyRenderer::FILLED:
			_aFilled->setChecked(true);
		break;
	}

	t1->addActions(ag1->actions());

	QToolBar* t2 = this->addToolBar(tr("Rendering tweaks"));
	_aSmooth = new QAction(tr("Smooth"), this);
	_aSmooth->setIcon(QIcon(":images/smooth.png"));
	_aSmooth->setToolTip(tr("Activate/deactivate smoothing"));
	_aSmooth->setCheckable(true);
	_aSmooth->connect(_aSmooth, SIGNAL(triggered(bool)), _renderer, SLOT(setSmoothingActivated(const bool&)));

	_aTexture = new QAction(tr("Textures"), this);
	_aTexture->setIcon(QIcon(":images/textures.png"));
	_aTexture->setToolTip(tr("Renderer textures"));
	_aTexture->setCheckable(true);
	_aTexture->connect(_aTexture, SIGNAL(triggered(bool)), _renderer, SLOT(setTextureVisible(const bool&)));

	t2->addAction(_aSmooth);
	t2->addAction(_aTexture);

	QToolBar* t3 = this->addToolBar(tr("Settings"));
	QAction* set = new QAction(tr("Settings"), this);
	set->setIcon(QIcon(":images/preferences-system.png"));
	set->setToolTip(tr("Opens the configuration dialog"));
	set->setCheckable(false);
	set->connect(set, SIGNAL(triggered(bool)), this, SLOT(showConfigDialog()));
	t3->addAction(set);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::updateInterface() {

	switch ( _renderer->renderingType() ) {
		case TopographyRenderer::POINTCLOUD:
			_aPointCloud->setChecked(true);
		break;
		case TopographyRenderer::MESH:
			_aMesh->setChecked(true);
		break;
		case TopographyRenderer::FILLEDMESH:
			_aFilledMesh->setChecked(true);
		break;
		case TopographyRenderer::FILLED:
			_aFilled->setChecked(true);
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setDatabase(DataModel::DatabaseQuery* query) {
	_query = query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setOrigin(DataModel::Origin* org) {

	if ( !_query ) {
		SEISCOMP_WARNING("Can't set origin: no query instance available");
		return;
	}

	if ( !org ) return;

	_renderer->clearArrivals();
	_renderer->clearStations();
	_renderer->clearHypocenters();

	if ( org->latitude() < _renderer->activeSettings().latitude().min
	        || org->latitude() > _renderer->activeSettings().latitude().max
	        || org->longitude() < _renderer->activeSettings().longitude().min
	        || org->longitude() > _renderer->activeSettings().longitude().max ) {
		SEISCOMP_DEBUG("Skipped object with publicID %s, out-of-range", org->publicID().c_str());
		return;
	}

	DataModel::EventPtr event = _query->getEvent(org->publicID());

	if ( !event ) {
		if ( !_paintOrphanOrigin ) return;
		SEISCOMP_WARNING("%s is an orphan but has received a painting request", org->publicID().c_str());
	}

	if ( org->arrivalCount() == 0 )
	    _query->loadArrivals(org);

	DataModel::MagnitudePtr mag = NULL;
	if ( event ) {
		mag = DataModel::Magnitude::Find(event->preferredMagnitudeID());
		if ( !mag )
		    mag = DataModel::Magnitude::Cast(_query->getObject(DataModel::Magnitude::TypeInfo(),
		        event->preferredMagnitudeID()));
	}

	bool hasDepth = false;
	float depthValue = .0;
	try {
		depthValue = org->depth().value();
		hasDepth = true;
	} catch ( ... ) {}

	bool hasLonUncertainty = false;
	float lonUncertainty = .0;
	try {
		lonUncertainty = org->longitude().uncertainty();
		hasLonUncertainty = true;
	} catch ( ... ) {}

	bool hasLatUncertainty = false;
	float latUncertainty = .0;
	try {
		latUncertainty = org->latitude().uncertainty();
		hasLatUncertainty = true;
	} catch ( ... ) {}

	bool hasDepthUncertainty = false;
	float depthUncertainty = .0;
	try {
		depthUncertainty = org->depth().uncertainty();
		hasDepthUncertainty = true;
	} catch ( ... ) {}

	bool hasMagnitude = false;
	float magnitude = .0;
	if ( mag ) {
		try {
			magnitude = mag->magnitude().value();
			hasMagnitude = true;
		} catch ( ... ) {}
	}


	Hypocenter* hypocenter = new Hypocenter;
	hypocenter->setName(org->publicID().c_str());
	hypocenter->setRendererSettings(_renderer->activeSettings());
	hypocenter->setGeoPosition(org->latitude().value(), org->longitude().value(), depthValue);

	if ( hasLatUncertainty && hasLonUncertainty && hasDepthUncertainty )
	    hypocenter->setGeoPositionUncertainties(latUncertainty, lonUncertainty, depthUncertainty);

	if ( hasMagnitude )
	    hypocenter->setMagnitude(magnitude);

	if ( hasDepth )
		hypocenter->setColor(Misc::getDepthColoration(depthValue));
	else
		hypocenter->setColor(Qt::white);

	if ( !_renderer->addHypocenter(hypocenter) ) {
		delete hypocenter;
		hypocenter = NULL;
	}

	if ( !hypocenter ) return;

	for (size_t i = 0; i < org->arrivalCount(); ++i) {

		DataModel::ArrivalPtr ar = org->arrival(i);

		if ( !ar ) continue;

		bool hasResiduals = false;
		float ares = .0;
		try {
			ares = ar->timeResidual();
			hasResiduals = true;
		} catch ( ... ) {}

		DataModel::PickPtr pick = DataModel::Pick::Find(ar->pickID());
		if ( !pick ) {
			pick = DataModel::Pick::Cast(_query->getObject(DataModel::Pick::TypeInfo(), ar->pickID()));
			if ( !pick ) continue;
		}

		DataModel::StationPtr station = _query->getStation(pick->waveformID().networkCode(),
		    pick->waveformID().stationCode(), Time::GMT());

		if ( !station ) continue;
		if ( station->latitude() < _renderer->activeSettings().latitude().min )
		    continue;
		if ( station->latitude() > _renderer->activeSettings().latitude().max )
		    continue;
		if ( station->longitude() < _renderer->activeSettings().longitude().min )
		    continue;
		if ( station->longitude() > _renderer->activeSettings().longitude().max )
		    continue;

		if ( !_renderer->getStation(pick->waveformID().networkCode().c_str(), station->code().c_str()) ) {

			Station* stationGeometry = new Station;
			stationGeometry->setName(station->code().c_str());
			stationGeometry->setRendererSettings(_renderer->activeSettings());
			stationGeometry->setNetwork(pick->waveformID().networkCode().c_str());
			stationGeometry->setGeoPosition(station->latitude(), station->longitude(), station->elevation());
			if ( hasResiduals )
			    stationGeometry->setColor(Misc::getResidualsColoration(ares));

			if ( !_renderer->addStation(stationGeometry) ) {
				delete stationGeometry;
				stationGeometry = NULL;
			}
		}

		Station* parent = _renderer->getStation(pick->waveformID().networkCode().c_str(),
		    station->code().c_str());

		if ( parent ) {
			if ( !_renderer->getArrival(parent->name(), ar->phase().code().c_str()) ) {

				Arrival* stationArrival = new Arrival;
				stationArrival->setName(ar->pickID().c_str());
				stationArrival->setHypocenter(hypocenter);
				stationArrival->setStation(parent);
				stationArrival->setPhaseCode(ar->phase().code().c_str());
				if ( hasResiduals ) stationArrival->setResiduals(ares);

				if ( !_renderer->addArrival(stationArrival) ) {
					delete stationArrival;
					stationArrival = NULL;
				}
			}
		}
		else {
			SEISCOMP_DEBUG("No parent station found for arrival %s", ar->pickID().c_str());
		}

		emit loadingPercentage(Core::Math::percentageOfSomething(org->arrivalCount(), i),
		    objectName(), "Painting origin object...");
	}

	_renderer->updateHypocenters();
	_renderer->updateStations();
	_renderer->updateArrivals();

	//! Emulate this with multi shot process so that the view slowly eases
	//! into hypocenter's position
	_renderer->camera()->lookAt(hypocenter->vertex().x(), hypocenter->vertex().y(), hypocenter->vertex().z());

	emit loadingPercentage(-1, objectName(), "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setOrigins(Core::OriginList* list) {

	_renderer->clearArrivals();
	_renderer->clearStations();
	_renderer->clearHypocenters();

	if ( !_query ) {
		SEISCOMP_WARNING("Can't set origins: no query instance available");
		return;
	}

	TopographyRendererSettings old = _renderer->activeSettings();

	size_t nbUpdate = 0;
	size_t idx = 0;
	for (Core::OriginList::const_iterator it = list->begin();
	        it != list->end(); ++it, ++idx) {

		DataModel::OriginPtr org = it->first;
		DataModel::EventPtr event = it->second;

		if ( !event )
		    if ( !_paintOrphanOrigin ) continue;

		DataModel::MagnitudePtr mag;
		if ( event ) {
			mag = DataModel::Magnitude::Find(event->preferredMagnitudeID());
			if ( !mag )
			    mag = DataModel::Magnitude::Cast(_query->getObject(DataModel::Magnitude::TypeInfo(),
			        event->preferredMagnitudeID()));
		}

		if ( !org ) continue;

		/*
		 if ( latIsOutOfResolution(org->latitude()) )
		 setNewLatResolution(org->latitude()), ++nbUpdate;
		 if ( lonIsOutOfResolution(org->longitude()) )
		 setNewLonResolution(org->longitude()), ++nbUpdate;
		 */

		//! Skip this origin if its position is outside of the context resolution
		if ( org->latitude() < _renderer->activeSettings().latitude().min
		        || org->latitude() > _renderer->activeSettings().latitude().max
		        || org->longitude() < _renderer->activeSettings().longitude().min
		        || org->longitude() > _renderer->activeSettings().longitude().max )
		    continue;


		bool hasDepth = false;
		float depthValue = .0;
		try {
			depthValue = org->depth().value();
			hasDepth = true;
		} catch ( ... ) {}

		if ( hasDepth ) {
			if ( depthIsOutOfResolution(depthValue * 1000) )
			    setNewDepthResolution(depthValue * 1000), ++nbUpdate;
		}

		bool hasLonUncertainty = false;
		float lonUncertainty = .0;
		try {
			lonUncertainty = org->longitude().uncertainty();
			hasLonUncertainty = true;
		} catch ( ... ) {}

		bool hasLatUncertainty = false;
		float latUncertainty = .0;
		try {
			latUncertainty = org->latitude().uncertainty();
			hasLatUncertainty = true;
		} catch ( ... ) {}

		bool hasDepthUncertainty = false;
		float depthUncertainty = .0;
		try {
			depthUncertainty = org->depth().uncertainty();
			hasDepthUncertainty = true;
		} catch ( ... ) {}

		bool hasMagnitude = false;
		float magnitude = .0;
		if ( mag ) {
			try {
				magnitude = mag->magnitude().value();
				hasMagnitude = true;
			} catch ( ... ) {}
		}

		Hypocenter* hypocenter = new Hypocenter;
		hypocenter->setName(org->publicID().c_str());
		hypocenter->setRendererSettings(_renderer->activeSettings());
		hypocenter->setGeoPosition(org->latitude().value(), org->longitude().value(), depthValue);

		if ( hasLatUncertainty && hasLonUncertainty && hasDepthUncertainty )
		    hypocenter->setGeoPositionUncertainties(latUncertainty, lonUncertainty, depthUncertainty);

		if ( hasMagnitude )
		    hypocenter->setMagnitude(magnitude);

		if ( hasDepth )
			hypocenter->setColor(Misc::getDepthColoration(depthValue));
		else
			hypocenter->setColor(Qt::white);

		if ( !_renderer->addHypocenter(hypocenter) ) {
			delete hypocenter;
			hypocenter = NULL;
		}

		emit loadingPercentage(Core::Math::percentageOfSomething<int>(list->size(), idx),
		    objectName(), "Rendering origins");
	}

	//! Reload the context to make sure everything gets properly rendered
	if ( nbUpdate > 0 ) {
		qDebug() << nbUpdate << " updates are to be performed...";
		qDebug() << "Orig. min lat. = " << old.latitude().min << " / vs new = " << _renderer->activeSettings().latitude().min;
		qDebug() << "Orig. max lat. = " << old.latitude().max << " / vs new = " << _renderer->activeSettings().latitude().max;

		qDebug() << "Orig. min long. = " << old.longitude().min << " / vs new = " << _renderer->activeSettings().longitude().min;
		qDebug() << "Orig. max long. = " << old.longitude().max << " / vs new = " << _renderer->activeSettings().longitude().max;

		qDebug() << "Orig. min ele. = " << old.elevation().min << " / vs new = " << _renderer->activeSettings().elevation().min;
		qDebug() << "Orig. max ele. = " << old.elevation().max << " / vs new = " << _renderer->activeSettings().elevation().max;

		qDebug("Reloading file");
//		_renderer->file()->reload();
		qDebug("Reassessing objects position");
		_renderer->reassessObjectsPosition();
		qDebug("Updating everything");
		_renderer->updateEverything();
	}
	_renderer->updateHypocenters();

	emit loadingPercentage(-1, objectName(), "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setCrossSection(CrossSection* cs) {

	_renderer->clearCrossSections();
	if ( cs ) _renderer->addCrossSection(cs);
	_renderer->updateCrossSections();

	qDebug("Set new cross section");

	emit loadingPercentage(-1, objectName(), "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyMap::latIsOutOfResolution(const double& v) const {
	return v < _renderer->activeSettings().latitude().min
	        || v > _renderer->activeSettings().latitude().max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyMap::lonIsOutOfResolution(const double& v) const {
	return v < _renderer->activeSettings().longitude().min
	        || v > _renderer->activeSettings().longitude().max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyMap::depthIsOutOfResolution(const double& v) const {
	return v < _renderer->activeSettings().elevation().min
	        || v > _renderer->activeSettings().elevation().max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setNewLatResolution(const double& v) {
	if ( v < _renderer->activeSettings().latitude().min )
	    _renderer->activeSettings().latitude().min = v;
	if ( v > _renderer->activeSettings().latitude().max )
	    _renderer->activeSettings().latitude().max = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setNewLonResolution(const double& v) {
	if ( v < _renderer->activeSettings().longitude().min )
	    _renderer->activeSettings().longitude().min = v;
	if ( v > _renderer->activeSettings().longitude().max )
	    _renderer->activeSettings().longitude().max = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setNewDepthResolution(const double& v) {
	if ( v < _renderer->activeSettings().elevation().min )
	    _renderer->activeSettings().elevation().min = v;
	if ( v > _renderer->activeSettings().elevation().max )
	    _renderer->activeSettings().elevation().max = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::setInventoryVisible(const bool& v) {

	_inventoryVisible = v;

	if ( !_query ) {
		SEISCOMP_ERROR("No query interface available");
		return;
	}

	_renderer->clearStations();

	if ( !_inventoryVisible ) return;

	DataModel::DatabaseReaderPtr reader = new DataModel::DatabaseReader(_query->driver());
	DataModel::InventoryPtr inv = new DataModel::Inventory();
	reader->loadNetworks(inv.get());

	for (size_t i = 0; i < inv->networkCount(); ++i) {
		DataModel::NetworkPtr network = inv->network(i);
		reader->load(network.get());

		for (size_t j = 0; j < network->stationCount(); ++j) {
			DataModel::StationPtr station = network->station(j);

			if ( !station ) continue;
			if ( station->latitude() < _renderer->activeSettings().latitude().min )
			    continue;
			if ( station->latitude() > _renderer->activeSettings().latitude().max )
			    continue;
			if ( station->longitude() < _renderer->activeSettings().longitude().min )
			    continue;
			if ( station->longitude() > _renderer->activeSettings().longitude().max )
			    continue;

//			if ( station->elevation() > _renderer->activeSettings().elevation().max )
//				continue;
//
//			if ( station->elevation() < _renderer->activeSettings().elevation().min )
//				continue;

			if ( !_renderer->getStation(station->network()->code().c_str(), station->code().c_str()) ) {

				Station* stationGeometry = new Station;
				stationGeometry->setName(QString("%1.%2").arg(station->network()->code().c_str()).arg(station->code().c_str()));
				stationGeometry->setRendererSettings(_renderer->activeSettings());
				stationGeometry->setNetwork(station->network()->code().c_str());
				stationGeometry->setGeoPosition(station->latitude(), station->longitude(), station->elevation());
				stationGeometry->setColor(Qt::red);

				if ( !_renderer->addStation(stationGeometry) ) {
					delete stationGeometry;
					stationGeometry = NULL;
				}
			}
		}

		emit loadingPercentage(Core::Math::percentageOfSomething<size_t>(inv->networkCount(), i),
		    objectName(), "Painting inventory object...");
	}

	emit loadingPercentage(98, _renderer->objectName(), "Updating stations...");

	_renderer->updateStations();

	emit loadingPercentage(-1, objectName(), "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::renderingTypeChanged(const bool& v) {

	Q_UNUSED(v);
	if ( _aPointCloud->isChecked() )
		_renderer->setRenderingType(TopographyRenderer::POINTCLOUD);
	else if ( _aMesh->isChecked() )
		_renderer->setRenderingType(TopographyRenderer::MESH);
	else if ( _aFilledMesh->isChecked() )
		_renderer->setRenderingType(TopographyRenderer::FILLEDMESH);
	else if ( _aFilled->isChecked() )
	    _renderer->setRenderingType(TopographyRenderer::FILLED);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::rendererSettingsChanged() {
	_renderer->clearStations();
	setInventoryVisible(_inventoryVisible);
	updateInterface();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::changeRendererSettings(const QString& profile) {
	if ( _renderer->availableSettings().contains(profile) ) {
		_renderer->setActiveSettings(_renderer->availableSettings()[profile]);
		updateConfigDialog();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::showConfigDialog() {

	_dialogUi->comboBoxTopographies->clear();
	int i = 0, idx = 0;
	for (TopographyRenderer::SettingsList::const_iterator it = _renderer->availableSettings().constBegin();
	        it != _renderer->availableSettings().constEnd(); ++it, ++i) {
		_dialogUi->comboBoxTopographies->addItem(it.key());
		if ( _renderer->activeSettings() == it.value() )
		    idx = i;
	}
	_dialogUi->comboBoxTopographies->setCurrentIndex(idx);
	updateConfigDialog();

	_configDialog->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::configDialogAccepted() {

	typedef QVector<float> VColor;

	VColor ambient;
	QStringList l = _dialogUi->lineEditAmbientLight->text().split(';');
	for (int i = 0; i < l.size(); ++i)
		ambient.append(l.at(i).toFloat());
	ambient.resize(4);

	VColor diffuse;
	l = _dialogUi->lineEditDiffuseLight->text().split(';');
	for (int i = 0; i < l.size(); ++i)
		diffuse.append(l.at(i).toFloat());
	diffuse.resize(4);

	VColor specular;
	l = _dialogUi->lineEditSpecularLight->text().split(';');
	for (int i = 0; i < l.size(); ++i)
		specular.append(l.at(i).toFloat());
	specular.resize(4);

	VColor position;
	l = _dialogUi->lineEditPositionLight->text().split(';');
	for (int i = 0; i < l.size(); ++i)
		position.append(l.at(i).toFloat());
	position.resize(4);

	_renderer->generalSettings().setAmbientLight(ambient);
	_renderer->generalSettings().setDiffuseLight(diffuse);
	_renderer->generalSettings().setSpecularLight(specular);
	_renderer->generalSettings().setPositionLight(position);

	_renderer->generalSettings().setAmbientLightEnabled(_dialogUi->checkBox_lightAmbient->isChecked());
	_renderer->generalSettings().setDiffuseLightEnabled(_dialogUi->checkBox_lightDiffuse->isChecked());
	_renderer->generalSettings().setSpecularLightEnabled(_dialogUi->checkBox_lightSpecular->isChecked());
	_renderer->generalSettings().setPositionLightEnabled(_dialogUi->checkBox_lightPosition->isChecked());


	_renderer->activeSettings().elevation().min = _dialogUi->doubleSpinBox_eleMin->value();
	_renderer->activeSettings().elevation().max = _dialogUi->doubleSpinBox_eleMax->value();
	_renderer->activeSettings().elevation().convRangeMinAboveZero = _dialogUi->doubleSpinBox_convEleMinAbove->value();
	_renderer->activeSettings().elevation().convRangeMinBelowZero = _dialogUi->doubleSpinBox_convEleMinBelow->value();
	_renderer->activeSettings().elevation().convRangeMaxAboveZero = _dialogUi->doubleSpinBox_convEleMaxAbove->value();
	_renderer->activeSettings().elevation().convRangeMaxBelowZero = _dialogUi->doubleSpinBox_convEleMaxBelow->value();

	_renderer->activeSettings().longitudeGraticule().position
	= static_cast<TopographyRendererSettings::Graticule::Position>(
	        _dialogUi->comboBoxLonGraticulePosition->currentIndex());
	_renderer->activeSettings().latitudeGraticule().position
	= static_cast<TopographyRendererSettings::Graticule::Position>(
	        _dialogUi->comboBoxLatGraticulePosition->currentIndex());

	_configDialog->accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::updateConfigDialog() {

	_dialogUi->checkBox_lightAmbient->setChecked(_renderer->generalSettings().ambientLightEnabled());
	_dialogUi->checkBox_lightDiffuse->setChecked(_renderer->generalSettings().diffuseLightEnabled());
	_dialogUi->checkBox_lightSpecular->setChecked(_renderer->generalSettings().specularLightEnabled());
	_dialogUi->checkBox_lightPosition->setChecked(_renderer->generalSettings().positionLightEnabled());

	QString txt;
	for (int i = 0; i < _renderer->generalSettings().ambientLight().size();
	        ++i) {
		txt.append(QString("%1").arg(_renderer->generalSettings().ambientLight().at(i)));
		if ( i != _renderer->generalSettings().ambientLight().size() - 1 )
		    txt.append("; ");
	}
	_dialogUi->lineEditAmbientLight->setText(txt);
	txt = "";
	for (int i = 0; i < _renderer->generalSettings().diffuseLight().size();
	        ++i) {
		txt.append(QString("%1").arg(_renderer->generalSettings().diffuseLight().at(i)));
		if ( i != _renderer->generalSettings().diffuseLight().size() - 1 )
		    txt.append("; ");
	}
	_dialogUi->lineEditDiffuseLight->setText(txt);
	txt = "";
	for (int i = 0; i < _renderer->generalSettings().specularLight().size();
	        ++i) {
		txt.append(QString("%1").arg(_renderer->generalSettings().specularLight().at(i)));
		if ( i != _renderer->generalSettings().specularLight().size() - 1 )
		    txt.append("; ");
	}
	_dialogUi->lineEditSpecularLight->setText(txt);
	txt = "";
	for (int i = 0; i < _renderer->generalSettings().positionLight().size();
	        ++i) {
		txt.append(QString("%1").arg(_renderer->generalSettings().positionLight().at(i)));
		if ( i != _renderer->generalSettings().positionLight().size() - 1 )
		    txt.append("; ");
	}
	_dialogUi->lineEditPositionLight->setText(txt);

	_dialogUi->labelFilepath->setText(_renderer->activeSettings().filepath());
	_dialogUi->doubleSpinBox_convEleMaxAbove->setValue(_renderer->activeSettings().elevation().convRangeMaxAboveZero);
	_dialogUi->doubleSpinBox_convEleMinAbove->setValue(_renderer->activeSettings().elevation().convRangeMinAboveZero);
	_dialogUi->doubleSpinBox_convEleMaxBelow->setValue(_renderer->activeSettings().elevation().convRangeMaxBelowZero);
	_dialogUi->doubleSpinBox_convEleMinBelow->setValue(_renderer->activeSettings().elevation().convRangeMinBelowZero);
	_dialogUi->doubleSpinBox_eleMax->setValue(_renderer->activeSettings().elevation().max);
	_dialogUi->doubleSpinBox_eleMin->setValue(_renderer->activeSettings().elevation().min);
	_dialogUi->doubleSpinBox_convLatMax->setValue(_renderer->activeSettings().latitude().convRangeMax);
	_dialogUi->doubleSpinBox_convLatMin->setValue(_renderer->activeSettings().latitude().convRangeMin);
	_dialogUi->doubleSpinBox_convLonMax->setValue(_renderer->activeSettings().longitude().convRangeMax);
	_dialogUi->doubleSpinBox_convLonMin->setValue(_renderer->activeSettings().longitude().convRangeMin);
	_dialogUi->doubleSpinBox_latMax->setValue(_renderer->activeSettings().latitude().max);
	_dialogUi->doubleSpinBox_latMin->setValue(_renderer->activeSettings().latitude().min);
	_dialogUi->doubleSpinBox_lonMax->setValue(_renderer->activeSettings().longitude().max);
	_dialogUi->doubleSpinBox_lonMin->setValue(_renderer->activeSettings().longitude().min);
	_dialogUi->checkBoxLatGraticuleLabels->setChecked(_renderer->activeSettings().latitudeGraticule().enabled);
	_dialogUi->checkBoxLonGraticuleLabels->setChecked(_renderer->activeSettings().longitudeGraticule().enabled);
	_dialogUi->checkBoxEleGraticuleLabels->setChecked(_renderer->activeSettings().elevationGraticule().enabled);

	_dialogUi->comboBoxLatGraticulePosition->setCurrentIndex(
	    static_cast<int>(_renderer->activeSettings().latitudeGraticule().position));
	_dialogUi->comboBoxLonGraticulePosition->setCurrentIndex(
	    static_cast<int>(_renderer->activeSettings().longitudeGraticule().position));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyMap::showStatusMessage(const QString& msg, const int& timeout) {
	_ui->statusBar->showMessage(msg, timeout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace OpenGL
} // namespace Gui
} // namespace IPGP
