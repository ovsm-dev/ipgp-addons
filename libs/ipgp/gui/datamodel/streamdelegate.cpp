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

#define SEISCOMP_COMPONENT IPGP_STREAMDELEGATE

#include <ipgp/gui/datamodel/streamdelegate.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/processing/streambuffer.h>


using namespace Seiscomp;
using namespace Seiscomp::IO;
using namespace Seiscomp::Core;


namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamDelegate::
StreamDelegate(RecordStream* stream, const Record::Hint& hint,
               Core::RecordCache* cache, const Mode& mode) :
		_recordStream(stream), _recordHint(hint), _cache(cache), _mode(mode),
		_receivedRecords(0), _stop(false), _isRunning(false) {

	qRegisterMetaType<QVector<qreal> >("QVector<qreal>");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamDelegate::~StreamDelegate() {

	//! Hard pointers are not managed by this class (recordstream, cache, filter)
	//! therefore the instance that create them has to handle them properly.
	//! So no pointer deletion here!
	SEISCOMP_DEBUG("StreamDelegate is terminated");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamDelegate::stopReading() {

	if ( !_isRunning ) return;

	_recordLock.lock();
	_stop = true;
	_recordLock.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamDelegate::readRecords() {

	_stop = false;
	RecordInput recInput(_recordStream, Array::DOUBLE, _recordHint);

	try {
		for (RecordIterator it = recInput.begin(); it != recInput.end(); ++it) {

			_isRunning = true;

			_recordLock.lock();
			bool stopRequest = _stop;
			_recordLock.unlock();

			if ( stopRequest ) break;

			Record* rec = *it;
			if ( rec ) {
				try {
					rec->endTime();
					if ( !storeRecord(rec) ) {
						delete rec;
						return;
					}
					_recordLock.lock();
					++_receivedRecords;
					_recordLock.unlock();
				}
				catch ( ... ) {
					SEISCOMP_INFO("Skipping invalid record for %s (fsamp: %.0f, nsamp: %d)",
					    rec->streamID().c_str(), rec->samplingFrequency(), rec->sampleCount());
					delete rec;
				}
			}
			else
				SEISCOMP_ERROR("An invalid record stream has been received");
		}
	}
	catch ( OperationInterrupted& e ) {
		SEISCOMP_ERROR("Interrupted record acquisition: %s", e.what());
		emit reportOperationException(e.what());
	}
	catch ( RecordStreamException& e ) {
		SEISCOMP_ERROR("Socket rse: %s", e.what());
		emit reportRecordStreamException(e.what());
	}
	catch ( std::exception& e ) {
		SEISCOMP_ERROR("Exception in acquisition: %s", e.what());
		emit reportAcquisitionException(e.what());
	}

	_isRunning = false;

	emit acquisitionTerminated();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamDelegate::storeRecord(Record* rec) {

	if ( !rec ) return false;

	_recordLock.lock();

	ArrayPtr tmp_ar;
	const DoubleArray* data = DoubleArray::ConstCast(rec->data());
	if ( !data ) {
		tmp_ar = rec->data()->copy(Array::DOUBLE);
		data = DoubleArray::ConstCast(tmp_ar);
		if ( !data ) {
			_recordLock.unlock();
			return false;
		}
	}

	if ( _mode & CachedRecords ) {
		RecordPtr r = rec;
		_cache->feed(r);
	}

	if ( _mode & RealTimeRecords )
	    emit newRecord(rec);

	QString s = QString("%1.%2").arg(rec->networkCode().c_str())
	    .arg(rec->stationCode().c_str());
	if ( _stationsList.contains(s) ) {
		_stationsList.removeOne(s);
		emit stationStreamsReady(s);
	}

	_recordLock.unlock();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
