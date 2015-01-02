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


#ifndef __IPGP_APPLICATION_BULLETINEXPORTERVIEW_H__
#define __IPGP_APPLICATION_BULLETINEXPORTERVIEW_H__


#include <ipgp/gui/defs.h>

#include <QObject>

#include <ipgp/core/datamodel/types.h>
#include <ipgp/gui/client/application.h>
#include <ipgp/gui/datamodel/eventlist/eventlistwidget.h>

#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/seismology/regions.h>

QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
QT_FORWARD_DECLARE_CLASS(QRadioButton);

class QProgressIndicator;

namespace Ui {
class BulletinExporterView;
class FilterBox;
class Dialog;
}

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(MapWidget);
DEFINE_IPGP_SMARTPOINTER(ProgressIndicator);

}
}

class BulletinExporterView : public IPGP::Gui::Client::Application {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum ISF_TYPE {
			IMS1_0, GSE2_0
		};

		enum EXPORT_TYPE {
			eGSE,
			eIMS,
			eQML,
			eHYPO71,
			eSC3ML
		};

		typedef std::vector<Seiscomp::DataModel::PickPtr> PickList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit BulletinExporterView(int& argc, char** argv);
		~BulletinExporterView();

	protected:
		// ------------------------------------------------------------------
		//  Protected virtual interface
		// ------------------------------------------------------------------
		bool run();
		bool initConfiguration();
		void setupGraphicalInterface();
		void setupApplicationMenus();

		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		/**
		 * @brief Writes an Hypo2000 origins catalog.
		 * @param event The event object pointer
		 * @param origin The origin object pointer
		 * @param mag The magnitude object pointer
		 * @param id The ID that links an entry to its phases
		 */
		void writeHypoCatalog(Seiscomp::DataModel::EventPtr,
		                      Seiscomp::DataModel::OriginPtr,
		                      Seiscomp::DataModel::MagnitudePtr,
		                      const std::string& id);

		//! Writes a bulletin in Hypo2000 format
		/**
		 * @brief Writes a phase bulletin in Hypo2000 format.
		 * @param str The container
		 * @param event The event object pointer
		 * @param origin The origin object pointer
		 */
		void writeHypoBulletin(std::string* str, Seiscomp::DataModel::EventPtr,
		                       Seiscomp::DataModel::OriginPtr);

		/**
		 * @brief  Writes a bulletin from temporary file to physical file.
		 * @param  file The file full path
		 * @param  type The bulletin export type
		 * @see    enum EXPORT_TYPE to have a list of available formats
		 * @return true on success, false otherwise
		 */
		bool writeBulletinToFile(const std::string&, const EXPORT_TYPE&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const std::string getEventType(Seiscomp::DataModel::EventType type) const;

		const int getHypoWeight(const PickList&,
		                        Seiscomp::DataModel::OriginPtr,
		                        const std::string& networkCode,
		                        const std::string stationCode,
		                        const std::string& phaseCode,
		                        const double& max);

		const double getTimeValue(const PickList&,
		                          const std::string& networkCode,
		                          const std::string& stationCode,
		                          const std::string& phaseCode,
		                          const size_t& rtype);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void databaseMessage(const QString& oldDB, const QString& newDB);
		void handleDatabaseState();

		void loadingPercentage(const int&, const QString& entity,
		                       const QString& message);

		//! Waiting widget actions
		void showWaitingWidget();
		void hideWaitingWidget();

		void handleSelection();
		void handleHiddenTypes();

		void exportData();
		bool exportAsGSE();
		bool exportAsIMS();
		bool exportAsHYPO71();
		bool exportAsQUAKEML();

		bool getOutputFile();
		void showOriginWarning();
		void updateMapItems();
		void indicateItem(const QString&);

		void hideUnlocalizedEvents(const bool&);
		void hideExternalEvents(const bool&);
		void hideNoTypeSetEvents(const bool&);
		void hideFakeEvents(const bool&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::BulletinExporterView* _ui;
		QLabel* _info;
		IPGP::Gui::MapWidget* _map;
		QDialog* _exportDialog;
		Ui::Dialog* _exportForm;

		QDialog* _filterForm;
		Ui::FilterBox* _filterBox;
		QLabel* _databaseLabel;

		IPGP::Gui::ProgressIndicator* _progressIndicator;
		QProgressIndicator* _pi;

		QRadioButton* _allCheck;
		QRadioButton* _autoCheck;
		QRadioButton* _manualCheck;
		QRadioButton* _confirmedCheck;
		QRadioButton* _noneCheck;
		QAction* _mapAction;

		IPGP::Core::ObjectCache _cache;
		IPGP::Gui::EventListWidget::Settings _eventListSettings;
		IPGP::Gui::EventListWidget* _eventListWidget;

		int _eventCount;
		int _originCount;
		int _pickCount;
		int _magnitudeCount;

		std::string _author;
		std::string _institute;
		std::string _hypoCatalog;
		std::string _quakemlSchemaFile;
		std::string _tempFolder;
		std::string _instituteTag;

		std::vector<IPGP::Core::Locators> _locators;

		bool _setEventAsFinal;
		bool _showNotLocatableOrigins;
		bool _showOutOfNetworkOrigins;
		bool _showOriginsWithNotType;
		bool _showNotExistingOrigins;
		bool _hasQuakeMLSchema;
		bool _useOneFilePerBulletin;

		QString _outputFile;
};

#endif

