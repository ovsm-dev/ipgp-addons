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



#ifndef __IPGP_USGSFEED_DATALOADWORKERPRIVATE_H__
#define __IPGP_USGSFEED_DATALOADWORKERPRIVATE_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QObject>
#include <QString>
#include <QList>


QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager);
QT_FORWARD_DECLARE_CLASS(QNetworkReply);

DEFINE_IPGP_SMARTPOINTER(QuakeEvent);

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(ParsingThread);

class SC_IPGP_GUI_API DataLoadWorkerPrivate : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DataLoadWorkerPrivate(QObject*);
		~DataLoadWorkerPrivate();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void update();
		void cancelUpdate();

		const QString& getUrl() const;
		void setUrl(const QString&);
		void stopParsingThread();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void clearModel();
		void updated();
		void updateError(const QString&);
		void processQuake(const QuakeEvent&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void handleNetRequestFinished(QNetworkReply*);
		void handleParsingThreadFinished();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void updateData();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QNetworkAccessManager* _netAccessManager;
		ParsingThread* _parsingThread;
		QNetworkReply* _reply;
		QString _url;
		bool _updating; // true during an update
};

} // namespace Gui
} // namespace IPGP

#endif
