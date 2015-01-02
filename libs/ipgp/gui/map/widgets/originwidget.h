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

#ifndef __IPGP_GUI_MAP_ORIGINWIDGET_H__
#define __IPGP_GUI_MAP_ORIGINWIDGET_H__

#include <QObject>

#include <ipgp/gui/api.h>
#include <ipgp/gui/map/mapwidget.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <QList>


namespace IPGP {
namespace Gui {

namespace Map {
DEFINE_IPGP_SMARTPOINTER(Arrival);
DEFINE_IPGP_SMARTPOINTER(Station);
DEFINE_IPGP_SMARTPOINTER(City);
DEFINE_IPGP_SMARTPOINTER(Epicenter);
}

DEFINE_IPGP_SMARTPOINTER(OriginWidget);
/**
 * @class   OriginWidget
 * @package IPGP::Gui
 * @brief   This is a simple map of an Seiscomp::DataModel::Origin
 */
class SC_IPGP_GUI_API OriginWidget : public MapWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		/**
		 * @brief Widget's constructor
		 * @param the list of maps names
		 * @param the list of maps paths
		 * @param the map default longitude
		 * @param the map default latitude
		 * @param parent QWidget
		 * @param QWidget window's flags
		 * @param paintOrphan Defines if an origin without parent (event)
		 *        is allowed to be painted
		 */
		explicit OriginWidget(const std::vector<std::string>&,
		                      const std::vector<std::string>&,
		                      const qreal& lon = .0, const qreal& lat = .0,
		                      QWidget* parent = NULL, Qt::WFlags f = 0,
		                      const bool& paintOrphan = false);

		~OriginWidget();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);
		void setOrigin(Seiscomp::DataModel::Origin*);

		bool addCity(Map::City*, const bool& makeLink = true);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setArrivalsVisible(const bool&);
		void setStationsVisible(const bool&);
		void clear();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void stationClicked(const std::string&);
		void epicenterClicked(const std::string&);
		void cityClicked(const QString&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void mapClicked(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		/**
		 * @brief Adds an arrival object on the map
		 * @param the arrival's pointer
		 * @param should we add it to the canvas directly(true) or in local
		 *        arrival list (false)
		 */
		bool addArrival(Map::Arrival*, const bool& toCanvas = false);
		Map::Arrival* getMapArrival(const QString&, const QString&);

		/**
		 * @brief Adds a station object on the map
		 * @param the station's pointer
		 * @param should we add it to the canvas directly(true) or in local
		 *        arrival list (false)
		 */
		bool addStation(Map::Station*, const bool& toCanvas = false);
		Map::Station* getMapStation(const QString&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::DataModel::DatabaseQueryPtr _query;
		Seiscomp::DataModel::OriginPtr _origin;
		QList<Map::Arrival*> _arrivals;
		QList<Map::Station*> _stations;

		Map::Epicenter* _epicenter;
		bool _paintOrphanOrigin;
};


} // namespace Gui
} // namespace IPGP

#endif
