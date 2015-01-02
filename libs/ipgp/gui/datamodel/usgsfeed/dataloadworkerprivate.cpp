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


#include <ipgp/gui/datamodel/usgsfeed/dataloadworkerprivate.h>
#include <ipgp/gui/datamodel/usgsfeed/parsingthread.h>
#include <ipgp/gui/datamodel/usgsfeed/quakeevent.h>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataLoadWorkerPrivate::DataLoadWorkerPrivate(QObject* parent) :
		QObject(parent), _reply(NULL), _updating(false) {

	_netAccessManager = new QNetworkAccessManager(this);
	connect(_netAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleNetRequestFinished(QNetworkReply*)));

	_parsingThread = new ParsingThread(this);
	connect(_parsingThread, SIGNAL(clearModel()), this, SIGNAL(clearModel()));
	connect(_parsingThread, SIGNAL(finished()), this, SLOT(handleParsingThreadFinished()));
	connect(_parsingThread, SIGNAL(addQuake(const QuakeEvent&)), this, SIGNAL(processQuake(const QuakeEvent&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataLoadWorkerPrivate::~DataLoadWorkerPrivate() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataLoadWorkerPrivate::setUrl(const QString& url) {
	_url = url;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& DataLoadWorkerPrivate::getUrl() const {
	return _url;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataLoadWorkerPrivate::stopParsingThread() {
	_parsingThread->_canceled = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataLoadWorkerPrivate::update() {

	if ( _updating ) return;

	_updating = true;
	_parsingThread->_canceled = false;

	QNetworkReply* reply = _netAccessManager->get(QNetworkRequest(QUrl(_url)));
	if ( !reply ) {
		emit updateError(tr("Could not connect to server"));
		_updating = false;
		return;
	}

	_reply = reply;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataLoadWorkerPrivate::handleNetRequestFinished(QNetworkReply* reply) {

	_reply = NULL;

	if ( reply->error() == QNetworkReply::NoError ) {

		if ( _parsingThread->isRunning() )
			reply->deleteLater();
		else {
			_parsingThread->_reply = reply;
			_parsingThread->start();
		}
	}
	else {
		emit updateError(QString("Network error: %1").arg(reply->errorString()));
		_updating = false;
		reply->deleteLater();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataLoadWorkerPrivate::handleParsingThreadFinished() {

	if ( !_parsingThread->_error.isEmpty() )
		emit updateError(_parsingThread->_error);
	else
		emit updated();

	_updating = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataLoadWorkerPrivate::cancelUpdate() {

	if ( !_updating ) return;

	if ( _reply )
	    _reply->abort();

	_parsingThread->_canceled = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP

