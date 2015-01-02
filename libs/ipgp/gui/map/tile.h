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


#ifndef __IPGP_GUI_MAP_TILE_H__
#define __IPGP_GUI_MAP_TILE_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/map/config.h>
#include <QString>


namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class Tile
 * @brief This class provides a tile interface
 * @note  A pattern filename can be provided in the TileLayer if the user pattern
 *        doesn't match the default pattern '/%1/osm_%1_%2_%3'. In this example,
 *        %1 stands for the zoom level, %2 the tile's column and %3 the tile's line.
 *        When requesting a tile path, the class will first check if a 'png'
 *        file exists, then a 'jpg'.
 */
class SC_IPGP_GUI_API Tile {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Tile(const int& x = 0,
		              const int& y = 0,
		              const int& z = MIN_ZOOM,
		              const QString& path = QString(),
		              const QString& pattern = QString());

		~Tile();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const int& x() const;
		const int& y() const;
		const int& z() const;
		const QString path() const;
		const QString suffix() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		int _x;
		int _y;
		int _z;
		QString _path;
		QString _pattern;
};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
