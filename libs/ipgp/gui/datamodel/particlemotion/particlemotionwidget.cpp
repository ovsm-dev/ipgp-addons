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

#define SEISCOMP_COMPONENT IPGP_PARTICLEMOTIONWIDGET

#include <ipgp/gui/datamodel/particlemotion/particlemotionwidget.h>
#include <ipgp/gui/datamodel/particlemotion/ui_particlemotionwidget.h>
#include <ipgp/gui/datamodel/particlemotion/ui_particlemotionsettings.h>
#include <ipgp/gui/datamodel/originrecordviewer/originrecordviewer.h>
#include <ipgp/gui/datamodel/frequencyviewer.h>
#include <ipgp/gui/datamodel/colormapviewer.h>
#include <ipgp/gui/opengl/widgets/particlemotionglwidget.h>
#include <ipgp/gui/datamodel/streamdelegate.h>
#include <ipgp/core/string/string.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/math/math.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <ipgp/gui/3rd-party/qprogressindicator/qprogressindicator.h>
#include <ipgp/gui/datamodel/qcustomitems.hpp>
#include <ipgp/gui/datamodel/logdialog.h>
#include <ipgp/gui/opengl/graphicrenderer.h>
#include <ipgp/core/math/math.h>

#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/fft.h>

#include <QtGui>
#include <QtConcurrentRun>

#include <fstream>
#include <iomanip>

#define timespanCoefficient 4.
#define amplitudeCoefficient 120.


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::String;
using namespace IPGP::Gui::Math;
using namespace IPGP::Gui::OpenGL;



namespace IPGP {
namespace Gui {


const QString appName = "ParticleMotion";



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
KeyboardFilter::KeyboardFilter(ParticleMotionWidget* pm, QObject* parent) :
		QObject(parent), _pm(pm) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool KeyboardFilter::eventFilter(QObject* dist, QEvent* event) {

	if ( event->type() == QEvent::KeyPress ) {

		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		QSet<int> keys;
		keys += keyEvent->key();

		if ( keyEvent->modifiers() & Qt::ShiftModifier ) {

			if ( keyEvent->modifiers() & Qt::ControlModifier ) {

				//! Shift+Ctrl+S instantiate the picker in Start pick mode
				if ( keys.contains(Qt::Key_S) ) {
					_pm->_startWavePick->setChecked(true);
					_pm->addStartPick();
				}

				//! Shift+Ctrl+E instantiate the picker in End pick mode
				if ( keys.contains(Qt::Key_E) ) {
					_pm->_endWavePick->setChecked(true);
					_pm->addEndPick();
				}
			}

			//! Shift+Right moves the streams to the left
			if ( keys.contains(Qt::Key_Right) )
			    _pm->_orv->moveVisibleRegion(1., RecordViewer::Right);

			//! Shift+Left moves the streams to the right
			if ( keys.contains(Qt::Key_Left) )
			    _pm->_orv->moveVisibleRegion(1., RecordViewer::Left);

			//! Shift+Enter validates the pick at the marker position
			if ( keys.contains(Qt::Key_Return) && _pm->_orv->picker()->pickerIsActivated() ) {
				_pm->_orv->picker()->deactivatePickingMode();
				_pm->_startWavePick->setChecked(false);
				_pm->_endWavePick->setChecked(false);
				SEISCOMP_DEBUG("Pick %s should be added at 'value'", _pm->_currentPickPicking.toStdString().c_str());
			}

			//! > decreases the stream visible timespan
			if ( keys.contains(Qt::Key_Greater) )
			    _pm->_orv->decreaseVisibleTimespan();
		}
		else {

			//! Escape deactivates the picker
			if ( keys.contains(Qt::Key_Escape) && _pm->_orv->picker()->pickerIsActivated() ) {
				_pm->_orv->picker()->deactivatePickingMode();
				_pm->_startWavePick->setChecked(false);
				_pm->_endWavePick->setChecked(false);
			}

			//! < increases the stream visible timespan
			if ( keys.contains(Qt::Key_Less) )
			    _pm->_orv->increaseVisibleTimespan();
		}

		if ( keyEvent->key() == Qt::Key_Greater && keyEvent->modifiers() == Qt::ShiftModifier
		        && keyEvent->modifiers() == Qt::AltModifier )
		    _pm->_orv->decreaseVisibleTimespan(.5);

		if ( keyEvent->key() == Qt::Key_Less && keyEvent->modifiers() == Qt::AltModifier )
		    _pm->_orv->increaseVisibleTimespan(.5);

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionWidget::Settings::Settings() {
	messagesDurationMS = 5000;
	acquisitionTimeout = 0;
	du = DU_KM;
	loadAfterInit = false;
	paintWaveRegionBox = false;
	paintGapsInStreams = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionWidget::WaveRegion::WaveRegion() {
	hasStartTimeSet = false;
	hasEndTimeSet = false;
	startTime = -1;
	endTime = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::WaveRegion::reset() {
	hasStartTimeSet = false;
	hasEndTimeSet = false;
	startTime = -1.;
	endTime = -1.;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionWidget::RecordItem::RecordItem() :
		time(.0), zValue(.0), nsValue(.0), ewValue(.0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionWidget::RecordItem::RecordItem(const qreal& t, const qreal& z,
                                             const qreal& ns, const qreal& ew) :
		time(t), zValue(z), nsValue(ns), ewValue(ew) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionWidget::
ParticleMotionWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		QMainWindow(parent, f), _ui(new Ui::ParticleMotionWidget),
		_state(FlagSet<Client::State>(Client::AppInit)),
		_delegateState(FlagSet<Client::State>(Client::AppInit)), _query(query),
		_filter(NULL), _origin(NULL), _pPick(NULL), _sPick(NULL), _delegate(NULL),
		_stopWatch(NULL) {

	_ui->setupUi(this);

	KeyboardFilter* filter = new KeyboardFilter(this, this);
	this->installEventFilter(filter);

	qRegisterMetaType<StationStreamStatus>("QVariant<StationStreamStatus>");
	qRegisterMetaTypeStreamOperators<int>("StationStreamStatus");

	_pi = new QProgressIndicator;
	_ui->statusBar->addPermanentWidget(_pi, 1);
	_pi->hide();

	_acquisitionTimer = NULL;
	_hideUnpickedStations = NULL;
	_unpickedStationsDistance = NULL;
	_addUnpickedStations = NULL;

	_waveTraceColor = QColor(128, 128, 128);

	_logDialog = new LogDialog(this, appName);
	_logDialog->hide();

	_pznsPlot = new QCustomPlot(this);
	_pznsPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_pznsPlot->setBackground(Qt::transparent);
	_pznsPlot->axisRect()->setBackground(Qt::white);
	_pznsPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_pznsPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_pznsPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
	_pznsPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
	_pznsPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	_pznsPlot->xAxis->setLabel("NS [South(-) North(+)]");
	_pznsPlot->yAxis->setLabel("Z [Down(-) Up(+)]");
	_pznsPlot->axisRect()->setupFullAxesBox();
	connect(_pznsPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), _pznsPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(_pznsPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), _pznsPlot->yAxis2, SLOT(setRange(QCPRange)));

	_pewnsPlot = new QCustomPlot(this);
	_pewnsPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_pewnsPlot->setBackground(Qt::transparent);
	_pewnsPlot->axisRect()->setBackground(Qt::white);
	_pewnsPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_pewnsPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_pewnsPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
	_pewnsPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
	_pewnsPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	_pewnsPlot->xAxis->setLabel("EW [West(-) East(+)]");
	_pewnsPlot->yAxis->setLabel("NS [South(-) North(+)]");
	_pewnsPlot->axisRect()->setupFullAxesBox();
	_pewnsPlot->xAxis->setScaleRatio(_pewnsPlot->yAxis, 1.);
	connect(_pewnsPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), _pewnsPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(_pewnsPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), _pewnsPlot->yAxis2, SLOT(setRange(QCPRange)));

	QBoxLayout* lpz = new QVBoxLayout(_ui->framePzns);
	QBoxLayout* lpewns = new QVBoxLayout(_ui->framePewns);

	_ui->framePzns->setLayout(lpz);
	_ui->framePewns->setLayout(lpewns);

	lpz->addWidget(_pznsPlot);
	lpewns->addWidget(_pewnsPlot);
	lpz->setMargin(0);
	lpewns->setMargin(0);

	//! Setup the frequency viewers
	_freqZ = new FrequencyViewerPlot(FreqAmpCounts, this);
	_freqNS = new FrequencyViewerPlot(FreqAmpCounts, this);
	_freqEW = new FrequencyViewerPlot(FreqAmpCounts, this);

	QBoxLayout* lfreqz = new QVBoxLayout(_ui->frameZFreq);
	_ui->frameZFreq->setLayout(lfreqz);
	lfreqz->addWidget(_freqZ);
	lfreqz->setMargin(0);

	QBoxLayout* lfreqns = new QVBoxLayout(_ui->frameNSFreq);
	_ui->frameNSFreq->setLayout(lfreqns);
	lfreqns->addWidget(_freqNS);
	lfreqns->setMargin(0);

	QBoxLayout* lfreqew = new QVBoxLayout(_ui->frameEWFreq);
	_ui->frameEWFreq->setLayout(lfreqew);
	lfreqew->addWidget(_freqEW);
	lfreqew->setMargin(0);

	//! Setup the spectrogram viewer
//	_zSpectro = new ColorMapViewerPlot(Spectrogram, this);

//	QBoxLayout* lzspectro = new QVBoxLayout(_ui->frameZSpectrogram);
//	_ui->frameZSpectrogram->setLayout(lzspectro);
//	lzspectro->addWidget(_zSpectro);
//	lzspectro->setMargin(0);
	_ui->tabWidgetStationData->removeTab(2);
//	_ui->spectrumsTab->setVisible(false);

	//! Setup the OriginRecordViewer
	_orv = new OriginRecordViewer(_query, &_cache, OriginRecordViewer::Raw, this);
	_orv->installEventFilter(filter);
	connect(_orv, SIGNAL(idling()), this, SIGNAL(idling()));
	connect(_orv, SIGNAL(working()), this, SIGNAL(working()));
	connect(this, SIGNAL(refreshStreams()), _orv, SLOT(drawRecordTraces()));
	connect(_orv->picker(), SIGNAL(newPick(const qreal&, const QString&)),
	    this, SLOT(addNewPick( const qreal&, const QString&)));

	QBoxLayout* lorv = new QVBoxLayout(_ui->frameStream);
	_ui->frameStream->setLayout(lorv);
	lorv->addWidget(_orv);
	lorv->setMargin(0);

	_pmgl = new ParticleMotionGLWidget(this);
	QBoxLayout* l3d = new QVBoxLayout(_ui->frame3dplot);
	_ui->frame3dplot->setLayout(l3d);
	l3d->addWidget(_pmgl);
	l3d->setMargin(0);

	connect(_ui->tableWidget_stations, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(stationSelected(QTableWidgetItem*)));
	connect(_ui->comboBox_component, SIGNAL(currentIndexChanged(int)), this, SLOT(componentChannelChanged(int)));
	connect(_ui->checkBox_useSminusP, SIGNAL(clicked(bool)), this, SLOT(useSMinusPTriggered(bool)));
	connect(_ui->actionClose, SIGNAL(triggered()), this, SLOT(close()));
	connect(_ui->actionShowLog, SIGNAL(triggered()), _logDialog, SLOT(show()));
	connect(_ui->actionSaveData, SIGNAL(triggered()), this, SLOT(saveCollectedData()));

	_recordStreamUrl = "slink://localhost:18000";
	_recordInputHint = Record::DATA_ONLY;

	_timeWindowSec = 360.;

	_ui->tableWidget_stations->setColumnCount(7);
	_ui->tableWidget_stations->setHorizontalHeaderLabels(
	    QStringList() << "Status" << "Network" << "Station" << "Azimuth"
	                  << "Distance" << "P time" << "S time");
	_ui->tableWidget_stations->verticalHeader()->setVisible(false);
	_ui->tableWidget_stations->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	_ui->tableWidget_stations->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->tableWidget_stations->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidget_stations->setSelectionMode(QAbstractItemView::SingleSelection);
	_ui->tableWidget_stations->resizeColumnsToContents();
	_ui->tableWidget_stations->resizeRowsToContents();
	_ui->tableWidget_stations->setSelectionMode(QTreeView::ExtendedSelection);
	_ui->tableWidget_stations->setSelectionBehavior(QTreeView::SelectRows);
	_ui->tableWidget_stations->setAlternatingRowColors(true);

	_ui->tableWidget_data->setColumnCount(4);
	_ui->tableWidget_data->setHorizontalHeaderLabels(
	    QStringList() << "Time(OT)" << "Z" << "North/South" << "East/West");
	_ui->tableWidget_data->verticalHeader()->setVisible(false);
	_ui->tableWidget_data->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	_ui->tableWidget_data->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->tableWidget_data->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidget_data->setSelectionMode(QAbstractItemView::SingleSelection);
	_ui->tableWidget_data->resizeColumnsToContents();
	_ui->tableWidget_data->resizeRowsToContents();
	_ui->tableWidget_data->setSelectionMode(QTreeView::ExtendedSelection);
	_ui->tableWidget_data->setSelectionBehavior(QTreeView::SelectRows);
	_ui->tableWidget_data->setAlternatingRowColors(true);

	QSettings settings("IPGP", appName);
	settings.beginGroup(appName);
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();

	setWindowTitle("Particle Motion");
	setWindowIcon(QIcon(":images/logo.png"));

	_ui->mainToolBar->setIconSize(QSize(24, 24));

	_autoReplot = new QAction("Update graphics", this);
	_autoReplot->setIcon(QIcon(":images/view-refresh.png"));
	_autoReplot->setToolTip("Update graphics whenever a change happens");
	_autoReplot->connect(_autoReplot, SIGNAL(triggered()), this, SLOT(addStartPick()));
	_autoReplot->setCheckable(true);
	_autoReplot->setChecked(false);

	_autoRescaleKeyAxes = new QAction(this);
	_autoRescaleKeyAxes->setToolTip("Rescale streams to fit window width");
	_autoRescaleKeyAxes->setIcon(QIcon(":images/two-ways-left-right.png"));
	_autoRescaleKeyAxes->setCheckable(true);
	_autoRescaleKeyAxes->setChecked(false);

	_autoRescaleValueAxes = new QAction(this);
	_autoRescaleValueAxes->setIcon(QIcon(":images/two-ways-up-down.png"));
	_autoRescaleValueAxes->setToolTip("Rescale streams values to fit window height");
	_autoRescaleValueAxes->setCheckable(true);
	_autoRescaleValueAxes->setChecked(true);

	_startWavePick = new QAction(this);
	_startWavePick->setToolTip("Pick wave start");
	_startWavePick->setIcon(QIcon(QPixmap(":images/filled-flag2.png")));
	_startWavePick->connect(_startWavePick, SIGNAL(triggered()), this, SLOT(addStartPick()));
	_startWavePick->setCheckable(true);
	_startWavePick->setChecked(false);

	_endWavePick = new QAction(this);
	_endWavePick->setToolTip("Pick wave end");
	_endWavePick->setIcon(QIcon(QPixmap(":images/filled-flag.png")));
	_endWavePick->connect(_endWavePick, SIGNAL(triggered()), this, SLOT(addEndPick()));
	_endWavePick->setCheckable(true);
	_endWavePick->setChecked(false);


	_filterBox = new QComboBox(this);
	_filterBox->setToolTip("Select data filtering method");
	connect(_filterBox, SIGNAL(currentIndexChanged(const QString&)),
	    this, SLOT(filterChanged(const QString&)));

	_applyButton = new QAction(this);
	_applyButton->setIcon(QIcon(":images/apply.png"));
	_applyButton->setToolTip("Click here to apply changes");
	_applyButton->connect(_applyButton, SIGNAL(triggered(bool)), this, SLOT(prepareData(const bool&)));

	_ui->mainToolBar->addAction(_autoReplot);
	_ui->mainToolBar->addAction(_autoRescaleKeyAxes);
	_ui->mainToolBar->addAction(_autoRescaleValueAxes);

	QToolBar* t1 = this->addToolBar("Picker");
	t1->setObjectName("PickerToolBar");
	t1->addAction(_startWavePick);
	t1->addAction(_endWavePick);

	QToolBar* t3 = this->addToolBar("Filtering");
	t3->setObjectName("FilteringToolBar");
	t3->addWidget(_filterBox);

	QToolBar* t4 = this->addToolBar("Validation");
	t4->setObjectName("ValidationToolBar");
	t4->addAction(_applyButton);

	_ui->actionSaveData->setIcon(QIcon(":images/document-save.png"));

	_state.remove(Client::AppInit);
	_state.set(Client::AppIdling);

	_configDialogUi = new Ui::ParticleMotionSettings;
	_configDialog = new QDialog(this);
	_configDialog->setObjectName("ParticleMotionConfigureDialog");
	_configDialog->setModal(true);
	_configDialogUi->setupUi(_configDialog);
	_configDialog->setWindowTitle("Settings");
	_configDialog->hide();

	_configDialogUi->checkBox_loadAfterInit->setChecked(_settings.loadAfterInit);
	_configDialogUi->checkBox_showGaps->setChecked(_settings.paintGapsInStreams);
	_configDialogUi->checkBox_showWaveRegion->setChecked(_settings.paintWaveRegionBox);
	_configDialogUi->spinBox_decimals->setValue(_scheme.generalPrecision());
	connect(_ui->actionConfigureParticleMotion, SIGNAL(triggered()), _configDialog, SLOT(show()));
	connect(_configDialogUi->buttonBox, SIGNAL(accepted()), this, SLOT(settingsValidated()));
	connect(_configDialogUi->buttonBox, SIGNAL(rejected()), _configDialog, SLOT(reject()));

	_configDialogUi->spinBox_decimals->setEnabled(false);
	_configDialogUi->spinBox_decimals->setToolTip("Disabled: use scheme.precision.general instead");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionWidget::~ParticleMotionWidget() {

	//! NOTE: No need to clear the cache here, its dtor will do it for us

	handleDelegateState();

	closeStream();

	if ( _stopWatch ) delete _stopWatch;
	_stopWatch = NULL;

	if ( _logDialog ) delete _logDialog;
	_logDialog = NULL;

	if ( _pi ) delete _pi;
	_pi = NULL;

	if ( _ui ) delete _ui;
	_ui = NULL;

	SEISCOMP_DEBUG("ParticleMotionWidget is terminated");
	emit nullifyQObject(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::closeEvent(QCloseEvent* event) {

	QSettings settings("IPGP", appName);
	settings.beginGroup(appName);
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.endGroup();

	QMainWindow::closeEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::keyPressEvent(QKeyEvent* event) {

	qDebug() << "New Event: " << event->text();

	/*
	 // Leave picking mode
	 if ( event->key() == Qt::Key_Escape && _orv->picker()->pickerIsActivated() ) {
	 _orv->picker()->deactivatePickingMode();
	 _startWavePick->setChecked(false);
	 _endWavePick->setChecked(false);
	 }

	 if ( event->modifiers() & Qt::ShiftModifier ) {

	 QSet<int> keys;
	 keys += event->key();

	 //		if ( event->key() == Qt::Key_S ) {
	 if ( keys.contains(Qt::Key_S) ) {
	 _startWavePick->setChecked(true);
	 addStartPick();
	 }

	 //		if ( event->key() == Qt::Key_E ) {
	 if ( keys.contains(Qt::Key_E) ) {
	 _endWavePick->setChecked(true);
	 addEndPick();
	 }
	 }
	 */


	/*
	 // Validate pick at picker position (time)
	 if ( event->key() == Qt::Key_Enter && _orv->picker()->pickerIsActivated() ) {
	 _orv->picker()->deactivatePickingMode();
	 _startWavePick->setChecked(false);
	 _endWavePick->setChecked(false);
	 SEISCOMP_DEBUG("Pick %s should be added at 'value'", _currentPickPicking.toStdString().c_str());
	 }


	 // Manage streams visible timespan
	 if ( event->key() == Qt::Key_Greater && event->modifiers() == Qt::ShiftModifier )
	 _orv->decreaseVisibleTimespan();

	 if ( event->key() == Qt::Key_Less )
	 _orv->increaseVisibleTimespan();

	 if ( event->key() == Qt::Key_Greater && event->modifiers() == Qt::ShiftModifier
	 && event->modifiers() == Qt::AltModifier )
	 _orv->decreaseVisibleTimespan(.5);

	 if ( event->key() == Qt::Key_Less && event->modifiers() == Qt::AltModifier )
	 _orv->increaseVisibleTimespan(.5);

	 if ( event->key() == Qt::Key_Left && event->modifiers() == Qt::ShiftModifier )
	 _orv->moveVisibleRegion(1., RecordViewer::Left);

	 if ( event->key() == Qt::Key_Right && event->modifiers() == Qt::ShiftModifier )
	 _orv->moveVisibleRegion(1., RecordViewer::Right);
	 */

	QWidget::keyPressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::log(Client::LogMessage lm, const QString& str) {
	_logDialog->addMessage(lm, appName, str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::settingsValidated() {

//	_settings.displayedDecimals = _configDialogUi->spinBox_decimals->value();
	_settings.du = static_cast<Settings::DistanceUnit>(_configDialogUi->comboBox_unit->currentIndex());
	_settings.loadAfterInit = _configDialogUi->checkBox_loadAfterInit->isChecked();
	_settings.paintGapsInStreams = _configDialogUi->checkBox_showGaps->isChecked();
	_settings.paintWaveRegionBox = _configDialogUi->checkBox_showWaveRegion->isChecked();
//	_orgStream->setStreamNormalizationMethod(static_cast<OriginStreamWidget::NormalizeStreamMethod>(
//	    _configDialogUi->comboBox_streamAmplitude->currentIndex()));
	_orv->setNormalizeAmplitudePolicy(static_cast<OriginRecordViewer::NormalizeAmplitudePolicy>
	    (_configDialogUi->comboBox_streamAmplitude->currentIndex()));

	_configDialog->hide();

	log(Client::LM_INFO, "new settings changes applied");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::addStartPick() {

	_currentPickPicking = "Start";

	if ( _endWavePick->isChecked() )
	    _endWavePick->setChecked(false);

	if ( _startWavePick->isChecked() )
		_orv->picker()->activatePickingMode("Start");
	else {
		_startWavePick->setChecked(false);
		_orv->picker()->deactivatePickingMode();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::addEndPick() {

	_currentPickPicking = "End";

	if ( _startWavePick->isChecked() )
	    _startWavePick->setChecked(false);

	if ( _endWavePick->isChecked() )
		_orv->picker()->activatePickingMode("End");
	else {
		_endWavePick->setChecked(false);
		_orv->picker()->deactivatePickingMode();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::addNewPick(const qreal& time,
                                      const QString& name) {

	if ( name.isEmpty() ) return;

	_orv->saveCurrentPicks();

	if ( name == "Start" ) {
		_startWavePick->setChecked(false);
		_wr.hasStartTimeSet = true;
		_wr.startTime = time;
		_ui->label_wstart->setText(QDateTime::fromMSecsSinceEpoch(_wr.startTime * 1000)
		        .toString("yyyy-MM-dd hh:mm:ss"));
	}
	else if ( name == "End" ) {
		_endWavePick->setChecked(false);
		_wr.hasEndTimeSet = true;
		_wr.endTime = time;
		_ui->label_wend->setText(QDateTime::fromMSecsSinceEpoch(_wr.endTime * 1000)
		        .toString("yyyy-MM-dd hh:mm:ss"));
	}

	if ( _orv->hasPick("Start") && _orv->hasPick("End") ) {

		_ui->checkBox_useSminusP->setChecked(false);

		if ( _autoReplot->isChecked() )
		    prepareData(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::initFilters(const QMap<QString, QString>& filters) {

	_filterBox->clear();
	_filterBox->insertItem(0, "No filter");

	_filters.clear();

	QString activeFilter;
	int i = 1, idx = -1;
	for (QMap<QString, QString>::const_iterator it = filters.begin();
	        it != filters.end(); ++it) {

		if ( it.key().left(1) == "@" ) {
			idx = i;
			activeFilter = it.key().mid(1, it.key().size());
			_filters.insert(it.key().mid(1, it.key().size()), it.value());
			_filterBox->insertItem(i, it.key().mid(1, it.key().size()));
		}
		else {
			_filters.insert(it.key(), it.value());
			_filterBox->insertItem(i, it.key());
		}
		++i;
	}

	if ( !activeFilter.isEmpty() ) {
		_filterBox->setCurrentIndex(idx);
		filterChanged(activeFilter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::filterChanged(const QString& filter) {

	if ( filter == "No filter" ) {
		_orv->setStreamFilter(NULL);

		if ( _filter )
		    delete _filter;
		_filter = NULL;
	}
	else {

		QString filterName;
		for (QMap<QString, QString>::iterator it = _filters.begin();
		        it != _filters.end(); ++it) {
			if ( it.key() == filter )
			    filterName = it.value();
		}

		if ( filterName.isEmpty() ) return;

		std::string error;
		Seiscomp::Math::Filtering::InPlaceFilter<double>* f = NULL;
		f = Seiscomp::Math::Filtering::InPlaceFilter<double>::Create(filterName.toStdString(), &error);

		if ( f ) {
			_filter = f;
			_orv->setStreamFilter(f->clone());
		}
		else
			QMessageBox::warning(this, tr("Seiscomp filter exception"), error.c_str());
	}

	//! TODO
	//! Sometimes when the filter is changed, streams are not properly resized,
	//! and they do not fit the stream windows correctly, see if there is
	//! something to do here to make sure that this behavior stays under control.
	emit refreshStreams();

	if ( _autoReplot->isChecked() )
	    prepareData(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::stationStreamsReady(const QString& code) {

	QStringList l = code.split(".");
	if ( l.size() == 0 ) return;

	if ( !_fetchedStationsStatus.contains(code) ) return;

	if ( _fetchedStationsStatus[code] == sssWaitingForData ) {

		_fetchedStationsStatus[code] = sssWaitingForQCCheck;

		for (int i = 0; i < _ui->tableWidget_stations->rowCount(); ++i) {
			// Assert on items...
			try {
				if ( _ui->tableWidget_stations->item(i, 1)->text() == l.at(0)
				        && _ui->tableWidget_stations->item(i, 2)->text() == l.at(1) ) {
					QVariant data;
					data.setValue(sssWaitingForQCCheck);
					_ui->tableWidget_stations->item(i, 0)->setData(Qt::UserRole, data);
					_ui->tableWidget_stations->item(i, 0)->setText("Waiting for QC check");
					_ui->tableWidget_stations->item(i, 0)->setForeground(Qt::black);
					_ui->tableWidget_stations->item(i, 1)->setForeground(Qt::black);
					_ui->tableWidget_stations->item(i, 2)->setForeground(Qt::black);
					_ui->tableWidget_stations->item(i, 3)->setForeground(Qt::black);
					_ui->tableWidget_stations->item(i, 4)->setForeground(Qt::black);
					_ui->tableWidget_stations->item(i, 5)->setForeground(Qt::black);
					_ui->tableWidget_stations->item(i, 6)->setForeground(Qt::black);
				}
			} catch ( ... ) {}
		}

		//! At this point, all records for this station have been fetched,
		//! We make it available so the user can interact with it, but,
		//! the gap count may not be accurate due to sequential r/w being
		//! performed on the cache as long as the delegate is active.
		//! Therefore a second gap check will be performed to update this
		//! number when the delegate sends off its end records stream signal.
		checkStreamsGaps(code, true);


//		QFuture<void> future = QtConcurrent::run(this, &ParticleMotionWidget::checkStreamsGaps, code);
//		QFutureWatcher<void> watcher;
//		watcher.setFuture(future);

		//	connect(_watcher, SIGNAL(finished()), this, SIGNAL(plottingFinished()));
		//	connect(this, SIGNAL(requestGapsCheck(const QString&)), this, SLOT(checkStreamsGaps(const QString&)));
//		*_future = QtConcurrent::run(this, &ParticleMotionWidget::checkStreamsGaps, code);
//		_watcher->setFuture(*_future);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::checkStreamsGaps(const QString& code,
                                            const bool& isFirstCheck) {

	QStringList sta = code.split(".");
	if ( sta.size() != 2 )
	    return;

	size_t gapCount = 0;

	for (int i = 0; i < _activeStreams.size(); ++i) {

		if ( _activeStreams.at(i).networkCode != sta.at(0)
		        or _activeStreams.at(i).stationCode != sta.at(1) )
		    continue;

		QString s = QString("%1.%2.%3.%4").arg(_activeStreams.at(i).networkCode)
		        .arg(_activeStreams.at(i).stationCode)
		        .arg(_activeStreams.at(i).locationCode)
		        .arg(_activeStreams.at(i).channelCode);

		std::vector<RecordPtr> recV = _cache.getRecords(s.toStdString());
		TimeWindowBuffer* twb = new TimeWindowBuffer(_tw);

		for (size_t v = 0; v < recV.size(); ++v) {

			RecordPtr rec = recV.at(v);
			if ( !rec ) continue;

			twb->feed(Record::ConstCast(rec));
		}

		if ( twb->gaps().size() > 0 ) {
			gapCount += twb->gaps().size();
			log(Client::LM_WARNING, QString("Gap(s) found in %1 records sequence: %2")
			        .arg(s).arg(twb->gaps().size()));

			for (size_t g = 0; g < twb->gaps().size(); ++g) {
				StreamGap sg;
				sg.streamID = s;
				sg.start = twb->gaps().at(g).startTime();
				sg.start = twb->gaps().at(g).endTime();
				_streamsGaps.push_back(sg);
			}
		}

		delete twb;
		twb = NULL;
	}

	for (int i = 0; i < _ui->tableWidget_stations->rowCount(); ++i) {
		if ( _ui->tableWidget_stations->item(i, 1)->text() == sta.at(0)
		        && _ui->tableWidget_stations->item(i, 2)->text() == sta.at(1) ) {
			if ( gapCount > 0 ) {
				QVariant data;
				data.setValue(sssMissingChunks);
				_ui->tableWidget_stations->item(i, 0)->setData(Qt::UserRole, data);
				(isFirstCheck) ?
				        _ui->tableWidget_stations->item(i, 0)->setText(QString("Pre-checked")) :
				        _ui->tableWidget_stations->item(i, 0)->setText(QString("%1 gap(s)").arg(gapCount));
				_ui->tableWidget_stations->item(i, 0)->setBackground(QColor(237, 152, 14, 50));
				_ui->tableWidget_stations->item(i, 0)->setToolTip(
				    QString("%1 gap(s) found in data stream.\n"
					    "Make sure the selected region doesn't contain any gap "
					    "in order for the sample to be properly collected.")
				            .arg(gapCount));
			}
			else {
				QVariant data;
				data.setValue(sssOK);
				_ui->tableWidget_stations->item(i, 0)->setData(Qt::UserRole, data);
				(isFirstCheck) ?
				        _ui->tableWidget_stations->item(i, 0)->setText("Pre-checked") :
				        _ui->tableWidget_stations->item(i, 0)->setText("OK");
				_ui->tableWidget_stations->item(i, 0)->setBackground(QColor(100, 230, 110, 50));
				_ui->tableWidget_stations->item(i, 0)->setToolTip("QC check is OK.\nNo gaps were found.");
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::handleMissingData(const QString& code) {

	QStringList sta = code.split(".");
	if ( sta.size() != 2 ) return;

	for (int i = 0; i < _ui->tableWidget_stations->rowCount(); ++i) {
		if ( _ui->tableWidget_stations->item(i, 1)->text() == sta.at(0)
		        && _ui->tableWidget_stations->item(i, 2)->text() == sta.at(1) ) {

			QVariant data;
			data.setValue(sssMissingData);
			_ui->tableWidget_stations->item(i, 0)->setData(Qt::UserRole, data);
			_ui->tableWidget_stations->item(i, 0)->setText("Missing data");
			_ui->tableWidget_stations->item(i, 0)->setBackground(Misc::noStreamDataColor);
			_ui->tableWidget_stations->item(i, 0)->setToolTip(
			    QString("%1 data have not been retrieved").arg(code));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::setOrigin(const std::string& publicID) {

	if ( !_query ) {
		SEISCOMP_ERROR("No query interface available");
		return;
	}

	handleDelegateState();

	if ( _origin && _origin->publicID() == publicID ) {
		log(Client::LM_INFO, QString("Ignoring command, object %1 is already in use")
		        .arg(publicID.c_str()));
		return;
	}

	log(Client::LM_OK, QString("GUI loading origin with publicID %1").arg(publicID.c_str()));

	_cache.clear();

	_ui->tableWidget_stations->clearSelection();
	_ui->tableWidget_stations->setRowCount(0);

//	_ui->tableWidget->clearSelection();
//	_ui->tableWidget->setRowCount(0);

	_ui->tableWidget_data->clearSelection();
	_ui->tableWidget_data->setRowCount(0);

	_pznsPlot->clearItems();
	_pznsPlot->replot();
	_pewnsPlot->clearItems();
	_pewnsPlot->replot();
	_pmgl->clear();

	_ui->label_stationName->setText("-");
	_ui->label_sminusp->setText("-");
	_ui->label_wstart->setText("-");
	_ui->label_wend->setText("-");
	_ui->label_waveDuration->setText("-");
	_ui->label_samplesCount->setText("-");

	_wr.reset();

	_orv->reset();

	_freqZ->reset();
	_freqNS->reset();
	_freqEW->reset();
//	_zSpectro->reset();

	PublicObjectPtr obj = _query->getObject(Origin::TypeInfo(), publicID);
	_origin = Origin::Cast(obj.get());

	if ( !_origin ) {
		SEISCOMP_ERROR("Origin with publicID %s not found in inventory", publicID.c_str());
		log(Client::LM_ERROR, QString("Origin with publicID %1 not found in inventory").arg(publicID.c_str()));
		return;
	}

	showWaitingWidget();


	if ( _recordStream ) {
		if ( _delegate )
		    _delegate->stopReading();

		_recordStream->close();
		_recordStream.reset();
	}

	openStream();

	Time st = _origin->time().value() - TimeSpan(_timeWindowSec / 2);
	Time ed;

	QList<PickPtr> picklist;

	QStringList stations;
	int idx = 0;
	int idx2 = 0;
	for (size_t i = 0; i < _origin->arrivalCount(); ++i) {

		ArrivalPtr ar = _origin->arrival(i);
		PickPtr pick = Pick::Find(ar->pickID());

		if ( !pick ) {
			PublicObjectPtr obj = _query->getObject(Pick::TypeInfo(), ar->pickID());
			pick = Pick::Cast(obj.get());
		}

		if ( !pick ) continue;

		if ( i == 0 ) ed = pick->time().value();

		if ( pick->time().value().seconds() > ed.seconds() )
		    ed = pick->time().value();

		QString itm = QString("%1.%2").arg(pick->waveformID().networkCode().c_str())
		        .arg(pick->waveformID().stationCode().c_str());

		picklist << pick;

		if ( !stations.contains(itm) ) {

			stations.append(itm);

			_fetchedStationsStatus.insert(itm, sssWaitingForData);

			_ui->tableWidget_stations->insertRow(_ui->tableWidget_stations->rowCount());

			_ui->tableWidget_stations->setItem(idx, 0, new QTableWidgetItem("Waiting for data"));
			_ui->tableWidget_stations->setItem(idx, 1, new QTableWidgetItem(
			    pick->waveformID().networkCode().c_str()));
			_ui->tableWidget_stations->setItem(idx, 2, new QTableWidgetItem(
			    pick->waveformID().stationCode().c_str()));
			_ui->tableWidget_stations->setItem(idx, 3, new NumberTableItem(
			    QString::fromUtf8("%1°").arg(QString::number(ar->azimuth(),
			        'f', _scheme.azimuthPrecision()))));

			if ( !_scheme.distanceInKM() )
				_ui->tableWidget_stations->setItem(idx, 4, new NumberTableItem(
				    QString::fromUtf8("%1°").arg(QString::number(ar->distance(),
				        'f', _scheme.distancePrecision()))));
			else
				_ui->tableWidget_stations->setItem(idx, 4, new NumberTableItem(
				    QString("%1 km").arg(QString::number(Seiscomp::Math::Geo::deg2km(ar->distance()),
				        'f', _scheme.distancePrecision()))));
			_ui->tableWidget_stations->setItem(idx, 5, new QTableWidgetItem("-"));
			_ui->tableWidget_stations->setItem(idx, 6, new QTableWidgetItem("-"));

			QVariant data;
			data.setValue(sssWaitingForData);
			_ui->tableWidget_stations->item(idx, 0)->setData(Qt::UserRole, data);

			_ui->tableWidget_stations->item(idx, 0)->setBackgroundColor(Misc::waitingForStreamDataColor);

			_ui->tableWidget_stations->item(idx, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_stations->item(idx, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_stations->item(idx, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_stations->item(idx, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_stations->item(idx, 4)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_stations->item(idx, 5)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_stations->item(idx, 6)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

			_ui->tableWidget_stations->item(idx, 0)->setForeground(Qt::lightGray);
			_ui->tableWidget_stations->item(idx, 1)->setForeground(Qt::lightGray);
			_ui->tableWidget_stations->item(idx, 2)->setForeground(Qt::lightGray);
			_ui->tableWidget_stations->item(idx, 3)->setForeground(Qt::lightGray);
			_ui->tableWidget_stations->item(idx, 4)->setForeground(Qt::lightGray);
			_ui->tableWidget_stations->item(idx, 5)->setForeground(Qt::lightGray);
			_ui->tableWidget_stations->item(idx, 6)->setForeground(Qt::lightGray);

			++idx;
			idx2 += 2;
		}
	}

	// updating picks time
	for (int i = 0; i < picklist.size(); ++i) {
		for (int j = 0; j < _ui->tableWidget_stations->rowCount(); ++j) {

			if ( _ui->tableWidget_stations->item(j, 1)->text().toStdString()
			        == picklist.at(i)->waveformID().networkCode()
			        && _ui->tableWidget_stations->item(j, 2)->text().toStdString()
			                == picklist.at(i)->waveformID().stationCode() ) {
				try {
					QDateTime t = QDateTime::fromMSecsSinceEpoch((double) picklist.at(i)->time().value() * 1000);
					if ( picklist.at(i)->phaseHint().code() == "P" )
						_ui->tableWidget_stations->item(j, 5)->setText(t.toString("hh:mm:ss.zzz"));
					else if ( picklist.at(i)->phaseHint().code() == "S" )
					    _ui->tableWidget_stations->item(j, 6)->setText(t.toString("hh:mm:ss.zzz"));
				} catch ( ... ) {}
			}
		}
	}

	_ui->tableWidget_stations->resizeRowsToContents();

	ed += TimeSpan(_timeWindowSec);

	_tw = Seiscomp::Core::TimeWindow(st, ed);
	setTimeWindow(Seiscomp::Core::TimeWindow(st, ed));
	_orv->timeLine()->setTimeWindow(_tw);
	_activeStreams.clear();

	setWindowTitle(QString("ID: %1, Stations: %2")
	        .arg(_origin->publicID().c_str())
	        .arg(stations.size()));

	for (int i = 0; i < stations.size(); ++i) {

		QStringList it = stations.at(i).split(".");
		PublicObjectPtr obj = _query->getStation(it.at(0).toStdString(),
		    it.at(1).toStdString(), _origin->time().value());

		StationPtr sta = Station::Cast(obj.get());
		if ( !sta ) continue;

		if ( sta->sensorLocationCount() == 0 )
		    _query->loadSensorLocations(sta.get());

		for (size_t l = 0; l < sta->sensorLocationCount(); ++l) {
			SensorLocationPtr sloc = sta->sensorLocation(l);

			try {
				if ( sloc->end() <= _origin->time().value() ) continue;
			} catch ( ... ) {}

			if ( sloc->start() > _origin->time().value() ) continue;

			if ( sloc->streamCount() == 0 )
			    _query->loadStreams(sloc.get());

			for (size_t k = 0; k < sloc->streamCount(); ++k) {
				StreamPtr stream = sloc->stream(k);

				try {
					if ( stream->end() <= _origin->time().value() ) continue;
				} catch ( ... ) {}

				if ( stream->start() > _origin->time().value() ) continue;

				addStream(it.at(0).toStdString(), it.at(1).toStdString(), sloc->code(), stream->code());

				ActiveStream as;
				as.networkCode = it.at(0);
				as.stationCode = it.at(1);
				as.locationCode = sloc->code().c_str();
				as.channelCode = stream->code().c_str();
				as.gainUnit = stream->gainUnit().c_str();
				_activeStreams.append(as);
			}
		}
	}


	//! TODO: try and use QtConcurrent instead of basic QThread. This will let
	//! Qt use its thread pool, and even induce the use of more than one CPU
	//! core, or at least a smarter thread handling unit...
	//! I suppose this will also prevent sporadic segfault happening when the
	//! delegate pointer is destroyed before releasing its obtained lock by the
	//! QThread death.
	QThread* thread = new QThread;

	_stopWatch = new Util::StopWatch;

	_delegate = new StreamDelegate(_recordStream.get(), _recordInputHint, &_cache, StreamDelegate::CachedRecords);
	_delegate->moveToThread(thread);
	_delegate->setStations(stations);

	//! Once a second check for any timeout in acquisition because socket
	//! exceptions are not catchable by interrogating the RecordStream itself,
	//! therefore error signals won't come out of the StreamDelegate...
	_acquisitionTimer = new QTimer(this);
	_acquisitionTimer->setInterval(1000);

	connect(_acquisitionTimer, SIGNAL(timeout()), this, SLOT(checkAcquisitionTimeout()));
	connect(thread, SIGNAL(started()), _acquisitionTimer, SLOT(start()));
	connect(thread, SIGNAL(started()), _delegate, SLOT(readRecords()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(_delegate, SIGNAL(acquisitionTerminated()), thread, SLOT(quit()));
	connect(this, SIGNAL(stopDelegate()), thread, SLOT(quit()));
	connect(_delegate, SIGNAL(acquisitionTerminated()), _delegate, SLOT(deleteLater()));
	connect(_delegate, SIGNAL(acquisitionTerminated()), this, SLOT(delegateIsOver()));
	connect(_delegate, SIGNAL(stationStreamsReady(const QString&)), this, SLOT(stationStreamsReady(const QString&)));
	connect(_delegate, SIGNAL(reportAcquisitionException(const QString&)), this, SLOT(acquisitionExceptionReported(const QString&)));
	connect(_delegate, SIGNAL(reportOperationException(const QString&)), this, SLOT(operationExceptionReported(const QString&)));


	/*
	 _delegate = QSharedPointer<StreamDelegate>(new StreamDelegate(_recordStream.get(), _recordInputHint,
	 &_cache, StreamDelegate::CachedRecords));
	 _delegate->moveToThread(thread);

	 connect(thread, SIGNAL(started()), _delegate.data(), SLOT(readRecords()));
	 connect(_delegate.data(), SIGNAL(acquisitionTerminated()), thread, SLOT(quit()));
	 connect(this, SIGNAL(stopDelegate()), thread, SLOT(quit()));
	 connect(_delegate.data(), SIGNAL(acquisitionTerminated()), _delegate.data(), SLOT(deleteLater()));
	 connect(_delegate.data(), SIGNAL(stationStreamsReady(const QString&)), this, SLOT(stationStreamsReady(const QString&)));
	 connect(_delegate.data(), SIGNAL(acquisitionTerminated()), this, SLOT(delegateIsOver()));
	 */

	_delegateState.set(Client::AppWorking);
	thread->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::checkAcquisitionTimeout() {

	if ( !_delegate || !_stopWatch || !_recordStream )
	    return;

	if ( _settings.acquisitionTimeout > 0 && _delegate->receivedRecords() == 0 )
	    if ( (size_t) _stopWatch->elapsed().seconds() >= _settings.acquisitionTimeout ) {
		    log(Client::LM_ERROR, QString("Record acquisition process from %1 "
			    "has exceeded the timeout period of %2 sec")
		            .arg(_recordStreamUrl.c_str()).arg(_settings.acquisitionTimeout));
		    _delegate->stopReading();
		    _recordStream->close();
	    }
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::handleDelegateState() {

	if ( _delegateState.isSet(Client::AppWorking) ) {

		hideWaitingWidget();

		if ( _delegate ) {
			_delegate->stopReading();
			_delegate->disconnect();

			SEISCOMP_DEBUG("Waiting for delegate thread to stop");
			if ( !_delegate->lock().tryLock() ) {
				SEISCOMP_DEBUG("Releasing plotting lock obtained from delegate thread");
				_delegate->lock().unlock();
			}


			if ( !_delegate->lock().tryLock() ) {
				SEISCOMP_DEBUG("Releasing plotting lock (2) obtained from delegate thread");
				_delegate->lock().unlock();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::delegateIsOver() {

	_delegateState.remove(Client::AppWorking);
	_delegateState.set(Client::AppIdling);

	hideWaitingWidget();
	closeStream();

	QMap<QString, StationStreamStatus>::iterator it = _fetchedStationsStatus.begin();
	for (; it != _fetchedStationsStatus.end(); ++it) {
		if ( it.value() == sssWaitingForQCCheck )
			checkStreamsGaps(it.key());
		else if ( it.value() == sssWaitingForData )
		    handleMissingData(it.key());
	}

	if ( _settings.loadAfterInit && _ui->tableWidget_stations->item(0, 0) ) {
		_ui->tableWidget_stations->setCurrentItem(_ui->tableWidget_stations->item(0, 0));
		stationSelected(_ui->tableWidget_stations->item(0, 0));
	}

//	log(Client::LM_INFO, QString("%1 record objects stored, cache size: %2KB")
//	        .arg(_cache.recordsCount()).arg(_cache.byteSize() / 1024));

	if ( _acquisitionTimer ) {
		_acquisitionTimer->stop();
		delete _acquisitionTimer;
		_acquisitionTimer = NULL;
	}

	QString elapsed;
	if ( _stopWatch ) {
		elapsed = QString(", elapsed time: %3").arg(Time(_stopWatch->elapsed()).toString("%T.%f").c_str());
		delete _stopWatch;
		_stopWatch = NULL;
	}

	QString message = QString("%1 record objects stored, cache size: %2KB %3")
	        .arg(_cache.recordsCount()).arg(_cache.byteSize() / 1024).arg(elapsed);
	log(Client::LM_INFO, message);
	statusBar()->showMessage(message, _settings.messagesDurationMS);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::acquisitionExceptionReported(const QString& msg) {

	log(Client::LM_ERROR, QString("AcquisitionException: %1").arg(msg));

	QMap<QString, StationStreamStatus>::iterator it = _fetchedStationsStatus.begin();
	for (; it != _fetchedStationsStatus.end(); ++it) {

		if ( it.value() == sssWaitingForData ) {

			QStringList station = it.key().split(".");

			if ( station.size() != 2 )
			    continue;

			it.value() = sssMissingData;

			for (int i = 0; i < _ui->tableWidget_stations->rowCount(); ++i) {
				if ( _ui->tableWidget_stations->item(i, 1)->text() == station.at(0)
				        && _ui->tableWidget_stations->item(i, 2)->text() == station.at(1) ) {
					QVariant data;
					data.setValue(sssMissingData);
					_ui->tableWidget_stations->item(i, 0)->setData(Qt::UserRole, data);
					_ui->tableWidget_stations->item(i, 0)->setText("Missing data");
					_ui->tableWidget_stations->item(i, 0)->setForeground(Qt::white);
					_ui->tableWidget_stations->item(i, 0)->setBackground(Misc::noStreamDataColor);
					_ui->tableWidget_stations->item(i, 0)->setToolTip(
					    QString("%1 data is missing.\nReason: acquisition error = %2")
					            .arg(it.key()).arg(msg));
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::operationExceptionReported(const QString& msg) {
	log(Client::LM_ERROR, QString("OperationException: %1").arg(msg));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ParticleMotionWidget::addStation(const std::string& networkCode,
                                      const std::string& stationCode) {
	if ( _recordStream )
	    return _recordStream->addStream(networkCode, stationCode, "??", "???");
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::setStartTime(const Seiscomp::Core::Time& time) {
	if ( _recordStream )
	    _recordStream->setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::setEndTime(const Time& time) {
	if ( _recordStream )
	    _recordStream->setEndTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ParticleMotionWidget::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {
	if ( !_recordStream ) return false;
	return _recordStream->setTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ParticleMotionWidget::addStream(const std::string& networkCode,
                                     const std::string& stationCode,
                                     const std::string& locationCode,
                                     const std::string& channelCode) {
	if ( _recordStream )
	    return _recordStream->addStream(networkCode, stationCode, locationCode, channelCode);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ParticleMotionWidget::openStream() {

	if ( _recordStream ) return false;

	_recordStream = IO::RecordStream::Open(_recordStreamUrl.c_str());

	return _recordStream.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::closeStream() {

	if ( _recordStream )
	    _recordStream->close();

//	_recordStream = NULL;
	_recordStream.reset();
//	_selectedStation = NULL;
//	_selectedStation.reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::stopFetching() {
	emit stopDelegate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::stationSelected(QTableWidgetItem* current) {

	//! Don't assign NULL value to smart pointers, it's better practice to
	//! free them properly than fooling around and not be sure they have been
	//! de-referenced (reference count dropped down)
	_pPick.reset();
	_sPick.reset();

	_pmgl->clear();

	_wr.hasEndTimeSet = false;
	_wr.hasStartTimeSet = false;

	std::string networkCode;
	std::string stationCode;

	StationStreamStatus currentStatus = sssOK;

	for (int i = 0; i < _ui->tableWidget_stations->rowCount(); ++i)
		if ( i == current->row() ) {
			currentStatus = _ui->tableWidget_stations->item(i, 0)->
			        data(Qt::UserRole).value<StationStreamStatus>();
			networkCode = _ui->tableWidget_stations->item(i, 1)->text().toStdString();
			stationCode = _ui->tableWidget_stations->item(i, 2)->text().toStdString();
		}

	if ( networkCode.empty() or stationCode.empty() ) {
		log(Client::LM_ERROR, "Couldn't find station in table list");
		return;
	}

	if ( currentStatus == sssWaitingForData ) {
		statusBar()->showMessage(QString("Records are still being fetched for station %1.%2, please wait.")
		        .arg(networkCode.c_str()).arg(stationCode.c_str()), 2000);
		return;
	}

	if ( currentStatus != sssOK && currentStatus != sssMissingChunks ) {
		log(Client::LM_ERROR, QString("Couldn't iterate thru %1.%2 records")
		        .arg(networkCode.c_str()).arg(stationCode.c_str()));
		statusBar()->showMessage(QString("Couldn't iterate thru %1.%2 records")
		        .arg(networkCode.c_str()).arg(stationCode.c_str()), 2000);
		return;
	}

	_currentStationPicking = QString("%1.%2").arg(networkCode.c_str())
	        .arg(stationCode.c_str());

	_ui->tabWidgetStationData->setCurrentIndex(1);

	_ui->comboBox_component->clear();
	QStringList components;

	for (int i = 0; i < _activeStreams.size(); ++i) {

		if ( _activeStreams.at(i).networkCode.toStdString() != networkCode
		        or _activeStreams.at(i).stationCode.toStdString() != stationCode )
		    continue;

		QString s = QString("%1.%2.%3.%4").arg(_activeStreams.at(i).networkCode)
		        .arg(_activeStreams.at(i).stationCode)
		        .arg(_activeStreams.at(i).locationCode)
		        .arg(_activeStreams.at(i).channelCode);
		QString comp = QString("%1?").arg(_activeStreams.at(i).channelCode.left(2));
		RecordPtr rec = _cache.get(s.toStdString());
		if ( rec && !components.contains(comp) )
		    components.append(comp);
	}
	_ui->comboBox_component->addItems(components);

	showWaitingWidget();

	for (size_t i = 0; i < _origin->arrivalCount(); ++i) {

		ArrivalPtr ar = _origin->arrival(i);
		PickPtr pick = Pick::Find(ar->pickID());

		if ( !pick ) {
			PublicObjectPtr obj = _query->getObject(Pick::TypeInfo(), ar->pickID());
			pick = Pick::Cast(obj.get());
		}

		if ( !pick )
		    continue;

		if ( stationCode != pick->waveformID().stationCode()
		        || pick->waveformID().networkCode() != networkCode )
		    continue;

		//! Arrivals are listed by time (I think so anyway) so the last valid P
		//! and S picks should be the ones of interest
		if ( ar->phase().code() == "P" )
		    _pPick = pick;

		if ( ar->phase().code() == "S" )
		    _sPick = pick;
	}

	_ui->label_stationName->setText(stationCode.c_str());

	if ( _pPick && _sPick ) {
		TimeSpan sp = _sPick->time().value() - _pPick->time().value();
		_ui->label_sminusp->setText(QString("%1 sec").arg(sp.seconds()));
		_ui->checkBox_useSminusP->setEnabled(true);
		_ui->checkBox_useSminusP->setChecked(true);
	}
	else {
		_ui->label_sminusp->setText("-");
		_ui->checkBox_useSminusP->setChecked(false);
		_ui->checkBox_useSminusP->setEnabled(false);
	}

	_orv->setStreamUrl(_recordStreamUrl);
	_orv->setOrigin(_origin->publicID());
	_orv->setAutoFetchRecords(false);
	_orv->setStation(networkCode, stationCode, components.at(0).left(2).toStdString());

	prepareData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::componentChannelChanged(int idx) {

	Q_UNUSED(idx);

	if ( !_ui->comboBox_component->currentText().isEmpty() )
	    _orv->showStationStream(_ui->comboBox_component->currentText().mid(0, 2));

	prepareData(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::useSMinusPTriggered(bool checked) {

	if ( checked ) {
		_ui->label_8->setEnabled(false);
		_ui->label_sminusp->setEnabled(false);
	}
	else {
		_ui->label_8->setEnabled(true);
		_ui->label_sminusp->setEnabled(true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::prepareData(const bool& checkStation) {

	if ( _cache.isEmpty() ) {
		log(Client::LM_ERROR, "Can't read anything from the cache because it is empty");
		hideWaitingWidget();
		return;
	}

	// Display the waiting glass and spin the progress indicator if this
	// method has been launched from a refresh request
	if ( _state.isSet(Client::AppIdling) )
	    showWaitingWidget();

	QStringList it = _currentStationPicking.split(".");

	if ( it.size() == 0 ) {
		log(Client::LM_ERROR, "No selected station to prepare data from");
		return;
	}

	QStringList streams;
	for (int i = 0; i < _activeStreams.size(); ++i) {

		if ( _activeStreams.at(i).networkCode != it.at(0)
		        or _activeStreams.at(i).stationCode != it.at(1) )
		    continue;

		if ( _activeStreams.at(i).channelCode.left(2) != _ui->comboBox_component->currentText().left(2) )
		    continue;

		streams << QString("%1.%2.%3.%4").arg(_activeStreams.at(i).networkCode)
		        .arg(_activeStreams.at(i).stationCode)
		        .arg(_activeStreams.at(i).locationCode)
		        .arg(_activeStreams.at(i).channelCode);
	}

	if ( streams.size() == 0 )
	    return;


	double regionStart = -1., regionEnd = -1.;
	if ( _pPick && _sPick && _ui->checkBox_useSminusP->isChecked() ) {
		regionStart = (double) _pPick->time().value();
		regionEnd = (double) _sPick->time().value();
	}
	else if ( _orv->hasPick("Start") && _orv->hasPick("End") ) {
		regionStart = _orv->pick("Start").time();
		regionEnd = _orv->pick("End").time();
	}

	ComponentData tmpZ;
	ComponentData tmpNS;
	ComponentData tmpEW;
	QStringList log;

	DoubleArray* fftdataZ = new DoubleArray;
	DoubleArray* fftdataNS = new DoubleArray;
	DoubleArray* fftdataEW = new DoubleArray;
	double fsamp = -1.;
	bool fsetup = false;
//	ColorMapViewerPlot::Stream zcstream;

	for (int i = 0; i < streams.size(); ++i) {

		std::vector<RecordPtr> recV = _cache.getRecords(streams.at(i).toStdString());
		for (size_t v = 0; v < recV.size(); ++v) {

			RecordPtr rec = recV.at(v);

			if ( !rec ) continue;

			double stime = rec->startTime();
			double delta = 1. / rec->samplingFrequency();

			ArrayPtr tmp_ar;
			const DoubleArray* data = DoubleArray::ConstCast(rec->data());
			if ( !data ) {
				tmp_ar = rec->data()->copy(Array::DOUBLE);
				data = DoubleArray::ConstCast(tmp_ar);
				if ( !data ) continue;
			}

			DoubleArray* data2 = DoubleArray::Cast(data->clone());
			if ( _filter ) {
				_filter->setStartTime(rec->startTime());
				_filter->setSamplingFrequency(rec->samplingFrequency());
				_filter->setStreamID(_activeStreams.at(i).networkCode.toStdString(),
				    _activeStreams.at(i).stationCode.toStdString(),
				    _activeStreams.at(i).locationCode.toStdString(),
				    _activeStreams.at(i).channelCode.toStdString());
				try {
					_filter->apply(*data2);
				}
				catch ( std::exception& e ) {
					log << e.what();
				}
			}

			for (int j = 0; j < data2->size(); ++j) {

				if ( streams.at(i).right(1) == "Z" or streams.at(i).right(1) == "A" )
				    tmpZ.insert(stime, data2->get(j));

				if ( streams.at(i).right(1) == "N" or streams.at(i).right(1) == "B"
				        or streams.at(i).right(1) == "1" )
				    tmpNS.insert(stime, data2->get(j));

				if ( streams.at(i).right(1) == "E" or streams.at(i).right(1) == "C"
				        or streams.at(i).right(1) == "2" )
				    tmpEW.insert(stime, data2->get(j));

				stime += TimeSpan(delta);
			}

			//! Compute frequency spectrum
			if ( (double) rec->startTime() > regionStart && ((double) rec->startTime() + delta) < regionEnd ) {

				if ( !fsetup ) {
					fsamp = rec->samplingFrequency();
					fsetup = true;
				}

//				ColorMapViewerPlot::RecordData r;
//				r.fsamp = rec->samplingFrequency();
//				r.stack = *data2;
//				r.time = rec->startTime();

				if ( streams.at(i).right(1) == "Z" or streams.at(i).right(1) == "A" ) {

					_zChannel.fsamp = rec->samplingFrequency();
					_zChannel.streamID = streams.at(i);
					fftdataZ->append(data2);

//					zcstream.data.append(r);
				}
				else if ( streams.at(i).right(1) == "N" or streams.at(i).right(1) == "B"
				        or streams.at(i).right(1) == "1" ) {
					_nsChannel.fsamp = rec->samplingFrequency();
					_nsChannel.streamID = streams.at(i);
					fftdataNS->append(data2);
				}
				else if ( streams.at(i).right(1) == "E" or streams.at(i).right(1) == "C"
				        or streams.at(i).right(1) == "2" ) {
					_ewChannel.fsamp = rec->samplingFrequency();
					_ewChannel.streamID = streams.at(i);
					fftdataEW->append(data2);
				}
			}
		}
	}


	FrequencyViewerPlot::Stream streamZ;
	streamZ.fsamp = fsamp;
	streamZ.data = *fftdataZ;
	streamZ.id = _zChannel.streamID;

	FrequencyViewerPlot::Stream streamNS;
	streamNS.fsamp = fsamp;
	streamNS.data = *fftdataNS;
	streamNS.id = _nsChannel.streamID;

	FrequencyViewerPlot::Stream streamEW;
	streamEW.fsamp = fsamp;
	streamEW.data = *fftdataEW;
	streamEW.id = _ewChannel.streamID;

	_freqZ->reset();
	_freqZ->addStream(streamZ);
	_freqZ->redraw();

	_freqNS->reset();
	_freqNS->addStream(streamNS);
	_freqNS->redraw();

	_freqEW->reset();
	_freqEW->addStream(streamEW);
	_freqEW->redraw();


//	zcstream.id = _zChannel.streamID;
//	_zSpectro->reset();
//	_zSpectro->addStream(zcstream);
//	_zSpectro->redraw();


	delete fftdataZ;
	fftdataZ = NULL;

	delete fftdataNS;
	fftdataNS = NULL;

	delete fftdataEW;
	fftdataEW = NULL;


	_zData.clear();
	_ewData.clear();
	_nsData.clear();

	// Cutting thru selection start and end time, making sure all collected
	// samples are perfectly in range by taking the Z component as reference.
	double start = tmpZ.begin().key();
	if ( start < tmpNS.begin().key() )
	    start = tmpNS.begin().key();
	if ( start < tmpEW.begin().key() )
	    start = tmpEW.begin().key();

	double endZ = .0;
	ComponentData::iterator it0 = tmpZ.begin();
	while ( it0 != tmpZ.end() ) {
		endZ = it0.key();
		++it0;
	}

	double endNS = .0;
	ComponentData::iterator it1 = tmpNS.begin();
	while ( it1 != tmpNS.end() ) {
		if ( endNS < it1.key() )
		    endNS = it1.key();
		++it1;
	}

	double endEW = .0;
	ComponentData::iterator it2 = tmpEW.begin();
	while ( it2 != tmpEW.end() ) {
		if ( endEW < it2.key() )
		    endEW = it2.key();
		++it2;
	}

	double end = endZ;
	if ( end > endNS )
	    end = endNS;
	if ( end > endEW )
	    end = endEW;

	ComponentData::iterator itZ = tmpZ.begin();
	while ( itZ != tmpZ.end() ) {
		if ( itZ.key() >= start && itZ.key() <= end ) {
			if ( (regionStart != -1.) && (regionEnd != -1.)
			        && (itZ.key() > regionStart) && (itZ.key() < regionEnd) )
			    _zData.insert(itZ.key(), itZ.value());
		}
		++itZ;
	}

	ComponentData::iterator itNS = tmpNS.begin();
	while ( itNS != tmpNS.end() ) {
		if ( itNS.key() >= start && itNS.key() <= end ) {
			if ( (regionStart != -1.) && (regionEnd != -1.)
			        && (itNS.key() > regionStart) && (itNS.key() < regionEnd) )
			    _nsData.insert(itNS.key(), itNS.value());
		}
		++itNS;
	}

	ComponentData::iterator itEW = tmpEW.begin();
	while ( itEW != tmpEW.end() ) {
		if ( itEW.key() >= start && itEW.key() <= end ) {
			if ( (regionStart != -1.) && (regionEnd != -1.)
			        && (itEW.key() > regionStart) && (itEW.key() < regionEnd) )
			    _ewData.insert(itEW.key(), itEW.value());
		}
		++itEW;
	}

	plotData();

	hideWaitingWidget();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::plotData() {

	QVector<RecordItem> items;

	QVector<qreal> zVector;
	ComponentData::iterator itZ = _zData.begin();
	while ( itZ != _zData.end() ) {
		zVector.append(itZ.value());
		items.append(RecordItem(itZ.key(), itZ.value(), .0, .0));
		++itZ;
	}

	QVector<qreal> nsVector;
	ComponentData::iterator itNS = _nsData.begin();
	while ( itNS != _nsData.end() ) {
		nsVector.append(itNS.value());
		++itNS;
	}

	QVector<qreal> ewVector;
	ComponentData::iterator itEW = _ewData.begin();
	while ( itEW != _ewData.end() ) {
		ewVector.append(itEW.value());
		++itEW;
	}

	for (int i = 0; i < _ui->tableWidget_data->rowCount(); ++i)
		_ui->tableWidget_data->removeRow(i);

	if ( zVector.size() == nsVector.size() && ewVector.size() == nsVector.size() ) {
		_ui->tableWidget_data->setRowCount(zVector.size());

		for (int i = 0; i < zVector.size(); ++i) {

			items[i].nsValue = nsVector.at(i);
			items[i].ewValue = ewVector.at(i);

			QDateTime t = QDateTime::fromMSecsSinceEpoch(items.at(i).time * 1000);
			_ui->tableWidget_data->setItem(i, 0, new QTableWidgetItem(t.toString("hh:mm:ss.zzz")));
			_ui->tableWidget_data->setItem(i, 1, new NumberTableItem(items.at(i).zValue, 0));
			_ui->tableWidget_data->setItem(i, 2, new NumberTableItem(items.at(i).nsValue, 0));
			_ui->tableWidget_data->setItem(i, 3, new NumberTableItem(items.at(i).ewValue, 0));
			_ui->tableWidget_data->item(i, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_data->item(i, 0)->setData(Qt::UserRole, QVariant(items.at(i).time));
			_ui->tableWidget_data->item(i, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_data->item(i, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			_ui->tableWidget_data->item(i, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

			if ( _ui->checkBox_colorsGradient->isChecked() ) {

				QColor c = QColor(sin(i * 0.3) * 100 + 100,
				    sin(i * 0.6 + 0.7) * 100 + 100,
				    sin(i * 0.4 + 0.6) * 100 + 100,
				    100);
				_ui->tableWidget_data->item(i, 0)->setBackgroundColor(c);
				_ui->tableWidget_data->item(i, 1)->setBackgroundColor(c);
				_ui->tableWidget_data->item(i, 2)->setBackgroundColor(c);
				_ui->tableWidget_data->item(i, 3)->setBackgroundColor(c);
			}
		}
	}

	_recordItems = items;

	float duration = _wr.endTime - _wr.startTime;
	if ( _ui->checkBox_useSminusP->isChecked() ) {
		if ( _pPick && _sPick ) {
			TimeSpan sp = _sPick->time().value() - _pPick->time().value();
			duration = sp.seconds();
			_ui->label_wstart->setText(_pPick->time().value().toString("%Y-%m-%d %H:%M:%S").c_str());
			_ui->label_wend->setText(_sPick->time().value().toString("%Y-%m-%d %H:%M:%S").c_str());
		}
	}
	_ui->tableWidget_data->resizeRowsToContents();
	_ui->label_samplesCount->setText(QString::number(_ui->tableWidget_data->rowCount()));
	_ui->label_waveDuration->setText(QString("%1 sec").arg(QString::number(duration, 'f', 2)));

	QPen defaultPen;
	defaultPen.setWidthF(.5);
	defaultPen.setColor(_waveTraceColor);

	QPen startPen;
	startPen.setWidthF(1.5);
	startPen.setColor(Qt::darkGreen);

	QPen endPen;
	endPen.setWidthF(1.5);
	endPen.setColor(Qt::darkRed);

	QString log;
	_pznsPlot->clearItems();
	bool p1error = false;
	bool p2error = false;

	if ( nsVector.size() == zVector.size() ) {

		for (int i = 0; i < nsVector.size() - 1; ++i) {

			QCPItemLine* line = new QCPItemLine(_pznsPlot);
			_pznsPlot->addItem(line);
			line->setAntialiased(false);
			if ( i == 0 ) {
				line->setPen(startPen);
				line->setHead(QCPLineEnding::esSpikeArrow);
				line->setTail(QCPLineEnding::esDisc);
				line->setTooltip("Sample's start");
			}
			else if ( i == nsVector.size() - 2 ) {
				line->setPen(endPen);
				line->setHead(QCPLineEnding::esDiamond);
				line->setTooltip("Sample's end");
			}
			else {
				if ( _ui->checkBox_colorsGradient->isChecked() ) {
					defaultPen.setColor(QColor(sin(i * 0.3) * 100 + 100,
					    sin(i * 0.6 + 0.7) * 100 + 100,
					    sin(i * 0.4 + 0.6) * 100 + 100,
					    100));
				}
				line->setPen(defaultPen);
				if ( _ui->checkBox_spikes->isChecked() )
				    line->setHead(QCPLineEnding::esSpikeArrow);
			}
			line->setClipAxisRect(_pznsPlot->axisRect());
			line->setClipToAxisRect(true);
			line->start->setType(QCPItemPosition::ptPlotCoords);
			line->start->setCoords(nsVector.at(i), zVector.at(i));
			line->end->setType(QCPItemPosition::ptPlotCoords);
			line->end->setCoords(nsVector.at(i + 1), zVector.at(i + 1));
		}

		//! We plot movements by using QCPItems which aren't handled by the lib
		//! internal resize method, therefore, we need to use setRange on
		//! each axis to get a properly rescaled view.
		//! TODO: try and set 1./1. ratio so that the symmetry could be respected
		_pznsPlot->xAxis->setRange(getMin(nsVector) - 10., getMax(nsVector) + 10.);
		_pznsPlot->yAxis->setRange(getMin(zVector) - 10., getMax(zVector) + 10.);
		_pznsPlot->replot();
	}
	else
		p1error = true, log.append("Z component data count differs from NS one.\n");

	_pewnsPlot->clearItems();
	if ( nsVector.size() == ewVector.size() ) {

		for (int i = 0; i < ewVector.size() - 1; ++i) {

			QCPItemLine* line = new QCPItemLine(_pewnsPlot);
			line->setAntialiased(false);
			if ( i == 0 ) {
				line->setPen(startPen);
				line->setHead(QCPLineEnding::esSpikeArrow);
				line->setTail(QCPLineEnding::esDisc);
				line->setTooltip("Sample's start");
			}
			else if ( i == ewVector.size() - 2 ) {
				line->setPen(endPen);
				line->setHead(QCPLineEnding::esDiamond);
				line->setTooltip("Sample's end");
			}
			else {
				if ( _ui->checkBox_colorsGradient->isChecked() ) {
					defaultPen.setColor(QColor(sin(i * 0.3) * 100 + 100,
					    sin(i * 0.6 + 0.7) * 100 + 100,
					    sin(i * 0.4 + 0.6) * 100 + 100,
					    100));
				}
				line->setPen(defaultPen);
				if ( _ui->checkBox_spikes->isChecked() )
				    line->setHead(QCPLineEnding::esSpikeArrow);
			}
			line->setClipAxisRect(_pewnsPlot->axisRect());
			line->setClipToAxisRect(true);
			line->start->setType(QCPItemPosition::ptPlotCoords);
			line->start->setCoords(ewVector.at(i), nsVector.at(i));
			line->end->setType(QCPItemPosition::ptPlotCoords);
			line->end->setCoords(ewVector.at(i + 1), nsVector.at(i + 1));
			_pewnsPlot->addItem(line);
		}

		//! We plot movements by using QCPItems which aren't handled by the lib
		//! internal resize method, therefore, we need to use setRange on
		//! each axis to get a properly rescaled view.
		//! TODO: try and set 1./1. ratio so that the symmetry could be respected
		_pewnsPlot->xAxis->setRange(getMin(ewVector) - 10., getMax(ewVector) + 10.);
		_pewnsPlot->yAxis->setRange(getMin(nsVector) - 10., getMax(nsVector) + 10.);
		_pewnsPlot->replot();
	}
	else
		p2error = true, log.append("EW component data count differs from NS one.\n");

	if ( zVector.size() == nsVector.size() && nsVector.size() == ewVector.size()
	        && zVector.size() != 0 )
		_pmgl->feed(ewVector, nsVector, zVector, _ui->checkBox_colorsGradient->isChecked());
	else {

		QString msg = QString("Data range error");

		if ( zVector.size() == 0 or nsVector.size() == 0 or ewVector.size() == 0 )
		    msg = QString("No data range defined");

		if ( p1error ) {
			QCPItemText* noData1 = new QCPItemText(_pznsPlot);
			noData1->setClipAxisRect(_pznsPlot->axisRect());
			noData1->setClipToAxisRect(true); // put it inside the axisRect
			noData1->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			noData1->position->setType(QCPItemPosition::ptAxisRectRatio);
			noData1->position->setCoords(.5, .5);
			noData1->setText(msg);
			noData1->setFont(QFont(this->font().family(), 16));
			noData1->setPen(QPen(Qt::black));
			_pznsPlot->addItem(noData1);
			_pznsPlot->replot();
		}

		if ( p2error ) {
			QCPItemText* noData2 = new QCPItemText(_pewnsPlot);
			noData2->setClipAxisRect(_pewnsPlot->axisRect());
			noData2->setClipToAxisRect(true); // put it inside the axisRect
			noData2->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			noData2->position->setType(QCPItemPosition::ptAxisRectRatio);
			noData2->position->setCoords(.5, .5);
			noData2->setText(msg);
			noData2->setFont(QFont(this->font().family(), 16));
			noData2->setPen(QPen(Qt::black));
			_pewnsPlot->addItem(noData2);
			_pewnsPlot->replot();
		}
	}

	if ( log.size() > 0 ) {
		this->log(Client::LM_ERROR, "Failed to plot graphs: gaps were found in data");
		log.append("This happens when chunks of data are missing "
			"within selected wave region.\nTIP: try and adjust wave region "
			"to solve this issue...");
		QMessageBox::critical(this, tr("Graphics error"), log);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::saveCollectedData() {

	QString filename = QFileDialog::getSaveFileName(this, tr("Save particle motion data"),
	    QDir::currentPath(), tr("Comma-Separated-Values (*.csv)"), 0, QFileDialog::DontUseNativeDialog);

	if ( filename.isEmpty() )
	    return;

	if ( filename.right(4) != ".csv" or filename.right(4) != ".CSV" )
	    filename.append(".csv");

	QFile file(filename);

	if ( file.open(QFile::WriteOnly | QFile::Truncate) ) {

		QTextStream data(&file);
		data.setCodec("UTF-8");
		QStringList strList;

		// Time as a double value
		strList << QString("\"Time(double)\"");

		for (int c = 0; c < _ui->tableWidget_data->columnCount(); ++c)
			strList << QString("\"%1\"")
			        .arg(_ui->tableWidget_data->horizontalHeaderItem(c)->
			        data(Qt::DisplayRole).toString());

		data << strList.join(";") << "\n";

		for (int r = 0; r < _ui->tableWidget_data->rowCount(); ++r) {

			strList.clear();

			for (int c = 0; c < _ui->tableWidget_data->columnCount(); ++c) {

				QTableWidgetItem* item = _ui->tableWidget_data->item(r, c);

				if ( !item || item->text().isEmpty() )
				    _ui->tableWidget_data->setItem(r, c, new QTableWidgetItem(""));

				if ( c == 0 )
				    strList << QString("\"%1\"").arg(QString::number(
				        _ui->tableWidget_data->item(r, c)->data(Qt::UserRole).toReal()));

				strList << QString("\"%1\"").arg(_ui->tableWidget_data->item(r, c)->text());
			}
			data << strList.join(";") + "\n";
		}

		file.close();
		statusBar()->showMessage(tr("Successfully saved file %1").arg(filename), 3000);
	}
	else
		statusBar()->showMessage(tr("Failed to saved file %1").arg(filename), 3000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::showWaitingWidget() {

	_state.remove(Client::AppIdling);
	_state.set(Client::AppWorking);
	this->setCursor(Qt::WaitCursor);
	_orv->setCursor(Qt::WaitCursor);
	_pi->startAnimation();
	_pi->show();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionWidget::hideWaitingWidget() {

	_state.remove(Client::AppWorking);
	_state.set(Client::AppIdling);
	this->setCursor(Qt::ArrowCursor);
	_orv->setCursor(Qt::ArrowCursor);
	_pi->stopAnimation();
	_pi->hide();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}// namespace Gui
} // namespace IPGP
