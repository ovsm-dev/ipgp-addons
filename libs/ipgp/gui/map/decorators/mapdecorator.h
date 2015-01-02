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



#ifndef __IPGP_GUI_MAPDECORATOR_H__
#define __IPGP_GUI_MAPDECORATOR_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/map/decorator.h>
#include <ipgp/gui/map/mapwidget.h>
#include <QObject>


namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class MapDecorator
 * @brief Each map decorator shall inherit directly from this class if an
 *        instance of the map is wanted.
 */
class SC_IPGP_GUI_API MapDecorator : public Decorator {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MapDecorator(MapWidget* map) :
				_target(map) {}

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		MapWidget* _target;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP
#endif
