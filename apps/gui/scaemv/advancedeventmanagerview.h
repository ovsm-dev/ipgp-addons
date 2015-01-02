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


#ifndef __IPGP_GUI_APPLICATION_SCAEMV_H__
#define __IPGP_GUI_APPLICATION_SCAEMV_H__

#include <ipgp/gui/defs.h>
#include <QObject>
#include <QList>

#include <ipgp/gui/client/application.h>
#include <ipgp/gui/datamodel/eventlist/eventlistwidget.h>
#include <ipgp/gui/opengl/topographyrenderer.h>
#include <ipgp/core/datamodel/types.h>
#include <ipgp/core/datamodel/objectcache.h>


class QCustomPlot;
class QCPAbstractPlottable;
class QProgressIndicator;

QT_FORWARD_DECLARE_CLASS(QDialog);
QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(QProgressBar);
QT_FORWARD_DECLARE_CLASS(QTimer);

namespace Ui {

class AdvancedEventManagerView;
class ConfigureDialog;

}


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(PlottingWidget);
DEFINE_IPGP_SMARTPOINTER(OriginPlot);
DEFINE_IPGP_SMARTPOINTER(MapWidget);
DEFINE_IPGP_SMARTPOINTER(OriginWidget);
DEFINE_IPGP_SMARTPOINTER(ParticleMotionWidget);
DEFINE_IPGP_SMARTPOINTER(StreamWidget);
DEFINE_IPGP_SMARTPOINTER(ProgressIndicator);

namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(TopographyMap);

}

}
}


namespace Seiscomp {

class Regions;

}


class AdvancedEventManagerView : public IPGP::Gui::Client::Application {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit AdvancedEventManagerView(int&, char**);
		~AdvancedEventManagerView();

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

		void showOriginSummary(Seiscomp::DataModel::OriginPtr);
		void showOriginSummary(const std::string&);

		void updateEventsMap();

		//! Setup PlottingWidget entity, make connections and stuffs
		template<typename T> T* initPlottingWidget(const QString& name);
		//! Fetches PlottingWidget object from local pool
		template<typename T> T* getPlottingWidget();

		//! Setup OriginPlot entity, make connections and stuffs
		IPGP::Gui::OriginPlot* initPlotGraph(const int& type, const QString& name);
		//! Fetches OriginPlot object from local pool
		IPGP::Gui::OriginPlot* getPlotGraph(const int& type);

	protected Q_SLOTS:
		// ------------------------------------------------------------------
		//  Protected Qt interface
		// ------------------------------------------------------------------
		void configAccepted();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		//! Displays object's information in summary widget
		//! @note Works for events and origins only
		void displayObjectInformation(const QString&);

		//! Waiting widget actions
		void showWaitingWidget();
		void hideWaitingWidget();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void showSummary();

		void databaseMessage(const QString& oldDB, const QString& newDB);
		void handleDatabaseState();

		void loadingPercentage(const int&, const QString& entity,
		                       const QString& message);

		void eventListViewUpdateRequested();
		void eventListViewUpdated();

		void updateCacheEngine();

		void showGlobalMap();
		void showTopographyMap();

		void showResidualDistanceGraph();
		void showResidualAzimuthGraph();
		void showResidualTakeOffGraph();
		void showGutenbergRichterGraph();
		void showMagnigtudeVarationGraph();
		void showEventTypeVarationGraph();
		void showHypocenterDriftGraph();
		void showUncertaintyGraph();
		void showMagnitudeDensityGraph();
		void showEventPhaseDurationGraph();
		void showEventEnergyGraph();
		void showWadatiGraph();
		void showStationStreamGraph();
		void showParticleMotionGraph();
		void showCrossSectionWidget();

		void removeCrossSectionFromMap();
		void clearPinsFromMap();
		void addCrossSectionToMap(const QPointF& start, const QPointF& end,
		                          const qreal& depthMin, const qreal& depthMax,
		                          const qreal& trench);

		void selectRegionByMouse();
		void nullifyQObject(QObject*);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void itemSelected(const QString&);
		void itemsHighlighted(const QStringList&);

	private:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QList<IPGP::Gui::PlottingWidget*> WidgetList;
		typedef QList<IPGP::Gui::OriginPlot*> PlotList;
		typedef QHash<QString, IPGP::Gui::OpenGL::TopographyRendererSettings> TopoSettings;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::AdvancedEventManagerView* _ui;
		Ui::ConfigureDialog* _configDialogUi;
		IPGP::Gui::ProgressIndicator* _progressIndicator;

		QDialog* _configDialog;
		QProgressIndicator* _pi;
		QTimer* _timer;

		IPGP::Core::ObjectCache _cache;

		WidgetList _widgetList;
		PlotList _plotList;

		IPGP::Gui::EventListWidget* _eventListWidget;
		IPGP::Gui::OriginWidget* _summaryMap;
		IPGP::Gui::MapWidget* _globalMap;
		IPGP::Gui::OpenGL::TopographyMap* _topoMap;
		TopoSettings _topoSettings;

		IPGP::Gui::StreamWidget* _streamWidget;
		IPGP::Gui::ParticleMotionWidget* _particleMotionWidget;

		bool _useNormalizedEpicenterSize;

		std::string _selectedObjectID;

		Seiscomp::Regions* _region;
		QLabel* _databaseLabel;

		std::vector<IPGP::Core::Locators> _locators;
		IPGP::Core::MagnitudeTypes _magnitudes;
		IPGP::Gui::EventListWidget::Settings _eventListSettings;
};

#endif

