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


#ifndef __IPGP_CORE_MISC_H__
#define __IPGP_CORE_MISC_H__

#include <ipgp/core/api.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/databasequery.h>

namespace IPGP {
namespace Core {
namespace Misc {


char* SC_IPGP_CORE_API getOperatingSystemName(const std::string& cmd = "lsb_release -ds");


/**
 * @brief Generates a random alphanumerical id
 * @param s the string pointer to write id in
 * @param length the length of the string to generate
 */
void SC_IPGP_CORE_API randomID(char *s, const int length);

/**
 * @brief Iterates arrival vector an returns arrival which pickID matches
 *        the one given in argument
 * @param pickID the ID of the pick linked to the arrival
 * @param vector the vector containing picks
 * @return DataModel::Arrival pointer or NULL
 */
Seiscomp::DataModel::Arrival* SC_IPGP_CORE_API
getArrival(const std::string& pickID,
           const std::vector<Seiscomp::DataModel::ArrivalPtr>&);

/**
 * @brief Iterates picks vector and recovers the one corresponding to the given id
 * @param pickID the id of the pick
 * @param vector the vector containing picks
 * @return DataModel::Pick pointer or NULL
 */
Seiscomp::DataModel::Pick* SC_IPGP_CORE_API
getPick(const std::string& pickID,
        const std::vector<Seiscomp::DataModel::PickPtr>&);

/**
 * @brief Iterates picks vector and recovers the one corresponding to the given
 *        station, network and code
 * @param station the station reference
 * @param network the network reference
 * @param code the code reference (phase)
 * @param vector the vector containing picks
 * @return DataModel::Pick pointer or NULL
 */
Seiscomp::DataModel::Pick* SC_IPGP_CORE_API
getPick(const std::string& station,
        const std::string& network,
        const std::string& code,
        const std::vector<Seiscomp::DataModel::PickPtr>&);

/**
 * @brief Iterates amplitudes vector and recovers the one matching the given pick id
 * @param pickID the pick id reference
 * @param vector the vector containing amplitudes
 * @return DataModel::Amplitude pointer or NULL
 */
Seiscomp::DataModel::Amplitude* SC_IPGP_CORE_API
getAmplitude(const std::string& pickID,
             const std::vector<
                     Seiscomp::DataModel::AmplitudePtr>&);


/**
 * @brief Iterates origin vector and recovers the one matching the given origin id
 * @param originID the origin id reference
 * @param vector the vector containing origins
 * @return DataModel::Origin pointer or NULL
 */
Seiscomp::DataModel::Origin* SC_IPGP_CORE_API
getOrigin(const std::string& originID,
          const std::vector<Seiscomp::DataModel::OriginPtr>&);


/**
 * @brief Iterates events vector and recovers the one matching the given event id
 * @param eventID the event id reference
 * @param vector the vector containing events
 * @return DataModel::Event pointer or NULL
 */
Seiscomp::DataModel::Event* SC_IPGP_CORE_API
getEvent(const std::string& eventID,
         const std::vector<Seiscomp::DataModel::EventPtr>&);

/**
 * @brief Iterate magnitudes vector and recovers the one matching the given magnitude id
 * @param magnitudeID the magnitude id reference
 * @param vector the vector containing magnitudes
 * @return DataModel::Magnitude pointer or NULL
 */
Seiscomp::DataModel::Magnitude* SC_IPGP_CORE_API
getMagnitude(const std::string& magnitudeID,
             const std::vector<Seiscomp::DataModel::MagnitudePtr>&);

/**
 * @brief Iterate station magnitudes vector and recovers the one matching the given amplitude id
 * @param amplitudeID the amplitude id reference
 * @param vector the vector containing station magnitudes
 * @return DataModel::StationMagnitude pointer or NULL
 */
Seiscomp::DataModel::StationMagnitude* SC_IPGP_CORE_API
getStationMagnitude(const std::string& amplitudeID,
                    const std::vector<Seiscomp::DataModel::StationMagnitudePtr>&);

/**
 * @brief HYPOBULLETIN function!
 *        Iterates picks vector and determines the polarity of given pick in Hypo
 *        Bulletin format
 * @param picks the vector containing picks
 * @param name the station name
 * @param phase the pick phase (P, S, etc)
 * @return U or D or " " regarding pick found polarity
 */
const std::string SC_IPGP_CORE_API
getH71PickPolarity(std::vector<Seiscomp::DataModel::PickPtr>,
                   const std::string& name, const std::string& phase);


Seiscomp::DataModel::MagnitudePtr SC_IPGP_CORE_API
getMagnitudeFromCache(const std::string& publicID, Seiscomp::DataModel::DatabaseQuery*);


} // namespace Misc
} // namespace Core
} // namespace IPGP
#endif

