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

#define SEISCOMP_COMPONENT MessageReader

#include <ipgp/gui/client/messagereader.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <QDebug>


using namespace Seiscomp;
using namespace Seiscomp::Communication;
using namespace Seiscomp::Core;

namespace IPGP {
namespace Gui {
namespace Client {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MessageReader::MessageReader(Connection* c, IO::DatabaseInterface* d,
                             const std::string& db) :
		_connection(c), _database(d), _db(db), _autoReconnect(true),
		_stopRequested(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MessageReader::~MessageReader() {
	SEISCOMP_INFO("MessageReader is terminated");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MessageReader::run() {
	SEISCOMP_INFO("Starting MessageReader thread");
	while ( readConnection() ) {}
	SEISCOMP_INFO("Leaving MessageReader thread");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MessageReader::readConnection() {

	if ( !_connection || stopRequested() ) return false;

	//! Connection will block here after initial network messages and will
	//! patiently await either disconnection or messages
	int error = _connection->readNetworkMessage(true);
	if ( error == Core::Status::SEISCOMP_SUCCESS ) {
		if ( _connection->queuedMessageCount() != 0 )
		    emit messageAvailable();
	}
	else {

		emit messagingConnectionLost();

		bool first = true;
		while ( !stopRequested() ) {
			_connection->reconnect();
			if ( _connection->isConnected() ) {
				emit messagingConnectionEstablished();
				SEISCOMP_INFO("Reconnected successfully");
				if ( _database ) {
					while ( !_database->isConnected() && autoReconnectRequested() ) {
						SEISCOMP_WARNING("Connection lost to database %s, trying to reconnect", _db.c_str());
						if ( _database->connect(_db.c_str()) ) {
							emit databaseConnectionEstablished();
							SEISCOMP_INFO("Reconnected successfully to %s", _db.c_str());
						}
						else
							sleep(2);
					}
				}
				else
					emit databaseConnectionLost();
				break;
			}
			else {
				if ( first ) {
					first = false;
					SEISCOMP_INFO("Reconnecting failed, trying again every 2 seconds");
				}
				sleep(2);
			}
		}

		if ( stopRequested() ) return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MessageReader::stopRequested() {
	QMutexLocker locker(&_mutex);
	return _stopRequested;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MessageReader::autoReconnectRequested() {
	QMutexLocker locker(&_mutex);
	return _autoReconnect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MessageReader::setAutoReconnect(const bool& value) {
	QMutexLocker locker(&_mutex);
	_autoReconnect = value;
	SEISCOMP_DEBUG("Setting automatic reconnect to: %d", (value) ? 1 : 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MessageReader::requestStop() {
	QMutexLocker locker(&_mutex);
	_stopRequested = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Client
} // namespace Gui
} // namespace IPGP
