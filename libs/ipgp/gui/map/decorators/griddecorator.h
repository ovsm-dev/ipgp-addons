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

#ifndef __IPGP_GUI_MAP_GRIDDECORATOR_H__
#define __IPGP_GUI_MAP_GRIDDECORATOR_H__

#include <QObject>
#include <ipgp/gui/api.h>
#include <ipgp/gui/map/decorators/mapdecorator.h>

namespace IPGP {
namespace Gui {

class MapWidget;

namespace Map {

/**
 * @class   GridDecorator
 * @package IPGP::Gui::Map
 * @brief   Provides a grid (graticule) decorator to the map.
 */
class SC_IPGP_GUI_API GridDecorator : public QObject, public MapDecorator {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit GridDecorator(MapWidget*, const bool& visible = true,
		                       const bool& antialiasing = false);
		~GridDecorator();

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		void paintEvent(QPainter&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setGraticules(const QList<int>&);
		void setMoreGraticules(const QList<int>&);

		void setMoreGraticulesEnabled(const bool& enabled = true,
		                              const int& value = 500);
		void setGridPen(const QPen&);
		void setGridFont(const QFont&);

		const bool& isVisible() const;

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
		bool _autoAdjust;
		int _autoAdjustValue;
		QList<int> _graticules;
		QList<int> _moreGraticules;
		QPen _pen;
		QFont _font;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
