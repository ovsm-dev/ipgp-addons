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

#ifndef __IPGP_GUI_DATAMODEL_QCUSTOMITEMS_HPP__
#define __IPGP_GUI_DATAMODEL_QCUSTOMITEMS_HPP__

#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QStandardItem>
#include <QRegExp>

/**
 * @brief This class provides sorting abilities for table widget items
 *        that are stored solely as numbers.
 */
class NumberTableItem : public QTableWidgetItem {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		NumberTableItem(const QString& str) :
				QTableWidgetItem(str) {}

		NumberTableItem(const qreal& number, const size_t& precision = 3) :
				QTableWidgetItem(QString::number(number, 'f', precision)) {}

	public:
		// ------------------------------------------------------------------
		//  Public Interface
		// ------------------------------------------------------------------
		bool operator<(const QTableWidgetItem& other) const {

			QString str = text();
			QString otherStr = other.text();

			QRegExp rx("[^0-9\\.]");
			str.replace(rx, "");
			otherStr.replace(rx, "");

			return str.toDouble() < otherStr.toDouble();
		}
};



/**
 * @brief This class provides sorting abilities for list widget items
 *        that are stored solely as numbers.
 */
class NumberListItem : public QListWidgetItem {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		NumberListItem(const QString& str) :
				QListWidgetItem(str) {}

		NumberListItem(const qreal& number, const size_t& precision = 3) :
				QListWidgetItem(QString::number(number, 'f', precision)) {}

	public:
		// ------------------------------------------------------------------
		//  Public Interface
		// ------------------------------------------------------------------
		bool operator<(const QListWidgetItem& other) const {
			return text().toDouble() < other.text().toDouble();
		}
};



/**
 * @brief This class provides sorting abilities for items containing mixed
 *        data values (e.g. '100km' / '10km' will be sorted in a neat way
 *        but the new comer '75km' will be sorted after them, which is
 *        typically the purpose of this class... Sort them out properly!).
 */
class NumberStandardItem : public QStandardItem {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		NumberStandardItem(const QString& str) :
				QStandardItem(str) {}

		NumberStandardItem(const qreal& number, const size_t& precision = 3) :
				QStandardItem(QString::number(number, 'f', precision)) {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool operator<(const QStandardItem& other) const {

			QString str = text();
			QString otherStr = other.text();

			QRegExp rx("[^0-9\\.]");
			str.replace(rx, "");
			otherStr.replace(rx, "");

			return str.toDouble() < otherStr.toDouble();
		}
};


#endif
