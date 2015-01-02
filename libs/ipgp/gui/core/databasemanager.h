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


#ifndef __IPGP_GUI_SQLITE_MANAGER_H__
#define __IPGP_GUI_SQLITE_MANAGER_H__

#include <ipgp/gui/api.h>
#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


namespace IPGP {
namespace Gui {
namespace Sqlite {

struct SC_IPGP_GUI_API StoredOrigin {
		QString id;
		QString scode;
		QString degree;
		QString comment;
		QString author;
		long time;
};


/**
 * @class   DatabaseManager
 * @package IPGP::GUI::SQLite
 * @brief   SQLite database API
 *
 * This class provides an interface to interact with a SQLite database.
 */
class SC_IPGP_GUI_API DatabaseManager : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DatabaseManager(QObject* parent = NULL,
		                         const QString& filename = "");
		~DatabaseManager();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Initializes the database, create the default 'Origin' entity
		 * @return True if the operation is properly done, false otherwise
		 */
		bool initDatabase();

		/**
		 * @brief Opens the specified database file
		 * @return True if DB is opened, false otherwise
		 */
		bool openDB(const QString& filename);


		/**
		 * @brief Database delete method!
		 * @return True if database successfully deleted, false otherwise
		 */
		bool deleteDB(const QString& path = "");

		/**
		 * @brief Check the database state
		 * @return True if the database has been successfully opened, false otherwise
		 */
		bool opened() const;

		/**
		 * @brief Inserts an origin in the database
		 * @param origin The origin structure
		 * @return The ID (record ID from auto-increment) of the inserted origin
		 */
		int insertOrigin(const StoredOrigin& origin);


		/**
		 * @brief Inserts an origin in the database
		 * @param originID The origin publicID
		 * @param scode The origin SCODE
		 * @param degree The origin degree code from 1 to X
		 * @param comment The comment comment
		 * @param author The origin author
		 * @return The ID (record ID from auto-increment) of the inserted origin
		 */
		int insertOrigin(const QString& originID, const QString& scode,
		                 const QString& degree, const QString& comment,
		                 const QString& author = "user");


		/**
		 * @brief Tells if an origin is present in the database
		 * @param originID The origin publicID to locate
		 * @return true if origin exists, false otherwise
		 */
		bool originExists(const QString& originID);


		/**
		 * @brief Gets an origin from the database
		 * @param originID The origin publicID to fetch
		 * @return An StoredOrigin structure object
		 */
		StoredOrigin getOrigin(const QString& originID);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QSqlDatabase _db;
		QString _dbFile;
};

} // namespace SQLite
} // namespace Gui
} // namespace IPGP

#endif
