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


#include <ipgp/gui/map/util.h>
#include <ipgp/gui/map/config.h>

using namespace std;

namespace IPGP {
namespace Gui {
namespace Map {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal PPLDAZL[] = {
                          .7111111111111111111,
                          1.42222222222222222222,
                          2.84444444444444444444,
                          5.68888888888888888888,
                          11.37777777777777777776,
                          22.75555555555555555552,
                          45.51111111111111111104,
                          91.02222222222222222208,
                          182.04444444444444444416,
                          364.08888888888888888832,
                          728.17777777777777777664,
                          1456.35555555555555555328,
                          2912.71111111111111110656,
                          5825.42222222222222221312,
                          11650.84444444444444442624,
                          23301.68888888888888885248,
                          46603.37777777777777770496,
                          93206.75555555555555540992,
                          186413.51111111111111081984
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
qreal getDistance(qreal lat1, qreal lon1, qreal lat2, qreal lon2) {

	const qreal a = pow(sin((lat2 - lat1) / 2), 2) + (cos(lat1) * cos(lat2)
	        * pow(sin((lon2 - lon1) / 2), 2));
	const qreal c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return RADIUS_EARTH_KM * c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool validCoordinates(const qreal &lat, const qreal &lon) {


	if ( (abs(lon) > 180.0) || (abs(lat) > MAX_N_S) ) {
		return false;
	}


	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool validCoordinatesTile(const qreal &x, const qreal &y, const int &zoom) {

	if ( (x >= pow2[zoom]) || (y >= pow2[zoom]) || (x < 0) || (y < 0)
	        || (zoom < MIN_ZOOM) || (zoom > MAX_ZOOM) ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool coord2tile(const qreal &lat, const qreal &lon, const int &zoom,
                qreal &x, qreal &y) {

	if ( !validCoordinates(lat, lon) ) {
		return false;
	}

	x = pow(2, zoom) / 360.0 * (lon + 180.0);

	qreal sinLatitude = sin(deg2rad(lat));

	y = (0.5 - log((1.0 + sinLatitude) / (1.0 - sinLatitude)) / (4.0 * M_PI))
	        * pow(2, zoom);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool tile2coord(const qreal& tileX, const qreal& tileY, const int& zoom,
                qreal& lat, qreal& lon) {

	if ( !validCoordinatesTile(tileX, tileY, zoom) ) {
		return false;
	}

	lon = tileX * TILE_SIZE / PPLDAZL[zoom] - 180.0;
	lat = rad2deg(asin((exp((tileY / pow(2, zoom) - 0.5) * -4.0 * M_PI) - 1.0)
	        / (1.0 + exp((tileY / pow(2, zoom) - 0.5) * -4.0 * M_PI))));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



/*
 // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 const bool screen2coord(const QPointF& screenCoords, const QPointF& startTile,
 QPointF& geoCoords, const int& zoomLevel,
 const bool& clipping) {

 QPointF coords;

 if ( !screen2tile(screenCoords, startTile, coords, zoomLevel, clipping) )
 return false;


 qreal lat, lon;
 bool retCode = tile2coord(coords.x(), coords.y(), zoomLevel, lat, lon);

 if ( retCode ) {
 geoCoords.setX(lon);
 geoCoords.setX(lat);
 }

 return retCode;
 }
 // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




 // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 const bool screen2tile(const QPointF& screenCoords, const QPointF& startTile,
 QPointF& tileCoords, const int& zoomLevel,
 const bool& clipping) {

 tileCoords.setX(screenCoords.x() / TILE_SIZE + startTile.x());
 tileCoords.setY(screenCoords.y() / TILE_SIZE + startTile.y());

 if ( !clipping ) {
 if ( tileCoords.x() < 0.0 ) {
 tileCoords.setX(tileCoords.x() + pow2[zoomLevel]);
 }
 else if ( tileCoords.x() > pow2[zoomLevel] ) {
 tileCoords.setX(tileCoords.x() - pow2[zoomLevel]);
 }

 if ( tileCoords.x() < 0.0 ) {
 tileCoords.setX(tileCoords.x() + pow2[zoomLevel]);
 }
 else if ( tileCoords.y() > pow2[zoomLevel] ) {
 tileCoords.setY(tileCoords.y() - pow2[zoomLevel]);
 }
 }

 return true;
 }
 // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
 */



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool tile2screen(const QPointF& tile, QPointF& screenCoords,
                 const QPointF& middleTile, const int& zoomLevel,
                 const bool& clipping) {

	if ( !validCoordinatesTile(tile.x(), tile.y(), zoomLevel) )
		return false;

	screenCoords.setX((tile.x() - middleTile.x()) * TILE_SIZE);
	screenCoords.setY((tile.y() - middleTile.y()) * TILE_SIZE);

	if ( !clipping ) {
		if ( screenCoords.x() < 0.0 )
			screenCoords.setX(screenCoords.x() + pow2[zoomLevel] * TILE_SIZE);

		if ( screenCoords.y() < 0.0 )
			screenCoords.setY(screenCoords.y() + pow2[zoomLevel] * TILE_SIZE);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool coord2screen(const QPointF& decGeoCoords, QPointF& screenCoords,
                  const QPointF& middleTile, const int& zoomLevel,
                  const bool& clipping) {

	qreal x;
	qreal y;

	if ( !coord2tile(decGeoCoords.y(), decGeoCoords.x(), zoomLevel, x, y) )
		return false;

	return tile2screen(QPointF(x, y), screenCoords, middleTile, zoomLevel, clipping);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Map::ElementPosition fromAtoB(const QPointF a, const QPointF b) {

	Map::ElementPosition pos = Unknown;

	if ( a.x() < b.x() && a.y() < b.y() )
		pos = TopLeft;
	if ( a.x() < b.x() && a.y() > b.y() )
		pos = BottomLeft;
	if ( a.x() > b.x() && a.y() < b.y() )
		pos = TopRight;
	if ( a.x() > b.x() && a.y() > b.y() )
		pos = BottomRight;
	if ( a.x() == b.x() && a.y() == b.y() )
		pos = United;
	if ( a.x() == b.x() && a.y() > b.y() )
		pos = BottomMiddle;
	if ( a.x() == b.x() && a.y() < b.y() )
		pos = TopMiddle;
	if ( a.x() > b.x() && a.y() == b.y() )
		pos = MiddleRight;
	if ( a.x() < b.x() && a.y() == b.y() )
		pos = MiddleLeft;

	return pos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}
