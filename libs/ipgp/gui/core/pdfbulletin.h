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

#ifndef __IPGP_GUI_CORE_PDFBULLETIN_H__
#define __IPGP_GUI_CORE_PDFBULLETIN_H__

#include <ipgp/gui/api.h>
#include <QObject>
#include <QProcess>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/magnitude.h>


namespace Seiscomp {
namespace DataModel {

class DatabaseQuery;

}
}


namespace IPGP {
namespace Gui {


/**
 * @class   PDFBulletin
 * @package IPGP::Gui::DataModel
 * @brief   Bulletin creator
 *
 * This class is used in order to produce summary bulletins that the user may
 * export into PDF files. Those bulletins are generated from origin and
 * magnitude objects into a custom HTML file which is finally converted in a
 * PDF document by using an external program (wkhtmltopdf).
 */
class SC_IPGP_GUI_API PDFBulletin : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit PDFBulletin(Seiscomp::DataModel::DatabaseQuery*,
		                     const QString& pdfConverter = QString("wkhtmltopdf"));
		~PDFBulletin();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void publishBulletin(Seiscomp::DataModel::OriginPtr,
		                     Seiscomp::DataModel::MagnitudePtr,
		                     const QString& scode,
		                     const std::vector<std::string>& mapsNames,
		                     const std::vector<std::string>& mapsPaths);

		const std::string& bulletinExportPath() const {
			return _bulletinExportPath;
		}

		void setBulletinExportPath(const std::string& bulletinExportPath) {
			_bulletinExportPath = bulletinExportPath;
		}

		const std::string& bulletinModelFile() const {
			return _bulletinModelFile;
		}

		void setBulletinModelFile(const std::string& bulletinModelFile) {
			_bulletinModelFile = bulletinModelFile;
		}

		const std::string& tmpFolder() const {
			return _tmpFolder;
		}

		void setTmpFolder(const std::string& tmpFolder) {
			_tmpFolder = tmpFolder;
		}

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		Seiscomp::DataModel::PickPtr getPick(const std::string& publicID);
		Seiscomp::DataModel::PickPtr getPick(Seiscomp::DataModel::OriginPtr,
		                                     const std::string& station,
		                                     const std::string& phase);
		Seiscomp::DataModel::ArrivalPtr getArrival(Seiscomp::DataModel::OriginPtr,
		                                           const std::string pickID);
		void log(bool, const QString&);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void bulletinFileReady(const QString&);
		void errorOcurred(const char*);
		void logHtml(const QString&);
		void logMessage(const int&, const QString&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::DataModel::DatabaseQuery* _query;
		std::string _bulletinModelFile;
		std::string _bulletinExportPath;
		std::string _tmpFolder;
		QString _pdfConverter;
};

} // namespace Gui
} // namespace IPGP

#endif
