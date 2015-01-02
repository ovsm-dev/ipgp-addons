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


#include <ipgp/gui/datamodel/eventlist/eventlistdelegate.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/types.h>
#include <ipgp/core/geo/geo.h>
#include <ipgp/core/math/math.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/datamodel/misc.h>
#include <QStandardItemModel>
#include <ipgp/gui/datamodel/qcustomstandarditem.h>
#include <QDebug>

using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::Geo;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

void freePtrs(QList<QStandardItem*>& ptrlist) {
	for (int i = 0; i < ptrlist.size(); ++i) {
		if ( !ptrlist.at(i) )
			continue;
		delete ptrlist.takeAt(i);
		ptrlist[i] = NULL;
	}
	ptrlist.clear();
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListDelegate::EventListFilter::EventListFilter() {

	filterEvaluationMode = false;
	filterMethodID = false;
	filterEarthModelID = false;
	filterDepth = false;
	filterLatitude = false;
	filterLongitude = false;
	filterRms = false;
	filterMagnitude = false;
	filterEventType = false;
	filterEventCertainty = false;
	filterOriginStatus = false;
	filterFakeEvent = false;
	filterUnlocalizedEvent = false;
	filterExternalEvents = false;
	filterUnlocalizedEvents = false;
	fetchEventComment = false;
	fetchPreferredOriginComment = false;
	fetchSiblingOriginComment = false;
	fetchUnassociatedOriginComment = false;
	showEventColorType = false;

	rmsMin = -999.;
	rmsMax = 999.;
	latitudeMin = -999.;
	latitudeMax = 999.;
	longitudeMin = -999.;
	longitudeMax = 999.;
	depthMin = -999.;
	depthMax = 999.;
	magnitudeMin = -999.;
	magnitudeMax = 999.;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListDelegate::EventListDelegate(DatabaseQuery* query,
                                     QStandardItemModel* model,
                                     Core::ObjectCache* cache,
                                     Seiscomp::Core::Time from,
                                     Seiscomp::Core::Time to,
                                     const EventListFilter& filter,
                                     QList<Seiscomp::DataModel::EventPtr>* events,
                                     QList<Seiscomp::DataModel::OriginPtr>* origins,
                                     const bool& fetchUnassociated) :
		_query(query), _model(model), _cache(cache), _filter(filter),
		_displayedDecimals(2), _events(events), _origins(origins), _from(from),
		_to(to), _fetchUnassociated(fetchUnassociated), _stopRequested(false) {

//	qRegisterMetaType<QString>("QVariant<QString>");

	qRegisterMetaType<IPGP::Gui::EventTypeCast>("EventTypeCast");
	qRegisterMetaTypeStreamOperators<int>("EventTypeCast");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListDelegate::~EventListDelegate() {
	emit log(true, "EventList delegate is terminated");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::populateModel() {
	getObjects(_from, _to, _filter, _fetchUnassociated);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::requestStop() {
	QMutexLocker locker(&_mutex);
	_stopRequested = true;
	qDebug() << "Received stop request!!";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::getObjects(const Time& from, const Time& to,
                                   const EventListFilter& filter,
                                   const bool& fetchUnassociated) {

	_origins->clear();
	_events->clear();
	_filter = filter;

	emit workInProgress();

	Util::StopWatch timer;

	getEvents(from, to);

	// Ask the widget to sort the treeView according to defined order so
	// that objects could be in legit position before adding unassociated
	// origins to it
	emit queryModelReorganization();

	if ( fetchUnassociated )
		getUnassociatedOrigins(from, to);

	emit log(true, QString("Delegate job finished in %1 sec")
	    .arg(Time(timer.elapsed()).toString("%T.%f").c_str()));

	emit jobFinished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventListDelegate::stopRequested() {
	QMutexLocker locker(&_mutex);
	return _stopRequested;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::getEvents(const Time& from, const Time& to) {

	Util::StopWatch timer;
	QList<EventPtr> eventList;
	DatabaseIterator it = _query->getEvents(from, to);
	for (; *it; ++it) {

		if ( stopRequested() )
			break;

		EventPtr evt = Event::Cast(*it);
		if ( evt && !eventList.contains(evt) )
			eventList.append(evt);
	}

	emit log(true, QString("%1 event(s) fetched in %2 sec").arg(eventList.size())
	    .arg(Time(timer.elapsed()).toString("%T.%f").c_str()));

	if ( !stopRequested() )
		addEvents(eventList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::getUnassociatedOrigins(const Time& from, const Time& to) {

	std::string query;
	query += "SELECT POrigin.publicID,Origin.* FROM Origin,PublicObject "
		"AS POrigin WHERE Origin._oid=POrigin._oid AND Origin.time_value >='";
	query += toString(from) + "' AND Origin.time_value <='" + toString(to) + "' ";
	query += "ORDER BY Origin.creationInfo_creationTime DESC";

	Util::StopWatch timer;
	QList<OriginPtr> originList;

	DatabaseIterator it = _query->getObjectIterator(query, Origin::TypeInfo());
	for (; *it; ++it) {

		if ( stopRequested() )
			break;

		OriginPtr org = Origin::Cast(*it);
		if ( org && !originList.contains(org) )
			originList.append(org);
	}

	emit log(true, QString("%1 origin(s) fetched in %2 sec").arg(originList.size())
	    .arg(Time(timer.elapsed()).toString("%T.%f").c_str()));

	if ( !stopRequested() )
		addUnassociatedOrigins(originList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::addEvents(const QList<EventPtr>& list) {

	Util::StopWatch timer;

	for (int i = 0; i < list.size(); ++i) {

		if ( stopRequested() )
			break;

		addEventItem(list.at(i));
		try {
			emit accomplishedWorkPercentage(Core::Math::percentageOfSomething<
			        int>(list.size(), i),
			    "EventListDelegate", "Loading events...");
		} catch ( ... ) {}
	}

	emit accomplishedWorkPercentage(-1, "EventListDelegate", "");
	emit log(true, QString("%1 event(s) dispatched in %2 sec").arg(list.size())
	    .arg(Time(timer.elapsed()).toString("%T.%f").c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::addEventItem(EventPtr event) {

	if ( !event )
		return;

	//! NOTE: filter everything that need to be filtered before calling
	//! 'new' on any pointers, otherwise dangling objects will for sure
	//! be the cause of widget's instability, memory consumption, and
	//! furthermore the parent application's instability.
	//! TODO: Try and use a TreeViewWidget instead of Model+Items and
	//! let Qt handle the created objects properly when the clear()
	//! method gets used.
	QList<QStandardItem*> ptrs;

	Regions region;
	bool eventIsFake = false;
	QFont boldFont;
	boldFont.setBold(true);

	try {
		if ( _filter.filterEventType )
			if ( event->type().toString() != _filter.evenType )
				return;

		if ( _filter.filterFakeEvent
		    && event->type() == NOT_EXISTING )
			return;

		if ( _filter.filterUnlocalizedEvent
		    && event->type() == NOT_LOCATABLE )
			return;

		if ( _filter.filterExternalEvents
		    && event->type() == OUTSIDE_OF_NETWORK_INTEREST )
			return;

		if ( event->type() == NOT_EXISTING )
			eventIsFake = true;
	}
	catch ( ... ) {

		// This way events with no type set won't get thru the filter
		// if this is set up.
		if ( _filter.filterEventType )
			return;
	}

	QString eventCertainty;
	try {
		eventCertainty = event->typeCertainty().toString();
		if ( _filter.filterEventCertainty )
			if ( event->typeCertainty().toString() == _filter.eventCertainty )
				return;
	}
	catch ( ... ) {
		if ( _filter.filterEventCertainty )
			return;
	}

	MagnitudePtr mag = _cache->getObjectByOwnID<MagnitudePtr>(event->preferredMagnitudeID());
	if ( !mag ) {
		mag = Magnitude::Find(event->preferredMagnitudeID());
		if ( !mag ) {
			PublicObjectPtr obj = _query->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID());
			mag = Magnitude::Cast(obj);
		}
	}

	if ( mag ) {
		_cache->addObject(ObjectCache::MultiKey(mag->publicID(), event->publicID()), mag);
		try {
			if ( _filter.filterMagnitude )
				if ( mag->magnitude().value() < _filter.magnitudeMin
				    || mag->magnitude().value() > _filter.magnitudeMax )
					return;
		}		catch ( ... ) {}
	}

	if ( !mag && _filter.filterMagnitude )
		return;

	OriginPtr prefOrigin = _cache->getObjectByOwnID<OriginPtr>(event->preferredOriginID());
	if ( !prefOrigin ) {
		prefOrigin = Origin::Find(event->preferredOriginID());

		if ( !prefOrigin ) {
			PublicObjectPtr obj1 = _query->getObject(Origin::TypeInfo(), event->preferredOriginID());
			prefOrigin = Origin::Cast(obj1);
		}

		if ( prefOrigin )
			_cache->addObject(ObjectCache::MultiKey(prefOrigin->publicID(), event->publicID()), prefOrigin);
	}

	QString eventEarthModelID;
	try {
		eventEarthModelID = prefOrigin->earthModelID().c_str();
		if ( _filter.filterEarthModelID )
			if ( prefOrigin->earthModelID() != _filter.earthModelID.toStdString() )
				return;
	}	catch ( ... ) {}

	QStandardItem* eventMethodID;
	try {
		if ( _filter.filterMethodID )
			if ( prefOrigin->methodID() != _filter.methodID.toStdString() )
				return;

		eventMethodID = new QStandardItem(QString::fromUtf8(prefOrigin->methodID().c_str()));
	}
	catch ( ... ) {
		eventMethodID = new QStandardItem("-");
	}
	ptrs << eventMethodID;

	if ( _filter.filterLatitude )
		if ( prefOrigin->latitude().value() < _filter.latitudeMin
		    || prefOrigin->latitude().value() > _filter.latitudeMax )
			return;

	if ( _filter.filterLongitude )
		if ( prefOrigin->longitude().value() < _filter.longitudeMin
		    || prefOrigin->longitude().value() > _filter.longitudeMax )
			return;

	try {
		if ( _filter.filterRms )
			if ( prefOrigin->quality().standardError() < _filter.rmsMin
			    || prefOrigin->quality().standardError() > _filter.rmsMax )
				return;
	}	catch ( ... ) {}


	double depthValue = .0;
	QCustomStandardItem* depth;
	try {
		depth = new QCustomStandardItem(QString("%1 km")
		    .arg(QString::number(prefOrigin->depth().value(), 'f', _displayedDecimals)),
		    QCustomStandardItem::NumberUnitRole);
		depthValue = prefOrigin->depth().value();
	}
	catch ( ... ) {
		depth = new QCustomStandardItem("-");
	}
	depth->setTextAlignment(Qt::AlignRight);
	ptrs << depth;

	if ( _filter.filterDepth ) {
		if ( depthValue < _filter.depthMin || depthValue > _filter.depthMax ) {
			freePtrs(ptrs);
			return;
		}
	}

	QStandardItem* eventType;
	QColor eventTypeColor = QColor(Qt::white);
	try {
		eventType = new QStandardItem(QString::fromUtf8(event->type().toString()));

		// Store event type in item's data so it will be accessible outside
		// of this iteration, allowing for outside methods to be aware of
		// event's type without requesting the whole shebang...
//		QVariant data;
//		data.setValue(QString(event->type().toString()));
		eventType->setData(QVariant(getEventTypeCast(event->type())));

		// Background row color
		if ( event->type() == NOT_LOCATABLE )
			eventTypeColor = Misc::lightYellow2();
		else if ( event->type() == NOT_EXISTING )
			eventTypeColor = Misc::lightOrange();
		else if ( event->type() == OUTSIDE_OF_NETWORK_INTEREST )
			eventTypeColor = Misc::lightYellow1();
	}
	catch ( ... ) {
		eventType = new QStandardItem("-");
		eventType->setData(QVariant(etc_NOT_SET));
		eventTypeColor = Misc::lightRed();
	}
	ptrs << eventType;

	QStandardItem* magnitude;
	QStandardItem* magnitudeType;
	if ( mag ) {
		try {
			magnitude = new QStandardItem(QString("%1")
			    .arg(QString::number(Core::Math::roundDouble(mag->magnitude().value(), 1), 'f', 1)));
		}		catch ( ... ) {}
		try {
			magnitudeType = new QStandardItem(mag->type().c_str());
		}		catch ( ... ) {}
	}
	else {
		magnitude = new QStandardItem(QString("-"));
		magnitudeType = new QStandardItem(QString("-"));
	}
	magnitude->setFont(boldFont);
	magnitude->setTextAlignment(Qt::AlignHCenter);
	ptrs << magnitude;
	ptrs << magnitudeType;


	std::string evaluationMode;
	try {
		evaluationMode = prefOrigin->evaluationMode().toString();
	}	catch ( ... ) {}

	std::string evaluationStatus;
	try {
		evaluationStatus = prefOrigin->evaluationStatus().toString();
		if ( _filter.filterOriginStatus )
			if ( prefOrigin->evaluationStatus().toString() != _filter.originStatus ) {
				freePtrs(ptrs);
				return;
			}
	}
	catch ( ... ) {
		if ( _filter.filterOriginStatus ) {
			freePtrs(ptrs);
			return;
		}
	}

	QStandardItem* mode;
	if ( !evaluationMode.empty() ) {

		if ( _filter.filterEvaluationMode )
			if ( evaluationMode != _filter.evaluationMode.toStdString() ) {
				freePtrs(ptrs);
				return;
			}

		QString modeStr;
		QBrush b;
		if ( evaluationMode == "automatic" ) {
			modeStr = "A";
			b.setColor(Qt::red);
		}
		else if ( evaluationMode == "manual" ) {
			modeStr = "M";
			b.setColor(Qt::darkGreen);
		}
		else {
			modeStr = evaluationMode.c_str();
			b.setColor(Qt::blue);
		}

		if ( evaluationStatus == "confirmed" && evaluationMode == "automatic" ) {
			modeStr = "C";
			b.setColor(Qt::red);
		}

		mode = new QStandardItem(modeStr);
		mode->setForeground(b);
	}
	else
		mode = new QStandardItem("-");
	mode->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	ptrs << mode;

	QCustomStandardItem* phases;
	try {
		phases = new QCustomStandardItem(QString("%1")
		    .arg(prefOrigin->quality().usedPhaseCount()),
		    QCustomStandardItem::NumberUnitRole);
//		phases = new QStandardItem(toString(prefOrigin->quality().usedPhaseCount()).c_str());
	}
	catch ( ... ) {
		phases = new QCustomStandardItem("-");
	}
	phases->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	ptrs << phases;


	QStandardItem* eventQuality;
	try {
		eventQuality = new QStandardItem(prefOrigin->quality().groundTruthLevel().c_str());
	}
	catch ( ... ) {
		eventQuality = new QStandardItem("");
	}
	eventQuality->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	ptrs << eventQuality;


	QString comment = "-";
	QStandardItem* seismicCode = new QStandardItem("-");
	seismicCode->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

	// Fetch event comment and retrieve seismic code
	if ( event->commentCount() == 0 )
		_query->loadComments(event.get());
	for (size_t i = 0; i < event->commentCount(); ++i) {

		if ( !event->comment(i) )			continue;

		if ( i == 0 ) {
			comment = "";
			QStringList v = QString(event->comment(i)->text().c_str()).split("|");
			if ( v.size() > 1 )
				seismicCode->setText(v.at(1));
		}
		comment.append(QString::fromUtf8(event->comment(i)->text().c_str()));
	}
	ptrs << seismicCode;

	if ( !_filter.eventSCODE.contains("*") ) {
		int count = 0;
		for (int i = 0; i < _filter.eventSCODE.size(); ++i) {

			//! Search for wildcard char '*' : this means that every string
			//! which starting characters up until '*' char is good
			size_t pos = _filter.eventSCODE.at(i).toStdString().find_first_of("*");
			if ( pos != std::string::npos ) {
				if ( !seismicCode->text().contains(_filter.eventSCODE.at(i).left(pos)) )
					count++;
			}
			else {
				//! No wildcard in this string so we look for the full string length
				if ( !_filter.eventSCODE.at(i).contains(seismicCode->text()) )
					count++;
			}
		}

		//! If the final count is > 0 then this event's scode has been
		//! blacklisted at least once and should not be added to the model
		if ( count == _filter.eventSCODE.size() ) {
			freePtrs(ptrs);
			return;
		}
	}

	QStandardItem* orgAgency;
	try {
		orgAgency = new QStandardItem(QString::fromUtf8(prefOrigin->creationInfo().agencyID().c_str()));
	}
	catch ( ... ) {
		orgAgency = new QStandardItem("-");
	}

	QStandardItem* orgAuthor;
	try {
		orgAuthor = new QStandardItem(QString::fromUtf8(prefOrigin->creationInfo().author().c_str()));
	}
	catch ( ... ) {
		orgAuthor = new QStandardItem("-");
	}

	QStandardItem* eventTime = new QStandardItem(prefOrigin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str());
	eventTime->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

	QList<QStandardItem*> eventRow;
	eventRow << eventTime;
	eventRow << new QStandardItem(eventCertainty);
	eventRow << eventType;
	eventRow << seismicCode;
	eventRow << magnitude;
	eventRow << magnitudeType;
	eventRow << eventQuality;
	eventRow << eventMethodID;
	eventRow << phases;
	eventRow << new QStandardItem(getStringPosition(prefOrigin->latitude().value(), Latitude).c_str());
	eventRow << new QStandardItem(getStringPosition(prefOrigin->longitude().value(), Longitude).c_str());
	eventRow << depth;
	eventRow << mode;
	eventRow << orgAgency;
	eventRow << orgAuthor;
	eventRow << new QStandardItem(QString::fromUtf8(region.getRegionName(
	    prefOrigin->latitude().value(), prefOrigin->longitude().value()).c_str()));
	eventRow << new QStandardItem(event->publicID().c_str());


	if ( _filter.fetchPreferredOriginComment ) {

		if ( prefOrigin->commentCount() == 0 )
			_query->loadComments(prefOrigin.get());

		for (size_t i = 0; i < prefOrigin->commentCount(); ++i) {

			if ( !prefOrigin->comment(i) )
				continue;

			if ( i == 0 && comment != "-" )
				comment.append(QString::fromUtf8(prefOrigin->comment(i)->text().c_str()));
		}
	}

//	QStringList sl = comment.split(" <> ");
//	QString formatedComment;
//	for (int i = 0; i < sl.size(); i++)
//		formatedComment.append(sl.at(i)), formatedComment.append("<br/>");

	QString eventToolTip = QString(
	    "<b>OT(GMT)</b>: %1<br/>"
		    "<b>Type</b>: %2<br/>"
		    "<b>SCODE</b>: %3<br/>"
		    "<b>Mag.</b>: %4 (%5)<br/>"
		    "<b>Phases</b>: %6<br/>"
		    "<b>Lat.</b>: %7<br/>"
		    "<b>Lon</b>: %8<br/>"
		    "<b>Depth</b>: %9<br/>"
		    "<b>MethodID</b>: %10<br/>"
		    "<b>EarthModelID</b>: %11<br/>"
		    "<b>Status</b>: %12<br/>"
		    "<b>Agency</b>: %13<br/>"
		    "<b>Author</b>: %14<br/>"
		    "<b>Region</b>: %15<br/>"
		    "<b>ID</b>: %16<br/>"
		    "<b>Observations</b>: %17")
	    .arg(prefOrigin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str())
	    .arg(eventType->text()).arg(seismicCode->text()).arg(magnitude->text())
	    .arg(magnitudeType->text()).arg(phases->text())
	    .arg(getStringPosition(prefOrigin->latitude().value(), Latitude).c_str())
	    .arg(getStringPosition(prefOrigin->longitude().value(), Longitude).c_str())
	    .arg(depth->text()).arg(eventMethodID->text()).arg(eventEarthModelID)
	    .arg(mode->text()).arg(orgAgency->text()).arg(orgAuthor->text())
	    .arg(QString::fromUtf8(region.getRegionName(prefOrigin->latitude().value(),
	    prefOrigin->longitude().value()).c_str()))
	    .arg(event->publicID().c_str()).arg(comment);

//	QString eventToolTip = QString("<b>OT(GMT)</b>: %1<br/><b>Type</b>: %2<br/>"
//		"<b>MethodID</b>: %3<br/><b>EarthModelID</b>: %4<br/><b>M</b>: %5 (%6)<br/>"
//		"<b>Phases</b>: %7<br/><b>Comment</b>: %8")
//	        .arg(prefOrigin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str())
//	        .arg(eventType->text()).arg(eventMethodID->text()).arg(eventEarthModelID)
//	        .arg(magnitude->text()).arg(magnitudeType->text()).arg(phases->text()).arg(comment);

	if ( eventIsFake )
		for (int i = 0; i < eventRow.size(); ++i)
			eventRow.at(i)->setForeground(Qt::lightGray);

	for (int i = 0; i < eventRow.size(); ++i) {

		eventRow.at(i)->setToolTip(eventToolTip);

		//! Paint entire row with background color to make singularities pop out
		//! of the list, this is quite useful when events from multiple agency
		//! are present in the list, therefore we can spot'em easily...
		if ( _filter.showEventColorType )
			eventRow.at(i)->setBackground(eventTypeColor);
	}

	_model->appendRow(eventRow);

	QFont italicFont;
	italicFont.setItalic(true);
	QStandardItem* parentNode = new QStandardItem("Origins");
	parentNode->setFont(italicFont);
	eventTime->appendRow(parentNode);

	if ( eventIsFake )
		parentNode->setForeground(Qt::lightGray);

	QStandardItem* ot = new QStandardItem(prefOrigin->time().value().toString("... %H:%M:%S").c_str());
	ot->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
	ot->setFont(boldFont);

	QList<QStandardItem*> originRow;
	originRow << ot;
	originRow << new QStandardItem("");
	originRow << new QStandardItem("");
	originRow << new QStandardItem("");
	originRow << new QStandardItem("");
	originRow << new QStandardItem("");
	originRow << eventQuality->clone();
	originRow << eventMethodID->clone();
	originRow << phases->clone();
	originRow << new QStandardItem(getStringPosition(prefOrigin->latitude().value(), Latitude).c_str());
	originRow << new QStandardItem(getStringPosition(prefOrigin->longitude().value(), Longitude).c_str());
	originRow << depth->clone();
	originRow << mode->clone();
	originRow << orgAgency->clone();
	originRow << orgAuthor->clone();
	originRow << new QStandardItem(QString::fromUtf8(region.getRegionName(
	    prefOrigin->latitude().value(), prefOrigin->longitude().value()).c_str()));
	originRow << new QStandardItem(prefOrigin->publicID().c_str());


	for (int i = 0; i < originRow.size(); ++i)
		originRow.at(i)->setToolTip(eventToolTip);

	parentNode->appendRow(originRow);

	std::vector<OriginPtr> list = _cache->getObjectsByParentID<OriginPtr>(event->publicID());
	if ( list.size() == 0 ) {

		DatabaseIterator it = _query->getOrigins(event->publicID());
		for (; *it; ++it) {
			OriginPtr child = Origin::Cast(*it);

			if ( !child )
				continue;

			if ( child->publicID() == event->preferredOriginID() )
				continue;

			list.push_back(child);
			_cache->addObject(ObjectCache::MultiKey(child->publicID(), event->publicID()), child);
		}
	}


	for (size_t i = 0; i < list.size(); ++i) {

		OriginPtr child = list.at(i);

		QCustomStandardItem* childDepth;
		try {
			childDepth = new QCustomStandardItem(QString("%1 km")
			    .arg(QString::number(child->depth().value(), 'f', _displayedDecimals)),
			    QCustomStandardItem::NumberUnitRole);
		}
		catch ( ... ) {
			childDepth = new QCustomStandardItem("-");
		}
		childDepth->setTextAlignment(Qt::AlignRight);

		std::string childEvaluationStatus;
		try {
			childEvaluationStatus = child->evaluationStatus().toString();
		}
		catch ( ... ) {}

		QStandardItem* childMode;
		std::string childEvaluationMode;
		try {
			childEvaluationMode = child->evaluationMode().toString();
		}
		catch ( ... ) {}

		if ( !childEvaluationMode.empty() ) {

			QString modeStr;
			QBrush b;
			if ( childEvaluationMode == "automatic" ) {
				modeStr = "A";
				b.setColor(Qt::red);
			}
			else if ( childEvaluationMode == "manual" ) {
				modeStr = "M";
				b.setColor(Qt::darkGreen);
			}
			else {
				modeStr = childEvaluationMode.c_str();
				b.setColor(Qt::blue);
			}

			if ( childEvaluationStatus == "confirmed" && childEvaluationMode == "automatic" ) {
				modeStr = "C";
				b.setColor(Qt::red);
			}

			childMode = new QStandardItem(modeStr);
			childMode->setForeground(b);
		}
		else
			childMode = new QStandardItem("-");

		childMode->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		QCustomStandardItem* childPhases;
		try {
			childPhases = new QCustomStandardItem(QString("%1")
			    .arg(child->quality().usedPhaseCount()));
		}
		catch ( ... ) {
			childPhases = new QCustomStandardItem("-");
		}
		childPhases->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		QStandardItem* childMethodID;
		try {
			childMethodID = new QStandardItem(QString::fromUtf8(child->methodID().c_str()));
		}
		catch ( ... ) {
			childMethodID = new QStandardItem("-");
		}

		QString childEarthModelID;
		try {
			childEarthModelID = child->earthModelID().c_str();
		}
		catch ( ... ) {}

		QStandardItem* childQuality;
		try {
			childQuality = new QStandardItem(child->quality().groundTruthLevel().c_str());
		}
		catch ( ... ) {
			childQuality = new QStandardItem("");
		}
		childQuality->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		QStandardItem* childAgency;
		try {
			childAgency = new QStandardItem(QString::fromUtf8(child->creationInfo().agencyID().c_str()));
		}
		catch ( ... ) {
			childAgency = new QStandardItem("-");
		}

		QStandardItem* childAuthor;
		try {
			childAuthor = new QStandardItem(QString::fromUtf8(child->creationInfo().author().c_str()));
		}
		catch ( ... ) {
			childAuthor = new QStandardItem("-");
		}

		QStandardItem* childTime = new QStandardItem(child->time().value().toString("     ... %H:%M:%S").c_str());
		childTime->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

		QList<QStandardItem*> childRow;
		childRow << childTime;
		childRow << new QStandardItem("");
		childRow << new QStandardItem("");
		childRow << new QStandardItem("");
		childRow << new QStandardItem("");
		childRow << new QStandardItem("");
		childRow << childQuality;
		childRow << childMethodID;
		childRow << childPhases;
		childRow << new QStandardItem(getStringPosition(child->latitude().value(), Latitude).c_str());
		childRow << new QStandardItem(getStringPosition(child->longitude().value(), Longitude).c_str());
		childRow << childDepth;
		childRow << childMode;
		childRow << childAgency;
		childRow << childAuthor;
		childRow << new QStandardItem(QString::fromUtf8(region.getRegionName(
		    child->latitude().value(), child->longitude().value()).c_str()));
		childRow << new QStandardItem(child->publicID().c_str());


		QString childComment = "-";
		if ( _filter.fetchSiblingOriginComment ) {

			if ( child->commentCount() == 0 )
				_query->loadComments(child.get());

			for (size_t i = 0; i < child->commentCount(); ++i) {

				if ( !child->comment(i) )
					continue;

				if ( i == 0 )
					childComment = "";
				childComment.append(QString::fromUtf8(child->comment(i)->text().c_str()));
			}
		}

//		QStringList sl = childComment.split(" <> ");
//		QString formatedChildComment;
//		for (int i = 0; i < sl.size(); i++)
//			formatedChildComment.append(sl.at(i)), formatedChildComment.append("<br/>");

		QString childToolTip = QString("<b>OT(GMT)</b>: %1<br/><b>MethodID</b>: %2<br/>"
			"<b>EarthModelID</b>: %3<br/><b>Phases</b>: %4<br/><b>Comment</b>: %5")
		    .arg(child->time().value().toString("%Y-%m-%d %H:%M:%S").c_str())
		    .arg(childMethodID->text()).arg(childEarthModelID).
		    arg(childPhases->text()).arg(childComment);


		if ( eventIsFake )
			for (int i = 0; i < childRow.size(); ++i)
				childRow.at(i)->setForeground(Qt::lightGray);

		for (int i = 0; i < childRow.size(); ++i)
			childRow.at(i)->setToolTip(childToolTip);

		parentNode->appendRow(childRow);
	}

	_events->append(event);
	_origins->append(prefOrigin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListDelegate::addUnassociatedOrigins(const QList<OriginPtr>& list) {

	//! NOTE
	//! No filter shall be applied here! Unassociated origins have to be free
	//! of use so the user can add them to an event if that's so

	Regions region;

	QStandardItem* unassociated = new QStandardItem("Unassociated");
	unassociated->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

	QList<QStandardItem*> ilist;
	ilist << unassociated;
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("<>");
	_model->appendRow(ilist);

	QFont font;
	font.setItalic(true);
	QStandardItem* parent = new QStandardItem("Origins");
	parent->setFont(font);

	ilist.clear();
	ilist << parent;
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");
	ilist << new QStandardItem("");

	unassociated->appendRow(ilist);

	Util::StopWatch timer;

	// If origin has a parent whom isn't itself, it's a unassociated origin
	// and therefore it should stay in the list...
	std::vector<OriginPtr> unassociatedOrigins;
	for (int i = 0; i < list.size(); ++i) {
		ObjectCache::ObjectInfo info = _cache->getObjectInfo<OriginPtr>(list.at(i)->publicID());
		if ( info != ObjectCache::ObjectHasAParent )
			unassociatedOrigins.push_back(list.at(i));
	}

	int count = 0;
	for (size_t i = 0; i < unassociatedOrigins.size(); ++i) {

		OriginPtr origin = unassociatedOrigins.at(i);

		if ( !_cache->getUnassociatedObject<OriginPtr>(origin->publicID()) ) {
			EventPtr event = _query->getEvent(origin->publicID());

			if ( event )
				continue;

			_cache->addObject(ObjectCache::MultiKey(origin->publicID(), origin->publicID()), origin);
		}

		++count;

		QCustomStandardItem* depth;
		try {
			depth = new QCustomStandardItem(QString("%1 km")
			    .arg(QString::number(origin->depth().value(), 'f', _displayedDecimals)),
			    QCustomStandardItem::NumberUnitRole);
		}
		catch ( ... ) {
			depth = new QCustomStandardItem("-");
		}
		depth->setTextAlignment(Qt::AlignRight);

		std::string evaluationStatus;
		try {
			evaluationStatus = origin->evaluationStatus().toString();
		}
		catch ( ... ) {}

		QStandardItem* mode;
		std::string evaluationMode;
		try {
			evaluationMode = origin->evaluationMode().toString();
		}
		catch ( ... ) {}

		if ( !evaluationMode.empty() ) {

			QString modeStr;
			QBrush b;
			if ( evaluationMode == "automatic" ) {
				modeStr = "A";
				b.setColor(Qt::red);
			}
			else if ( evaluationMode == "manual" ) {
				modeStr = "M";
				b.setColor(Qt::darkGreen);
			}
			else {
				modeStr = evaluationMode.c_str();
				b.setColor(Qt::blue);
			}

			if ( evaluationStatus == "confirmed" && evaluationMode == "automatic" ) {
				modeStr = "C";
				b.setColor(Qt::red);
			}

			mode = new QStandardItem(modeStr);
			mode->setForeground(b);
		}
		else
			mode = new QStandardItem("-");
		mode->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		QCustomStandardItem* phases;
		try {
			phases = new QCustomStandardItem(QString("%1").arg(origin->quality().usedPhaseCount()));
		}
		catch ( ... ) {
			phases = new QCustomStandardItem("");
		}
		phases->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		QStandardItem* methodID;
		try {
			methodID = new QStandardItem(QString::fromUtf8(origin->methodID().c_str()));
		}
		catch ( ... ) {
			methodID = new QStandardItem("-");
		}
		QString earthModelID;
		try {
			earthModelID = origin->earthModelID().c_str();
		}
		catch ( ... ) {}

		QStandardItem* quality;
		try {
			quality = new QStandardItem(origin->quality().groundTruthLevel().c_str());
		}
		catch ( ... ) {
			quality = new QStandardItem("");
		}
		quality->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		QStandardItem* time = new QStandardItem(origin->time().value().toString("     ... %H:%M:%S").c_str());
		time->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

		QList<QStandardItem*> originRow;
		originRow << time;
		originRow << new QStandardItem("");
		originRow << new QStandardItem("");
		originRow << new QStandardItem("");
		originRow << new QStandardItem("");
		originRow << new QStandardItem("");
		originRow << quality;
		originRow << methodID;
		originRow << phases;
		originRow << new QStandardItem(getStringPosition(origin->latitude().value(), Latitude).c_str());
		originRow << new QStandardItem(getStringPosition(origin->longitude().value(), Longitude).c_str());
		originRow << depth;
		originRow << mode;
		originRow << new QStandardItem(QString::fromUtf8(origin->creationInfo().agencyID().c_str()));
		originRow << new QStandardItem(QString::fromUtf8(origin->creationInfo().author().c_str()));
		originRow << new QStandardItem(QString::fromUtf8(region.getRegionName(
		    origin->latitude().value(), origin->longitude().value()).c_str()));
		originRow << new QStandardItem(origin->publicID().c_str());


		QString comment = "-";
		if ( _filter.fetchUnassociatedOriginComment ) {
			if ( origin->commentCount() == 0 )
				_query->loadComments(origin.get());

			for (size_t i = 0; i < origin->commentCount(); ++i) {

				if ( !origin->comment(i) )
					continue;

				if ( i == 0 )
					comment = "";
				comment.append(QString::fromUtf8(origin->comment(i)->text().c_str()));
			}
		}

//		QStringList sl = comment.split(" <> ");
//		QString formatedComment;
//		for (int i = 0; i < sl.size(); i++)
//			formatedComment.append(sl.at(i)), formatedComment.append("\n");

		QString childToolTip = QString("<b>OT(GMT)</b>: %1<br/><b>MethodID</b>: %2<br/>"
			"<b>EarthModelID</b>: %3<br/><b>Phases</b>: %4<br/><b>Comment</b>: %5")
		    .arg(origin->time().value().toString("%Y-%m-%d %H:%M:%S").c_str())
		    .arg(methodID->text()).arg(earthModelID).arg(phases->text()).arg(comment);

		for (int i = 0; i < originRow.size(); ++i)
			originRow.at(i)->setToolTip(childToolTip);

		parent->appendRow(originRow);
	}

	emit log(true, QString("%1 unassociated origin(s) out of %2 dispatched in %3 sec")
	    .arg(count).arg(unassociatedOrigins.size())
	    .arg(Time(timer.elapsed()).toString("%T.%f").c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
