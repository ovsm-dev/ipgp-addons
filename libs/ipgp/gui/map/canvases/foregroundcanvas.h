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

#ifndef __IPGP_GUI_MAP_FOREGROUNDCANVAS_H__
#define __IPGP_GUI_MAP_FOREGROUNDCANVAS_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/map/canvas.h>
#include <ipgp/gui/map/layers/citylayer.h>
#include <ipgp/gui/map/layers/geometrylayer.h>
#include <QObject>

namespace IPGP {
namespace Gui {
namespace Map {

DEFINE_IPGP_SMARTPOINTER(ForegroundCanvas);

/**
 * @class   ForegroundCanvas
 * @package IPGP::Gui::Map
 * @brief   Foreground canvas implementation
 *
 * A ForegroundCanvas is the shelter of objects that have to appear in the
 * foreground of the widget. Drawables, geometries [etc] should be added
 * inside this canvas. By default, it ships with a GeometryLayer, a CityLayer.
 */
class SC_IPGP_GUI_API ForegroundCanvas : public Canvas {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		ForegroundCanvas();
		~ForegroundCanvas();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);
		void mouseMoveEvent(QMouseEvent*);
		void addCities(const std::vector<Seiscomp::Math::Geo::CityD>&);

	public:
		// ------------------------------------------------------------------
		//  Public drawing interface
		// ------------------------------------------------------------------
		void clearGeometries();

		/**
		 * @brief  Adds a geometry to the GeometryLayer.
		 * @param  geometry Object pointer.
		 * @return true if operation success, false otherwise.
		 */
		bool addGeometry(Geometry*);

		/**
		 * @brief Adds a list of geometries into GeometryLayer.
		 * @param list A list of objects pointers.
		 * @return The number of geometries added
		 */
		size_t addGeometries(const QList<Geometry*>&);

		/**
		 * @brief Removes a geometry from the GeometryLayer.
		 * @param id The name (ID) of the geometry to be removed
		 * @note  This method removes all geometries which the name matches
		 *        the argument ID, and, therefore, is to be used with caution.
		 */
		void removeGeometry(const QString&, const bool& update = true);

		/**
		 * @brief Removes a geometry from the GeometryLayer which type and
		 *        ID match arguments.
		 * @param type The type of the geometry to be removed
		 * @param id The name (ID) of the geometry to be removed
		 */
		void removeGeometry(const Geometry::Type&, const QString&,
		                    const bool& update = true);

		/**
		 * @brief Removes geometries from the GeometryLayer.
		 * @param type The type of the objects that should be removed
		 */
		void removeGeometries(const Geometry::Type&,
		                      const bool& update = true);

		const bool containsGeometry(const QString&) const;

		const QList<Geometry*>& geometries() const {
			return _geometryLayer.geometries();
		}

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void highlightGeometry(const QString&);
		void restoreGeometriesSizes();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QList<Layer*> _layers;
		CityLayer _cityLayer;
		GeometryLayer _geometryLayer;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
