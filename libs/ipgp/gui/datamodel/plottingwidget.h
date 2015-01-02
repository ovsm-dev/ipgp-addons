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


#ifndef __IPGP_GUI_DATAMODEL_PLOTTINGWIDGET_H__
#define __IPGP_GUI_DATAMODEL_PLOTTINGWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QWidget>
#include <QTimer>
#include <QList>
#include <ipgp/gui/datamodel/toolbox/toolbox.h>
#include <ipgp/core/datamodel/types.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <ipgp/core/datamodel/objectcache.h>


namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
}
}

namespace IPGP {
namespace Gui {

/**
 * @class   PlottingWidget
 * @package IPGP::Gui::DataModel
 * @brief   Plotting widget macro
 *
 * This class is a macro for QWidgets whom jobs is to paint, plot, etc by using
 * a custom dataset (OriginList).
 * Retrieving objects may be done using a cache engine if objects have already
 * been fetched from database.
 */
class SC_IPGP_GUI_API PlottingWidget : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(WidgetBehaviour)

	Q_PROPERTY(int _blinkFrequency READ blinkFrequency WRITE setBlinkFrequency)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum WidgetBeahaviour {
			WB_Default,			  //! Widget accepts event
			WB_WidgetCloses,	  //! Widget closes
			WB_WidgetHidesOnClose //! Widget ignores event and hides [systray ?]
		};
		typedef QList<Seiscomp::DataModel::PickPtr> PickList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit PlottingWidget(Seiscomp::DataModel::DatabaseQuery*,
		                        QWidget* parent = NULL, Qt::WFlags = 0);
		virtual ~PlottingWidget();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		virtual void closeEvent(QCloseEvent*);
		virtual bool loadSettings() {
			return true;
		}

		Seiscomp::DataModel::PickPtr getPick(const std::string& publicID,
		                                     const bool& loadArrivals = false);
		Seiscomp::DataModel::PickPtr getPick(const std::string& networkCode,
		                                     const std::string& stationCode,
		                                     const std::string& phaseCode,
		                                     const bool& loadArrivals = false);
		PickList getPicks(Seiscomp::DataModel::OriginPtr,
		                  const std::string& phaseCode,
		                  const bool& loadArrivals = false);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setDatabase(Seiscomp::DataModel::DatabaseQuery* q) {
			_query = q;
		}

		/**
		 * @brief Sets the objects list by reference or by copy.
		 * @param OriginList the list
		 * @param bool copy the list or not
		 * @note  Copying the list allows to use data even if the reference
		 *        list is null or empty, but you'll have to reset it every
		 *        time the reference list changes.
		 * @note  The user may reimplement this method to perform operations
		 *        right after the event list has been setup.
		 */
		virtual void setEvents(Core::OriginList*, const bool& copy = false);
		Core::OriginList* originList() const {
			return _events;
		}

		/**
		 * @brief Sets the cache engine by reference or by copy
		 * @param ObjectCache the cache
		 * @param bool copy the cache or not
		 * @note  Copying the cache engine allows to use data even if the
		 *        reference cache is empty or null, but you'll have to reset it
		 *        every time the reference cache changes
		 */
		virtual void setCache(Core::ObjectCache*, const bool& copy = false);
		Core::ObjectCache* cache() const {
			return _cache;
		}

		virtual void setMagnitudeTypes(const Core::MagnitudeTypes&);
		const Core::MagnitudeTypes& magnitudeTypes() const {
			return _magnitudes;
		}


		void setBlinkFrequency(const int&);
		const int& blinkFrequency() const {
			return _blinkFrequency;
		}

		void setWidgetBehaviour(const WidgetBeahaviour& behaviour) {
			_wb = behaviour;
		}
		const WidgetBeahaviour& widgetBehaviour() const {
			return _wb;
		}

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public virtual Qt interface
		// ------------------------------------------------------------------
		virtual void replot()=0;
		virtual void print(const ToolBox::ExportConfig&)=0;

		virtual void startBlinking();
		virtual void stopBlinking();
		virtual void updateBlinker();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private virtual Qt interface
		// ------------------------------------------------------------------
		virtual void setPrintOptionVisible(bool);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void eventListModified();
		void plottingError(const QString&);
		void plottingWarning(const QString&);
		void plottingInfo(const QString&);
		void elementClicked(const QString&);

		void working();
		void idling();

		/**
		 * @brief Signal emitted when this object is loading and an waiting
		 *        box should be displayed...
		 * @param percent The loading percentage (-1=hide, >-1 && <100=show)
		 * @param name The widget's name
		 * @param message The loading message (action performed)
		 */
		void loadingPercentage(const int&, const QString&, const QString&);

		//! Signal emitted when this object is destroyed
		void nullifyQObject(QObject*);

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		// Plain C pointer to preserve query instance since this object shall
		// not be deleted by any methods of this class whatsoever
		Seiscomp::DataModel::DatabaseQuery* _query;
		Core::OriginList* _events;
		Core::ObjectCache* _cache;
		ToolBox* _toolBox;
		Core::MagnitudeTypes _magnitudes;
		bool _listIsACopy;
		bool _cacheIsACopy;

		// Blinking stuff
		QTimer* _timer;
		QColor _color1;
		QColor _color2;
		bool _buttonState;
		int _blinkFrequency;

		WidgetBeahaviour _wb;
};

} // namespace Gui
} // namespace IPGP

#endif
