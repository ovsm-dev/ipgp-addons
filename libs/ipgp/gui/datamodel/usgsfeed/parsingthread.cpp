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



#include <ipgp/gui/datamodel/usgsfeed/parsingthread.h>
#include <ipgp/gui/datamodel/usgsfeed/quakeevent.h>
#include <QDebug>
#include <QMapIterator>


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParsingThread::ParsingThread(QObject* parent) :
		QThread(parent), _reply(NULL), _canceled(false) {

	_xmlTags.append("id");
	_xmlTags.append("title");
	_xmlTags.append("updated");
	_xmlTags.append("summary");
	_xmlTags.append("point");
	_xmlTags.append("elev");
	_xmlTags.append("link");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParsingThread::run() {

	Q_ASSERT(_reply != NULL);

	if ( _canceled )
		_error = tr("Parsing canceled");
	else {

		_error = QString();

		QXmlStreamReader xml;
		QXmlStreamReader::TokenType type;
		QString fieldName;
		QString fieldValue;
		QuakeEvent event;
		bool gotValue = false;
		bool gotEntry = false;
		QMap<QString, QuakeEvent> events;

		xml.setDevice(_reply);

		while ( !xml.atEnd() && !_canceled ) {

			type = xml.readNext();
			QString tag = xml.name().toString().toLower();
			switch ( type ) {

				case QXmlStreamReader::StartElement: {
					gotValue = false;
					if ( tag == "entry" ) {
						gotEntry = true;
					}
					else if ( _xmlTags.contains(tag) ) {
						fieldName = tag;
					}
					else
						fieldName = QString(); // ignore
				}
				break;

				case QXmlStreamReader::Characters: {
					// save aside any text
					if ( gotEntry && !fieldName.isEmpty() && !gotValue ) {
						fieldValue = xml.text().toString();
						gotValue = true;
					}
				}
				break;

				case QXmlStreamReader::EndElement: {
					// save aside this value
					if ( gotValue && tag != "entry" ) {
						event.set(fieldName, fieldValue);
					}
					else if ( tag == "entry" ) {
						events.insert(event.id(), event);
						event.clear();
						gotEntry = false;
						gotValue = false;
					}
				}
				break;

				default:
					break;
			}
		}

		if ( _canceled )
			_error = tr("Parsing canceled");

		else if ( xml.hasError() )
			emit invalidXMLStream();

		else if ( !xml.hasError() ) {

			emit clearModel();

			for (QMap<QString, QuakeEvent>::const_iterator i = events.constBegin();
			        i != events.constEnd(); ++i)
				emit addQuake(i.value());
		}

		else if ( !_canceled )
		    _error = tr("Could not interpret the server's response: %1")
		            .arg(xml.errorString());
	}

	_reply->deleteLater();
	_reply = NULL;
	_canceled = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}// namespace Gui
} // namespace IPGP



