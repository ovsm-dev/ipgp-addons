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

#include <ipgp/gui/datamodel/eventlist/eventlistwidget.h>
#include <ipgp/gui/datamodel/eventlist/ui_eventlistwidget.h>
#include <ipgp/gui/datamodel/eventlist/eventlistdelegate.h>
#include <ipgp/gui/client/application.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/utils/timer.h>



using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

/** @brief Table headers enum */
enum ETableHeaders {
	thOT_GMT, thCERTAINTY, thTYPE, thSCODE, thMAGNITUDE, thTP, thQUALITY,
	thMETHOD_ID, thPHASES, thLATITUDE, thLONGITUDE, thDEPTH, thSTATUS,
	thAGENCY, thAUTHOR, thREGION, thPUBLIC_ID
};

/**
 * @brief Table headers enum values
 *        The user has to make sure that those values matches (in order) the
 *        defined ETableHeaders enum so it can properly be setup.
 *        When the name of an element has to be retrieved, the user needs only
 *        instantiate the vector containing the names:
 *        @code
 *                std::string elementName = STableHeaders[thMAGNITUDE];
 *                or
 *                std::string elementName = TableHeadersString[thMAGNITUDE];
 *        @encode
 */
const char* STableHeaders[] =
        {
          "OT(GMT)", "Certainty", "Type", "Scode", "M", "TP", "Quality", "MethodID",
          "Phases", "Lat", "Lon", "Depth", "Stat", "Agency", "Author",
          "Region", "ID"
        };
const std::vector<const char*> TableHeadersString(STableHeaders, STableHeaders
        + sizeof(STableHeaders) / sizeof(STableHeaders[0]));

/**
 * @brief  Retrieves header position from header vector.
 * @param  e the STableHeader enum value
 * @return The header's position
 * @note   Valid header position are okay above zero, fake underneath.
 */
const size_t getHeaderPosition(const ETableHeaders& e) {
	char* str;
	for (size_t i = 0; i < TableHeadersString.size(); ++i) {
		str = (char*) TableHeadersString.at(i);
		if ( str == STableHeaders[e] )
		    return i;
	}
	return -1;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



namespace IPGP {
namespace Gui {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListWidget::Settings::Settings() {

	blinkFrequency = 300;
	displayedDecimals = 2;
	useCacheEngine = true;
	useMultiThreads = true;
	fetchUnassociated = true;
	useAdvancedOptions = true;
	decorateItems = true;
	fetchEventComment = false;
	fetchPreferredOriginComment = false;
	fetchSiblingOriginComment = false;
	fetchUnassociatedOriginComment = false;
	eventsAreCheckable = false;
	showEventColorType = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListWidget::EventListWidget(QWidget* parent, DatabaseQuery* query,
                                 ObjectCache* cache, Qt::WFlags f) :
		QWidget(parent, f), _ui(new Ui::EventListWidget), _query(query),
		_cache(cache), _displayedDecimals(2), _hasOwnCache(false) {

	if ( cache )
		_cache = cache;
	else {
		_cache = new ObjectCache;
		_hasOwnCache = true;
	}

	initWidget();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::initWidget() {

	_ws = ELW_Idle;

	_ui->setupUi(this);
	_ui->frame->setVisible(false);
	_ui->originCounter->setText("");

	_objects = NULL;

	connect(&_timer, SIGNAL(timeout()), this, SLOT(updateBlinker()));
	_buttonState = true;
	_color1 = _ui->pushButton_read->palette().color(QPalette::Button);
	_color2 = QColor::fromRgb(229, 130, 9, 255);

	_ui->treeView->setModel(&_treeModel);
	_ui->treeView->setSortingEnabled(true);

	// Multiples selections allow user to try and do irrational things
	// like deleting an entire (big) range of events/origins by mistake,
	// even creating new event from origin... Although this may actually be
	// what he wants to do... oops my bad, He/she is admin!
	_ui->treeView->setSelectionMode(QTreeView::ExtendedSelection);
	_ui->treeView->setSelectionBehavior(QTreeView::SelectRows);

	_ui->treeView->setAlternatingRowColors(true);
	_ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui->treeView->sortByColumn(0, Qt::AscendingOrder);

	_ui->lineEdit_scode->setText("*");
	connect(_ui->lineEdit_scode, SIGNAL(editingFinished()), this, SLOT(scodeEditionFinished()));

	// Setup table headers
	for (size_t i = 0; i < TableHeadersString.size(); ++i) {
		QStandardItem* itm = new QStandardItem(TableHeadersString[i]);
		itm->setTextAlignment(Qt::AlignHCenter);
		_treeModel.setHorizontalHeaderItem(i, itm);
	}

	connect(_ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(setFiltersVisible(bool)));
	connect(_ui->pushButton_read, SIGNAL(clicked()), this, SLOT(readEvents()));
	connect(_ui->comboBox_locator, SIGNAL(currentIndexChanged(int)), this, SLOT(locationMethodChanged(int)));
	connect(_ui->checkBox_hideFakeEvents, SIGNAL(clicked()), this, SLOT(hideFakeEventsClicked()));
	connect(_ui->checkBox_hideUnlocalizedEvents, SIGNAL(clicked()), this, SLOT(hideUnlocalizedEventsClicked()));

	for (int i = 0; i < _treeModel.columnCount(); ++i)
		_ui->treeView->resizeColumnToContents(i);

	_deleteEventAction = new QAction(tr("&Delete"), this);
	_deleteOriginAction = new QAction(tr("&Delete"), this);
	_eventPropertyAction = new QAction(tr("&Property"), this);
	_originPropertyAction = new QAction(tr("&Property"), this);
	_copyRowAction = new QAction(tr("&Copy row to clipboard"), this);
	_copyCellAction = new QAction(tr("&Copy objectID to clipboard"), this);
	_splitOriginAction = new QAction(tr("&Split origin and create new event"), this);


	connect(_deleteEventAction, SIGNAL(triggered()), this, SLOT(deleteEvent()));
	connect(_deleteOriginAction, SIGNAL(triggered()), this, SLOT(deleteOrigin()));
	connect(_eventPropertyAction, SIGNAL(triggered()), this, SLOT(showEventProperties()));
	connect(_originPropertyAction, SIGNAL(triggered()), this, SLOT(showOriginProperties()));
	connect(_splitOriginAction, SIGNAL(triggered()), this, SLOT(splitOrigin()));
	connect(_copyRowAction, SIGNAL(triggered()), this, SLOT(copyRow()));
	connect(_copyCellAction, SIGNAL(triggered()), this, SLOT(copyCell()));
	connect(_ui->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(treeViewMenu(const QPoint&)));
	connect(_ui->treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(rowClicked(const QModelIndex&)));
	connect(_ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(rowDoubleClicked(const QModelIndex&)));

	QHeaderView* horizontalHeader;
	horizontalHeader = _ui->treeView->header();
	horizontalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(horizontalHeader, SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(treeViewHeaderMenu(const QPoint&)));

	_otAction = new QAction(tr("&OT(GMT)"), this);
	_otAction->setCheckable(false);
	_otAction->setChecked(true);
	_certaintyAction = new QAction(tr("&Certainty"), this);
	_certaintyAction->setCheckable(true);
	_certaintyAction->setChecked(false);
	_typeAction = new QAction(tr("&Type"), this);
	_typeAction->setCheckable(true);
	_typeAction->setChecked(true);
	_seismicCodeAction = new QAction(tr("&Scode"), this);
	_seismicCodeAction->setCheckable(true);
	_seismicCodeAction->setChecked(true);
	_magnitudeAction = new QAction(tr("&Magnitude"), this);
	_magnitudeAction->setCheckable(true);
	_magnitudeAction->setChecked(true);
	_tpAction = new QAction(tr("&TP"), this);
	_tpAction->setCheckable(true);
	_tpAction->setChecked(true);
	_qualityAction = new QAction(tr("&Quality"), this);
	_qualityAction->setCheckable(true);
	_qualityAction->setChecked(false);
	_locatorAction = new QAction(tr("&Locator"), this);
	_locatorAction->setCheckable(true);
	_locatorAction->setChecked(false);
	_phasesAction = new QAction(tr("&Phases"), this);
	_phasesAction->setCheckable(true);
	_phasesAction->setChecked(true);
	_latitudeAction = new QAction(tr("&Lat"), this);
	_latitudeAction->setCheckable(true);
	_latitudeAction->setChecked(true);
	_longitudeAction = new QAction(tr("&Lon"), this);
	_longitudeAction->setCheckable(true);
	_longitudeAction->setChecked(true);
	_depthAction = new QAction(tr("&Depth"), this);
	_depthAction->setCheckable(true);
	_depthAction->setChecked(true);
	_statusAction = new QAction(tr("&Status"), this);
	_statusAction->setCheckable(true);
	_statusAction->setChecked(true);
	_agencyAction = new QAction(tr("&Agency"), this);
	_agencyAction->setCheckable(true);
	_agencyAction->setChecked(true);
	_authorAction = new QAction(tr("&Author"), this);
	_authorAction->setCheckable(true);
	_authorAction->setChecked(true);
	_regionAction = new QAction(tr("&Region"), this);
	_regionAction->setCheckable(true);
	_regionAction->setChecked(true);
	_idAction = new QAction(tr("&ID"), this);
	_idAction->setCheckable(true);
	_idAction->setChecked(true);

	connect(_certaintyAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_typeAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_seismicCodeAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_magnitudeAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_tpAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_qualityAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_locatorAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_phasesAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_latitudeAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_longitudeAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_depthAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_statusAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_agencyAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_authorAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_regionAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));
	connect(_idAction, SIGNAL(triggered()), this, SLOT(showHideTreeViewHeaderItems()));

	_ui->pushButtonSelectRegion->setText("");
	_ui->pushButtonSelectRegion->setIconSize(QSize(24, 24));
	_ui->pushButtonSelectRegion->setIcon(QIcon(":images/selectionarrow.png"));
	connect(_ui->pushButtonSelectRegion, SIGNAL(clicked()), this, SIGNAL(enableRegionFilterByMouse()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setLayoutMargins(const int& left, const int& top,
                                       const int& right, const int& bottom) {
	_ui->gridLayout_3->setContentsMargins(left, top, right, bottom);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setSelectRegionWithMouse(const bool& value) {
	_ui->pushButtonSelectRegion->setEnabled(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setDatabase(Seiscomp::DataModel::DatabaseQuery* query) {
	_query = query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setCache(ObjectCache* cache) {
	_hasOwnCache = false;
	(cache) ? _cache = cache : _cache = new ObjectCache, _hasOwnCache = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setSettings(const Settings& settings) {

	_settings = settings;

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thOT_GMT)) )
		    _treeModel.item(i, getHeaderPosition(thOT_GMT))->setCheckable(_settings.eventsAreCheckable);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::keyPressEvent(QKeyEvent* event) {
	event->matches(QKeySequence::Copy) ? copyRow() : QWidget::keyPressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::treeViewHeaderMenu(const QPoint& point) {

	Q_UNUSED(point);
	QMenu menu(this);
	menu.addAction(_otAction);
	menu.addAction(_certaintyAction);
	menu.addAction(_typeAction);
	menu.addAction(_seismicCodeAction);
	menu.addAction(_magnitudeAction);
	menu.addAction(_tpAction);
	menu.addAction(_qualityAction);
	menu.addAction(_locatorAction);
	menu.addAction(_phasesAction);
	menu.addAction(_latitudeAction);
	menu.addAction(_longitudeAction);
	menu.addAction(_depthAction);
	menu.addAction(_statusAction);
	menu.addAction(_agencyAction);
	menu.addAction(_authorAction);
	menu.addAction(_regionAction);
	menu.addAction(_idAction);
	menu.exec(QCursor::pos());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::showHideTreeViewHeaderItems() {

	_certaintyAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thCERTAINTY))
	    : _ui->treeView->hideColumn(getHeaderPosition(thCERTAINTY));
	_typeAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thTYPE))
	    : _ui->treeView->hideColumn(getHeaderPosition(thTYPE));
	_seismicCodeAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thSCODE))
	    : _ui->treeView->hideColumn(getHeaderPosition(thSCODE));
	_magnitudeAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thMAGNITUDE))
	    : _ui->treeView->hideColumn(getHeaderPosition(thMAGNITUDE));
	_tpAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thTP))
	    : _ui->treeView->hideColumn(getHeaderPosition(thTP));
	_qualityAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thQUALITY))
	    : _ui->treeView->hideColumn(getHeaderPosition(thQUALITY));
	_locatorAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thMETHOD_ID))
	    : _ui->treeView->hideColumn(getHeaderPosition(thMETHOD_ID));
	_phasesAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thPHASES))
	    : _ui->treeView->hideColumn(getHeaderPosition(thPHASES));
	_latitudeAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thLATITUDE))
	    : _ui->treeView->hideColumn(getHeaderPosition(thLATITUDE));
	_longitudeAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thLONGITUDE))
	    : _ui->treeView->hideColumn(getHeaderPosition(thLONGITUDE));
	_depthAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thDEPTH))
	    : _ui->treeView->hideColumn(getHeaderPosition(thDEPTH));
	_statusAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thSTATUS))
	    : _ui->treeView->hideColumn(getHeaderPosition(thSTATUS));
	_agencyAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thAGENCY))
	    : _ui->treeView->hideColumn(getHeaderPosition(thAGENCY));
	_authorAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thAUTHOR))
	    : _ui->treeView->hideColumn(getHeaderPosition(thAUTHOR));
	_regionAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thREGION))
	    : _ui->treeView->hideColumn(getHeaderPosition(thREGION));
	_idAction->isChecked() ? _ui->treeView->showColumn(getHeaderPosition(thPUBLIC_ID))
	    : _ui->treeView->hideColumn(getHeaderPosition(thPUBLIC_ID));

	for (int i = 0; i < _treeModel.columnCount(); ++i)
		_ui->treeView->resizeColumnToContents(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideFakeEventsClicked() {

	if ( _ui->checkBox_hideFakeEvents->isChecked() ) {
		int hidden = 0;
		for (int i = 0; i < _treeModel.rowCount(); ++i)
			if ( _treeModel.item(i, getHeaderPosition(thTYPE))->text() == "not existing" )
			    hidden++, _ui->treeView->setRowHidden(i, QModelIndex(), true);

		_ui->originCounter->setText(QString("%1 event(s) <> %2 fake event(s) hidden")
		        .arg(_objects->size()).arg(hidden));
		return;
	}

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thTYPE))->text() == "not existing" )
		    _ui->treeView->setRowHidden(i, QModelIndex(), false);
	_ui->originCounter->setText(QString("%1 event(s)").arg(_objects->size()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideUnlocalizedEventsClicked() {

	if ( _ui->checkBox_hideUnlocalizedEvents->isChecked() ) {
		int hidden = 0;
		for (int i = 0; i < _treeModel.rowCount(); ++i)
			if ( _treeModel.item(i, getHeaderPosition(thTYPE))->text() == "not locatable" )
			    hidden++, _ui->treeView->setRowHidden(i, QModelIndex(), true);

		_ui->originCounter->setText(QString("%1 event(s) <> %2 unlocalized event(s) hidden")
		        .arg(_objects->size()).arg(hidden));
		return;
	}

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thTYPE))->text() == "not locatable" )
		    _ui->treeView->setRowHidden(i, QModelIndex(), false);
	_ui->originCounter->setText(QString("%1 event(s)").arg(_objects->size()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::lockReading() {

	_ws = ELW_Locked;

	QPalette palette(_ui->pushButton_read->palette());
	palette.setColor(QPalette::Button, QColor(193, 255, 193));
	_ui->pushButton_read->setPalette(palette);
	_ui->pushButton_read->setText("Cache update");
//	_ui->pushButton_read->setDisabled(true);
	_ui->pushButton_read->update();
//	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::unlockReading() {

	_ws = ELW_Idle;
	QPalette palette(_ui->pushButton_read->palette());
	palette.setColor(QPalette::Button, _color1);
	_ui->pushButton_read->setPalette(palette);
	_ui->pushButton_read->setText("Read");
//	_ui->pushButton_read->setDisabled(false);
	_ui->pushButton_read->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::treeViewMenu(const QPoint& point) {

	Q_UNUSED(point);

	if ( !_settings.useAdvancedOptions )
	    return;

	if ( _ws == ELW_PopulatingModel ) {
		widgetStatusError();
		return;
	}

	_objectIDs.clear();

	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);
	_deleteOriginAction->setEnabled(
	    (!_treeModel.rowCount()) ? false : true);
	_deleteEventAction->setEnabled((!_treeModel.rowCount()) ? false : true);

	QModelIndexList indexList = _ui->treeView->selectionModel()
	        ->selectedRows(getHeaderPosition(thPUBLIC_ID));
	QModelIndex index = indexList.back();

	for (int i = 0; i < indexList.size(); ++i)
		_objectIDs.push_back(indexList.at(i).data().toString());

	if ( _objectIDs.size() == 0 )
	    return;

	PublicObjectPtr obj = _query->getObject(Event::TypeInfo(), _objectIDs.at(0).toStdString());
	EventPtr evt = Event::Cast(obj.get());

	obj = _query->getObject(Origin::TypeInfo(), _objectIDs.at(0).toStdString());
	OriginPtr org = Origin::Cast(obj.get());

	if ( _objectIDs.size() < 2 ) { // Single line is selected

		if ( evt ) {
			_deleteEventAction->setText(QString("&Delete event %1").arg(_objectIDs.at(0)));
			menu->addAction(_deleteEventAction);
			menu->addSeparator();
			menu->addAction(_eventPropertyAction);
			menu->addSeparator();
			menu->addAction(_copyRowAction);
			menu->addAction(_copyCellAction);
			menu->exec(QCursor::pos());
			return;
		}

		if ( org ) {
			_deleteOriginAction->setText(QString("&Delete origin %1").arg(_objectIDs.at(0)));
			_splitOriginAction->setText(QString("&Create a new event from this origin"));
			menu->addAction(_splitOriginAction);
			menu->addAction(_deleteOriginAction);
			menu->addSeparator();
			menu->addAction(_originPropertyAction);
			menu->addSeparator();
			menu->addAction(_copyRowAction);
			menu->addAction(_copyCellAction);
			menu->exec(QCursor::pos());
			return;
		}
	}

	else if ( _objectIDs.size() > 1 ) { // Multiples lines are selected

		if ( evt ) {
			_deleteEventAction->setText(QString("&Delete selected events"));
			_copyCellAction->setText(QString("&Copy selected events IDs"));
			_copyRowAction->setText(QString("&Copy selected events rows"));
			menu->addAction(_deleteEventAction);
			menu->addSeparator();
			menu->addAction(_copyRowAction);
			menu->addAction(_copyCellAction);
			menu->exec(QCursor::pos());
			return;
		}

		if ( org ) {
			_deleteOriginAction->setText(QString("&Delete selected origins"));
			_copyCellAction->setText(QString("&Copy selected origins IDs"));
			_copyRowAction->setText(QString("&Copy selected origins rows"));
			menu->addAction(_deleteOriginAction);
			menu->addSeparator();
			menu->addAction(_copyRowAction);
			menu->addAction(_copyCellAction);
			menu->exec(QCursor::pos());
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::rowClicked(const QModelIndex& model) {

	if ( _ws == ELW_PopulatingModel ) {
		widgetStatusError();
		return;
	}

	QModelIndexList indexList = _ui->treeView->selectionModel()
	        ->selectedRows(getHeaderPosition(thPUBLIC_ID));

	if ( indexList.size() == 0 ) {
		QStandardItem* itm = _treeModel.item(model.row(), getHeaderPosition(thPUBLIC_ID));
		if ( itm ) {
			emit rowClicked(itm->text());
			return;
		}
	}
	else {
		QModelIndex index = indexList.back();
		emit rowClicked(index.data().toString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::rowDoubleClicked(const QModelIndex& model) {

	if ( _ws == ELW_PopulatingModel ) {
		widgetStatusError();
		return;
	}

	QModelIndexList indexList = _ui->treeView->selectionModel()
	        ->selectedRows(getHeaderPosition(thPUBLIC_ID));

	if ( indexList.size() == 0 ) {
		QStandardItem* itm = _treeModel.item(model.row(), getHeaderPosition(thPUBLIC_ID));
		if ( itm ) {
			emit rowDoubleClicked(itm->text());
			return;
		}
	}
	else {
		QModelIndex index = indexList.back();
		emit rowDoubleClicked(index.data().toString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::widgetStatusError() {
	QMessageBox::warning(this, tr("User interaction"),
	    QString("It seems like this operation is not allowed at the moment"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::deleteEvent() {

	_ws = ELW_DeletingObject;

	for (int i = 0; i < _objectIDs.size(); ++i) {

		QString question = QString("Are sure about deleting event %1 ?").arg(_objectIDs.at(i));
		QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Event entry deletion"),
		    question, QMessageBox::Yes | QMessageBox::No);

		if ( reply == QMessageBox::No )
		    return;

		// Making sure we've got the last instance of the object stored in db
		PublicObjectPtr obj = _query->getObject(Event::TypeInfo(), _objectIDs.at(i).toStdString());
		EventPtr evt = Event::Cast(obj.get());

		int count = 0;
		if ( evt && _query->removeTree(evt.get(), _objectIDs.at(i).toStdString(), &count) )
		    QMessageBox::about(this, tr("Information"), QString("Event %1 deleted with %2 children")
		            .arg(evt->publicID().c_str()).arg(count));
	}

	sleep((unsigned int) 1);
	emit readEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::deleteOrigin() {

	_ws = ELW_DeletingObject;

	for (int i = 0; i < _objectIDs.size(); ++i) {

		QString question = QString("Are sure about deleting origin %1 ?").arg(_objectIDs.at(i));
		QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Origin entry deletion"),
		    question, QMessageBox::Yes | QMessageBox::No);

		if ( reply == QMessageBox::No )
		    return;

		// Making sure we've got the last instance of the object stored in db
		PublicObjectPtr obj = _query->getObject(Origin::TypeInfo(), _objectIDs.at(i).toStdString());
		OriginPtr org = Origin::Cast(obj.get());

		if ( !org )
		    return;

		// Origin has a parent
		EventPtr evt = _query->getEvent(org->publicID());
		if ( evt ) {
			if ( _query->remove(org.get(), evt->publicID()) )
			    QMessageBox::about(this, tr("Information"), QString("Origin %1 deleted")
			            .arg(org->publicID().c_str()));
		}
		else {
			if ( _query->remove(org.get(), "") )
			    QMessageBox::about(this, tr("Information"), QString("Orphan origin %1 deleted")
			            .arg(org->publicID().c_str()));
		}
	}

	sleep((unsigned int) 1);
	emit readEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::showEventProperties() {
	QMessageBox::information(this, tr("Information"),
	    QString("This menu is available only for admins"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::showOriginProperties() {
	QMessageBox::information(this, tr("Information"),
	    QString("This menu is available only for admins"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::splitOrigin() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::copyRow() {

	QModelIndexList indexes = _ui->treeView->selectionModel()->selectedIndexes();

	if ( indexes.size() < 1 )
	    return;

	std::sort(indexes.begin(), indexes.end());

	QModelIndex previous = indexes.first();
	indexes.removeFirst();
	QString text;
	QModelIndex current;

	for (int i = 0; i < indexes.size(); ++i) {
		current = indexes.at(i);
		text.append(_ui->treeView->model()->data(previous).toString());
		if ( current.row() != previous.row() )
			text.append(QLatin1Char('\n'));
		else
			text.append(QLatin1Char('|'));
		previous = current;
	}

	text.append(_ui->treeView->model()->data(current).toString());
	text.append(QLatin1Char('\n'));
	QApplication::clipboard()->setText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::copyCell() {

	QString data;
	QModelIndexList indexList = _ui->treeView->selectionModel()
	        ->selectedRows(getHeaderPosition(thPUBLIC_ID));
	for (int i = 0; i < indexList.size(); ++i) {
		data.append(indexList.at(i).data().toString());
		if ( i != indexList.size() - 1 )
		    data.append(QLatin1Char('\n'));
	}
	QApplication::clipboard()->setText(data);


	/*
	 QModelIndexList indexes = _ui->treeView->selectionModel()->selectedRows(12);
	 if ( indexes.size() < 1 )
	 return;

	 std::sort(indexes.begin(), indexes.end());

	 QModelIndex previous = indexes.first();
	 indexes.removeFirst();
	 QString text;
	 QModelIndex current;

	 for (int i = 0; i < indexes.size(); i++) {
	 current = indexes.at(i);
	 text.append(_ui->treeView->model()->data(previous).toString());
	 if ( current.row() != previous.row() )
	 text.append(QLatin1Char('\n'));
	 else
	 text.append(QLatin1Char('|'));
	 previous = current;
	 }

	 text.append(_ui->treeView->model()->data(current).toString());
	 text.append(QLatin1Char('\n'));
	 QApplication::clipboard()->setText(text);
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListWidget::~EventListWidget() {

	// Only delete cache if it is our own, otherwise let the object supplying
	// the cache deal with releasing allocated memory...
	if ( _hasOwnCache && _cache ) {
//		_cache->clear();
		delete _cache;
		_cache = NULL;
	}

	delete _ui, delete _deleteEventAction, delete _deleteOriginAction,
	        delete _eventPropertyAction, delete _originPropertyAction,
	        delete _copyRowAction, delete _copyCellAction,
	        delete _splitOriginAction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::clear() {
	for (int i = 0; i < _treeModel.rowCount(); ++i)
		_treeModel.removeRows(i, _treeModel.rowCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setLocationProfiles(const std::vector<Locators>& loc) {

	_ui->comboBox_locator->insertItem(0, "- unset -");
	_ui->comboBox_earthModelID->insertItem(0, "- unset -");

	for (size_t i = 0; i < loc.size(); ++i)
		_ui->comboBox_locator->insertItem((int) i + 1, loc.at(i).first.c_str());

	_locators = loc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::locationMethodChanged(int) {

	_ui->comboBox_earthModelID->clear();
	_ui->comboBox_earthModelID->insertItem(0, "- unset -");

	if ( _ui->comboBox_locator->currentText() == "- unset -" )
	    return;

	for (size_t i = 0; i < _locators.size(); ++i) {

		if ( _locators.at(i).first != _ui->comboBox_locator->currentText().toStdString() )
		    continue;

		for (size_t j = 0; j < _locators.at(i).second.size(); ++j)
			_ui->comboBox_earthModelID->insertItem((int) j + 1, _locators.at(i).second.at(j).c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::readEvents() {


	if ( _ws == ELW_Locked || _ws == ELW_PopulatingModel ) {

		QMessageBox::StandardButton q1 = QMessageBox::question(this, tr("Cache lock"),
		    QString("A reading operation is in progress and the cache engine "
			    "has been consequently locked. Do you wish to stop this process ?"),
		    QMessageBox::Yes | QMessageBox::No);

		if ( q1 == QMessageBox::Yes ) {
			emit interruptDelegateRun();
			QApplication::processEvents();
		}

		return;
	}

	Time startTime;
	startTime.set(_ui->dateTimeEdit_start->date().year(),
	    _ui->dateTimeEdit_start->date().month(),
	    _ui->dateTimeEdit_start->date().day(),
	    _ui->dateTimeEdit_start->time().hour(),
	    _ui->dateTimeEdit_start->time().minute(),
	    _ui->dateTimeEdit_start->time().second(),
	    _ui->dateTimeEdit_start->time().msec());

	Time endTime;
	endTime.set(_ui->dateTimeEdit_end->date().year(),
	    _ui->dateTimeEdit_end->date().month(),
	    _ui->dateTimeEdit_end->date().day(),
	    _ui->dateTimeEdit_end->time().hour(),
	    _ui->dateTimeEdit_end->time().minute(),
	    _ui->dateTimeEdit_end->time().second(),
	    _ui->dateTimeEdit_end->time().msec());

	_ui->pushButton_read->setText("Stop update");

	addItems(startTime, endTime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::readEvents(const QDateTime& from,
                                 const QDateTime& to) {

	if ( _ws & ELW_Locked ) {
		QMessageBox::information(this, tr("Cache lock"),
		    QString("An update operation is in progress and the cache engine"
			    "has been consequently locked, please wait until the pending"
			    "task is over"));
		return;
	}

	Time startTime;
	startTime.set(from.date().year(),
	    from.date().month(),
	    from.date().day(),
	    from.time().hour(),
	    from.time().minute(),
	    from.time().second(),
	    from.time().msec());

	Time endTime;
	endTime.set(to.date().year(),
	    to.date().month(),
	    to.date().day(),
	    to.time().hour(),
	    to.time().minute(),
	    to.time().second(),
	    to.time().msec());

	addItems(startTime, endTime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::scodeEditionFinished() {
	QStringList s = _ui->lineEdit_scode->text().split(";");
	if ( s.size() == 1 && s.at(0).isEmpty() )
	    _ui->lineEdit_scode->setText("*");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::addItems(const Time& from, const Time& to) {

	stopBlinking();

	if ( !_query )
	    return;

	bool error = false;
	if ( AppInstance )
	    if ( !AppInstance->database() || !AppInstance->database()->isConnected() )
	        error = true;

	if ( error ) {
		_ui->pushButton_read->setText(tr("Read"));
		emit logMessage(Client::LM_ERROR, "EventListWidget", tr("No query instance available"));
		QMessageBox::warning(this, tr("Database error"),
		    QString("The connection to the database isn't available. "
			    "Your request can't be performed at the moment."));
		return;
	}

	clear();

	EventListDelegate::EventListFilter filter;

	if ( _ui->depthCheckBox->isChecked() ) {
		filter.filterDepth = true;
		filter.depthMin = _ui->depthMin->value();
		filter.depthMax = _ui->depthMax->value();
	}
	else
		filter.filterDepth = false;

	if ( _ui->latitudeCheckBox->isChecked() ) {
		filter.filterLatitude = true;
		filter.latitudeMin = _ui->latitudeMin->value();
		filter.latitudeMax = _ui->latitudeMax->value();
	}
	else
		filter.filterLatitude = false;

	if ( _ui->longitudeCheckBox->isChecked() ) {
		filter.filterLongitude = true;
		filter.longitudeMin = _ui->longitudeMin->value();
		filter.longitudeMax = _ui->longitudeMax->value();
	}
	else
		filter.filterLongitude = false;

	if ( _ui->rmsCheckBox->isChecked() ) {
		filter.filterRms = true;
		filter.rmsMin = _ui->rmsMin->value();
		filter.rmsMax = _ui->rmsMax->value();
	}
	else
		filter.filterRms = false;

	if ( _ui->comboBox_earthModelID->currentText() != "- unset -" ) {
		filter.filterEarthModelID = true;
		filter.earthModelID = _ui->comboBox_earthModelID->currentText();
	}
	else
		filter.filterEarthModelID = false;

	if ( _ui->comboBox_evalutationMode->currentText() != "- unset -" ) {
		filter.filterEvaluationMode = true;
		filter.evaluationMode = _ui->comboBox_evalutationMode->currentText();
	}
	else
		filter.filterEvaluationMode = false;


	if ( _ui->comboBox_locator->currentText() != "- unset -" ) {
		filter.filterMethodID = true;
		filter.methodID = _ui->comboBox_locator->currentText();
	}
	else {
		filter.filterMethodID = false;
	}

	if ( _ui->comboBox_eventType->currentText() != "- unset -" ) {
		filter.filterEventType = true;
		filter.evenType = _ui->comboBox_eventType->currentText();
	}
	else
		filter.filterEventType = false;


	if ( _ui->comboBox_eventCertainty->currentText() != "- unset -" ) {
		filter.filterEventCertainty = true;
		filter.eventCertainty = _ui->comboBox_eventCertainty->currentText();
	}
	else
		filter.filterEventCertainty = false;

	if ( _ui->magnitudeCheckBox->isChecked() ) {
		filter.filterMagnitude = true;
		filter.magnitudeMin = _ui->magMin->value();
		filter.magnitudeMax = _ui->magMax->value();
	}
	else
		filter.filterMagnitude = false;

	if ( _ui->comboBox_originStatus->currentText() != "- unset -" ) {
		filter.filterOriginStatus = true;
		filter.originStatus = _ui->comboBox_originStatus->currentText();
	}
	else
		filter.filterOriginStatus = false;

	if ( _ui->checkBox_hideFakeEvents->isChecked() )
		filter.filterFakeEvent = true;
	else
		filter.filterFakeEvent = false;

	if ( _ui->checkBox_hideUnlocalizedEvents->isChecked() )
		filter.filterUnlocalizedEvent = true;
	else
		filter.filterUnlocalizedEvent = false;

	filter.fetchEventComment = _settings.fetchEventComment;
	filter.fetchPreferredOriginComment = _settings.fetchPreferredOriginComment;
	filter.fetchSiblingOriginComment = _settings.fetchSiblingOriginComment;
	filter.fetchUnassociatedOriginComment = _settings.fetchUnassociatedOriginComment;
	filter.showEventColorType = _settings.showEventColorType;
//	filter.filterExternalEvents = !_settings.showExternalEvents;

	filter.eventSCODE = _ui->lineEdit_scode->text().split(";");


	if ( _objects ) {
		//! TODO
		//! Ensure objects pointers are de-referenced properly by boost
		_objects->clear();
		delete _objects;
		_objects = NULL;
	}

	_objects = new OriginList;
	_ws = ELW_PopulatingModel;
	_ui->originCounter->setText("0 event(s)");

	//! TODO: Try new custom queries and make this part blazing fast...
	if ( _settings.useMultiThreads ) {

		//! Pure multi-threaded solution allows the GUI to be responsive when
		//! the heavy work of "fetching/associating/ordering origins/events"
		//! takes place. Immediate interaction is therefore possible with
		//! the objects in the model, though a slight but noticeable loading
		//! time difference exists.
		{
			QThread* thread = new QThread;
			EventListDelegate* delegate = new EventListDelegate(_query, &_treeModel,
			    _cache, from, to, filter, &_events, &_origins, _settings.fetchUnassociated);
			delegate->moveToThread(thread);

			connect(delegate, SIGNAL(workInProgress()), this, SIGNAL(viewUpdateRequested()));
			connect(delegate, SIGNAL(jobFinished()), this, SLOT(delegateFinished()));
			connect(thread, SIGNAL(started()), delegate, SLOT(populateModel()));
			connect(delegate, SIGNAL(queryModelReorganization()), this, SLOT(reorganizeModel()));
			connect(delegate, SIGNAL(jobFinished()), thread, SLOT(quit()));
			connect(delegate, SIGNAL(jobFinished()), delegate, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(delegate, SIGNAL(log(bool,const QString&)), this, SLOT(log(bool,const QString&)));
			connect(delegate, SIGNAL(accomplishedWorkPercentage(const int&, const QString&, const QString&)),
			    this, SIGNAL(updateProgress(const int&, const QString&, const QString&)));
			connect(this, SIGNAL(interruptDelegateRun()), delegate, SLOT(requestStop()));
			thread->start();
		}
	}
	else {

		//! Mono-thread solution is moderately quicker than the multi-threaded
		//! solution, but the main GUI thread is drastically monopolized by
		//! the worker, which makes the populating process appears slower
		//! and consequently more annoying.
		{
			EventListDelegate delegate(_query, &_treeModel, _cache, from, to, filter,
			    &_events, &_origins, _settings.fetchUnassociated);
			connect(&delegate, SIGNAL(workInProgress()), this, SIGNAL(viewUpdateRequested()));
			connect(&delegate, SIGNAL(jobFinished()), this, SLOT(delegateFinished()));
			connect(&delegate, SIGNAL(queryModelReorganization()), this, SLOT(reorganizeModel()));
			connect(&delegate, SIGNAL(log(bool,const QString&)), this, SLOT(log(bool,const QString&)));
			connect(&delegate, SIGNAL(accomplishedWorkPercentage(const int&, const QString&, const QString&)),
			    this, SIGNAL(updateProgress(const int&, const QString&, const QString&)));
			connect(this, SIGNAL(interruptDelegateRun()), &delegate, SLOT(requestStop()));
			delegate.populateModel();
		}

	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::delegateFinished() {
	_ws = ELW_Idle;
	_ui->pushButton_read->setText("Read");
	emit viewUpdated();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginPtr EventListWidget::getOrigin(const std::string& publicID) {

	for (size_t i = 0; i < _objects->size(); ++i)
		if ( _objects->at(i).first->publicID() == publicID )
		    return _objects->at(i).first;

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventPtr EventListWidget::getEvent(const std::string& publicID) {

	for (size_t i = 0; i < _objects->size(); ++i)
		if ( _objects->at(i).second->publicID() == publicID )
		    return _objects->at(i).second;

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString EventListWidget::getSeismicCode(const QString& eventID) {

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thSCODE))
		        && _treeModel.item(i, getHeaderPosition(thPUBLIC_ID)) ) {
			if ( _treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text() == eventID )
			    return _treeModel.item(i, getHeaderPosition(thSCODE))->text();
		}

	return QString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QVector<QPair<QString, bool> >
EventListWidget::eventsCheckStateVector() {

	QVector<QPair<QString, bool> > list;

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thOT_GMT))
		        && _treeModel.item(i, getHeaderPosition(thPUBLIC_ID)) ) {
			bool checked = false;
			if ( _treeModel.item(i, getHeaderPosition(thOT_GMT))->checkState() == Qt::Checked
			        && !_treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text().isEmpty() )
			    checked = true;
			QPair<QString, bool> p(_treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text(), checked);
			list.append(p);
		}

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QMap<QString, bool> EventListWidget::eventsCheckStateMap() {

	QMap<QString, bool> list;

//	if ( !_settings.eventsAreCheckable )
//		return list;

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thOT_GMT))
		        && _treeModel.item(i, getHeaderPosition(thPUBLIC_ID)) ) {
			bool checked = false;
			if ( _treeModel.item(i, getHeaderPosition(thOT_GMT))->checkState() == Qt::Checked
			        && !_treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text().isEmpty() )
			    checked = true;
			list.insert(_treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text(), checked);
		}

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventListWidget::objectIsInList(int* position,
                                     const std::string& publicID) const {

	if ( _objects->size() == 0 )
	    return false;

	for (size_t i = 0; i < _objects->size(); ++i)
		if ( (_objects->at(i).first->publicID() == publicID)
		        || (_objects->at(i).second->publicID() == publicID) ) {
			position = (int*) i;
			return true;
		}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPair<QDateTime, QDateTime> EventListWidget::range() const {
	return QPair<QDateTime, QDateTime>(_ui->dateTimeEdit_start->dateTime(),
	    _ui->dateTimeEdit_end->dateTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setRange(const size_t& days) {

	int dayspan = days * 3600 * 24;
	Time end = Time::GMT();
	TimeSpan ts;
	long timespan = end.seconds() - dayspan;
	ts.set(timespan);

	Time start = (Time) ts;
	int y, m, d, h, min, s, us;
	start.get(&y, &m, &d, &h, &min, &s, &us);
	QDate sda;
	sda.setDate(y, m, d);

	QDateTime ed = QDateTime::currentDateTimeUtc();
	QDateTime sd;
	sd.setDate(sda);
	sd.setTime(ed.time());

	_ui->dateTimeEdit_start->setDateTime(sd);
	_ui->dateTimeEdit_end->setDateTime(ed);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setRange(const QDateTime& from, const QDateTime& to) {
	_ui->dateTimeEdit_start->setDateTime(from);
	_ui->dateTimeEdit_end->setDateTime(to);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::setFiltersVisible(bool visible) {
	if ( visible || _ui->toolButton->arrowType() == Qt::RightArrow )
		_ui->frame->setVisible(true), _ui->toolButton->setArrowType(Qt::DownArrow);
	else
		_ui->frame->setVisible(false), _ui->toolButton->setArrowType(Qt::RightArrow);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::reorganizeModel() {

	showHideTreeViewHeaderItems();

	_ui->treeView->sortByColumn(0, Qt::DescendingOrder);

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thOT_GMT)) )
		    _treeModel.item(i, getHeaderPosition(thOT_GMT))->setCheckable(_settings.eventsAreCheckable);

	for (int i = 0; i < _treeModel.columnCount(); ++i)
		_ui->treeView->resizeColumnToContents(i);

	if ( _events.size() != _origins.size() )
	    return;

	for (int i = 0; i < _events.size(); ++i)
		_objects->push_back(ParentedOrigin(_origins.at(i), _events.at(i)));

	QApplication::processEvents();

	log(true, QString("%1 object(s) cached").arg(_cache->size()));

	_ui->originCounter->setText(QString("%1 event(s)").arg(_objects->size()));
	_ui->label_information->setText("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::selectItem(const QString& publicID) {

	EventPtr event = _cache->getObjectByOwnID<EventPtr>(publicID.toStdString());

	QString eventID;
	if ( event )
		eventID = publicID;
	else
		eventID = _cache->getParentID(publicID.toStdString()).c_str();

	if ( eventID.isEmpty() ) {
		log(false, QString("Object with publicID %1 not found in cache").arg(publicID));

		//! Go back to publicID value... It might be possible that the object
		//! pointer has been destroyed and its resource freed by the smart
		//! pointer handling mechanism. Therefore if the object itself isn't
		//! stored in the cache anymore, there is a good chance that its entry
		//! might still be in the model and we should try and locate it.
		//! It's good for us to signal this in the log anyway... Although this
		//! behavior might be the usual suspect in some dangling pointers due to
		//! anticipated resource release...
		//! TODO: try and investigate thoroughly on this matter!
		eventID = publicID;
	}

	bool found = false;
	int pos = -1;
	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text() == eventID )
		    pos = i, found = true;

	if ( !found ) {
		log(false, QString("Couldn't find object with publicID %1 in the list").arg(eventID));
		return;
	}

	_ui->treeView->selectionModel()->clearSelection();
	QModelIndex idx = _ui->treeView->model()->index(pos, getHeaderPosition(thOT_GMT));
	_ui->treeView->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::uncheckAll() {

	if ( !_settings.eventsAreCheckable )
	    return;

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thOT_GMT)) )
		    _treeModel.item(i, getHeaderPosition(thOT_GMT))->setCheckState(Qt::Unchecked);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::
setItemsChecked(const QList<EvaluationModeSelection>& list) {

	if ( !_settings.eventsAreCheckable )
	    return;

	// Uncheck everything first
	for (int i = 0; i < _treeModel.rowCount(); ++i) {
		if ( !_treeModel.item(i, getHeaderPosition(thOT_GMT)) )
		    continue;
		_treeModel.item(i, getHeaderPosition(thOT_GMT))->setCheckState(Qt::Unchecked);
	}

	const int col = getHeaderPosition(thSTATUS);
	QVector<int> rows;
	for (int x = 0; x < list.size(); ++x)
		for (int i = 0; i < _treeModel.rowCount(); ++i) {

			if ( !_treeModel.item(i, getHeaderPosition(thOT_GMT)) )
			    continue;

			if ( (_treeModel.item(i, col)->text().contains("M") && list.at(x) == emsMANUAL)
			        or (_treeModel.item(i, col)->text().contains("C") && list.at(x) == emsCONFIRMED)
			        or (_treeModel.item(i, col)->text().contains("A") && list.at(x) == emsAUTOMATIC)
			        or (list.at(x) == emsALL) )
			    rows << i;
		}

	for (int i = 0; i < rows.size(); ++i)
		_treeModel.item(rows.at(i), getHeaderPosition(thOT_GMT))->setCheckState(Qt::Checked);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::colorizeEventsRow() {

	if ( !_settings.showEventColorType )
	    return;

	/*
	 for (int i = 0; i < _treeModel.rowCount(); ++i)
	 if ( _treeModel.item(i, 0) && _treeModel.item(i, 1) ) {
	 EventType t = _treeModel.item(i, 1)->data(Qt::UserRole).
	 value<EventType>();
	 if ( t == type )
	 _treeModel.item(i, 0)->setCheckState(Qt::Checked);
	 }
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideUnlocalizedEvents(const bool& hide) {
	hideItemsOfType("not locatable", hide);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideExternalEvents(const bool& hide) {
	hideItemsOfType("outside of network interest", hide);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideNoTypeSetEvents(const bool& hide) {
	hideItemsOfType("-", hide);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideFakeEvents(const bool& hide) {
	hideItemsOfType("not existing", hide);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::regionSelected(const QPointF& lonLatMin,
                                     const QPointF& lonLatMax) {

	_ui->longitudeCheckBox->setChecked(true);
	_ui->latitudeCheckBox->setChecked(true);
	_ui->longitudeMin->setValue(lonLatMin.x());
	_ui->latitudeMin->setValue(lonLatMax.y());
	_ui->longitudeMax->setValue(lonLatMax.x());
	_ui->latitudeMax->setValue(lonLatMin.y());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::hideItemsOfType(const QString& type,
                                      const bool& hide) {
	for (int i = 0; i < _treeModel.rowCount(); ++i)
		if ( _treeModel.item(i, getHeaderPosition(thTYPE))
		        && _treeModel.item(i, getHeaderPosition(thTYPE))->text() == type )
		    _ui->treeView->setRowHidden(i, QModelIndex(), hide);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::highlightItems(const QStringList& list) {

	if ( !_settings.decorateItems )
	    return;

	for (int i = 0; i < _treeModel.rowCount(); ++i)
		for (int j = 0; j < _treeModel.columnCount(); ++j)
			_treeModel.item(i, j)->setBackground(QBrush());

	QStringList events;
	for (int z = 0; z < list.size(); ++z) {

		QString eventID = _cache->getParentID(list.at(z).toStdString()).c_str();

		if ( eventID.isEmpty() )
		    continue;

		events << eventID;
	}

	if ( events.size() == 0 ) {
		_ui->label_information->setText("");
		return;
	}

	_ui->label_information->setText(QString("%1 objects pre-selected").arg(events.size()));

	for (int j = 0; j < events.size(); ++j)
		for (int i = 0; i < _treeModel.rowCount(); ++i)
			if ( _treeModel.item(i, getHeaderPosition(thPUBLIC_ID))->text() == events.at(j) )
			    for (int k = 0; k < _treeModel.columnCount(); ++k)
				    _treeModel.item(i, k)->setBackground(QColor::fromRgb(100, 230, 110, 120));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::startBlinking() {
	_timer.start(_settings.blinkFrequency);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::updateBlinker() {

	(_buttonState) ? _buttonState = false : _buttonState = true;

	QPalette palette(_ui->pushButton_read->palette());
	palette.setColor(QPalette::Button, (_buttonState) ? _color1 : _color2);
	_ui->pushButton_read->setPalette(palette);
	_ui->pushButton_read->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::stopBlinking() {

	if ( !_timer.isActive() )
	    return;

	_timer.stop();

	QPalette palette(_ui->pushButton_read->palette());
	palette.setColor(QPalette::Button, _color1);
	_ui->pushButton_read->setPalette(palette);
	_ui->pushButton_read->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListWidget::log(bool status, const QString& msg) {
	int stat;
	(status) ? stat = Client::LM_OK : stat = Client::LM_ERROR;
	emit logMessage(stat, "EventListWidget", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}// namespace Gui
} // namespace IPGP
