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


#include <ipgp/gui/core/databasemanager.h>
#include <QVariant>
#include <iostream>
#include <QFile>
#include <QDir>


namespace IPGP {
namespace Gui {
namespace Sqlite {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseManager::DatabaseManager(QObject* parent, const QString& filename) :
		_dbFile(filename) {
	openDB(filename);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseManager::~DatabaseManager() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseManager::openDB(const QString& file) {

	_db = QSqlDatabase::addDatabase("QSQLITE");
	_db.setDatabaseName(file);

	return _db.open();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseManager::initDatabase() {

	bool ret = false;

	if ( _db.isOpen() ) {
		QSqlQuery query(QString("CREATE TABLE \"origin\" (\"id\" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE,"
			" \"origin_id\" VARCHAR, \"time\" DATETIME, \"scode\" VARCHAR NOT NULL  DEFAULT INCONNU,"
			" \"degree\" VARCHAR, \"comment\" VARCHAR, \"author\" VARCHAR NOT NULL  DEFAULT user)")
		    );

		if ( query.next() )
			ret = true;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseManager::deleteDB(const QString& path) {

	_db.close();

	QString fpath;

	// NOTE: We have to store database file into user home folder in Linux
	//       therefore we add the separators properly...
	if ( path.isEmpty() )
		fpath = QString("%1%2.ipgp%3scwuv%4db%5db").arg(QDir::home().path())
		        .arg(QDir::separator()).arg(QDir::separator())
		        .arg(QDir::separator()).arg(QDir::separator());
	else
		fpath = path;

	fpath.append(QDir::separator()).append(_dbFile);
	fpath = QDir::toNativeSeparators(fpath);

	return QFile::remove(fpath);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseManager::opened() const {
	return _db.isOpen();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseManager::insertOrigin(const StoredOrigin& origin) {

	int newId = -1;
	bool ret = false;

	if ( _db.isOpen() ) {
		QString queryString;

		if ( originExists(origin.id) ) {
			queryString = QString("update origin set time = '%1', scode='%2', degree='%3', comment='%4', author='%5' where origin_id LIKE '%6'")
			        .arg(origin.time).arg(origin.scode).arg(origin.degree)
			        .arg(origin.comment).arg(origin.author).arg(origin.id);
		}
		else {
			queryString = QString("insert into origin values(NULL,'%1', '%2', '%3', '%4', '%5', '%6')")
			        .arg(origin.id).arg(origin.time).arg(origin.scode).
			        arg(origin.degree).arg(origin.comment).arg(origin.author);
		}

		QSqlQuery query;
		ret = query.exec(queryString);

		if ( ret )
			newId = query.lastInsertId().toInt();
	}

	return newId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseManager::insertOrigin(const QString& originID, const QString& scode,
                                  const QString& degree, const QString& comment,
                                  const QString& author) {

	int newId = -1;
	bool ret = false;

	if ( _db.isOpen() ) {

		QSqlQuery query;
		ret = query.exec(QString("insert into origin values(NULL,'%1', '%2', '%3', '%4', '%5', '%6')")
		        .arg(originID).arg("time").arg(scode).arg(degree).arg(comment).arg(author));

		if ( ret )
			newId = query.lastInsertId().toInt();
	}

	return newId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseManager::originExists(const QString& originID) {

	bool ret = false;

	if ( _db.isOpen() ) {
		QSqlQuery query(QString("select * from origin where origin_id LIKE '%1'")
		        .arg(originID));

		if ( query.next() )
			ret = true;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StoredOrigin DatabaseManager::getOrigin(const QString& originID) {

	StoredOrigin org;

	if ( _db.isOpen() ) {
		QSqlQuery query(QString("select * from origin where origin_id LIKE '%1'")
		        .arg(originID));

		if ( query.next() ) {
			org.id = query.value(1).toString();
			org.time = query.value(2).toInt();
			org.scode = query.value(3).toString();
			org.degree = query.value(4).toString();
			org.comment = query.value(5).toString();
			org.author = query.value(6).toString();
		}
	}

	return org;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}// Sqlite
} // Gui
} // IPGP
