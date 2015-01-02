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



#ifndef __IPGP_GUI_MAP_CITY_H__
#define __IPGP_GUI_MAP_CITY_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>
#include <ipgp/gui/map/config.h>
#include <QSizeF>

class QPointF;
class QPen;
class QBrush;
class QSizeF;

namespace IPGP {
namespace Gui {
namespace Map {

class Epicenter;

class SC_IPGP_GUI_API City : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum IndicatorShape {
			is_rectangle,
			is_ellipse,
			is_triangle
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit City(const qint32 population = 0) :
				_population(population), _shape(City::is_rectangle),
				_position(MiddleRight), _paintName(false), _epicenter(0) {
			setType(Geometry::d_City);
		}

		virtual ~City() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		const qint32& population() const;
		void setPopulation(const qint32&);

		const City::IndicatorShape& shape() const;
		void setShape(const City::IndicatorShape&);


		const ElementPosition& position() const;
		void setPosition(const ElementPosition&);

		const bool& isPaintName() const;
		void setPaintName(const bool&);

		void linkToParent(Epicenter*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		qint32 _population;
		City::IndicatorShape _shape;
		ElementPosition _position;
		bool _paintName;
		Epicenter* _epicenter;

};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif

