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



#ifndef __IPGP_USGSFEED_PARSINGTHREAD_H__
#define __IPGP_USGSFEED_PARSINGTHREAD_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QObject>
#include <QThread>
#include <QString>
#include <QList>
#include <QXmlStreamReader>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


DEFINE_IPGP_SMARTPOINTER(QuakeEvent);

namespace IPGP {
namespace Gui {

class SC_IPGP_GUI_API ParsingThread : public QThread {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ParsingThread(QObject*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void run();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void addQuake(const QuakeEvent&);
		void addQuakeEvents(const QMap<QString, QuakeEvent>&);
		void clearModel();
//		void xmlParsingIsTerminated();
		void invalidXMLStream();

	public:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QNetworkReply* _reply;
		QList<QString> _xmlTags;
		QString _error;
		bool _canceled;
};

} // namespace Gui
} // namespace IPGP


#endif
