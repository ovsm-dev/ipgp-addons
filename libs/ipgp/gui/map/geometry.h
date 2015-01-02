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


#ifndef __IPGP_GUI_MAP_GEOMETRY_H__
#define __IPGP_GUI_MAP_GEOMETRY_H__

#include <ipgp/gui/api.h>
#include <QString>
#include <QBrush>
#include <QPen>
#include <QSizeF>
#include <QPointF>
#include <QRect>
#include <QPainter>


namespace IPGP {
namespace Gui {
namespace Map {



/**
 * @class   Geometry
 * @package IPGP::Gui::Map
 * @brief   Geometry macro for any kind of geometry.
 *
 * If an element has to be painted on the map, it is highly recommended that
 * this item inherits from this class first.
 */
class SC_IPGP_GUI_API Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Type {
			Circle = 0x0001,
			Line = 0x0002,
			Point = 0x0004,
			Arc = 0x0008,
			Rectangle = 0x0010,
			Triangle = 0x0020,
			Star = 0x0040,
			d_Arrival = 0x0080,
			d_Epicenter = 0x0100,
			d_City = 0x0200,
			d_Station = 0x0400,
			d_Indicator = 0x0800,
			d_CrossSection = 0x1000,
			d_Plate = 0x1001,
			d_Pin = 0x1002,
			Unknown = 0xFFFF
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		/**
		 * @brief Geometry class constructor!
		 *        This class offers a basic implementation of items the user
		 *        wants to paint on the MapWidget. It therefore offers various
		 *        key elements that should be carefully handled.
		 * @param name The name the geometry. It should always be properly
		 *        registered by the user.
		 * @param geopPosition The geographic position of the item
		 * @param screenPosition The position on the screen of the item
		 *        (It will be modified each time the map central position changes)
		 * @param toolTip The tooltip of the item
		 * @param type The type of the item
		 * @param brush The brush of the item
		 * @param pen The pen of the item
		 * @param size The size of the item
		 * @param opacity The opacity of the item
		 * @param antialiased Defines if the item should be drawn with anti-aliasing
		 * @param visible Defines if the item is visible
		 * @param sizeInKM Defines if the item size is in KM unit or in pixels
		 */
		explicit Geometry(const QString& name = QString(),
		                  const QPointF& geoPosition = QPointF(.0, .0),
		                  const QPointF& screenPosition = QPointF(.0, .0),
		                  const QString& toolTip = QString(),
		                  const Geometry::Type& type = Geometry::Unknown,
		                  const QBrush& brush = QBrush(),
		                  const QPen& pen = QPen(),
		                  const QSizeF& size = QSizeF(),
		                  const qreal& opacity = 1.,
		                  const bool& antialiased = false,
		                  const bool& visible = true,
		                  const bool& sizeInKM = true);

		virtual ~Geometry();

	public:
		// ------------------------------------------------------------------
		//  Operators
		// ------------------------------------------------------------------
		/**
		 * @brief  Compares the given geometry and see if it is the same as
		 *         this geometry object by checking on its name, its type and
		 *         finally its geoposition.
		 * @param  item The geometry to be checked
		 * @return true if the geometries match, false otherwise
		 */
		bool operator==(const Geometry&) const;

		//! @see   See operator==
		bool operator!=(const Geometry&) const;

		/**
		 * @brief  Checks if the current geometry is valid by verifying if
		 *         it has a proper name. Geometries should always have a name.
		 * @return true if valid, false otherwise
		 */
		bool operator!() const;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Drawing virtual method.
		 * @note  Each Geometry item has to re-implement this method so it can
		 *        draw itself. Otherwise it will not be painted at all.
		 * @param painter the main widget painter reference
		 * @param first tile coordinates
		 * @param viewport of the main widget
		 * @param zoom level of the main widget
		 */
		virtual void draw(QPainter&, const QPointF&, const QRect&, const int&)=0;

		QBrush& brush();
		void setBrush(const QBrush&);

		const QPointF& geoPosition() const;
		void setGeoPosition(const QPointF&);

		const QString& name() const;
		void setName(const QString&);

		QPen& pen();
		void setPen(const QPen&);

		const QPointF& screenPosition() const;
		void setScreenPosition(const QPointF&);

		const QString& toolTip() const;
		void setToolTip(const QString&);

		const QSizeF& size() const;
		void setSize(const QSizeF&);

		const QSizeF& screenSize() const;

		Geometry::Type type() const;
		void setType(const Geometry::Type&);

		const bool& isVisible() const;
		void setVisible(bool visible);

		const bool& isAntialiased() const;
		void setAntialiased(const bool&);

		const qreal& opacity() const;
		void setOpacity(const qreal&);

		const bool& isSizeInKm() const;
		void useSizeInKm(const bool&);

		const bool& highlight() const;
		void setHighlighted(const bool&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const bool touches(const Geometry*) const;
		const bool contains(const Geometry*) const;

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString _name;
		QPointF _geoPosition;
		QPointF _screenPosition;
		QString _toolTip;
		Geometry::Type _type;
		QBrush _brush;
		QPen _pen;
		QSizeF _size;
		QSizeF _screenSize;
		QSizeF _highlightedSize;
		qreal _opacity;
		bool _antialiased;
		bool _visible;
		bool _sizeInKM;
		bool _highlight;
};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
