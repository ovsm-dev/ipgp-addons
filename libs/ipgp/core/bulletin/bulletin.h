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

#ifndef __IPGP_CORE_BULLETIN_H__
#define __IPGP_CORE_BULLETIN_H__

#include <ipgp/core/api.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/types.h>
#include <string>
#include <vector>



namespace IPGP {
namespace Core {


/**
 * @class   Bulletin
 * @package IPGP::Core::Bulletin
 * @brief   Seismic bulletin generator.
 *
 * This class provides an interface for producing seismic bulletins based
 * upon five standard bulletins:
 * 	- GSE 2.0,
 * 	- HYPO71,
 * 	- HYPO2000,
 * 	- IMS 1.0,
 * 	- QUAKEML.
 */
class SC_IPGP_CORE_API Bulletin {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Type {
			GSE2_0 = 0x000,   //! GSE 2.0
			HYPO71 = 0x001,   //! Hypo71
			HYPO2000 = 0x002, //! Hypo2000 (HypoInverse)
			IMS1_0 = 0x004,   //! IMS 1.0
			QUAKEML = 0x008   //! QuakeML
		};

		enum IDMethod {
			FILEID = 0x000,
			ORIGINID = 0x001,
			EVENTID = 0x002
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Bulletin(Seiscomp::DataModel::Event*,
		                  Seiscomp::DataModel::DatabaseQuery*);
		~Bulletin();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief  Creates seismic bulletin.
		 * @param  Type the type of bulletin to create.
		 * @return std::string of the bulletin value
		 */
		const std::string getBulletin(const Type&);

		/**
		 * @brief Sets the bulletin author name.
		 * @note  8 chars maximum
		 */
		void setAuthor(const std::string&);

		/**
		 * @brief  Fetches objects attached to current event.
		 * @return True on success, false otherwise.
		 */
		bool getObjects();

		/**
		 * @brief  Bulletin instance log file.
		 * @return A list of log entries added by methods while commputing.
		 */
		const std::vector<std::string>& getLog() const {
			return _log;
		}

		/**
		 * @brief  Hypo2000 catalog event string.
		 * @param  idmethod the identification method of the line. The user
		 *         may choose between FILEID, ORIGINID or EVENTID.
		 * @param  idstr the string extension to add into the generated id
		 *         of the catalog line when the id method is FILEID.
		 *         The default line will print:
		 *         DATE_HOURMIN00a.%idstr%
		 * @param  seismicCode the seismic code of the event.
		 * @return std::string line of Hypo2000 catalog.
		 */
		std::string getHypo2000EventString(const IDMethod& idmethod = FILEID,
		                                   const std::string& idstr = "mq0",
		                                   const std::string& seismicCode = "");

		//! Not implemented yet!
		std::string getHypo71Picks();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		std::string getGSEBulletin();
		std::string getSC3MLBulletin();
		std::string getIMSBulletin();
		const std::string getEventType(Seiscomp::DataModel::EventType) const;

		//! Not implemented yet!
		std::string getHypo2000Bulletin();
		std::string getHypo71Bulletin();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::vector<Seiscomp::DataModel::ArrivalPtr> _arrivalList;
		std::vector<Seiscomp::DataModel::PickPtr> _pickList;
		std::vector<Seiscomp::DataModel::AmplitudePtr> _amplitudeList;
		std::vector<Seiscomp::DataModel::StationMagnitudePtr> _stationMagnitudeList;
		std::vector<Seiscomp::DataModel::CommentPtr> _commentList;
		Seiscomp::DataModel::EventPtr _event;
		Seiscomp::DataModel::DatabaseQueryPtr _query;
		Seiscomp::DataModel::OriginPtr _origin;
		Seiscomp::DataModel::MagnitudePtr _magnitude;
		int _originCount;
		int _pickCount;
		int _magnitudeCount;
		std::string _author;
		std::vector<std::string> _log;
};


} // namespace Core
} // namespace IPGP


#endif
