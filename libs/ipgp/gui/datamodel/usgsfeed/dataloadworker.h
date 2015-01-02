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



#ifndef __IPGP_USGSFEED_DATALOADWORKER_H__
#define __IPGP_USGSFEED_DATALOADWORKER_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QList>


DEFINE_IPGP_SMARTPOINTER(QuakeEvent);

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(DataLoadWorkerPrivate);


/**
 * @class   DataLoadWorker
 * @package IPGP::Gui
 * @brief   USGS's Atom XML reader.
 *
 * This class provides a simple interface to fetch, parse and retrieve
 * earthquake events from USGS's Atom URLs, and offers the user two methods
 * to reach them. The first one is to connect to the signal newQuakeEvent()
 * and act on received event. This process can be quite expensive on the event
 * handler if a great number of events are processed. The second method is to
 * wait for the updated() signal and then used the list of events stored
 * internally to do whatever.
 * @todo    Another well thought out solution would be passing a pointer to the
 *          model so that objects could be added on the fly when processed...
 *          The GUI will thank us ;)
 */
class SC_IPGP_GUI_API DataLoadWorker : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DataLoadWorker(const QString& url,
		                        QObject* parent = NULL);
		~DataLoadWorker();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setUrl(const QString&);
		const QString& getUrl() const;
		const QList<QuakeEvent>& quakeEventList() const {
			return _eventList;
		}
		const bool containsQuakeEvent(const QString&) const;

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void clearModel();
		void updated();
		void updateError(const QString&);
		void newQuakeEvent(const QuakeEvent&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void update();
		void cancelUpdate();
		void requestExit();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		/**
		 * @brief Adds QuakeEvent objects to the local event list.
		 * @param event The QuakeEvent to be added
		 */
		void appendQuakeEvent(const QuakeEvent&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<DataLoadWorkerPrivate> _d;
		QList<QuakeEvent> _eventList;
};

} // namespace Gui
} // namespace IPGP

#endif
