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

#ifndef __IPGP_CORE_DATAMODEL_OBJECTCACHE_H__
#define __IPGP_CORE_DATAMODEL_OBJECTCACHE_H__


#include <ipgp/gui/api.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <vector>

namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
}
}

namespace IPGP {
namespace Core {


/**
 * @class   ObjectCache
 * @package IPGP::Core::DataModel
 * @brief   PublicObject standard cache engine
 *
 * This class provides a PublicObject cache in which objects can be retrieved
 * by their own publicID or their parents publicID.
 * All Public Objects types can be added, but at the moment, the implementation
 * only offers methods for retrieving Origins, Events and Magnitudes.
 * Objects using this class should let it manage its data: stored objects are
 * smart pointers, therefore the user doesn't have to handle any pointers at all.
 */
class SC_IPGP_GUI_API ObjectCache {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef std::pair<std::string, std::string> MultiKey;
		typedef Seiscomp::DataModel::PublicObjectPtr PO;
		typedef std::pair<MultiKey, PO> ObjectPair;
		typedef std::vector<ObjectPair> Tuple;

		enum ObjectInfo {
			ObjectNotFound = 0x01,
			ObjectHasAParent = 0x02,
			ObjectIsOwnParent = 0x04
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ObjectCache() :
				_errMsg(0) {}

		/**
		 * @brief Object's dtor.
		 * @note  clear() method gets called, therefore, objects using this
		 *        class don't have to worry about cleaning allocated memory:
		 *        each record is properly destroyed.
		 */
		~ObjectCache();


	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		PO getObjectByOwnID(const std::string& publicID);
		std::vector<PO> getObjectsByParentID(const char& name,
		                                     const std::string& publicID);

		std::string getParentID(const std::string& publicID) const;

		template<typename T> T
		getObjectByChildID(const std::string& publicID);

		template<typename T> T
		getObjectByOwnID(const std::string& publicID);

		template<typename T> ObjectInfo
		getObjectInfo(const std::string& publicID);

		template<typename T> T
		getUnassociatedObject(const std::string& publicID);

		template<typename T> bool
		doesObjectHaveParent(const std::string& publicID);

		template<typename T>
		std::vector<T> getObjectsByParentID(const std::string& publicID);

		/**
		 * @brief Inserts an object into cache
		 * @param object MultiKey
		 * @note  Object is check before being added into cache
		 */
		void addObject(MultiKey, PO);

		/**
		 * @brief Removes all objects from cache which MultiKey matches the
		 *        defined one
		 * @param MultiKey the key of objects to be removed
		 * @note  Objects stored being smart pointers, this method resets them
		 *        by hand to make sure the occupied space is freed immediately
		 *        and doesn't wait up for the deallocation manager to kick in...
		 */
		const int removeObject(MultiKey);

		/**
		 * @brief Clears objects from cache
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

		const size_t size() const {
			return _cache.size();
		}

		/**
		 * @brief Performs an update on each object in cache.
		 * @todo  Implement this for future use, plan to use shared instance
		 *        of query interface thru thread safe function
		 */
		void updateObjects(Seiscomp::DataModel::DatabaseQuery*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Tuple _cache;
		size_t _errMsg;
};


} // namsapce Core
} // namespace IPGP

#endif
