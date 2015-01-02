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


#include <ipgp/gui/api.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <vector>
#include <string>


namespace IPGP {
namespace Core {

typedef std::vector<std::string> EarthModelIDs;
typedef std::pair<std::string, EarthModelIDs> Locators;
//typedef std::vector<Locators> LocatorsList;

typedef std::pair<Seiscomp::DataModel::OriginPtr,
        Seiscomp::DataModel::EventPtr> ParentedOrigin;
typedef std::vector<ParentedOrigin> OriginList;

typedef std::pair<Seiscomp::DataModel::OriginPtr,
        Seiscomp::DataModel::MagnitudePtr> ParentedMagnitude;
typedef std::vector<ParentedMagnitude> MagnitudeList;

typedef std::vector<std::string> MagnitudeTypes;

}
}


#endif
