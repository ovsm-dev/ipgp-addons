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

#ifndef __IPGP_GUI_MAP_MOTIONMAP_H__
#define __IPGP_GUI_MAP_MOTIONMAP_H__

#include <QObject>

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/misc.h>
#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/mapdescriptor.hpp>

#include <QList>
#include <QTimer>


namespace Seiscomp {
namespace DataModel {
class Origin;
class DatabaseQuery;
}
}

namespace IPGP {
namespace Gui {

namespace Map {
DEFINE_IPGP_SMARTPOINTER(Arrival);
DEFINE_IPGP_SMARTPOINTER(Station);
DEFINE_IPGP_SMARTPOINTER(City);
DEFINE_IPGP_SMARTPOINTER(Epicenter);

DEFINE_IPGP_SMARTPOINTER(AnimatedCanvas);
/**
 * @class AnimatedCanvas
 * @package IPGP::Gui::Map::Widgets
 * @brief   Animated canvas
 *
 * This custom canvas is meant to be repainted dynamically several times
 * a second and should therefore harbor only small and fast painting elements
 * like basic shapes (circles, triangles, etc).
 */
class SC_IPGP_GUI_API AnimatedCanvas : public Canvas {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		AnimatedCanvas();
		~AnimatedCanvas();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

	public:
		// ------------------------------------------------------------------
		//  Public drawing interface
		// ------------------------------------------------------------------
		void clearGeometries();
		bool addGeometry(Geometry*) throw (GeneralException);
		int addGeometries(const QList<Geometry*>&);
		void removeGeometry(const QString&);
		void removeGeometry(const Geometry::Type&, const QString&);
		void removeGeometries(const Geometry::Type&);
		Geometry* geometry(const QString&) throw (GeneralException);

		const QList<Geometry*>& geometries() const {
			return _geometries;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QList<Geometry*> _geometries;
};

} // namespace Map



DEFINE_IPGP_SMARTPOINTER(MotionMap);
/**
 * @class   MotionMap
 * @package IPGP::Gui::Map::Widgets
 * @brief   Animated motion of an Seiscomp::DataModel::Origin
 *
 * This custom implementation of the MapWidget proposes an automated animation
 * of an Seiscomp::DataModel::Origin thru time. The elementary elements being
 * the epicenter and its stations, the new AnimatedCanvas is painted
 * synchronously with the propagation time travel table obtained from the origin
 * data.
 * @todo   Implements double buffering in order to speed up the repainting
 *         time and reduce the CPU resources used.
 */
class SC_IPGP_GUI_API MotionMap : public MapWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_PROPERTY(int _animationRate READ animationRate WRITE setAnimationRate)

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MotionMap(const MapDescriptor&, QWidget* parent = NULL,
		                   Qt::WFlags f = 0);
		~MotionMap();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void paintEvent(QPaintEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);
		void setOrigin(Seiscomp::DataModel::Origin*);

		void setAnimationRate(const int& r) {
			_animationRate = r;
		}
		const int& animationRate() const {
			return _animationRate;
		}

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setStationsVisible(const bool&);
		void clear();

		void startAnimation();
		void pauseAnimation();
		void stopAnimation();

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
		void updateAnimation();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		/**
		 * @brief Adds a station object on the map
		 * @param the station's pointer
		 * @param should we add it to the canvas directly(true) or in local
		 *        station list (false)
		 */
		bool addStation(Map::Station*, const bool& toCanvas = false);
		Map::Station* getMapStation(const QString&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::DataModel::DatabaseQuery* _query;
		Seiscomp::DataModel::Origin* _origin;
		Map::Epicenter* _epicenter;
		QTimer* _timer;
		Map::AnimatedCanvas _ac;

		double _animationTime;
		int _animationRate;

		QList<Map::Station*> _stations;
};


}
}

#endif
