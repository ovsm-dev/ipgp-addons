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

#define SEISCOMP_COMPONENT IPGP_OBJECTCACHE

#include <ipgp/core/datamodel/objectcache.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/timer.h>
#include <algorithm>

using namespace Seiscomp;
using namespace Seiscomp::DataModel;


namespace IPGP {
namespace Core {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ObjectCache::~ObjectCache() {
	clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ObjectCache::updateObjects(Seiscomp::DataModel::DatabaseQuery* query) {

	if ( !query ) {
		SEISCOMP_ERROR("Provided query instance is broken, no cache update performed");
		return;
	}

	// Make a deep copy of the tuple, free the original, store a fresh version
	// of the newly fetched objects...
	Tuple copy = _cache;

	clear();

	Util::StopWatch sw;
	for (size_t i = 0; i < copy.size(); ++i) {
		PO obj = query->getObject(copy[i].second->typeInfo(), copy[i].second->publicID());
		if ( obj )
			addObject(copy[i].first, obj);
	}
	SEISCOMP_INFO("All objects from the cache have been updated: %s",
	    Seiscomp::Core::Time(sw.elapsed()).toString("%T.%f").c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> ObjectCache::ObjectInfo
ObjectCache::getObjectInfo<OriginPtr>(const std::string& publicID) {

	ObjectInfo info = ObjectNotFound;

	for (size_t i = 0; i < _cache.size(); ++i) {
		if ( _cache[i].first.first == publicID && _cache[i].first.second == publicID )
			info = ObjectIsOwnParent;
		if ( _cache[i].first.first == publicID && _cache[i].first.second != publicID )
			info = ObjectHasAParent;
	}

	return info;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> OriginPtr
ObjectCache::getUnassociatedObject<OriginPtr>(const std::string& publicID) {

	// Object own publicID is stored first in the MultiKey
	OriginPtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i) {
		if ( _cache[i].first.first == publicID && _cache[i].first.second == publicID )
			obj = Origin::Cast(_cache[i].second.get());
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> bool
ObjectCache::doesObjectHaveParent<OriginPtr>(const std::string& publicID) {

	bool answer = false;
	for (size_t i = 0; i < _cache.size(); ++i) {
		if ( _cache[i].first.first == publicID
		        && _cache[i].second->TypeInfo() == Origin::TypeInfo()
		        && _cache[i].first.second != "" )
			return true;
	}

	return answer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ObjectCache::PO
ObjectCache::getObjectByOwnID(const std::string& publicID) {

	// Object own publicID is stored first in the MultiKey
	PO obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.first == publicID )
			obj = _cache[i].second;

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string ObjectCache::getParentID(const std::string& publicID) const {

	std::string id;
	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.first == publicID )
			id = _cache[i].first.second;

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> EventPtr
ObjectCache::getObjectByChildID(const std::string& publicID) {

	// Object's parent publicID is stored second in the MultiKey
	EventPtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID )
			obj = Event::Cast(_cache[i].second.get());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> MagnitudePtr
ObjectCache::getObjectByChildID(const std::string& publicID) {

	// Object's parent publicID is stored second in the MultiKey
	MagnitudePtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID )
			obj = Magnitude::Cast(_cache[i].second.get());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> OriginPtr
ObjectCache::getObjectByChildID(const std::string& publicID) {

	// Object's parent publicID is stored second in the MultiKey
	OriginPtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID )
			obj = Origin::Cast(_cache[i].second.get());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> EventPtr
ObjectCache::getObjectByOwnID(const std::string& publicID) {

	// Object own publicID is stored first in the MultiKey
	EventPtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.first == publicID )
			obj = Event::Cast(_cache[i].second.get());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> MagnitudePtr
ObjectCache::getObjectByOwnID(const std::string& publicID) {

	// Object own publicID is stored first in the MultiKey
	MagnitudePtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.first == publicID )
			obj = Magnitude::Cast(_cache[i].second.get());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> OriginPtr
ObjectCache::getObjectByOwnID(const std::string& publicID) {

	// Object own publicID is stored first in the MultiKey
	OriginPtr obj = NULL;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.first == publicID )
			obj = Origin::Cast(_cache[i].second.get());

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<ObjectCache::PO>
ObjectCache::getObjectsByParentID(const char& name,
                                  const std::string& publicID) {

	std::vector<PO> list;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID )
			list.push_back(_cache[i].second);

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> std::vector<OriginPtr>
ObjectCache::getObjectsByParentID(const std::string& publicID) {

	std::vector<OriginPtr> list;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID
		        && _cache[i].second->TypeInfo() == Origin::TypeInfo() )
			list.push_back(Origin::Cast(_cache[i].second.get()));

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> std::vector<MagnitudePtr>
ObjectCache::getObjectsByParentID(const std::string& publicID) {

	std::vector<MagnitudePtr> list;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID
		        && _cache[i].second->TypeInfo() == Magnitude::TypeInfo() )
			list.push_back(Magnitude::Cast(_cache[i].second.get()));

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> std::vector<AmplitudePtr>
ObjectCache::getObjectsByParentID(const std::string& publicID) {

	std::vector<AmplitudePtr> list;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID
		        && _cache[i].second->TypeInfo() == Magnitude::TypeInfo() )
			list.push_back(Amplitude::Cast(_cache[i].second.get()));

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> std::vector<CommentPtr>
ObjectCache::getObjectsByParentID(const std::string& publicID) {

	std::vector<CommentPtr> list;

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first.second == publicID
		        && _cache[i].second->TypeInfo() == Comment::TypeInfo() )
			list.push_back(Comment::Cast(_cache[i].second.get()));

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ObjectCache::addObject(MultiKey key, PO obj) {

	if ( !obj )
		return;

	if ( key.first != obj->publicID() || key.second.empty() )
		return;

	bool found = false;
	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].first == key && _cache[i].second == obj )
			found = true;

	if ( !found )
		_cache.push_back(ObjectPair(key, obj));
	else {
		if ( _errMsg < 4 )
			SEISCOMP_DEBUG("Already cached object with multikey: %s/%s",
			    key.first.c_str(), key.second.c_str());
		if ( _errMsg == 3 )
			SEISCOMP_DEBUG("This message will not be repeated for other objects");
		++_errMsg;
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int ObjectCache::removeObject(MultiKey key) {

	int count = 0;
	for (Tuple::iterator i = _cache.begin(); i < _cache.end(); ++i)
		if ( i->first == key ) {
			i->second.reset();
			_cache.erase(i);
			count++;
		}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ObjectCache::clear() {

	SEISCOMP_DEBUG("Number of objects destroyed from cache: %d", (int )_cache.size());

	for (size_t i = 0; i < _cache.size(); ++i)
		if ( _cache[i].second )
			_cache[i].second.reset();

	_cache.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int ObjectCache::byteSize() const {

	int size = 0;
	for (size_t i = 0; i < _cache.size(); ++i)
		size += sizeof(_cache.at(i));

	return size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Core
} // namespace IPGP
