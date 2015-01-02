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



#include <ipgp/core/geo/geo.h>
#include <cstdlib>
#include <stdio.h>
#include <seiscomp3/core/strings.h>
#include <ipgp/core/string/string.h>
#include <iostream>


using namespace Seiscomp;
using namespace Seiscomp::Core;

namespace IPGP {
namespace Core {
namespace Geo {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double degreesToRadians(const double& x) {

	//! PI constant
	//    const double PIx = 3.141592653589793;
	return x * compassPi / 180;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double radiansToDegrees(const double& x) {
	return x * 180 / compassPi;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double distanceBetweenCoordinates(const double& lon1, const double& lat1,
                                  const double& lon2, const double& lat2) {

	// Equatorial radius
	//const double RADIUS = 6378.16;

	//! Mean radius
	const double RADIUS = 6371.00;

	double dlon = degreesToRadians(lon2 - lon1);
	double dlat = degreesToRadians(lat2 - lat1);
	double a = (sin(dlat / 2) * sin(dlat / 2)) + cos(lat1) * cos(lat2)
	        * (sin(dlon / 2) * sin(dlon / 2));
	double angle = 2 * atan2(sqrt(a), sqrt(1 - a));

	return angle * RADIUS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double arcInRadians(const double& lat1, const double& lon1,
                    const double& lat2, const double& lon2) {

	double latitudeArc = (lat2 - lat1) * DEG_TO_RAD;
	double longitudeArc = (lon2 - lon1) * DEG_TO_RAD;

	double latitudeH = sin(latitudeArc * 0.5);
	latitudeH *= latitudeH;

	double lontitudeH = sin(longitudeArc * 0.5);
	lontitudeH *= lontitudeH;

	double tmp = cos(lat2 * DEG_TO_RAD) * cos(lat1 * DEG_TO_RAD);

	return 2.0 * asin(sqrt(latitudeH + tmp * lontitudeH));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double distanceInMeters(const double& lat1, const double& lon1,
                        const double& lat2, const double& lon2) {
	return EARTH_RADIUS_IN_METERS * arcInRadians(lat1, lon1, lat2, lon2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double bearingCalculate(double lat1, double lon1,
                              double lat2, double lon2) {

	double deltaLong = degreesToRadians(lon2 - lon1);
	lat1 = degreesToRadians(lat1);
	lat2 = degreesToRadians(lat2);
	double y = sin(deltaLong) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(deltaLong);
	double result = radiansToDegrees(atan2(y, x));
	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string compassDirection(const double azimuth, bool useShortenString) {

	std::cout << "azi value: " << azimuth << std::endl;

	std::string const shortValue[17] = { "", "E", "ENE", "NE", "NNE", "N",
	                                     "NNW",
	                                     "NW",
	                                     "WNW",
	                                     "W", "WSW", "SW", "SSW", "S",
	                                     "SSE",
	                                     "SE", "ESE" };


	std::string const longValue[17] = { "", "à l'est", "à l'est-nord-est",
	                                    "au nord-est",
	                                    "au nord-nord-est", "au nord",
	                                    "au nord-nord-ouest",
	                                    "au nord-ouest",
	                                    "à l'ouest-nord-ouest",
	                                    "à l'ouest",
	                                    "à l'ouest-sud-ouest",
	                                    "au sud-ouest", "au sud-sud-ouest",
	                                    "au sud",
	                                    "au sud-sud-est",
	                                    "au sud-est",
	                                    "à l'est-sud-est" };


	int x = (int) (azimuth * sizeof(shortValue)) / (2 * compassPi);
	int val = (x % 17) + 1;

	std::cout << "x value: " << x << std::endl;
	std::cout << "val value: " << val << std::endl;

	std::string retValue;
	if ( useShortenString )
		retValue = shortValue[val];
	else
		retValue = longValue[val];

	return retValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string bearingDegreesToDirectionString(const double& bearing,
                                                  bool useShortString) {

	double absResult = ::fabs(bearing);
	int val;
	std::string* array;
	std::string* shortArray;

	array = new std::string[9];
	shortArray = new std::string[9];

	if ( bearing > 0 ) {
		array[0] = "au Nord";
		array[1] = "au Nord-Nord-Est";
		array[2] = "au Nord-Est";
		array[3] = "à Est-Nord-Est";
		array[4] = "à Est";
		array[5] = "à Est-Sud-Est";
		array[6] = "au Sud-Est";
		array[7] = "au Sud-Sud-Est";
		array[8] = "au Sud";

		shortArray[0] = "N";
		shortArray[1] = "NNE";
		shortArray[2] = "NE";
		shortArray[3] = "ENE";
		shortArray[4] = "E";
		shortArray[5] = "ESE";
		shortArray[6] = "SE";
		shortArray[7] = "SSE";
		shortArray[8] = "S";

	}
	else if ( bearing < 0 ) {
		array[0] = "au Nord";
		array[1] = "au Nord-Nord-Ouest";
		array[2] = "au Nord-Ouest";
		array[3] = "à Ouest-Nord-Ouest";
		array[4] = "à Ouest";
		array[5] = "à Ouest-Sud-Ouest";
		array[6] = "au Sud-Ouest";
		array[7] = "au Sud-Sud-Ouest";
		array[8] = "au Sud";

		shortArray[0] = "N";
		shortArray[1] = "NNW";
		shortArray[2] = "N-W";
		shortArray[3] = "WNW";
		shortArray[4] = "W";
		shortArray[5] = "WSW";
		shortArray[6] = "SW";
		shortArray[7] = "SSW";
		shortArray[8] = "S";

	}

	if ( absResult == 0.0 )
		val = 0;
	if ( (absResult > 0.0) && (absResult < 29.99) )
		val = 1;
	if ( (absResult > 29.99) && (absResult < 59.99) )
		val = 2;
	if ( (absResult > 59.99) && (absResult < 89.99) )
		val = 3;
	if ( absResult == 90.0 )
		val = 4;
	if ( (absResult > 89.99) && (absResult < 119.99) )
		val = 5;
	if ( (absResult > 119.99) && (absResult < 149.99) )
		val = 6;
	if ( (absResult > 149.99) && (absResult < 179.99) )
		val = 7;
	if ( absResult == 180.0 )
		val = 8;

	if ( useShortString )
		return shortArray[val];
	else
		return array[val];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string getPositionStatus(const double& lat1, const double& lon1,
                              const double& lat2, const double& lon2) {

	std::string info;

	if ( lat1 < lat2 ) {
		if ( lon1 < lon2 ) {
			info = "Seismic is on the upper right of the town";
		}
		if ( lon1 > lon2 ) {
			info = "Seismic is on the upper left of the town";
		}
		if ( lon1 == lon2 ) {
			info = "Seismic is right up to the town";
		}
	}
	else if ( lat1 > lat2 ) {
		if ( lon1 < lon2 ) {
			info = "Seismic is on the bottom right of the town";
		}
		if ( lon1 > lon2 ) {
			info = "Seismic is on the bottom left of the town";
		}
		if ( lon1 == lon2 ) {

			info = "Seismic is right down to the town";
		}
	}
	return info;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void geodeticOffsetInv(const double& refLat, const double& refLon,
                       const double& lat, const double& lon,
                       double& xOffset, double& yOffset) {

	double a = GD_semiMajorAxis;
	double b = GD_TranMercB;
	double f = GD_geocentF;

	double L = lon - refLon;
	double U1 = atan((1 - f) * tan(refLat));
	double U2 = atan((1 - f) * tan(lat));
	double sinU1 = sin(U1);
	double cosU1 = cos(U1);
	double sinU2 = sin(U2);
	double cosU2 = cos(U2);

	double lambda = L;
	double lambdaP;
	double sinSigma;
	double sigma;
	double cosSigma;
	double cosSqAlpha;
	double cos2SigmaM;
	double sinLambda;
	double cosLambda;
	double sinAlpha;
	int iterLimit = 100;

	do {
		sinLambda = sin(lambda);
		cosLambda = cos(lambda);
		sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) +
		        (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) *
		                (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
		if ( sinSigma == 0 ) {
			xOffset = 0.0;
			yOffset = 0.0;
			return; // co-incident points
		}
		cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
		sigma = atan2(sinSigma, cosSigma);
		sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
		cosSqAlpha = 1 - sinAlpha * sinAlpha;
		cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;
		if ( cos2SigmaM != cos2SigmaM ) //isNaN
		{
			cos2SigmaM = 0; // equatorial line: cosSqAlpha=0 (§6)
		}
		double C = f / 16 * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));
		lambdaP = lambda;
		lambda = L + (1 - C) * f * sinAlpha * (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
	}
	while ( fabs(lambda - lambdaP) > 1e-12 && --iterLimit > 0 );

	if ( iterLimit == 0 ) {

		xOffset = 0.0;
		yOffset = 0.0;
		return; // formula failed to converge
	}

	double uSq = cosSqAlpha * (a * a - b * b) / (b * b);
	double A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
	double B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));
	double deltaSigma = B * sinSigma * (cos2SigmaM + B / 4 * (cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM) -
	        B / 6 * cos2SigmaM * (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));
	double s = b * A * (sigma - deltaSigma);

	double bearing = atan2(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
	xOffset = sin(bearing) * s;
	yOffset = cos(bearing) * s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string decimalToSexagesimal(const double value) {

	std::string output;
	char dec[5];
	double i = floor(fabs(value));
	double d = (fabs(value) - i) * 60;
	d = floor(d * 100) / 100;
	sprintf(dec, "%#05.2f", d);
	output = toString(i) + "-" + toString(dec);
	return output;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string sexagesimalToDecimal(const double deg, const double min,
                                       std::string pol) {

	std::string value;
	double x = min / 60;
	double y = fabs(deg) + x;
	if ( (pol == "S") or (pol == "W") or (pol == "s") or (pol == "w") ) {
		y = -y;
	}
	value = toString(y);
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string msk2str(double d) {

	std::string value;
	static std::string const roman[23] =
	        { "I", "I-II", "II", "II-III",
	          "III",
	          "III-IV", "IV", "IV-V", "V",
	          "V-VI",
	          "VI",
	          "VI-VII",
	          "VII", "VII-VIII",
	          "VIII",
	          "VIII-IX", "IX", "IX-X", "X", "X-XI",
	          "XI",
	          "XI-XII", "XII" };
	if ( d < 1 )
		d = 1.00;
	int i = floor(d * 2) - 1;
	value = roman[i];
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string doubleToRoman(const double m) {

	int n = (int) m;
	if ( n > 3999999 || n < 0 )
		return "<< OUT OF RANGE >>";
	if ( n == 0 )
		return "N";
	static std::string const number_table[4][10] =
	        {
	          { "", "I", "II", "III",
	            "IV",
	            "V", "VI", "VII",
	            "VIII",
	            "IX" },
	          { "", "X", "XX", "XXX",
	            "XL",
	            "L", "LX", "LXX",
	            "LXXX",
	            "XC" },
	          { "", "C", "CC", "CCC",
	            "CD",
	            "D", "DC", "DCC",
	            "DCCC",
	            "CM" },
	          { "", "M", "MM", "MMM" }
	        };
	int place, power_of_10;
	for (place = 0, power_of_10 = 10; power_of_10 <= n; power_of_10 *= 10)
		++place;
	std::string bars, number;
	for (; power_of_10 /= 10, place >= 0; --place) {
		int digit = (n / power_of_10) % 10;
		if ( (place > 3 || place == 3) && digit > 3 ) {
			std::string roman_digit = number_table[place - 3][digit];
			number += roman_digit;
			bars += std::string(roman_digit.length(), '_');
		}
		else
			number += number_table[place][digit];
	}

	return number;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string getStringPosition(const double& value,
                                    const enum GeographicCoordinate pos) {

	std::string str = String::stringify("%.4f", fabs(value));

	switch ( pos ) {

		// Latitude
		case Latitude:
			if ( value > .0 )
				str += " N";
			else
				str += " S";
		break;

			// Longitude
		case Longitude:
			if ( value > .0 )
				str += " E";
			else
				str += " W";
		break;
		default:
			str += " ?";
		break;
	}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string getCardLetter(const double& value,
                                const enum GeographicCoordinate pos) {

	std::string str;
	switch ( pos ) {

		case Latitude:
			if ( value > .0 )
				str = "N";
			else
				str = "S";
		break;

		case Longitude:
			if ( value > .0 )
				str = "E";
			else
				str = "W";
		break;

		default:
			str = "-";
		break;
	}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// end namespace geo
} // end namespace core
} // end namespace ipgp
