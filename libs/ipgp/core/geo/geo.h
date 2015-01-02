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


#ifndef __IPGP_CORE_GEO_H__
#define __IPGP_CORE_GEO_H__

#include <ipgp/core/api.h>
#include <iosfwd>
#include <string>
#include <math.h>


namespace IPGP {
namespace Core {
namespace Geo {

enum SC_IPGP_CORE_API GeographicCoordinate {
	Latitude, Longitude
};

static const double GD_semiMajorAxis = 6378137.0;
static const double GD_TranMercB = 6356752.314245;
static const double GD_geocentF = 0.003352810664;

// The usual PI/180 constant
static const double DEG_TO_RAD = 0.017453292519943295769236907684886;

// Earth's quatratic mean radius for WGS-84
static const double EARTH_RADIUS_IN_METERS = 6372797.560856;

// PI constant
static const double compassPi = 4.0 * atan(1.0);


/**
 * @brief Converts degrees to Radians
 * @param x value degrees
 * @return radian value of degrees
 */
const double SC_IPGP_CORE_API degreesToRadians(const double& x);

/**
 * @brief Converts radian to degrees
 * @param x value radians
 * @return degrees value of radian
 */
const double SC_IPGP_CORE_API radiansToDegrees(const double& x);

/**
 * @brief Calculates distance between to latitudes and longitudes
 * @param lon1 A point longitude in degrees
 * @param lat1 A point latitude in degrees
 * @param lon2 B point longitude in degrees
 * @param lat2 B point latitude in degrees
 * @return double value of distance between A and B points in km
 */
double SC_IPGP_CORE_API distanceBetweenCoordinates(const double& lon1, const double& lat1,
                                  const double& lon2, const double& lat2);

/**
 * @brief Computes the arc, in radian, between two WGS-84 positions.
 * The result is equal to <code>Distance(from,to)/EARTH_RADIUS_IN_METERS</code>
 *    <code>= 2*asin(sqrt(h(d/EARTH_RADIUS_IN_METERS )))</code>
 * where:<ul>
 *    <li>d is the distance in meters between 'from' and 'to' positions.</li>
 *    <li>h is the haversine function: <code>h(x)=sin²(x/2)</code></li>
 * </ul>
 * The haversine formula gives:
 *    <code>h(d/R) = h(from.lat-to.lat)+h(from.lon-to.lon)+cos(from.lat)*cos(to.lat)</code>
 * @sa http://en.wikipedia.org/wiki/Law_of_haversines
 */
double SC_IPGP_CORE_API arcInRadians(const double& lat1, const double& lon1,
                    const double& lat2, const double& lon2);

/**
 * @brief Computes the distance, in meters, between two WGS-84 positions.
 * The result is equal to <code>EARTH_RADIUS_IN_METERS*ArcInRadians(from,to)</code>
 * @sa ArcInRadians
 * @return km values of m input
 */
double SC_IPGP_CORE_API distanceInMeters(const double& lat1, const double& lon1,
                        const double& lat2, const double& lon2);




/**
 * @brief Calculates the angle from point A to point B using x axis as reference.
 * The value [-180/180] start with North pole as 0°.\n
 * See Haversine formula! http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1 A point latitude
 * @param lon1 A point longitude
 * @param lat2 B point latitude
 * @param lon2 B point longitude
 * @return bearing angle in degrees (double)
 */
const double SC_IPGP_CORE_API bearingCalculate(double lat1, double lon1,
                              double lat2, double lon2);



/**
 * @brief Gives compass orientation from point A to point B
 * @param bearing the azimuth value [-180 to 180]
 * @param useShortString return value as short version ?
 * @return std::string value of bearing
 */
const std::string SC_IPGP_CORE_API bearingDegreesToDirectionString(const double& bearing,
                                                  bool useShortString = false);


/**
 * @brief Gives intel about relative position of B point to A point
 * @param lat1 A point latitude
 * @param lon1 A point longitude
 * @param lat2 B point latitude
 * @param lon2 B point longitude
 * @return info as std::string
 */
std::string SC_IPGP_CORE_API getPositionStatus(const double& lat1, const double& lon1,
                              const double& lat2, const double& lon2);


void SC_IPGP_CORE_API geodeticOffsetInv(const double& refLat, const double& refLon,
                       const double& lat, const double& lon, double& xOffset,
                       double& yOffset);

/**
 * @brief Converts decimal lat/lon into sexagesimal values deg-min.dec
 * @param value lat/lon as decimal
 * @return (string) sexagesimal value of decimal input
 **/
std::string SC_IPGP_CORE_API decimalToSexagesimal(const double value);


/**
 * @brief Converts sexagesimal coordinates to decimal coordinates
 * @param deg degrees value
 * @param min minutes value
 * @param pol the polar situation (S/W or s/w)
 * @return the converted decimal string value of sexagesimal input
 */
const std::string SC_IPGP_CORE_API sexagesimalToDecimal(const double deg, const double min,
                                       std::string pol);


const std::string SC_IPGP_CORE_API msk2str(double d);


const std::string SC_IPGP_CORE_API doubleToRoman(const double m);

/**
 * @brief Evaluates the cardinal string position (N,S,E,W) of value
 * @param value the decimal value of latitude/longitude
 * @param pos Latitude/Longitude
 * @return lat/lon with N/S/E/W as a string
 */
const std::string SC_IPGP_CORE_API getStringPosition(const double& value,
                                    const enum GeographicCoordinate pos);

const std::string SC_IPGP_CORE_API getCardLetter(const double& value, const enum GeographicCoordinate pos);


} // end namespace geo
} // end namespace core
} // end namespace ipgp


#endif

