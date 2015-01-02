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

#ifndef __IPGP_GUI_DATAMODEL_EVENTLISTWIDGET_H__
#define __IPGP_GUI_DATAMODEL_EVENTLISTWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/core/datamodel/types.h>
#include <ipgp/gui/datamodel/misc.h>
#include <ipgp/gui/client/misc.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <ipgp/core/datamodel/objectcache.h>
#include <QtGui>


namespace Ui {
class EventListWidget;
}

namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
}
}

namespace IPGP {
namespace Gui {


/**
 * @package IPGP::Gui::Widgets
 * @class   EventListWidget
 * @brief   Advanced event list
 *
 * This class offers a custom widget for an advanced listing of origins
 * regrouped in events if they have one, otherwise they are listed in a group
 * named "Unassociated".
 * Custom context menus are available for basic event/origin interactions.
 * The list's model may be populated by delegate worker thru child thread, so
 * the main GUI interface stays interactive.
 * This widget may work with a cache engine if the user provides one, otherwise
 * it will create its own and operate from it.
 */
class SC_IPGP_GUI_API EventListWidget : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(WidgetStatus)
	Q_ENUMS(EvaluationModeSelection)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum WidgetStatus {
			ELW_Idle = 0x01,
			ELW_PopulatingModel = 0x02,
			ELW_DeletingObject = 0x04,
			ELW_Locked = 0x08
		};

		enum EvaluationModeSelection {
			emsALL,
			emsNONE,
			emsMANUAL,      //! Status = 'M'
			emsAUTOMATIC,   //! Status = 'A'
			emsCONFIRMED    //! Status = 'C'
		};


		struct Settings {

				Settings();
				~Settings() {}

				int blinkFrequency;
				int displayedDecimals;
				bool useCacheEngine;
				bool useMultiThreads;
				bool fetchUnassociated;
				bool useAdvancedOptions;
				bool decorateItems;
				bool fetchEventComment;
				bool fetchPreferredOriginComment;
				bool fetchSiblingOriginComment;
				bool fetchUnassociatedOriginComment;
				bool eventsAreCheckable;
				bool showEventColorType;
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit EventListWidget(QWidget* parent = NULL,
		                         Seiscomp::DataModel::DatabaseQuery* query = NULL,
		                         Core::ObjectCache* cache = NULL,
		                         Qt::WFlags = 0);
		~EventListWidget();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void initWidget();
		virtual void keyPressEvent(QKeyEvent*);

		/**
		 * @brief Internal method to hide events which EventType matches
		 *        argument.
		 * @param String type of EventType ('-' is used for events with no type set)
		 * @param Bool to hide/show items
		 */
		void hideItemsOfType(const QString&, const bool&);

		/**
		 * @brief Populates the model with objects from the database.
		 * @param from The start datetime
		 * @param to The end datetime
		 */
		void addItems(const Seiscomp::Core::Time& from,
		              const Seiscomp::Core::Time& to);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);

		void setCache(Core::ObjectCache*);
		Core::ObjectCache* cache() const {
			return _cache;
		}

		//! @return What's the current events range ?
		QPair<QDateTime, QDateTime> range() const;

		/**
		 * @brief Sets the timespan in days back from now (Time::GMT()) in
		 *        which events shall be fetched from the database.
		 * @param days The number of days to fetch
		 * @note  This method only set the timespan in the QDateTimeEdit
		 *        widgets and the user has to interact to refresh the list.
		 */
		void setRange(const size_t& days);

		/**
		 * @brief Sets the time range in which events shall be fetched from
		 *        the database.
		 * @param from The start datetime
		 * @param to The end datetime
		 * @note  This method only set the timespan in the QDateTimeEdit
		 *        widgets and the user has to interact to refresh the list.
		 */
		void setRange(const QDateTime& from, const QDateTime& to);


		Core::OriginList* originList() const {
			return _objects;
		}

		Seiscomp::DataModel::EventPtr getEvent(const std::string&);
		Seiscomp::DataModel::OriginPtr getOrigin(const std::string&);
		const QString getSeismicCode(const QString& eventID);

		/**
		 * @brief A list of checked events
		 * @note  The user has to copy it first and shall not try to iterate
		 *        over it, it will result in memory dump otherwise.
		 *        On another matter, the list is sorted by event publicID.
		 */
		const QMap<QString, bool> eventsCheckStateMap();
		const QVector<QPair<QString, bool> > eventsCheckStateVector();
		bool objectIsInList(int* position, const std::string& publicID) const;

		const WidgetStatus& widgetStatus() const {
			return _ws;
		}

		void setLocationProfiles(const std::vector<Core::Locators>&);
		void setSettings(const Settings&);
		const Settings& settings() const {
			return _settings;
		}

		void startBlinking();
		void stopBlinking();

		void setLayoutMargins(const int&, const int&, const int&, const int&);

		void setSelectRegionWithMouse(const bool&);

		/**
		 * @brief Locks the widget's read button.
		 * This method is to be called whenever the cache engine isn't
		 * available for querying objects, e.g. objects are being updated, so
		 * the read button gets locked and awaits the pending operation.
		 */
		void lockReading();

		/**
		 * @brief Unlocks the widget's read button.
		 * This method is to be called whenever the cache engine gets available
		 * for querying objects.
		 */
		void unlockReading();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void clear();
		void readEvents();
		void readEvents(const QDateTime& from, const QDateTime& to);

		void selectItem(const QString&);
		void highlightItems(const QStringList&);

		/**
		 * @brief Un-checks checked items.
		 * @note  Works only if items are checkable in the first place...
		 */
		void uncheckAll();

		/**
		 * @brief Checks items.
		 * @param Type of events to be checked
		 * @note  By passing an empty list, the eventlist will be reseted
		 *        and all items will be 'Unchecked'.
		 */
		void
		setItemsChecked(const QList<EventListWidget::EvaluationModeSelection>&);

		/**
		 * @brief Makes singularities pop out of the list by coloring row's
		 *        background with predefined color for events types.
		 */
		void colorizeEventsRow();

		void hideUnlocalizedEvents(const bool& hide = true);
		void hideExternalEvents(const bool& hide = true);
		void hideNoTypeSetEvents(const bool& hide = true);
		void hideFakeEvents(const bool& hide = true);

		void regionSelected(const QPointF& lonLatMin, const QPointF& lonLatMax);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void setFiltersVisible(bool);

		void reorganizeModel();
		void delegateFinished();

		void treeViewMenu(const QPoint&);
		void rowClicked(const QModelIndex&);
		void rowDoubleClicked(const QModelIndex&);

		void deleteEvent();
		void deleteOrigin();
		void showEventProperties();
		void showOriginProperties();
		void splitOrigin();
		void copyRow();
		void copyCell();

		void updateBlinker();

		void scodeEditionFinished();

		void locationMethodChanged(int);
		void hideFakeEventsClicked();
		void hideUnlocalizedEventsClicked();

		void treeViewHeaderMenu(const QPoint&);
		void showHideTreeViewHeaderItems();

		void widgetStatusError();

		void log(bool, const QString&);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void viewUpdateRequested();
		void viewUpdated();
		void rowClicked(const QString&);
		void rowDoubleClicked(const QString&);
		void objectDeleted(const QString&);
		void logMessage(const int&, const QString&, const QString&);
		void updateProgress(const int&, const QString&, const QString&);
		void interruptDelegateRun();
		void enableRegionFilterByMouse();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::EventListWidget* _ui;

		// Plain C pointer to preserve query instance since this object shall
		// not be deleted by any methods of this class whatsoever
		Seiscomp::DataModel::DatabaseQuery* _query;

		Core::ObjectCache* _cache;

		WidgetStatus _ws;
		int _displayedDecimals;
		QStandardItemModel _treeModel;
		QAction* _deleteEventAction;
		QAction* _deleteOriginAction;
		QAction* _eventPropertyAction;
		QAction* _originPropertyAction;
		QAction* _copyRowAction;
		QAction* _copyCellAction;
		QAction* _splitOriginAction;
		QAction* _otAction;
		QAction* _certaintyAction;
		QAction* _typeAction;
		QAction* _seismicCodeAction;
		QAction* _magnitudeAction;
		QAction* _tpAction;
		QAction* _qualityAction;
		QAction* _locatorAction;
		QAction* _phasesAction;
		QAction* _latitudeAction;
		QAction* _longitudeAction;
		QAction* _depthAction;
		QAction* _statusAction;
		QAction* _agencyAction;
		QAction* _authorAction;
		QAction* _regionAction;
		QAction* _idAction;
		QStringList _objectIDs;
		Core::OriginList* _objects;
		QList<Seiscomp::DataModel::EventPtr> _events;
		QList<Seiscomp::DataModel::OriginPtr> _origins;
		QTimer _timer;
		QColor _color1;
		QColor _color2;
		bool _buttonState;
		bool _hasOwnCache;
		std::vector<Core::Locators> _locators;
		Settings _settings;
};

} // namespace Gui
} // namespace IPGP

#endif
