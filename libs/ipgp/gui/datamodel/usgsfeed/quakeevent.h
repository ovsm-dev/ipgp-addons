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



#ifndef __IPGP_USGSFEED_QUAKEEVENT_H__
#define __IPGP_USGSFEED_QUAKEEVENT_H__

#include <QString>
#include <QPair>
#include <QDateTime>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <ipgp/gui/api.h>



class SC_IPGP_GUI_API QuakeEvent {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Roles {
			DataRole = Qt::UserRole + 1,
			WhenRole
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		QuakeEvent();
		bool operator==(const QuakeEvent&) const;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		QString id() const;
		QString summary() const;
		QDateTime when() const;
		QString where() const;
		qreal magnitude() const;
		QPointF position() const;
		qreal elevation() const;
		QString html() const;

		QString capURL() const;
		QString summaryURL() const;
		QString pagerURL() const;
		QString dyfiURL() const;
		QString shakemapURL() const;

		void set(const QString& name, const QString& value);
		QString get(const QString& name) const;

		bool isEmpty() const;
		void clear();

	private:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief  Tag value parser
		 * @param  src The source string
		 * @param  tag The tag to identify
		 * @param  exp The expression to find
		 * @return The value of the seeked expression or an empty string.
		 */
		QString findTag(const QString&, const QString&, const QString&) const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QMap<QString, QString> _data;

};

Q_DECLARE_METATYPE(QuakeEvent)

#endif
