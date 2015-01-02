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

#ifndef __IPGP_GUI_MAP_SCALEDECORATOR_H__
#define __IPGP_GUI_MAP_SCALEDECORATOR_H__

#include <QObject>
#include <ipgp/gui/api.h>
#include <ipgp/gui/map/decorators/mapdecorator.h>

namespace IPGP {
namespace Gui {

class MapWidget;

namespace Map {

/**
 * @class   ScaleDecorator
 * @package IPGP::Gui::Map
 * @brief   Provides a simple scale decorator to the map.
 */
class SC_IPGP_GUI_API ScaleDecorator : public QObject, public MapDecorator {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Position {
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ScaleDecorator(MapWidget*, const bool& visible = true,
		                        const bool& antialiasing = false);
		~ScaleDecorator();

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		void paintEvent(QPainter&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setPen(const QPen&);
		void setFont(const QFont&);
		void setPosition(const Position&);
		const bool& isVisible() const;
		const Position& position() const;

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setVisible(const bool&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		bool _antialiased;
		bool _visible;
		QPen _pen;
		QFont _font;
		QList<int> _distances;
		Position _position;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
