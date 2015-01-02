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

#ifndef __IPGP_CORE_DATAMODEL_RECORDCACHE_H__
#define __IPGP_CORE_DATAMODEL_RECORDCACHE_H__

#include <ipgp/gui/api.h>
#include <seiscomp3/core/record.h>
#include <vector>
#include <iterator>

namespace IPGP {
namespace Core {

/**
 * @class   RecordCache
 * @package IPGP::Core::DataModel
 * @brief   Record cache engine.
 *
 * This class provides a cache in which records objects can be stored, fetched,
 * and removed.
 * Objects using this class should store and use records from this class and
 * let it manage its data: objects are stored within smart pointers and therefore
 * the user doesn't have to manage them.
 * @todo    Implement RecordSequence too... if necessary!
 */
class SC_IPGP_GUI_API RecordCache {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef std::vector<Seiscomp::RecordPtr> Tuple;
		typedef Tuple::iterator iterator;
		typedef Tuple::const_iterator const_iterator;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit RecordCache();

		/**
		 * @brief Object's dtor.
		 * @note  clear() method gets called, therefore, objects using this
		 *        class don't have to worry about cleaning allocated memory:
		 *        each record is properly destroyed.
		 */
		~RecordCache();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Inserts a record into cache
		 * @param record smart pointer
		 * @note  The record is check before being added into cache
		 */
		void feed(Seiscomp::RecordPtr);

		/**
		 * @brief  Iterates cache and returns first record occurrence that
		 *         matches typed start, end and streamID
		 * @param  the record's start time
		 * @param  the record's end time
		 * @param  the record's streamID
		 * @return Record's smart pointer
		 */
		Seiscomp::RecordPtr get(const Seiscomp::Core::Time& start,
		                        const Seiscomp::Core::Time& end,
		                        const std::string& streamID);

		/**
		 * @brief  Fetches records that match argument
		 * @param  the records streamID
		 * @return Tuple of Record smart pointers
		 */
		Tuple getRecords(const std::string& streamID);

		/**
		 * @brief  Fetches records owned by a station
		 * @param  the station network code
		 * @param  the station code
		 * @return Tuple  of Record smart pointers
		 */
		Tuple getStationRecords(const std::string& networkCode,
		                        const std::string& stationCode);

		/**
		 * @brief  Iterates cache and returns first record occurrence that
		 *         matches typed streamID
		 * @param  the record's streamID
		 * @return Record's smart pointer
		 */
		Seiscomp::RecordPtr get(const std::string& streamID);

		/**
		 * @brief  Iterates cache and returns record occurrence that
		 *         matches typed position
		 * @param  the record's position in cache
		 * @return Record's smart pointer
		 */
		Seiscomp::RecordPtr get(const size_t& pos);

		/**
		 * @brief  Removes all objects from cache that match arguments
		 * @note   Records pointer are reseted and memory occupied deallocated
		 * @param  the record's start time
		 * @param  the record's end time
		 * @param  the record's streamID
		 * @return The number of objects removed
		 */
		const int remove(const Seiscomp::Core::Time& start,
		                 const Seiscomp::Core::Time& end,
		                 const std::string& streamID);

		/**
		 * @brief Clears the cache from all its records
		 * @note  This function resets the pointers : this means that their
		 *        reference count decreases by one, and if it reaches 0, the
		 *        pointer instance gets destroyed (freed) immediately. This
		 *        intended feature has for sole goal a faster memory
		 *        deallocation by opposition to natural destruction.
		 */
		void clear();

		/**
		 * @brief  Evaluates the size in bytes of cache content
		 * @return the byte size of the cache
		 */
		const int byteSize() const;

		const bool isEmpty() const {
			return _records.empty();
		}

		const size_t recordsCount() const {
			return _records.size();
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Tuple _records;
		size_t _errMsg;
};


} // namespace Core
} // namespace IPGP

#endif
