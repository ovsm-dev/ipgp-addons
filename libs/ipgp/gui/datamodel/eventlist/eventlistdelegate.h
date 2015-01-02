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

#ifndef __IPGP_GUI_EVENTLISTDELEGATE_H__
#define __IPGP_GUI_EVENTLISTDELEGATE_H__


#include <ipgp/gui/api.h>
#include <QObject>
#include <QList>
#include <QStringList>
#include <QMutex>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <ipgp/core/datamodel/objectcache.h>

class QStandardItemModel;

namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
}
}

namespace IPGP {
namespace Gui {

/**
 * @package IPGP::Gui::Widgets
 * @class   EventListDelegate
 * @brief   Event list model feeder
 *
 * This class fetches objects and populate the model with'em. Heavy work is
 * done internally be the delegate. Data get filtered given a certain model
 * passed by the user. Although the cache engine is fed by this class for
 * the event list, the lifetime of the objects isn't handled. This operation
 * should be done by the event list itself thru the cache methods.
 */
class SC_IPGP_GUI_API EventListDelegate : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		//! One needs to filter elements available within Origin table
		//! from the query first, this will speed up the response time a
		//! great deal...
		//! Magnitude has to be filtered from the widget itself when adding
		//! the origin in the view
		struct EventListFilter {

				EventListFilter();
				~EventListFilter() {}

				bool filterEvaluationMode;
				QString evaluationMode;

				bool filterMethodID;
				QString methodID;

				bool filterEarthModelID;
				QString earthModelID;

				bool filterDepth;
				qreal depthMin;
				qreal depthMax;

				bool filterLatitude;
				qreal latitudeMin;
				qreal latitudeMax;

				bool filterLongitude;
				qreal longitudeMin;
				qreal longitudeMax;

				bool filterRms;
				qreal rmsMin;
				qreal rmsMax;

				bool filterMagnitude;
				qreal magnitudeMin;
				qreal magnitudeMax;

				bool filterEventType;
				QString evenType;

				QStringList eventSCODE;

				bool filterEventCertainty;
				QString eventCertainty;

				bool filterOriginStatus;
				QString originStatus;

				bool filterFakeEvent;
				bool filterUnlocalizedEvent;
				bool filterExternalEvents;
				bool filterUnlocalizedEvents;

				bool fetchEventComment;
				bool fetchPreferredOriginComment;
				bool fetchSiblingOriginComment;
				bool fetchUnassociatedOriginComment;
				bool showEventColorType;
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit EventListDelegate(Seiscomp::DataModel::DatabaseQuery* query,
		                           QStandardItemModel*, Core::ObjectCache*,
		                           Seiscomp::Core::Time from, Seiscomp::Core::Time to,
		                           const EventListFilter& filter,
		                           QList<Seiscomp::DataModel::EventPtr>*,
		                           QList<Seiscomp::DataModel::OriginPtr>*,
		                           const bool& fetchUnassociated = true);
		~EventListDelegate();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void getObjects(const Seiscomp::Core::Time& from,
		                const Seiscomp::Core::Time& to,
		                const EventListFilter&, const bool& fetchUnassociated = true);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		bool stopRequested();
		void getEvents(const Seiscomp::Core::Time& from,
		               const Seiscomp::Core::Time& to);
		void getUnassociatedOrigins(const Seiscomp::Core::Time& from,
		                            const Seiscomp::Core::Time& to);

		void addEvents(const QList<Seiscomp::DataModel::EventPtr>&);
		void addEventItem(Seiscomp::DataModel::EventPtr);
		void addUnassociatedOrigins(const QList<Seiscomp::DataModel::OriginPtr>&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		//! Fetches objects and populates the item model
		void populateModel();
		//! Interrupts any ongoing operation, subsequently eases this object into suicide
		void requestStop();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void workInProgress();
		void queryModelReorganization();
		void jobFinished();
		void accomplishedWorkPercentage(const int&, const QString&, const QString&);
		void log(bool, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::DataModel::DatabaseQuery* _query;
		QStandardItemModel* _model;
		Core::ObjectCache* _cache;
		EventListFilter _filter;
		qint32 _displayedDecimals;
		QList<Seiscomp::DataModel::EventPtr>* _events;
		QList<Seiscomp::DataModel::OriginPtr>* _origins;
		Seiscomp::Core::Time _from;
		Seiscomp::Core::Time _to;
		bool _fetchUnassociated;
		bool _stopRequested;
		QMutex _mutex;
};



} // namespace Gui
} // namespace IPGP

#endif
