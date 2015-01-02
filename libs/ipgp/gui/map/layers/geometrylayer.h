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

#ifndef __IPGP_GUI_MAP_GEOMETRYLAYER_H__
#define __IPGP_GUI_MAP_GEOMETRYLAYER_H__

#include <ipgp/gui/defs.h>
#include <ipgp/gui/api.h>
#include <ipgp/gui/map/layer.h>
#include <ipgp/gui/map/geometry.h>

#include <QObject>
#include <QList>

QT_BEGIN_NAMESPACE
class QEvent;
class QMouseEvent;
class QString;
class QPainter;
class QPointF;
class QRect;
class QEvent;
QT_END_NAMESPACE


namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class GeometryLayer
 * @brief Provides a layer in which geometries will be organized and painted
 *        on the map. Children's slots and signals connections are handled when
 *        added and when removed from the layer.
 */
class SC_IPGP_GUI_API GeometryLayer : public Layer {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit GeometryLayer(const QString& name = QString(),
		                       const QString& desc = QString(),
		                       const bool& visible = true,
		                       const bool& antialiasing = false);

		~GeometryLayer();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);
		void mouseMoveEvent(QMouseEvent*);

		bool addGeometry(Geometry*);
		void removeGeometries();
		void removeGeometries(const Geometry::Type&);
		void removeGeometry(const Geometry::Type&, const QString&);

		/**
		 * @brief Removes geometries which the name is equal to or contains
		 *        the specified pattern.
		 * @param string The name or name pattern
		 */
		void removeGeometry(const QString&);

		const QList<Geometry*>& geometries() const {
			return _geometries;
		}

		const bool contains(const QString&) const;

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void mouseOnGeometry(QEvent*);
		void geometryClicked(QMouseEvent*);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const Geometry* geometryAt(const QPoint&);
		void removeGeometry(Geometry*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QList<Geometry*> _geometries;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP
#endif
