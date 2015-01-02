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



#include <ipgp/gui/client/application.h>
#include <ipgp/gui/client/ui_connectionsetup.h>
#include <ipgp/gui/client/ui_plugindialog.h>
#include <ipgp/gui/client/ui_about.h>
#include <ipgp/gui/datamodel/logdialog.h>
#include <ipgp/gui/datamodel/mainwindow.h>
#include <ipgp/gui/client/messagereader.h>
#include <ipgp/core/string/string.h>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/client/pluginregistry.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/datamodel/version.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/communication/connectioninfo.h>
#include <seiscomp3/system/environment.h>

#include <QtGui>



static int const SplashAlignment = Qt::AlignBottom | Qt::AlignRight;
static QColor const SplashTextColor = Qt::darkGray;
static QColor const SplashWarningTextColor = Qt::darkRed;


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Communication;


namespace IPGP {
namespace Gui {
namespace Client {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! Singleton
Application* Application::_instance = NULL;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int& argc, char** argv, Type type, Flags flags) :
		QApplication(argc, argv, type), Seiscomp::Client::Application(argc, argv),
		_appState(AppInit), _startFullScreen(false), _flags(flags) {

	if ( _instance != this && _instance ) {
		SEISCOMP_WARNING("Another application object exists already. "
			"This usage is not intended. "
			"The Application::staticInstance() method will return "
			"the last created application.");
	}

	_instance = this;

	_mainWindow = NULL;
	_connectionDialog = NULL;
	_connectionDialogUi = NULL;
	_pluginDialog = NULL;
	_pluginDialogUi = NULL;
	_splash = NULL;
	_messageReader = NULL;
	_log = NULL;
	_aboutDialog = NULL;
	_aboutUi = NULL;

	if ( flags & SHOW_SPLASH )
		_showSplah = true;
	else
		_showSplah = false;

	if ( flags & AUTO_APPLY_NOTIFIER )
	    setAutoApplyNotifierEnabled(true);

	if ( flags & INTERPRETE_NOTIFIER )
	    setInterpretNotifierEnabled(true);

	if ( flags & LOAD_CONFIGMODULE )
	    setLoadConfigModuleEnabled(true);

	if ( flags & LOAD_INVENTORY )
	    setLoadInventoryEnabled(true);

	if ( flags & LOAD_STATIONS )
	    setLoadStationsEnabled(true);

	_primaryGroup = "LISTENER_GROUP";
	_messageGroup.push_back("GUI");

	qRegisterMetaType<Client::LogMessage>("LogMessage");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {

	if ( mainWindow() ) {

		saveApplicationSettings();

		if ( _connectionDialogUi )
		    delete _connectionDialogUi, _connectionDialogUi = NULL;

		if ( _aboutUi )
		    delete _aboutUi, _aboutUi = NULL;

		if ( _pluginDialogUi )
		    delete _pluginDialogUi, _pluginDialogUi = NULL;

		delete _mainWindow, _mainWindow = NULL;
	}

	if ( _instance == this )
	    _instance = NULL;

	SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d",
	    Seiscomp::Core::BaseObject::ObjectCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createCommandLineDescription() {

	// Messaging
	commandline().addGroup("Messaging");
	commandline().addOption("Messaging", "user,u", "client name used when connecting to the messaging", &_messagingUser);
	commandline().addOption("Messaging", "host,H", "messaging host (host[:port])", &_messagingHost);
	commandline().addOption("Messaging", "timeout,t", "connection timeout in seconds", &_messagingTimeout);
	commandline().addOption("Messaging", "primary-group,g", "the primary message group of the client", &_messagingPrimaryGroup);
	commandline().addOption("Messaging", "encoding", "sets the message encoding (binary or xml)", &_messagingEncoding);

	// Database
	commandline().addGroup("Database");
	commandline().addOption("Database", "db-driver-list", "list all supported database drivers");
	commandline().addOption("Database", "database,d", "the database connection string, format: service://user:pwd@host/database", &_db, false);
	commandline().addOption("Database", "inventory-db", "load the inventory database from a given XML file", &_inventoryDB, false);
	commandline().addOption("Database", "config-db", "load the config database from a given XML file", &_configDB, false);

	// User interface
	commandline().addGroup("User interface");
	commandline().addOption("User interface", "stylesheet", "set the application style sheet", &_styleSheet);
	commandline().addOption("User interface", "full-screen,F", "starts the application in fullscreen");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::startMessageReaderThread() {

	if ( _messageReader ) {
		SEISCOMP_WARNING("Another instance of MessageReader is already running, "
			"make sure that only one reader is available throughout an session");
		return;
	}

	_messageReader = new MessageReader(connection(), database(), _dbParameters);

	connect(_messageReader, SIGNAL(finished()), _messageReader, SLOT(deleteLater()));
	connect(_messageReader, SIGNAL(messageAvailable()), this, SLOT(readerFoundMessages()));
	connect(_messageReader, SIGNAL(messagingConnectionLost()), this, SLOT(readerLostMessagingConnection()));
	connect(_messageReader, SIGNAL(messagingConnectionEstablished()), this, SLOT(readerEstablishedMessagingConnection()));
	connect(_messageReader, SIGNAL(databaseConnectionLost()), this, SLOT(readerLostDatabaseConnection()));
	connect(_messageReader, SIGNAL(databaseConnectionEstablished()), this, SLOT(readerEstablishedDatabaseConnection()));

	_messageReader->start();
	_messageReader->setAutoReconnect(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::stopMessageReaderThread() {

	if ( !_messageReader ) return;

	_messageReader->setAutoReconnect(false);
	_messageReader->requestStop();

	if ( _connection )
	    _connection->disconnect();

	SEISCOMP_DEBUG("Waiting for MessageReader to finish");

	_messageReader->wait();

	delete _messageReader;
	_messageReader = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::readerLostDatabaseConnection() {

	if ( !mainWindow() ) return;

	_connectionDialogUi->databaseButton->setText("Connect");
	_connectionDialogUi->connectionLine->setDisabled(false);
	_connectionDialogUi->dbType->setDisabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::readerEstablishedDatabaseConnection() {

	if ( !mainWindow() ) return;

	_connectionDialogUi->databaseButton->setText("Disconnect");
	_connectionDialogUi->connectionLine->setDisabled(true);
	_connectionDialogUi->dbType->setDisabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::readerLostMessagingConnection() {

	if ( !mainWindow() ) return;

	_connectionDialogUi->messagingButton->setText("Connect");
	_connectionDialogUi->userLine->setDisabled(false);
	_connectionDialogUi->serverLine->setDisabled(false);
	_connectionDialogUi->primaryGroupLine->setDisabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::readerEstablishedMessagingConnection() {

	if ( !mainWindow() ) return;

	_connectionDialogUi->messagingButton->setText("Disconnect");
	_connectionDialogUi->userLine->setDisabled(true);
	_connectionDialogUi->serverLine->setDisabled(true);
	_connectionDialogUi->primaryGroupLine->setDisabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::readerFoundMessages() {

	//! Do fancy stuff with any object found...

	if ( !_connection ) return;

	Seiscomp::Core::MessagePtr msg;
	NetworkMessagePtr nmsg;
	while ( true ) {
		NetworkMessage *tmp_msg = NULL;
		msg = _connection->readQueuedMessage(Connection::READ_ALL, &tmp_msg);
		nmsg = tmp_msg;
		if ( !msg ) {
			if ( !nmsg ) break;

			emit messageSkipped(nmsg.get());

			continue;
		}

		emit messageAvailable(msg.get(), nmsg.get());

		NotifierMessage* nm = NotifierMessage::Cast(msg);
		if ( !nm ) continue;

		if ( isAutoApplyNotifierEnabled() ) {
			for (NotifierMessage::iterator it = nm->begin();
			        it != nm->end(); ++it) {
				SEISCOMP_DEBUG("Notifier for '%s'", (*it)->parentID().c_str());
				(*it)->apply();
			}
		}

		for (NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it)
			notifyFromMessaging(it->get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMasterMessagingGroup(const char* group) {
	_primaryGroup = group;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setStyleSheetFromFile(const std::string& file) {
	QFile stylesheet(file.c_str());
	stylesheet.open(QFile::ReadOnly);
	QString setSheet = QLatin1String(stylesheet.readAll());
	setStyleSheet(setSheet);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPixmap Application::databaseIcon() const {
	if ( _database && _database->isConnected() )
		return _connected;
	else
		return _disconnected;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string Application::databaseServer() const {

	std::string str = _dbParameters;
	std::vector<std::string> tmp;
	Seiscomp::Core::split(tmp, str.c_str(), "@");

	return tmp[1];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setApplicationDescriptor(const ApplicationDescriptor& ad) {
	_appDescriptor = ad;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	stopMessageReaderThread();
	Seiscomp::Client::Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {

	//! Can't find out why QAppcliation::exit(-1) reports exit(1)... This is
	//! weird, if the user awaits for the return code of an application to do
	//! something accordingly, this may be the cause of it.
	//! Maybe this behavior is proper to the QCoreApplication event's handler
	//! and should be properly handled in this class too.
	//! TODO: check this theory out!
	QApplication::exit(returnCode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exitGui() {
	(mainWindow()) ? QApplication::exit(0) : done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleDatabaseSetup() {

	if ( _connectionDialogUi->connectionLine->isEnabled() ) {

		_db = _connectionDialogUi->connectionLine->text().toStdString();

		if ( _db.empty() ) {
			showErrorBox("Please fill up DATABASE information");
			return;
		}

		handleReconnect();

		if ( _database->isConnected() ) {

			_connectionDialogUi->connectionLine->setDisabled(true);
			_connectionDialogUi->dbType->setDisabled(true);
			_connectionDialogUi->databaseButton->setText("Disconnect");

			updateMainWindowTitle();

			emit databaseChanged("", _db.c_str());
		}
		else {
			QMessageBox::critical(mainWindow(), "Database error",
			    tr("The connection has not been established."));
			showErrorBox("");
		}

	}
	else {

		if ( _database->isConnected() )
		    _database->disconnect();

		SEISCOMP_NOTICE("Connection to database %s has been dropped", _db.c_str());

		_connectionDialogUi->connectionLine->setDisabled(false);
		_connectionDialogUi->dbType->setDisabled(false);
		_connectionDialogUi->databaseButton->setText("Connect");
	}
	emit changeDatabaseIcon();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleFullScreen() {
	(mainWindow()->windowState() == Qt::WindowFullScreen) ?
	        mainWindow()->setWindowState(Qt::WindowNoState) :
	        mainWindow()->setWindowState(Qt::WindowFullScreen);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleInitializationError(Stage stage) {

	SEISCOMP_INFO("Trying to configure connection from settings dialog");
	bool retCode = false;

	if ( stage == MESSAGING || stage == DATABASE ) {
		showConnectionDialog();
		retCode = true;
	}

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::notifyFromMessaging(Notifier* n) {

	if ( isInterpretNotifierEnabled() ) {
		switch ( n->operation() ) {
			case OP_ADD:
				emit addObject(n->parentID().c_str(), n->object());
			break;
			case OP_REMOVE:
				emit removeObject(n->parentID().c_str(), n->object());
			break;
			case OP_UPDATE:
				emit updateObject(n->parentID().c_str(), n->object());
			break;
			default:
				break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleMessagingSetup() {

	if ( _connectionDialogUi->userLine->isEnabled() ) {

		_messagingUser = _connectionDialogUi->userLine->text().toStdString();
		_messagingHost = _connectionDialogUi->serverLine->text().toStdString();
		_primaryGroup = _connectionDialogUi->primaryGroupLine->text().toStdString();

		if ( _messagingUser.empty() || _messagingHost.empty() || _primaryGroup.empty() ) {
			QMessageBox::critical(mainWindow(), tr("Error"),
			    tr("Please fill up USERNAME, SERVER and PRIMARY GROUP information"));
			return;
		}

		handleDisconnect();

		if ( _connection && _connection->isConnected() ) {

			_connectionDialogUi->userLine->setDisabled(true);
			_connectionDialogUi->serverLine->setDisabled(true);
			_connectionDialogUi->primaryGroupLine->setDisabled(true);
			_connectionDialogUi->messagingButton->setText("Disconnect");

			if ( !initSubscriptions() )
			    SEISCOMP_ERROR("No valid messaging interface to subscribe services to");

			//! Try and connected to database after retrieving its parameters
			//! from messaging new instance
			Util::StopWatch fetchTimeout;
			while ( fetchTimeout.elapsed() < TimeSpan(1.) ) {
				while ( _connection->readNetworkMessage(false) == Status::SEISCOMP_SUCCESS ) {
					MessagePtr msg = _connection->readMessage(false);
					Communication::DatabaseProvideMessagePtr dbrmsg = Communication::DatabaseProvideMessage::Cast(msg);
					if ( dbrmsg ) {
						std::string dbType = dbrmsg->service();
						std::string dbParameters = dbrmsg->parameters();
						_db = dbType + "://" + dbParameters;
						_dbType = dbType;
						_dbParameters = dbParameters;

						char delimiter = ',';
						std::vector<std::string> v;
						Core::String::explodeString(dbType, delimiter, v);

						if ( v.size() == 0 )
							_databaseInterface.push_back(dbType);
						else
							for (size_t i = 0; i < v.size(); ++i)
								_databaseInterface.push_back(v[i]);

						for (size_t i = 0; i < _databaseInterface.size();
						        ++i) {
							if ( i == 0 )
							    _dbType = _databaseInterface.at(i) + std::string("://");
							_connectionDialogUi->dbType->insertItem((int) i + 1, _databaseInterface[i].c_str());
						}
						_connectionDialogUi->dbType->setCurrentIndex(1);

						SEISCOMP_INFO("Received database service parameters");
						SEISCOMP_INFO("Trying to connect to %s://%s", dbrmsg->service(), dbrmsg->parameters());
						IO::DatabaseInterfacePtr db = dbrmsg->database();
						if ( db ) {
							setDatabase(NULL);
							setDatabase(db.get());
							SEISCOMP_INFO("Connected successfully");
							_connectionDialogUi->connectionLine->setText(_dbParameters.c_str());
							_connectionDialogUi->connectionLine->setDisabled(true);
							_connectionDialogUi->databaseButton->setText("Disconnect");
							emit databaseChanged("", _db.c_str());
							emit changeDatabaseIcon();
							break;
						}
						else
							SEISCOMP_WARNING("Database connection to %s://%s failed",
							    dbrmsg->service(), dbrmsg->parameters());
						continue;
					}

					if ( fetchTimeout.elapsed() >= TimeSpan(1.) )
					    break;
				}
			}

			if ( _connection->isConnected() )
			    startMessageReaderThread();
			if ( _messageReader )
			    _messageReader->setAutoReconnect(true);

			updateMainWindowTitle();
		}
		else {
			QMessageBox::critical(mainWindow(), tr("Messaging error"),
			    tr("The connection has not been established."));
		}

	}
	else {

		stopMessageReaderThread();

		SEISCOMP_NOTICE("Connection to master '%s' has been dropped for user %s",
		    _messagingHost.c_str(), _messagingUser.c_str());

		_connectionDialogUi->userLine->setDisabled(false);
		_connectionDialogUi->serverLine->setDisabled(false);
		_connectionDialogUi->primaryGroupLine->setDisabled(false);
		_connectionDialogUi->messagingButton->setText("Connect");
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleDisconnect() {

	if ( _connection ) {

		stopMessageReaderThread();

		SEISCOMP_INFO("Waiting for messaging disconnection in settings dialog");
		_connection->disconnect();
		_connection = NULL;
		SEISCOMP_INFO("Connection has been successfully closed");
	}

	SEISCOMP_INFO("Requesting messaging connection to %s using username %s",
	    _messagingHost.c_str(), _messagingUser.c_str());

	_connection = Connection::Create(_messagingHost, _messagingUser,
	    _primaryGroup, Protocol::PRIORITY_DEFAULT, 3 * 1000);

	if ( _connection ) {

		Seiscomp::Client::ApplicationStatusMessage stat(name(),
		    _messagingUser, Seiscomp::Client::STARTED);

		_connection->send(Communication::Protocol::STATUS_GROUP, &stat);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleReconnect() {

	if ( _database ) {

		if ( _database->isConnected() ) {
			SEISCOMP_INFO("Waiting for database disconnection in settings dialog");
			_database->disconnect();
			_database = NULL;
			SEISCOMP_DEBUG("Database connection successfully closed");
		}
		else {
			SEISCOMP_INFO("Requesting database connection in settings dialog");
			_dbParameters = _connectionDialogUi->connectionLine->text().toStdString();
			_database->connect(_db.c_str());
		}
	}
	else {

		SEISCOMP_INFO("Requesting database connection in settings dialog");
		_db = _dbType + _dbParameters;
		setDatabase(IO::DatabaseInterface::Open(_db.c_str()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {

	setState(AppInit);

	if ( commandline().hasOption("help") ) {
		printUsage();
		exit(-1);
		return false;
	}

	if ( commandline().hasOption("version") ) {
		printVersion();
		exit(-1);
		return false;
	}

	if ( !Seiscomp::Client::Application::validateParameters() ) {
		std::cerr << "Try --help for help" << std::endl;
		exit(-1);
		return false;
	}


	if ( !initInterface() ) return false;

	setMessagingEnabled(false);
	setDatabaseEnabled(false, false);

	bool retCode = Seiscomp::Client::Application::init();

	if ( commandline().hasOption("full-screen") )
	    _startFullScreen = true;

	if ( !retCode )
	    return false;


	if ( _flags & WANT_MESSAGING ) {

		setMessagingEnabled(true);
		setPrimaryMessagingGroup("LISTENER_GROUP");

		int status = 0;
		while ( !_exitRequested && !_connection ) {

			SEISCOMP_DEBUG("Trying to connect to %s@%s with primary group = %s",
			    _messagingUser.c_str(), _messagingHost.c_str(), _primaryGroup.c_str());
			_connection = Connection::Create(_messagingHost, _messagingUser,
			    _primaryGroup, Protocol::PRIORITY_DEFAULT,
			    _messagingTimeout * 1000, &status);

			if ( _connection ) {
				//					startListeningMessage();
				break;
			}

			bool shouldBreak = false;
			switch ( status ) {
				case Status::SEISCOMP_CONNECT_ERROR: {
					showWarning(Status::StatusToStr(status));
					QMessageBox::warning(mainWindow(), tr("Connection error"),
					    tr("It appears that no scmaster instance is currently running on this host (%1),"
						    " the application will start in offline mode...")
					            .arg(_messagingHost.c_str()));
					shouldBreak = true;
				}
				break;
				case Status::SEISCOMP_CLIENT_NAME_NOT_UNIQUE: {
					showWarning(Status::StatusToStr(status));
					QMessageBox::warning(mainWindow(), tr("Connection error"),
					    tr("Connection to messaging couldn't been established:<br/>"
						    "Host: %1<br/>User: %2<br/>Group: %3<br>Timeout: %4<br/></br>Reason: %5")
					            .arg(_messagingHost.c_str())
					            .arg(_messagingUser.c_str())
					            .arg(_primaryGroup.c_str())
					            .arg(_messagingTimeout * 1000)
					            .arg(Status::StatusToStr(status)));
					shouldBreak = true;
				}
				break;
				default: {
					showWarning(Status::StatusToStr(status));
					QMessageBox::critical(mainWindow(), tr("Connection error"),
					    Status::StatusToStr(status));
					shouldBreak = true;
				}
				break;
			}

			/*
			 if ( status == Status::SEISCOMP_WRONG_SERVER_VERSION ) {
			 showWarning("Wrong server version");
			 QMessageBox::critical(_mainWindow, tr("Messaging error"), tr("Wrong server version"));
			 break;
			 }

			 if ( status == Status::SEISCOMP_TIMEOUT_ERROR ) {
			 showWarning("Timeout while waiting for acknowledgment message");
			 QMessageBox::critical(_mainWindow, tr("Messaging error"),
			 tr("Timeout while waiting for acknowledgment message"));
			 break;
			 }

			 if ( status == Status::SEISCOMP_CONNECT_ERROR ) {
			 showWarning(Status::StatusToStr(status));
			 QMessageBox::warning(_mainWindow, tr("Connection error"),
			 tr("It appears that no scmaster instance is currently running on this host (%1),"
			 " the application will start in offline mode...")
			 .arg(_messagingHost.c_str()));
			 break;
			 }

			 if ( status == Status::SEISCOMP_CLIENT_NAME_NOT_UNIQUE ) {
			 showWarning(Status::StatusToStr(status));
			 QMessageBox::warning(_mainWindow, tr("Connection error"),
			 tr("Connection to messaging couldn't been established for:<br/>"
			 "Host: %1<br/>User: %2<br/>Group: %3<br>Timeout: %4<br/></br>Why: %5")
			 .arg(_messagingHost.c_str())
			 .arg(_messagingUser.c_str())
			 .arg(_primaryGroup.c_str())
			 .arg(_messagingTimeout * 1000)
			 .arg(Status::StatusToStr(status)));
			 break;
			 }
			 */

			//! This may be uncalled for...
			Seiscomp::Core::sleep(2);

			if ( shouldBreak ) break;
		}

		if ( _connection ) {

			SEISCOMP_NOTICE("Connection to %s established", _messagingHost.c_str());

			Seiscomp::Core::Version localSchemaVersion =
			        Seiscomp::Core::Version(DataModel::Version::Major, DataModel::Version::Minor);

			if ( _connection->schemaVersion() > localSchemaVersion ) {
				std::stringstream ss;
				ss << "Local schema v" << localSchemaVersion.toString()
				   << " is older than the one the server supports (v"
				   << _connection->schemaVersion().toString()
				   << ") , incoming messages will not be readable but sending will work.";
				showWarning(ss.str().c_str());
				SEISCOMP_WARNING("%s", ss.str().c_str());
			}
			else if ( _connection->schemaVersion() < localSchemaVersion ) {
				std::stringstream ss;
				ss << "Local schema v" << localSchemaVersion.toString()
				   << " is more recent than the one the server supports (v"
				   << _connection->schemaVersion().toString()
				   << ") , not all information can be handled by the server and will be ignored.";
				showWarning(ss.str().c_str());
				SEISCOMP_WARNING("%s", ss.str().c_str());
			}

			MessageEncoding enc;
			if ( enc.fromString(_messagingEncoding.c_str()) ) {
				SEISCOMP_INFO("Setting message encoding to %s", _messagingEncoding.c_str());
				_connection->setEncoding(enc);
			}

			if ( !initSubscriptions() )
			    SEISCOMP_ERROR("No valid messaging interface to subscribe services to");
		}

		if ( !_connection || !_connection->isConnected() )
		    if ( _flags & OPEN_CONNECTION_DIALOG )
		        showConnectionDialog();

	} // end WANT_MESSAGING


	if ( (_flags & WANT_DATABASE) && _connection ) {

		setDatabaseEnabled(true, false);
		setDatabase(NULL);

		if ( !_db.empty() ) {
			SEISCOMP_INFO("Read database service parameters from configfile");
			SEISCOMP_INFO("Trying to connect to %s", _db.c_str());

			IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(_db.c_str());
			if ( db ) {

				SEISCOMP_INFO("Connected successfully");
				setDatabase(db.get());

				_dbParameters = _db.substr(8, _db.size());
				return !query()->hasError();
			}
			else {

				if ( _flags & FETCH_DATABASE ) {
					setDatabaseEnabled(true, true);
					SEISCOMP_WARNING("Database connection to %s failed, trying to fetch the service message", _db.c_str());
				}
				else {
					SEISCOMP_WARNING("Database connection to %s failed", _db.c_str());
					return false;
				}
			}
		}

		Util::StopWatch fetchTimeout;

		// Poll for 5 seconds for a valid database provided message
		while ( fetchTimeout.elapsed() < TimeSpan(5.) ) {

			while ( _connection->readNetworkMessage(false) == Status::SEISCOMP_SUCCESS ) {
				MessagePtr msg = _connection->readMessage(false);

				Communication::DatabaseProvideMessagePtr dbrmsg = Communication::DatabaseProvideMessage::Cast(msg);

				if ( dbrmsg ) {

					std::string dbType = dbrmsg->service();
					std::string dbParameters = dbrmsg->parameters();
					_db = dbType + "://" + dbParameters;
					_dbType = dbType;
					_dbParameters = dbParameters;

					{
						//! This part is actually bounded to the delimiter used in
						//! order to separate multi database types (mysql, pgsql, sqlite)
						//! since the API can handle them. Although we're not sure
						//! what it is yet, we suppose it's represented by ','
						//! for the time being, or maybe '|' ... ;)

						char delimiter = ',';
						std::vector<std::string> v;
						IPGP::Core::String::explodeString(dbType, delimiter, v);

						if ( v.size() == 0 )
							_databaseInterface.push_back(dbType);
						else
							for (size_t i = 0; i < v.size(); i++)
								_databaseInterface.push_back(v[i]);

						for (size_t i = 0; i < _databaseInterface.size();
						        i++) {
							if ( i == 0 )
							    _dbType = _databaseInterface[i] + std::string("://");
							_connectionDialogUi->dbType->insertItem((int) i + 1, _databaseInterface[i].c_str());
						}
						_connectionDialogUi->dbType->setCurrentIndex(1);
					}


					SEISCOMP_INFO("Received database service parameters");
					SEISCOMP_INFO("Trying to connect to %s://%s", dbrmsg->service(), dbrmsg->parameters());
					IO::DatabaseInterfacePtr db = dbrmsg->database();
					if ( db ) {
						setDatabase(db.get());
						SEISCOMP_INFO("Connected successfully");
						_connectionDialogUi->databaseButton->setText("Disconnect");
						return !query()->hasError();
					}
					else
						SEISCOMP_WARNING("Database connection to %s://%s failed",
						    dbrmsg->service(), dbrmsg->parameters());
					continue;
				}

				if ( fetchTimeout.elapsed() >= TimeSpan(5.) )
				    break;
			}
		}

		_connectionDialogUi->databaseButton->setText("Connect");
		SEISCOMP_ERROR("Timeout while waiting for database to provide message");

		return false;
	}

	setState(AppIdling);
	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initConfiguration() {

	if ( !Seiscomp::Client::Application::initConfiguration() )
	    return false;

	//! Load all items of global group and all Qt stuff like scheme, map,
	//! user interface preferences and others...
	//! Act like a real super-class!

	// Map configuration variables
	try {
		_mapDescriptor.setNames(configGetStrings("map.names"));
	} catch ( ... ) {}
	try {
		std::vector<std::string> l = configGetStrings("map.paths");
		for (size_t i = 0; i < l.size(); ++i)
			l[i] = Environment::Instance()->absolutePath(l.at(i));
		_mapDescriptor.setPaths(l);
	} catch ( ... ) {}
	try {
		_mapDescriptor.setPlatesFilepath(configGetString(
		    Environment::Instance()->absolutePath("map.platesFilepath")));
	} catch ( ... ) {}
	try {
		_mapDescriptor.setTilePattern(configGetString("map.tilePattern"));
	} catch ( ... ) {}
	try {
		_mapDescriptor.setDefaultLatitude(configGetDouble("map.defaultLatitude"));
	} catch ( ... ) {}
	try {
		_mapDescriptor.setDefaultLongitude(configGetDouble("map.defaultLongitude"));
	} catch ( ... ) {}
	if ( _mapDescriptor.names().size() != _mapDescriptor.paths().size()
	        && _mapDescriptor.names().size() != 0 ) {
		SEISCOMP_ERROR("Incoherent association between map names and paths");
		return false;
	}


	// Scheme configuration variables
	try {
		_scheme.setGeneralPrecision(configGetInt("scheme.precision.general"));
	} catch ( ... ) {}
	try {
		_scheme.setDepthPrecision(configGetInt("scheme.precision.depth"));
	} catch ( ... ) {}
	try {
		_scheme.setDistancePrecision(configGetInt("scheme.precision.distance"));
	} catch ( ... ) {}
	try {
		_scheme.setLocationPrecision(configGetInt("scheme.precision.location"));
	} catch ( ... ) {}
	try {
		_scheme.setRmsPrecision(configGetInt("scheme.precision.rms"));
	} catch ( ... ) {}
	try {
		_scheme.setAzimuthPrecision(configGetInt("scheme.precision.azimuth"));
	} catch ( ... ) {}
	try {
		_scheme.setPickTimePrecision(configGetInt("scheme.precision.pickTime"));
	} catch ( ... ) {}
	try {
		_scheme.setPgaPrecision(configGetInt("scheme.precision.pga"));
	} catch ( ... ) {}
	try {
		_scheme.setUseDistanceInKM(configGetBool("scheme.unit.distanceInKM"));
	} catch ( ... ) {}
	try {
		_scheme.setFontFamilty(configGetString("scheme.fonts.base").c_str());
	} catch ( ... ) {}
	try {
		_scheme.setStyleSheet(configGetString("scheme.styleSheet").c_str());
	} catch ( ... ) {}
	try {
		_scheme.setFontSize(configGetDouble("scheme.fonts.normal"));
	} catch ( ... ) {}


	// GlobalSettings configuration variables
	try {
		_globalSettings.setLoadEventDB(configGetInt("loadEventDB"));
	} catch ( ... ) {}
	try {
		_globalSettings.setStreamAcquisitionTimeout(configGetInt("recordstream.timeout"));
	} catch ( ... ) {}
	try {
		_globalSettings.setStreamURI(configGetString("recordstream.uri").c_str());
	} catch ( ... ) {}
	try {
		std::vector<std::string> filters = configGetStrings("picker.filters");
		if ( filters.size() > 0 ) {
			QMap<QString, QString> map;
			for (size_t i = 0; i < filters.size(); ++i) {
				QStringList s = QString::fromStdString(filters.at(i)).split(";");
				if ( s.size() > 0 )
				    map.insert(s.at(0), s.at(1));
			}
			_globalSettings.setStreamFilters(map);
		}
	} catch ( ... ) {}


	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initInterface() {

	bool retCode = true;

	try {

		if ( !_styleSheet.empty() )
		    setStyleSheetFromFile(_styleSheet);

		QPixmap logo(":images/logo.png");
		logo = logo.scaled(120, 120, Qt::KeepAspectRatio, Qt::FastTransformation);

		_mainWindow = new MainWindow;
		_mainWindow->setWindowIcon(QIcon(logo));
		QApplication::setActiveWindow(_mainWindow);

		_log = new LogDialog(mainWindow(), "");
		_log->hide();

		_aboutDialog = new QDialog(mainWindow());
		_aboutDialog->setWindowModality(Qt::WindowModal);
		_aboutDialog->hide();
		_aboutUi = new Ui::About;
		_aboutUi->setupUi(_aboutDialog);
		_aboutUi->label_logo->setPixmap(logo);
		// Seal the GNU license container and prevent edition
		_aboutUi->textEdit->setReadOnly(true);

		QFont splashFont;
		splashFont.setPointSizeF(9.);

		QPixmap pix(":images/splash.png");
		_splash = new QSplashScreen(mainWindow(), pix);
		_splash->setFont(splashFont);
		_splash->setMask(pix.mask()); // transparent background

		_connectionDialog = new QDialog(mainWindow(), Qt::Dialog);
#ifdef __APPLE__
		_connectionDialog->setWindowModality(Qt::ApplicationModal);
#else
		_connectionDialog->setWindowModality(Qt::WindowModal);
#endif
		_connectionDialog->hide();

		_connectionDialogUi = new Ui::ConnectionSetup;
		_connectionDialogUi->setupUi(_connectionDialog);

		QStringList list;
		list.append("STATUS_GROUP");
		list.append("IMPORT_GROUP");
		list.append("AMPLITUDE");
		list.append("CONFIG");
		list.append("EVENT");
		list.append("FOCMECH");
		list.append("GUI");
		list.append("INVENTORY");
		list.append("LOCATION");
		list.append("LOGGING");
		list.append("MAGNITUDE");
		list.append("PICK");
		list.append("PUBLICATION");
		list.append("QC");
		list.append("ROUTING");
		list.append("SERVICE_PROVIDE");
		list.append("SERVICE_REQUEST");

		for (int x = 0; x < list.size(); ++x) {
			QListWidgetItem* item = new QListWidgetItem(list.value(x), _connectionDialogUi->subscriptionList);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			(isSubscribedTo(list.value(x).toStdString().c_str())) ?
			        item->setCheckState(Qt::Checked) : item->setCheckState(Qt::Unchecked);
		}
		connect(_connectionDialogUi->subscriptionList, SIGNAL(itemClicked(QListWidgetItem*)),
		    this, SLOT(subscriptionItemClicked(QListWidgetItem*)));

		_pluginDialog = new QDialog(mainWindow());
		_pluginDialog->setWindowModality(Qt::WindowModal);
		_pluginDialog->hide();
		_pluginDialogUi = new Ui::PluginDialog;
		_pluginDialogUi->setupUi(_pluginDialog);

		//! Default database setup
		//! TODO: Make this configurable in config file
		_dbType = "mysql://";
		_dbParameters = "sysop:sysop@" + _messagingHost + "/seiscomp3";

	}
	catch ( ... ) {
		retCode = false;
	}

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initSubscriptions() {

	showMessage("Initiating subscriptions");

	bool requestAllGroups = false;
	for (std::list<std::string>::iterator it = _messageGroup.begin();
	        it != _messageGroup.end(); ++it) {
		if ( (*it) == "*" || (*it) == "..." ) {
			requestAllGroups = true;
			break;
		}
	}

	std::string errmsg = "Could not subscribe to group ";
	if ( requestAllGroups ) {
		for (int i = 0; i < _connection->groupCount(); ++i) {
			if ( _connection->subscribe(_connection->group(i)) != Status::SEISCOMP_SUCCESS ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", _connection->group(i));
				errmsg.append(_connection->group(i));
				showWarning(errmsg.c_str());
				return false;
			}
		}
	}
	else {
		for (std::list<std::string>::iterator it = _messageGroup.begin();
		        it != _messageGroup.end(); ++it) {
			if ( _connection->subscribe(it->c_str()) != Status::SEISCOMP_SUCCESS ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", it->c_str());
				errmsg.append(it->c_str());
				showWarning(errmsg.c_str());
				return false;
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool Application::isSubscribedTo(const char* section) {

	bool found = false;

	for (std::list<std::string>::iterator it = _messageGroup.begin();
	        it != _messageGroup.end(); ++it) {
		if ( (*it) == static_cast<std::string>(section) )
		    found = true;
	}

	return found;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMainWindow* Application::mainWindow() {
	return qobject_cast<QMainWindow*>(_mainWindow);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow* Application::getMainWindow() {
	return _mainWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::makeConnections() {

	connect(_connectionDialogUi->continueButton, SIGNAL(clicked()), this, SLOT(validateConnectionSettings()));
	connect(_connectionDialogUi->databaseButton, SIGNAL(clicked()), this, SLOT(handleDatabaseSetup()));
	connect(_connectionDialogUi->messagingButton, SIGNAL(clicked()), this, SLOT(handleMessagingSetup()));
	connect(_connectionDialogUi->subsSelect, SIGNAL(clicked()), this, SLOT(subscribeAllOptions()));
	connect(_connectionDialogUi->subsDeselect, SIGNAL(clicked()), this, SLOT(unsubscribeAllOptions()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::subscribeAllOptions() {

	for (int i = 0; i < _connectionDialogUi->subscriptionList->count();
	        i++) {

		if ( _connection->subscribe(_connectionDialogUi->subscriptionList->
		        item(i)->text().toStdString()) == Status::SEISCOMP_SUCCESS )
			_connectionDialogUi->subscriptionList->item(i)->setCheckState(Qt::Checked);
		else
			SEISCOMP_ERROR("Failed to join group %s", _connectionDialogUi->
			        subscriptionList->item(i)->text().toStdString().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::unsubscribeAllOptions() {

	for (int i = 0; i < _connectionDialogUi->subscriptionList->count();
	        i++) {

		if ( _connection->unsubscribe(_connectionDialogUi->subscriptionList->
		        item(i)->text().toStdString()) == Status::SEISCOMP_SUCCESS )
			_connectionDialogUi->subscriptionList->item(i)->setCheckState(Qt::Unchecked);
		else
			SEISCOMP_ERROR("Failed to leave group %s", _connectionDialogUi->
			        subscriptionList->item(i)->text().toStdString().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::messageSubscription(const char* section) {
	_messageGroup.push_back(section);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::debug(const char* msg) {
	SEISCOMP_DEBUG("%s", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::debugInfo(const char* msg) {
	SEISCOMP_INFO("%s", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::debugWarning(const char* msg) {
	SEISCOMP_WARNING("%s", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::debugError(const char* msg) {
	SEISCOMP_ERROR("%s", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showInfoBox(const char* msg) {
	if ( mainWindow()->isVisible() )
		QMessageBox::information(mainWindow(), tr("Info"), msg);
	else
		debugInfo(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showWarningBox(const char* msg) {
	if ( mainWindow()->isVisible() )
		QMessageBox::warning(mainWindow(), tr("Warning"), msg);
	else
		debugWarning(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showErrorBox(const char* msg) {
	if ( mainWindow()->isVisible() )
		QMessageBox::critical(mainWindow(), tr("Error"), msg);
	else
		debugError(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::notify(QObject* receiver, QEvent* event) {

	bool retCode;
	try {
		retCode = QApplication::notify(receiver, event);
	}
	catch ( std::exception& e ) {
		QMessageBox::information(mainWindow(), "Notifier",
		    QString::fromUtf8("<i>%1</i>").arg(e.what()));
		retCode = false;
	}

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::readApplicationSettings() {

	QSettings settings("IPGP", name().c_str());
	settings.beginGroup(name().c_str());
	mainWindow()->restoreGeometry(settings.value("geometry").toByteArray());
	mainWindow()->restoreState(settings.value("state").toByteArray());
	settings.endGroup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::saveApplicationSettings() {

	if ( name().empty() ) {
		SEISCOMP_ERROR("Application's name not defined by user");
		return;
	}

	QSettings settings("IPGP", name().c_str());
	settings.beginGroup(name().c_str());
	settings.setValue("geometry", mainWindow()->saveGeometry());
	settings.setValue("state", mainWindow()->saveState());
	settings.endGroup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setState(const State& state) {
	_appState = state;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setInterfaceName(const std::string& name) {

	QApplication::setApplicationName(name.c_str());
	_interfaceName = name;
	updateMainWindowTitle();
	_mainWindow->setAppName(name.c_str());
	_log->setAppName(name.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setBinaryName(const std::string& name) {
	_binaryName = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setupGraphicalInterface() {

	_pluginDialogUi->appNameLabel->setText(name().c_str());

	_connected = QPixmap(":images/db_ok.png");
	_disconnected = QPixmap(":images/db_nok.png");

	makeConnections();
	updateMainWindowTitle();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showLog() {
	if ( !_log ) return;
	_log->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::log(const LogMessage& lm, const QString& module,
                      const QString& str) {
	_log->addMessage(lm, (module.isEmpty()) ? applicationName() : module, str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::log(const int& lm, const QString& module,
                      const QString& str) {
	log(static_cast<LogMessage>(lm), module, str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showAbout() {

	if ( !_aboutDialog || !_aboutUi ) return;

	//! Read everything properly, just in case the application's descriptor
	//! has changed...
	_aboutDialog->setWindowTitle(QString("About ") + _appDescriptor.name);
	_aboutUi->label_appName->setText(_appDescriptor.name);
	_aboutUi->label_shortDescription->setText(_appDescriptor.shortDescription);
	_aboutUi->label_fullDescription->setText(_appDescriptor.fullDescription);
	_aboutUi->label_version->setText(_appDescriptor.version);

	_aboutDialog->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showGeneralDocumentation() {

	QString file = QString("%1/doc/seiscomp3/html/index.html")
	        .arg(Environment::Instance()->shareDir().c_str());

	QFile f(file);

	(f.exists()) ?
	        QDesktopServices::openUrl(QUrl(file)) :
	        QMessageBox::critical(mainWindow(), tr("Error"),
	            QString("No general documentation index found!"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showDocumentation() {

	QString file = QString("%1/doc/seiscomp3/html/apps/%2.html")
	        .arg(Environment::Instance()->shareDir().c_str())
	        .arg(binaryName().c_str());

	QFile f(file);

	(f.exists()) ?
	        QDesktopServices::openUrl(QUrl(file)) :
	        QMessageBox::critical(mainWindow(), tr("Error"),
	            QString("Documentation for %1 hasn't been found!").arg(applicationName()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showConnectionDialog() {

	if ( _appState == AppInit && !_connection ) {

		_connectionDialogUi->messagingButton->setText("Connect");
		_connectionDialogUi->databaseButton->setText("Connect");

	}
	else {
		if ( _connection->isConnected() ) {
			_connectionDialogUi->userLine->setDisabled(true);
			_connectionDialogUi->serverLine->setDisabled(true);
			_connectionDialogUi->primaryGroupLine->setDisabled(true);
		}
		if ( _database->isConnected() ) {
			_connectionDialogUi->connectionLine->setDisabled(true);
			_connectionDialogUi->dbType->setDisabled(true);
		}
	}

	_connectionDialogUi->primaryGroupLine->setText(_primaryGroup.c_str());
	_connectionDialogUi->connectionLine->setText(_dbParameters.c_str());
	_connectionDialogUi->userLine->setText(_messagingUser.c_str());
	_connectionDialogUi->serverLine->setText(_messagingHost.c_str());

	_connectionDialog->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showFullScreen(bool hasSplash) {

	if ( hasSplash )
	    terminateSplash();

	if ( _connection && _connection->isConnected() )
	    startMessageReaderThread();

	mainWindow()->setWindowState(Qt::WindowFullScreen);
	mainWindow()->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showMessage(const char* message) {

	if ( !_showSplah || !_splash )
	    return;

	_splash->show();
	_splash->showMessage(message, SplashAlignment, SplashTextColor);

	QApplication::processEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showNormal(bool hasSplash) {

	if ( hasSplash )
	    terminateSplash();

	if ( _connection && _connection->isConnected() )
	    startMessageReaderThread();

	if ( _startFullScreen )
	    mainWindow()->setWindowState(Qt::WindowFullScreen);

	mainWindow()->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showPlugins() {

	QString str;
	for (PluginRegistry::iterator it = PluginRegistry::Instance()->begin();
	        it != PluginRegistry::Instance()->end(); ++it) {
		QString tmp = QString("<p>"
			"<b>%1</b><br/>"
			"File: <i><u>%2</u></i><br/>"
			"Author: %3<br/>"
			"Version: %4.%5.%6"
			"</p>")
		        .arg((*it)->description().description.c_str())
		        .arg(it->filename.c_str())
		        .arg((*it)->description().author.c_str())
		        .arg((*it)->description().version.major)
		        .arg((*it)->description().version.minor)
		        .arg((*it)->description().version.revision);
		str.append(tmp);
	}

	_pluginDialogUi->textEdit->setText(str);
	_pluginDialogUi->textEdit->setReadOnly(true);
	_pluginDialog->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showWarning(const char* message) {

	if ( !_showSplah ) return;
	if ( !_splash ) return;

	_splash->show();
	_splash->showMessage(message, SplashAlignment, SplashWarningTextColor);

	QApplication::processEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::subscriptionItemClicked(QListWidgetItem* item) {

	if ( item->checkState() == Qt::Checked ) {
		if ( _connection->subscribe(item->text().toStdString()) != Status::SEISCOMP_SUCCESS )
		    SEISCOMP_ERROR("Failed to join group %s", item->text().toStdString().c_str());
	}
	else {
		if ( _connection->unsubscribe(item->text().toStdString()) != Status::SEISCOMP_SUCCESS ) {
			SEISCOMP_ERROR("Failed to leave group %s", item->text().toStdString().c_str());
			return;
		}
		SEISCOMP_INFO("Leaving group %s", item->text().toStdString().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::terminateSplash() {
	_splash->finish(mainWindow());
	delete _splash;
	_splash = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::updateMainWindowTitle() {
	mainWindow()->setWindowTitle(QString("%1 <> %2@%3").arg(_interfaceName.c_str())
	        .arg(_messagingUser.c_str()).arg(_messagingHost.c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::validateConnectionSettings() {

	if ( _connectionDialogUi->userLine->isEnabled() ) {
		QMessageBox::critical(mainWindow(), "Connection settings error",
		    QString::fromUtf8("Connection to messaging is not established, "
			    "please review configuration"));
		return;
	}

	_messagingUser = _connectionDialogUi->userLine->text().toStdString();
	_messagingHost = _connectionDialogUi->serverLine->text().toStdString();
	_db = _dbType + _dbParameters;

	_connectionDialog->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namesapce Client
} // namespace Gui
} // namespace IPGP

