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

#ifndef __IPGP_GUI_MAP_CITYLAYER_H__
#define __IPGP_GUI_MAP_CITYLAYER_H__

#include <ipgp/gui/defs.h>
#include <ipgp/gui/api.h>
#include <ipgp/gui/map/layer.h>
#include <ipgp/gui/map/drawables/city.h>
#include <vector>
#include <QList>

class QString;
class QPainter;
class QPointF;
class QRect;

namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class CityLayer
 * @brief Provides a layer in which cities can be painted.
 */
class SC_IPGP_GUI_API CityLayer : public Layer {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit CityLayer(const QString& name = QString(),
		                   const QString& desc = QString(),
		                   const bool& visible = true);

		virtual ~CityLayer();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		virtual void draw(QPainter&, const QPointF&, const QRect&, const int&);

		void addCity(City*);
		void removeCity(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void removeCity(City* city);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QList<City*> _cities;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
