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



#ifndef __IPGP_GUI_APPLICATION_WORLDEARTHQUAKEVIEW_H__
#define __IPGP_GUI_APPLICATION_WORLDEARTHQUAKEVIEW_H__

#include <ipgp/gui/client/application.h>
#include <seiscomp3/core/enumeration.h>
#include <ipgp/gui/datamodel/eventlist/eventlistwidget.h>
#include <QObject>

class QProgressIndicator;

QT_FORWARD_DECLARE_CLASS(QTableWidgetItem);
QT_FORWARD_DECLARE_CLASS(QDialog);
QT_FORWARD_DECLARE_CLASS(QTimer);
QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(QAction);
QT_FORWARD_DECLARE_CLASS(QPushButton);
QT_FORWARD_DECLARE_CLASS(QWebView);

class QuakeEvent;

namespace Ui {
class WorldEarthquakeView;
class ConfigurationDialog;
}

namespace IPGP {
namespace Gui {
DEFINE_IPGP_SMARTPOINTER(MapWidget);
class DataLoadWorker;

namespace Map {
class Geometry;
}

}
}

using namespace Seiscomp::Core;

MAKEENUM(FeedPeriod, EVALUES(PastHour,PastDay,Past7Days,Past30Days),
    ENAMES("Past Hour", "Past Day", "Past 7 Days","Past 30 Days"));


MAKEENUM(EarthquakeFeedType, EVALUES(Significant,M4_5,M2_5,M1_0,All),
    ENAMES("Significant","M4.5+","M2.5+","M1.0+","All"));


class WorldEarthquakeView : public IPGP::Gui::Client::Application {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit WorldEarthquakeView(int&, char**);
		~WorldEarthquakeView();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool run();
		bool initConfiguration();
		void setupGraphicalInterface();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void setupApplicationMenus();
		const QString updateFeedURL();
		void loadWebPage(const QString&, const QString&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void eventSelected(QTableWidgetItem*);
		void paintStations();
		void feedPeriodChanged();
		void feedEarthquakeTypeChanged();
		void showHideMenu(int);
		void requestDataUpdate();
		void settingsChanged();
		void updateError(const QString&);

		void pinEventListItem(const QString&);
		void updateItemStatus(const QString&);

		void databaseMessage(const QString&, const QString&);
		void handleDatabaseState();

		void showDYFI();
		void showShakemap();
		void showPager();

		void closeWebpages();

		void mapElementClicked(const QString&);

		void tableContextMenu(const QPoint&);
		void addOrigin();

		void populateFromWorker();
		void populateFromLocalDatabase();

		void handleSelection();
		void handleHiddenTypes();

		void hideUnlocalizedEvents(const bool&);
		void hideExternalEvents(const bool&);
		void hideNoTypeSetEvents(const bool&);
		void hideFakeEvents(const bool&);

		void clearPins();

		void showEventMap();
		void showEventList();

		void showWaitingWidget();
		void hideWaitingWidget();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void exitRequested();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::WorldEarthquakeView* _ui;
		Ui::ConfigurationDialog* _configDialogUi;
		IPGP::Gui::MapWidget* _map;
		IPGP::Gui::DataLoadWorker* _worker;
		IPGP::Gui::EventListWidget* _eventListWidget;
		QDialog* _configDialog;
		QPushButton* _clearPins;
		QTimer* _timer;
		QLabel* _databaseLabel;
		QAction* _customMenu;
		QProgressIndicator* _pi;

		std::vector<IPGP::Core::Locators> _locators;
		IPGP::Gui::EventListWidget::Settings _eventListSettings;

		FeedPeriod _feedPeriod;
		EarthquakeFeedType _feedType;

		std::string _feed;
		std::string _otherFeed;
		std::string _defaultAuthor;
		std::string _defaultAgency;
		std::string _defaultMethodID;
		std::string _defaultEarthModelID;

		QString _pagerURL;
		QString _shakemapURL;
		QString _dyfiURL;

		QList<QWebView*> _webpages;

		int _refreshDelay;
		bool _autoRefresh;
		bool _exitRequested;
		bool _showNotLocatableOrigins;
		bool _showOutOfNetworkOrigins;
		bool _showOriginsWithNotType;
		bool _showNotExistingOrigins;
		bool _showInventoryComponents;
		bool _useOwnWebViewer;
};

#endif

