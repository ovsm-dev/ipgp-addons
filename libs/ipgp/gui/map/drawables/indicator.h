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

#ifndef __IPGP_GUI_MAP_INDICATOR_H__
#define __IPGP_GUI_MAP_INDICATOR_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>
#include <ipgp/gui/map/config.h>
#include <qnamespace.h>

class QSize;
class QPointF;

namespace IPGP {
namespace Gui {
namespace Map {

class SC_IPGP_GUI_API Indicator : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Indicator(const Map::ElementPosition& position = Map::MiddleRight) :
				_position(position), _textElideMode(Qt::ElideRight),
				_textAlignement(Qt::AlignCenter), _rectSizeFactor(3) {

			setType(Geometry::d_Indicator);
		}

		virtual ~Indicator() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		const Qt::Alignment& textAlignement() const;
		void setTextAlignement(const Qt::Alignment&);

		Qt::TextElideMode textElideMode() const;
		void setTextElideMode(Qt::TextElideMode);

		Map::ElementPosition position() const;

		/**
		 * Sets the indicator position regarding defined geo coordinates
		 * @note only middle position (top, bottom, left and right) are supported
		 * @param position the indicator position (enum Map::position)
		 */
		void setPosition(Map::ElementPosition position);

		const qint32& rectSizeFactor() const;
		void setRectSizeFactor(const qint32&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Map::ElementPosition _position;
		Qt::TextElideMode _textElideMode;
		Qt::Alignment _textAlignement;
		qint32 _rectSizeFactor;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
