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



#ifndef __IPGP_GUI_CLIENT_APPLICATION_H__
#define __IPGP_GUI_CLIENT_APPLICATION_H__

#define SEISCOMP_COMPONENT IPGP_Application


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/client/scheme.h>
#include <ipgp/gui/client/misc.h>
#include <ipgp/gui/map/mapdescriptor.hpp>
#include <seiscomp3/client/application.h>
#include <QApplication>
#include <QObject>
#include <QPixmap>


QT_FORWARD_DECLARE_CLASS(QDialog);
QT_FORWARD_DECLARE_CLASS(QSplashScreen);
QT_FORWARD_DECLARE_CLASS(QMainWindow);
QT_FORWARD_DECLARE_CLASS(QListWidgetItem);


namespace Ui {
class Log;
class ConnectionSetup;
class PluginDialog;
class About;
}


#define AppInstance (IPGP::Gui::Client::Application::staticInstance())


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(MainWindow);
DEFINE_IPGP_SMARTPOINTER(LogDialog);

namespace Client {

DEFINE_IPGP_SMARTPOINTER(MessageReader);


/**
 * @class   Application
 * @package IPGP::Gui::Client
 * @author  Stephen Roselia <stephen.roselia@supinfo.com>
 * @brief   SCGUI super class!
 *
 * One must expend this class to dispose of QApplication and
 * Seiscomp::Client::Application methods, variables.
 * Since those two super classes are quite heavy and loaded with lots and lots
 * of stuff, it is important to understand the way those combined stuffs have
 * been designed and the proper way they should to be used...
 *
 * GUI app example.
 * @code
 * class MyNewGUI : public IPGP::Gui::Client::Application {
 * 		Q_OBJECT
 *
 *  // ----------------------------------------------------------------------
 * 	//  Re-implement protected interface
 * 	// ----------------------------------------------------------------------
 * 	protected:
 * 		bool run() {
 *      	// Run stuff here
 *      	showMessage("Setup user interface");
 *	    	setupGraphicalInterface();
 *			readApplicationSettings();
 *			setupApplicationMenus();
 *			setInterfaceName("MyNewGUI");
 *			setBinaryName("mngui");
 *			setApplicationDescriptor(ApplicationDescriptor("MyNewGui",
 *				"This is a brief description", "This is a full description",
 *				"0.1.0");
 *			handlePlugins();
 *			handleDatabaseState();
 *			showNormal();
 *
 *			// IMPORTANT: convert 0 to true and anything else to false
 *			return !QApplication::exec();
 * 		}
 *		bool initConfiguration() {
 *			// do init stuff here
 *		}
 *		void createCommandLineDescription() {
 *			// create cmd line stuff here
 *		}
 *      bool validateParameters() {
 *      	// Validate parameters here
 *      }
 *      void setupGraphicalInterface() {
 *      	IPGP::Gui::Client::Application::setupGraphicalInterface();
 *      	// Do graphical interface init stuff here
 *      	//
 *      }
 * };
 *
 * int main(int argc, char** argv) {
 * 		MyNewGUI app(argc, argv);
 * 		return app.Instance()->exec();
 * }
 * @endcode
 *
 * The user is invited to take a look at any GUI from the package and see
 * more detailed example of this class use cases, how signals get fired and
 * slots triggered...
 */
class SC_IPGP_GUI_API Application : public QApplication,
        public Seiscomp::Client::Application {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Flags {
			//! Show splash screen on startup
			SHOW_SPLASH = 0x001,
			//! The application wants a database connection
			WANT_DATABASE = 0x002,
			//! The application wants a messaging connection
			WANT_MESSAGING = 0x004,
			//! The connection dialog should be opened when
			//! either one of the connections that has been
			//! requested with 'WANT_[SERVICE]' failed to create
			OPEN_CONNECTION_DIALOG = 0x008,
			//! If WANT_DATABASE is enabled and no custom settings
			//! are provided in the configuration file it tries to
			//! fetch the database connection from the messaging
			FETCH_DATABASE = 0x010,
			//! Should received notifier messages be applied or not
			AUTO_APPLY_NOTIFIER = 0x020,
			//! Should received notifier messages be interpreted or not
			//! When this flag is not set the signals 'addObject',
			//! 'removeObject' and 'updateObject' are not fired
			INTERPRETE_NOTIFIER = 0x040,
			LOAD_STATIONS = 0x080,
			LOAD_INVENTORY = 0x100,
			LOAD_CONFIGMODULE = 0x200,
			SHOW_FULLSCREEN = 0x400,

			DEFAULT_WITHOUT_SPLASH = WANT_DATABASE | WANT_MESSAGING | OPEN_CONNECTION_DIALOG |
			        FETCH_DATABASE | AUTO_APPLY_NOTIFIER | INTERPRETE_NOTIFIER,
			DEFAULT = SHOW_SPLASH | WANT_DATABASE | WANT_MESSAGING | OPEN_CONNECTION_DIALOG |
			        FETCH_DATABASE | AUTO_APPLY_NOTIFIER | INTERPRETE_NOTIFIER
		};

		typedef std::vector<std::string> StringVector;
		typedef std::list<std::string> StringList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Application(int&, char**, QApplication::Type type
		            = QApplication::GuiClient, Flags = DEFAULT);
		virtual ~Application();

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		virtual void readApplicationSettings();
		virtual void saveApplicationSettings();
		virtual bool notify(QObject*, QEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		static Application* staticInstance() {
			return _instance;
		}
		QMainWindow* mainWindow();
		MainWindow* getMainWindow();

		inline const std::string& interfaceName() const {
			return _interfaceName;
		}
		void setInterfaceName(const std::string&);

		inline const std::string& binaryName() const {
			return _binaryName;
		}

		//! Sets the name of this application's binary. It is later used
		//! when fetching the associated documentation.
		void setBinaryName(const std::string&);

		void setStyleSheetFromFile(const std::string&);
		inline const std::string& styleSheetFile() const {
			return _styleSheet;
		}

		//! Returns the application scheme
		inline const Scheme& scheme() const {
			return _scheme;
		}
		Scheme& scheme() {
			return _scheme;
		}

		//! Returns the application global settings
		inline const GlobalSettings& globalSettings() const {
			return _globalSettings;
		}
		GlobalSettings& globalSettings() {
			return _globalSettings;
		}

		//! Returns the application map descriptor
		inline const MapDescriptor& mapDescriptor() const {
			return _mapDescriptor;
		}
		MapDescriptor& mapDescriptor() {
			return _mapDescriptor;
		}

		//! Returns the application state
		inline const State& state() const {
			return _appState;
		}
		void setState(const State&);

		//! Updates the main window title accordingly to predefined pattern
		//! which is like "APPFULLNAME <> appname@hostname:port"
		void updateMainWindowTitle();

		/**
		 * @brief Exits the application in a clean way.
		 * @param returnCode The return code.
		 * @note  Anything that isn't 0 is considered as an error.
		 */
		void exit(int);

		const QPixmap databaseIcon() const;
		const std::string databaseServer() const;

		//! Sets the application's descriptor
		void setApplicationDescriptor(const ApplicationDescriptor&);

		/**
		 * @brief Shows main window after splashscreen destruction
		 * @param hasSplash indicate if the splashscreen has to be destroyed
		 *        before the showing the main window
		 */
		void showNormal(bool hasSplash = true);

		/**
		 * @brief Shows main window in fullscreen after splashscreen desctruction
		 * @param hasSplash indicate if the splashscreen has to be destroyed
		 *        before the showing the main window
		 */
		void showFullScreen(bool hasSplash = true);

		/**
		 * @brief Tells if application instance has subscription with typed group
		 * @param the group name
		 * @return true when subscribed, false otherwise
		 */
		const bool isSubscribedTo(const char*);

		/**
		 * @brief Sets the primary master group to join when initializing the
		 *        application. If not used in application constructor,
		 *        the default group will be LISTENER_GROUP
		 * @param string name of the master group
		 */
		void setMasterMessagingGroup(const char*);

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		virtual void setupGraphicalInterface();
		void createCommandLineDescription();
		bool init();
		void done();
		virtual bool initConfiguration();
		void messageSubscription(const char*);
		void showMessage(const char*);
		void showWarning(const char*);

		void handleReconnect();
		void handleDisconnect();
		bool handleInitializationError(Stage stage);

		void notifyFromMessaging(Seiscomp::DataModel::Notifier*);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		bool initInterface();
		void makeConnections();
		bool initSubscriptions();

		void startMessageReaderThread();
		void stopMessageReaderThread();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public virtual Qt interface
		// ------------------------------------------------------------------
		virtual void showAbout();
		virtual void showPlugins();
		virtual void showLog();
		void showGeneralDocumentation();
		void showDocumentation();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void exitGui();
		void showConnectionDialog();
		void handleFullScreen();

		//! Debugs messages thru SEISCOMP_INFO
		void debugInfo(const char*);

		//! Debugs messages thru SEISCOMP_WARNING
		void debugWarning(const char*);

		//! Debugs messages thru SEISCOMP_ERROR
		void debugError(const char*);

		//! Debugs messages this SEISCOMP_DEBUG
		void debug(const char*);

		void showInfoBox(const char*);
		void showWarningBox(const char*);
		void showErrorBox(const char*);

		/**
		 * @brief Add a log message.
		 * @param lm the message's type @see LogMessage enum
		 * @param sender the name of the module sending the message
		 * @param message the message itself
		 * @note  If the module's name is empty, the application's name
		 *        will be used as default module
		 */
		void log(const LogMessage&, const QString&, const QString&);
		void log(const int&, const QString&, const QString&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void subscriptionItemClicked(QListWidgetItem*);
		void handleMessagingSetup();
		void handleDatabaseSetup();
		void validateConnectionSettings();
		void terminateSplash();
		void subscribeAllOptions();
		void unsubscribeAllOptions();

		void readerLostDatabaseConnection();
		void readerEstablishedDatabaseConnection();
		void readerLostMessagingConnection();
		void readerEstablishedMessagingConnection();
		void readerFoundMessages();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt Signals
		// ------------------------------------------------------------------
		void databaseChanged(const QString& oldDB, const QString& newDB);
		void changeDatabaseIcon();

		void messageSkipped(Seiscomp::Communication::NetworkMessage*);
		void messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*);

		void addObject(const QString& parentID, Seiscomp::DataModel::Object*);
		void removeObject(const QString& parentID, Seiscomp::DataModel::Object*);
		void updateObject(const QString& parentID, Seiscomp::DataModel::Object*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		static Application* _instance;
		mutable State _appState;

		MessageReader* _messageReader;
		MainWindow* _mainWindow;
		QSplashScreen* _splash;
		QPixmap _connected;
		QPixmap _disconnected;
		QDialog* _connectionDialog;
		QDialog* _pluginDialog;
		QDialog* _aboutDialog;
		LogDialog* _log;
		Ui::PluginDialog* _pluginDialogUi;
		Ui::ConnectionSetup* _connectionDialogUi;
		Ui::About* _aboutUi;

		ApplicationDescriptor _appDescriptor;
		Scheme _scheme;
		GlobalSettings _globalSettings;
		MapDescriptor _mapDescriptor;

		StringList _messageGroup;
		StringVector _databaseInterface;

		std::string _interfaceName;
		std::string _binaryName;
		std::string _dbParameters;
		std::string _dbType;
		std::string _primaryGroup;
		std::string _styleSheet;

		bool _showSplah;
		bool _startFullScreen;

		int _flags;
};



} // namesapce Client
} // namespace Gui
} // namespace IPGP


#endif
