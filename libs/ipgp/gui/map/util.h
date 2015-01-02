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



#ifndef __IPGP_GUI_MAP_UTIL_H__
#define __IPGP_GUI_MAP_UTIL_H__

#include <cmath>
#include <QPoint>
#include <QPointF>
#include <QtGlobal>
#include <ipgp/gui/map/config.h>

namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @brief Converts a value in degrees into radians
 * @param deg the value to convert
 * @return the radians value of degrees
 */
inline qreal deg2rad(qreal deg) {
	return deg * M_PI / 180;
}

/**
 * @brief Converts a value in radians into degreesradians
 * @param rad the value to convert
 * @return the degrees value of radians
 */
inline qreal rad2deg(qreal rad) {
	return rad / M_PI * 180;
}

/**
 * @brief Calculates the distance between two points on the surface of the earth.
 * @param lat1 latitude of point A
 * @param lon1 longitude of point A
 * @param lat2 latitude of point B
 * @param lon2 longitude of point B
 * @return Distance in radian from A to B
 */
qreal getDistance(qreal lat1, qreal lon1, qreal lat2, qreal lon2);


/**
 * @brief Coordinates checker
 * @param lat latitude of point A
 * @param lon longitude of point B
 * @return true if coordinate is valid, false otherwise
 */
bool validCoordinates(const qreal &lat, const qreal &lon);

inline bool validCoordinates(const QPointF &pos) {
	return validCoordinates(pos.y(), pos.x());
}

bool validCoordinatesTile(const qreal &x, const qreal &y, const int &zoom);

inline bool validCoordinatesTile(const QPointF &pos, const int &zoom) {
	return validCoordinatesTile(pos.x(), pos.y(), zoom);
}



bool coord2tile(const qreal &lat, const qreal &lon, const int &zoom,
                qreal &x, qreal &y);

bool tile2coord(const qreal &tileX, const qreal &tileY, const int &zoom,
                qreal &lat, qreal &lon);

/*
 const bool screen2coord(const QPointF& screenCoords, const QPointF& startTile,
 QPointF& geoCoords, const int& zoomLevel,
 const bool& clipping = false);

 const bool screen2tile(const QPointF& screenCoords, const QPointF& startTile,
 QPointF& tileCoords, const int& zoomLevel,
 const bool& clipping = false);
 */

/**
 * @brief Converts tile coordinates (x/y) into screen coordinates (x/y)
 * @param tile the tile QPointF(x,y) to convert
 * @param screenCoords the converted QPointF(x,y) reference
 * @param middleTile the middle tile QPointF(x,y)
 * @param zoomLevel the zoom level
 * @param clipping enable clipping this if negative painting is required
 * @return true on success, false otherwise
 */
bool tile2screen(const QPointF& tile, QPointF& screenCoords,
                 const QPointF& middleTile, const int& zoomLevel,
                 const bool& clipping = false);

/**
 * @brief Converts geographic coordinates (lon/lat) into screen coordinates (x/y)
 * @param decGeoCoords the QPointF(lon,lat) to convert
 * @param screenCoords the converted QPointF(x,y) reference
 * @param middleTile the middle tile location (x,y)
 * @param zoomLevel the zoom level
 * @param clipping enable clipping this if negative painting is required
 * @return true on success, false otherwise
 */
bool coord2screen(const QPointF& decGeoCoords, QPointF& screenCoords,
                  const QPointF& middleTile, const int& zoomLevel,
                  const bool& clipping = false);


Map::ElementPosition fromAtoB(const QPointF a, const QPointF b);


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
