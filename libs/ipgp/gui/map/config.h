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


#ifndef __IPGP_GUI_MAP_CONFIG_H__
#define __IPGP_GUI_MAP_CONFIG_H__

#include <ipgp/gui/api.h>
#include <QString>
#include <QRect>
#include <QPointF>
#include <QSize>
#include <QPainter>
#include <vector>
#include <string>

namespace IPGP {
namespace Gui {
namespace Map {

const double RADIUS_EARTH_KM = 6371.;
const double MAX_N_S = 85.05112878;
const double WGS84CELLSIZE = 0.597164;

const int MIN_ZOOM = 3;
const int MAX_ZOOM = 14;
const int BUTTON_PADDING = 5;
const int ZOOM_BUTTON_SIZE = 30;
const int TILE_OFFSET = 2;
const int TILE_SIZE = 256;
const int GRATICULE_TEXT_OFFSET = 2;


const std::string DEFAULT_TILE_PATTERN = "%1/osm_%1_%2_%3";



/**
 * @brief The maximum tile X or Y at a given zoom level (Tile Max X Y, TMXY)
 *        which should be in range from 0 to 19. (1-18)
 * @see   http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
 */
const int TMXY[] = {
                     0,
                     1,
                     3,
                     7,
                     15,
                     31,
                     63,
                     127,
                     255,
                     511,
                     1023,
                     2047,
                     4095,
                     8191,
                     16383,
                     32767,
                     65535,
                     131071,
                     262143
};


//! Powers of 2
const int pow2[] = {
                     1,
                     2,
                     4,
                     8,
                     16,
                     32,
                     64,
                     128,
                     256,
                     512,
                     1024,
                     2048,
                     4096,
                     8192,
                     16384,
                     32768,
                     65536,
                     131072,
                     262144
};



struct SC_IPGP_GUI_API MapSettings {

		MapSettings() {
			interactive = true;
			showTileset = false;
			showLayers = true;
			paintDefaultBackground = false;

			defaultZoomLevel = 3;
			defaultOptionalObjectsPadding = 5;
			defaultLatitude = defaultLongitude = defaultMagnification = .0;
			defaultTilePattern = "%1/osm_%1_%2_%3";
		}

		~MapSettings() {}

		QString tilePath;
		QString tilePattern;
		QColor defaultBackground;
		bool showLayers;
		bool interactive;
		bool showTileset;
		bool paintDefaultBackground;

		int defaultZoomLevel;
		int defaultOptionalObjectsPadding;
		qreal defaultLatitude;
		qreal defaultLongitude;
		qreal defaultMagnification;
		std::vector<std::string> mapNames;
		std::vector<std::string> mapPaths;
		QString defaultTilePattern;
};

enum SC_IPGP_GUI_API ElementPosition {
	TopLeft,
	TopMiddle,
	TopRight,
	MiddleLeft,
	Middle,
	MiddleRight,
	BottomLeft,
	BottomMiddle,
	BottomRight,
	United,
	Unknown
};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif

