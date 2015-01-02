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

#ifndef __IPGP_GUI_MAP_PIN_H__
#define __IPGP_GUI_MAP_PIN_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>
#include <ipgp/gui/map/config.h>
#include <qnamespace.h>
#include <QPixmap>
#include <QSize>

class QPointF;

namespace IPGP {
namespace Gui {
namespace Map {

class SC_IPGP_GUI_API Pin : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum PinType {
			PT_GoogleDrop, // pin will be drawn TopRight | TopSemiRight
			PT_Unknown     // pin will be drawn x+width / y+height
		};



	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Pin(const Pin::PinType& pt = PT_GoogleDrop,
		             const Map::ElementPosition& position = Map::TopMiddle,
		             const bool& legendEnabled = true) :
				_pinType(pt), _position(position), _textAlignement(Qt::AlignLeft),
				_rectSizeFactor(4), _legendEnabled(legendEnabled), _shadowEnabled(false) {

			setType(Geometry::d_Pin);
		}

		virtual ~Pin() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		const Qt::Alignment& textAlignement() const;
		void setTextAlignement(const Qt::Alignment& align = Qt::AlignLeft);

		Map::ElementPosition position() const;

		/**
		 * Sets the Pin position regarding defined geo coordinates
		 * @note only middle position (top, bottom, left and right) are supported
		 * @param position the Pin position (enum Map::position)
		 */
		void setPosition(Map::ElementPosition position);

		const qint32& rectSizeFactor() const;
		void setRectSizeFactor(const qint32&);

		const QPixmap& pix() const;
		void setPix(const QPixmap&, const QSize& size = QSize(24, 24));

		const bool& isLegendEnabled() const;
		void setLegendEnabled(const bool&);

		const bool& isShadowEnabled() const;
		void setShadowEnabled(const bool&);


	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Pin::PinType _pinType;
		Map::ElementPosition _position;
		Qt::Alignment _textAlignement;
		qint32 _rectSizeFactor;
		QPixmap _pix;
		QSize _pixSize;
		bool _legendEnabled;
		bool _shadowEnabled;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
