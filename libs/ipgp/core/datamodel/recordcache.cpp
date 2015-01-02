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

#define SEISCOMP_COMPONENT IPGP_RECORDCACHE

#include <ipgp/core/datamodel/recordcache.h>
#include <seiscomp3/logging/log.h>

using namespace Seiscomp;


namespace IPGP {
namespace Core {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordCache::RecordCache() :
		_errMsg(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordCache::~RecordCache() {
	clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordCache::feed(Seiscomp::RecordPtr rec) {

	if ( !rec ) {
		SEISCOMP_ERROR("Adding NULL record to cache is not allowed");
		return;
	}

	if ( !get(rec->startTime(), rec->endTime(), rec->streamID()) )
		_records.push_back(rec);
	else {
		if ( _errMsg < 4 )
			SEISCOMP_DEBUG("Already cached record with stream: %s", rec->streamID().c_str());
		if ( _errMsg == 3 )
			SEISCOMP_DEBUG("This message will not be repeated for other records");
		++_errMsg;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordPtr RecordCache::get(const Seiscomp::Core::Time& start,
                           const Seiscomp::Core::Time& end,
                           const std::string& streamID) {

	RecordPtr rec = NULL;
	for (size_t i = 0; i < _records.size(); ++i)
		if ( _records.at(i)->startTime() == start
		        && _records.at(i)->endTime() == end
		        && _records.at(i)->streamID() == streamID ) {
			rec = _records.at(i);
			break;
		}

	return rec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordPtr RecordCache::get(const std::string& streamID) {

	RecordPtr rec = NULL;
	for (size_t i = 0; i < _records.size(); ++i)
		if ( _records.at(i)->streamID() == streamID ) {
			rec = _records.at(i);
			break;
		}

	return rec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordCache::Tuple RecordCache::getRecords(const std::string& streamID) {

	std::vector<RecordPtr> rec;
	for (size_t i = 0; i < _records.size(); ++i)
		if ( _records.at(i)->streamID() == streamID )
			rec.push_back(_records.at(i));

	return rec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordCache::Tuple
RecordCache::getStationRecords(const std::string& networkCode,
                               const std::string& stationCode) {

	std::vector<RecordPtr> rec;
	for (size_t i = 0; i < _records.size(); ++i)
		if ( _records.at(i)->networkCode() == networkCode
		        && _records.at(i)->stationCode() == stationCode )
			rec.push_back(_records.at(i));

	return rec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordPtr RecordCache::get(const size_t& pos) {
	return _records.at(pos);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int RecordCache::remove(const Seiscomp::Core::Time& start,
                              const Seiscomp::Core::Time& end,
                              const std::string& streamID) {

	int count = 0;
	size_t i = 0;

	iterator it = _records.begin();
	for (; it != _records.end(); ++it) {

		if ( !(*it) )
			continue;

		if ( (*it)->startTime() == start && (*it)->endTime() == end
		        && (*it)->streamID() == streamID ) {
			_records[i].reset();
			_records.erase(it);
			++count;
		}
		++i;
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordCache::clear() {

	SEISCOMP_DEBUG("Number of records destroyed from cache: %d", (int )_records.size());

	for (size_t i = 0; i < _records.size(); ++i)
		if ( _records[i] )
			_records[i].reset();

	_records.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int RecordCache::byteSize() const {

	int size = 0;
	for (size_t i = 0; i < _records.size(); ++i)
		size += sizeof(_records.at(i));

	return size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}// namespace Core
} // namespace IPGP
