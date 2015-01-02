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


#ifndef __IPGP_GUI_CLIENT_MESSAGEREADER_H__
#define __IPGP_GUI_CLIENT_MESSAGEREADER_H__


#include <QWaitCondition>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <string>

namespace Seiscomp {

namespace Communication {
class Connection;
}

namespace IO {
class DatabaseInterface;
}

}

namespace IPGP {
namespace Gui {
namespace Client {

DEFINE_IPGP_SMARTPOINTER(MessageReader);

/**
 * @class   MessageReader
 * @package IPGP::Gui::Client
 * @brief   Seiscomp message reader for GUI apps
 *
 * Default Seiscomp::Client::Application reader isn't properly functional
 * once executed within a GUI thread (GUI QThread v.s. boost::thread) and
 * therefore this class provides a new version of it.
 * Though, this implementation doesn't handle 'messages' but signals to the
 * main application there presence.
 */
class SC_IPGP_GUI_API MessageReader : public QThread {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MessageReader(Seiscomp::Communication::Connection*,
		                       Seiscomp::IO::DatabaseInterface*,
		                       const std::string&);
		~MessageReader();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void run();
		bool readConnection();
		bool stopRequested();
		bool autoReconnectRequested();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void setAutoReconnect(const bool&);
		void requestStop();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt Signals
		// ------------------------------------------------------------------
		void messagingConnectionLost();
		void messagingConnectionEstablished();

		void messageAvailable();

		void databaseConnectionLost();
		void databaseConnectionEstablished();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::Communication::Connection* _connection;
		Seiscomp::IO::DatabaseInterface* _database;
		std::string _db;

		bool _autoReconnect;
		bool _stopRequested;
		QMutex _mutex;
};

} // namespace Client
} // namespace Gui
} // namespace IPGP

#endif

