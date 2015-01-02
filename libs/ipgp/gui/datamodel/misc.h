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

#ifndef __IPGP_GUI_DATAMODEL_MISC_H__
#define __IPGP_GUI_DATAMODEL_MISC_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/types.h>
#include <QString>
#include <QMap>
#include <QList>
#include <QPair>
#include <QMetaType>
#include <exception>
#include <string>


namespace IPGP {
namespace Gui {


/**
 * @class   GeneralException
 * @package IPGP::Gui::DataModel
 * @brief   Macro for class exceptions
 */
class SC_IPGP_GUI_API GeneralException : public std::exception {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit GeneralException(const char* msg) :
				_msg(msg) {}
		explicit GeneralException(const std::string& msg) :
				_msg(msg) {}
		explicit GeneralException(const QString& msg) :
				_msg(msg.toStdString()) {}
		~GeneralException() throw () {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const char* what() const throw () {
			return this->_msg.c_str();
		}
		const std::string& whatString() const throw () {
			return this->_msg;
		}
		const QString whatQString() const throw () {
			return this->_msg.c_str();
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::string _msg;
};



/**
 * @brief ID what kind(s) of sensor(s) a station may contain...
 *        Source: seed reference manual @2007 v2.4
 *        - UNITS:
 *           M = Seismometer
 *           M/S = Seismometer
 *           M/S**2 = Accelerometer
 *        - INSTRUMENT CODE:
 *           H = High Gain Seismometer
 *           L = Low Gain Seismometer
 *           G = Gravimeter
 *           M = Mass Position Seismometer
 *           N = Accelerometer
 */
struct SC_IPGP_GUI_API ActiveStream {
		QString networkCode;
		QString stationCode;
		QString locationCode;
		QString channelCode;
		QString gainUnit;
		bool inUse;
};
typedef QList<ActiveStream> ActiveStreamList;

typedef QMap<qreal, qreal> ComponentData;



/**
 * @brief When fetching records, indicates the current state of the fetcher...
 *        Logical order is:
 *          a\ wait for data
 *          b\ wait for QC check
 *          c\ QC is OK | chunks are missing (gaps) | data are missing (no data)
 * @note  Don't forget to declare meta objects if the user plans on using this
 *        enum inside QVariant objects:
 *          qRegisterMetaType<StationStreamStatus>("QVariant<StationStreamStatus>");
 *          qRegisterMetaTypeStreamOperators<int>("StationStreamStatus");
 */
enum StationStreamStatus {
	sssOK,                //! Streams have been fetched and passed the QC check
	sssWaitingForData,    //! Streams are yet to be fetched and tested
	sssWaitingForQCCheck, //! Streams have been fetched and are waiting for QC check
	sssMissingChunks,     //! Streams have been fetched but have failed QC
	sssMissingData        //! The delegate didn't find any data
};
typedef QMap<QString, StationStreamStatus> StationStreamStatusList;

/**
 * @brief ID a gap located on a certain stream.
 */
struct StreamGap {
		QString streamID;
		Seiscomp::Core::Time start;
		Seiscomp::Core::Time end;
};
typedef QList<StreamGap> StreamsGapsList;


/**
 * @brief Seiscomp::DataModel::EventType re-implementation
 *        This enum is to be declared as QMetaType.
 */
enum EventTypeCast {
	etc_NOT_EXISTING = 0,
	etc_NOT_LOCATABLE = 1,
	etc_OUTSIDE_OF_NETWORK_INTEREST = 2,
	etc_EARTHQUAKE = 3,
	etc_INDUCED_EARTHQUAKE = 4,
	etc_QUARRY_BLAST = 5,
	etc_EXPLOSION = 6,
	etc_CHEMICAL_EXPLOSION = 7,
	etc_NUCLEAR_EXPLOSION = 8,
	etc_LANDSLIDE = 9,
	etc_ROCKSLIDE = 10,
	etc_SNOW_AVALANCHE = 11,
	etc_DEBRIS_AVALANCHE = 12,
	etc_MINE_COLLAPSE = 13,
	etc_BUILDING_COLLAPSE = 14,
	etc_VOLCANIC_ERUPTION = 15,
	etc_METEOR_IMPACT = 16,
	etc_PLANE_CRASH = 17,
	etc_SONIC_BOOM = 18,
	etc_DUPLICATE = 19,
	etc_OTHER_EVENT = 20,
	etc_NOT_SET = 21
};


inline const EventTypeCast
getEventTypeCast(const Seiscomp::DataModel::EventType& type) {

	EventTypeCast etc = etc_NOT_SET;
	try {
		switch ( type ) {
			case Seiscomp::DataModel::NOT_EXISTING:
				etc = etc_NOT_EXISTING;
			break;
			case Seiscomp::DataModel::NOT_LOCATABLE:
				etc = etc_NOT_LOCATABLE;
			break;
			case Seiscomp::DataModel::OUTSIDE_OF_NETWORK_INTEREST:
				etc = etc_OUTSIDE_OF_NETWORK_INTEREST;
			break;
			case Seiscomp::DataModel::EARTHQUAKE:
				etc = etc_EARTHQUAKE;
			break;
			case Seiscomp::DataModel::INDUCED_EARTHQUAKE:
				etc = etc_INDUCED_EARTHQUAKE;
			break;
			case Seiscomp::DataModel::QUARRY_BLAST:
				etc = etc_QUARRY_BLAST;
			break;
			case Seiscomp::DataModel::EXPLOSION:
				etc = etc_EXPLOSION;
			break;
			case Seiscomp::DataModel::CHEMICAL_EXPLOSION:
				etc = etc_CHEMICAL_EXPLOSION;
			break;
			case Seiscomp::DataModel::NUCLEAR_EXPLOSION:
				etc = etc_NUCLEAR_EXPLOSION;
			break;
			case Seiscomp::DataModel::LANDSLIDE:
				etc = etc_LANDSLIDE;
			break;
			case Seiscomp::DataModel::ROCKSLIDE:
				etc = etc_ROCKSLIDE;
			break;
			case Seiscomp::DataModel::SNOW_AVALANCHE:
				etc = etc_SNOW_AVALANCHE;
			break;
			case Seiscomp::DataModel::DEBRIS_AVALANCHE:
				etc = etc_DEBRIS_AVALANCHE;
			break;
			case Seiscomp::DataModel::MINE_COLLAPSE:
				etc = etc_MINE_COLLAPSE;
			break;
			case Seiscomp::DataModel::BUILDING_COLLAPSE:
				etc = etc_BUILDING_COLLAPSE;
			break;
			case Seiscomp::DataModel::VOLCANIC_ERUPTION:
				etc = etc_VOLCANIC_ERUPTION;
			break;
			case Seiscomp::DataModel::METEOR_IMPACT:
				etc = etc_METEOR_IMPACT;
			break;
			case Seiscomp::DataModel::PLANE_CRASH:
				etc = etc_PLANE_CRASH;
			break;
			case Seiscomp::DataModel::SONIC_BOOM:
				etc = etc_SONIC_BOOM;
			break;
			case Seiscomp::DataModel::DUPLICATE:
				etc = etc_DUPLICATE;
			break;
			case Seiscomp::DataModel::OTHER_EVENT:
				etc = etc_OTHER_EVENT;
			break;
			default:
				etc = etc_NOT_SET;
			break;
		}
	} catch ( ... ) {}

	return etc;
}


} // namespace Gui
} // namespace IPGP

Q_DECLARE_METATYPE(IPGP::Gui::StationStreamStatus)
Q_DECLARE_METATYPE(IPGP::Gui::EventTypeCast)



#endif
