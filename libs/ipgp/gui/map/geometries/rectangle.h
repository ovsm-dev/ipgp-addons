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

#ifndef __IPGP_GUI_MAP_RECTANGLE_H__
#define __IPGP_GUI_MAP_RECTANGLE_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/map/geometry.h>
#include <ipgp/gui/map/config.h>

class QPointF;
class QPainter;
class QRect;

namespace IPGP {
namespace Gui {
namespace Map {

class SC_IPGP_GUI_API Rectangle : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Rectangle();
		~Rectangle();


	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
