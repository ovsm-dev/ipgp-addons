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


#ifndef __IPGP_CORE_TYPES_H__
#define __IPGP_CORE_TYPES_H__

#include <ipgp/core/api.h>
#include <string>

namespace IPGP {
namespace Core {

struct SC_IPGP_CORE_API FirstArrivalStation {
		double refTime, refTimeSec, prevRefTime, prevRefTimeSec;
		int refTimeMin, prevRefTimeMin;
		std::string refTimeYear, refTimeMonth, refTimeDay, refTimeHour,
		        refStation;
		std::string prevRefTimeYear, prevRefTimeMonth, prevRefTimeDay,
		        prevRefTimeHour, prevRefStation;
		double distance;
		double magnitude;
		std::string network;
};

struct SC_IPGP_CORE_API StationPGA {
		std::string network;
		std::string name;
		double magnitude;
		double distance;
		bool isFas;
};

/**
 * @struct	ShakemapsCity
 * @brief	This structure refers to a shakemap city information
 *		used in order to produce theoretical PGA estimations.
 */
struct SC_IPGP_CORE_API ShakemapsCity {
		int code;
		double latitude;
		double longitude;
		std::string cityName;
		std::string islandName;
		int factor;
};

/**
 * @struct	ShakemapsMagnitudeNomination
 * @brief	This structure refers to the nomination attributed
 *		to a seismic event based on degrees of importance.
 */
struct SC_IPGP_CORE_API ShakemapsMagnitudeNomination {
		int code;
		std::string fr_name;
		std::string en_name;
};

struct SC_IPGP_CORE_API ShakemapsColoration {
		int r;
		int g;
		int b;
};

struct SC_IPGP_CORE_API pgaPlot {
		double hypoDistKm;
		double epiDistKm;
		double value;
		double uncertaintyMin;
		double uncertaintyMax;
		double magUncertainty;
		double mskValue;
		double mskMinValue;
		double mskMaxValue;
		std::string siteName;
};



} // namespace Core
} // namespace IPGP

#endif

