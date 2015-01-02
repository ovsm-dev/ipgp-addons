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

#ifndef __IPGP_GUI_DATAMODEL_QCUSTOMSTANDARDITEM_H__
#define __IPGP_GUI_DATAMODEL_QCUSTOMSTANDARDITEM_H__

#include <QStandardItem>


/**
 * @class   QCustomStandardItem
 * @package IPGP::Gui::DataModel
 * @brief   Custom Qt StandarItem
 *
 * This class provides sorting abilities for items containing mixed data values
 * (e.g. '100km' / '10km' will be sorted in a neat way but the new comer '75km'
 * will be sorted after them, which is typically the purpose of this class...
 * Sort them out properly!).
 */
class QCustomStandardItem : public QStandardItem {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum DataRole {
			NumberUnitRole = Qt::UserRole + 1,
			MixedRole = Qt::UserRole + 2
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		/**
		 * @brief Constructor ;)
		 * @param text the data
		 * @param role the DataRole to use
		 * @param pattern the regex pattern to use as a filter
		 */
		explicit QCustomStandardItem(const QString& text = QString(),
		                             const DataRole& role = NumberUnitRole,
		                             const QString& pattern = "[^\\d+\\.?\\d+]");

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool operator<(const QStandardItem& other) const;
		void setDataRole(DataRole);

		const QString& pattern() const;
		void setPattern(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		DataRole _dataRole;
		QString _pattern;
};


#endif
