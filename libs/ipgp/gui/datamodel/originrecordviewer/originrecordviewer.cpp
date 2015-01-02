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

#define SEISCOMP_COMPONENT IPGP_ORIGINRECORDVIEWER

#include <ipgp/gui/datamodel/originrecordviewer/originrecordviewer.h>
#include <ipgp/gui/datamodel/streamdelegate.h>
#include <ipgp/core/string/string.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/math/math.h>

#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/logging/log.h>

#include <seiscomp3/processing/streambuffer.h>
#include <seiscomp3/processing/amplitudes/ML.h>

#include <seiscomp3/math/filter/chainfilter.h>
#include <seiscomp3/math/filter/abs.h>
#include <seiscomp3/math/filter/const.h>
#include <seiscomp3/math/filter/rca.h>
#include <seiscomp3/math/filter/op2filter.h>
#include <seiscomp3/math/filter/stalta.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/math/filter/iirintegrate.h>
#include <seiscomp3/math/filter/iirdifferentiate.h>
#include <seiscomp3/math/filter/taper.h>
#include <seiscomp3/math/filter/rmhp.h>
#include <seiscomp3/math/filter/biquad.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/math/filter/taper.h>

#include <QMenu>

using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::String;

namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordViewer::
OriginRecordViewer(DatabaseQuery* query, Core::RecordCache* cache,
                   const StreamView& sv, QWidget* parent, Qt::WFlags f) :
		RecordViewer(parent, f, onMouseRelease, Scroll), _query(query),
		_filter(NULL), _cache(cache), _origin(NULL), _sv(sv), _picker(NULL) {

	prepareEntities(sv);

	_autoRescaleStreamsKeyAxis = false;
	_autoRescaleStreamsValueAxis = true;
	_resetRequired = true;

	_timeWindow = 360.;
	_waveTraceColor = Misc::defaultTraceColor;

	_autoFetchRecords = true;

	_recordStreamUrl = "slink://localhost:18000";
	_recordInputHint = Record::DATA_ONLY;

	_usedStream = new QAction(this);

	_picker = new OriginRecordPicker(this, _sc);
	_picker->resize(_sc->size());
	_picker->show(); //! Force show, otherwise it won't show up^^
	connect(this, SIGNAL(requestPickerUpdate()), _picker, SLOT(timeLineUpdated()));
	connect(_sc, SIGNAL(resizeRequested(const QSize&)), _picker, SLOT(resizeRequested(const QSize&)));

	preparePickerComponents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginRecordViewer::~OriginRecordViewer() {

	if ( _filter )
	    delete _filter;
	_filter = NULL;

	closeStream();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::prepareEntities(const StreamView& sv) {

	removeAllRecordEntities();

	//! Add entities in an ordered way
	addRecordEntity("Z");
	if ( sv & Frequency )
	    addRecordEntity("Zf");
	addRecordEntity("1");
	if ( sv & Frequency )
	    addRecordEntity("1f");
	addRecordEntity("2");
	if ( sv & Frequency )
	    addRecordEntity("2f");

	if ( sv & Frequency ) {
		recordEntity("Zf").setType(RecordEntity::Frequency);
		recordEntity("1f").setType(RecordEntity::Frequency);
		recordEntity("2f").setType(RecordEntity::Frequency);
	}

	setupRecordEntities();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::preparePickerComponents() {

	if ( !_picker ) return;

	_picker->nativeReset();
	_picker->addComponent("Z", "Z");
	if ( _sv & Frequency )
	    _picker->addComponent("Zf", "Zf", true);
	_picker->addComponent("1", "1");
	if ( _sv & Frequency )
	    _picker->addComponent("1f", "1f", true);
	_picker->addComponent("2", "2");
	if ( _sv & Frequency )
	    _picker->addComponent("2f", "2f", true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::showStationStream(const QString& comp) {

	if ( comp.length() != 2 ) return;

	for (int i = 0; i < _activeStreams.size(); ++i) {
		if ( _activeStreams.at(i).channelCode.mid(0, 2) == comp )
			_activeStreams[i].inUse = true;
		else
			_activeStreams[i].inUse = false;
	}

	drawRecordTraces();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::setOrigin(const std::string& publicID) {

	if ( (_origin && _origin->publicID() != publicID) || !_origin ) {

		if ( !_query ) {
			SEISCOMP_ERROR("No query interface available");
			return;
		}

		PublicObjectPtr obj = _query->getObject(Origin::TypeInfo(), publicID);
		_origin = Origin::Cast(obj.get());

		if ( !_origin ) return;

		_resetRequired = true;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::
setStreamFilter(Seiscomp::Math::Filtering::InPlaceFilter<double>* filter) {
	if ( _filter )
	    delete _filter;
	_filter = filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::resetStreams() {

	recordEntity("Z").reset("Z");
	recordEntity("1").reset("1");
	recordEntity("2").reset("2");

	if ( _sv & Frequency ) {
		recordEntity("Zf").reset("Zf");
		recordEntity("1f").reset("1f");
		recordEntity("2f").reset("2f");
	}

	updatePlots();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::reset() {
	resetStreams();
	_picker->reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::saveCurrentPicks() {

	if ( _picker->picks().size() == 0 ) return;

	QString key = QString("%1.%2").arg(_networkCode).arg(_stationCode);
	for (int i = 0; i < _picker->picks().size(); ++i) {
		if ( _picks.find(key) != _picks.end() )
		    _picks.remove(key);
		_picks.insert(key, _picker->picks());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool OriginRecordViewer::hasPick(const QString& pickID) {

	bool retCode = false;

	QList<OriginRecordPicker::Marker> list = _picks.value(
	    QString("%1.%2").arg(_networkCode).arg(_stationCode));
	for (int i = 0; i < list.size(); ++i)
		if ( list.at(i).code() == pickID )
		    retCode = true;

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OriginRecordPicker::Marker
OriginRecordViewer::pick(const QString& pickID) {

	OriginRecordPicker::Marker m;

	QList<OriginRecordPicker::Marker> list = _picks.value(
	    QString("%1.%2").arg(_networkCode).arg(_stationCode));
	for (int i = 0; i < list.size(); ++i)
		if ( list.at(i).code() == pickID )
		    m = list.at(i);

	return m;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginRecordViewer::addStation(const std::string& networkCode,
                                    const std::string& stationCode) {
	if ( _recordStream )
	    return _recordStream->addStream(networkCode, stationCode, "??", "???");

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::setStartTime(const Seiscomp::Core::Time& time) {
	if ( _recordStream ) _recordStream->setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::setEndTime(const Time& time) {
	if ( _recordStream ) _recordStream->setEndTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool
OriginRecordViewer::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {
	if ( !_recordStream ) return false;
	_tw = tw;
	return _recordStream->setTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginRecordViewer::addStream(const std::string& networkCode,
                                   const std::string& stationCode,
                                   const std::string& locationCode,
                                   const std::string& channelCode) {
	if ( _recordStream )
	    return _recordStream->addStream(networkCode, stationCode, locationCode, channelCode);

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginRecordViewer::openStream() {
	if ( _recordStream ) return false;
	_recordStream = IO::RecordStream::Open(_recordStreamUrl.c_str());
	return _recordStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::closeStream() {
	if ( _recordStream ) _recordStream->close();
	_recordStream = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::fetchWaveForms(const QStringList& streamIDs) {

	emit working();

	if ( streamIDs.size() > 0 ) {

		if ( !_recordStream ) openStream();

		Time st = _origin->time().value() - TimeSpan(_timeWindow / 2);
		Time ed = _origin->time().value() + TimeSpan(_timeWindow / 2);

		setTimeWindow(Seiscomp::Core::TimeWindow(st, ed));
		timeLine()->setTimeWindow(st, ed);

		for (int i = 0; i < streamIDs.size(); ++i) {
			QStringList l = streamIDs.at(i).split(".");
			addStream(l.at(0).toStdString(), l.at(1).toStdString(),
			    l.at(2).toStdString(), l.at(3).toStdString());
		}

		QThread* thread = new QThread;
		StreamDelegate* delegate = new StreamDelegate(_recordStream.get(),
		    _recordInputHint, _cache, StreamDelegate::CachedRecords);
		delegate->moveToThread(thread);

		connect(thread, SIGNAL(started()), delegate, SLOT(readRecords()));
		connect(delegate, SIGNAL(acquisitionTerminated()), thread, SLOT(quit()));
		connect(this, SIGNAL(stopDelegate()), thread, SLOT(quit()));
		connect(delegate, SIGNAL(acquisitionTerminated()), delegate, SLOT(deleteLater()));
		connect(delegate, SIGNAL(acquisitionTerminated()), this, SLOT(drawRecordTraces()));
		connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

		thread->start();
		SEISCOMP_DEBUG("Record delegate thread started");
	}
	else
		drawRecordTraces();

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::stopFetching() {
	emit stopDelegate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::setStation(const std::string& networkCode,
                                    const std::string& stationCode,
                                    const std::string& channelCode) {

	if ( !_origin ) {
		SEISCOMP_ERROR("No origin set, no streams fetched");
		return;
	}

	StationPtr station;
	_networkCode.clear();
	_stationCode.clear();
	_activeStreams.clear();

	_picker->reset();
	_picker->addOrigin(_origin.get());

	QString key = QString("%1.%2").arg(networkCode.c_str()).arg(stationCode.c_str());
	if ( _picks.find(key) != _picks.end() )
	    _picker->addPicks(_picks.find(key).value());

	PublicObjectPtr obj = _query->getStation(networkCode, stationCode, _origin->time().value());
	station = Station::Cast(obj.get());

	if ( !station ) {
		SEISCOMP_ERROR("Station %s/%s not found in inventory", networkCode.c_str(),
		    stationCode.c_str());
		return;
	}

	for (size_t i = 0; i < _origin->arrivalCount(); ++i) {

		ArrivalPtr ar = _origin->arrival(i);
		PickPtr pick = Pick::Find(ar->pickID());

		if ( !pick ) {
			PublicObjectPtr obj = _query->getObject(Pick::TypeInfo(), ar->pickID());
			pick = Pick::Cast(obj.get());
		}

		if ( !pick ) continue;

		if ( stationCode != pick->waveformID().stationCode()
		        || pick->waveformID().networkCode() != networkCode )
		    continue;

		_picker->addPick(pick.get(), ar.get());
	}


	if ( station->sensorLocationCount() == 0 )
	    _query->loadSensorLocations(station.get());

	QString defaultGain, defaultChannel;
	for (size_t l = 0; l < station->sensorLocationCount(); ++l) {
		SensorLocationPtr sloc = station->sensorLocation(l);

		try {
			if ( sloc->end() <= _origin->time().value() ) continue;
		} catch ( ... ) {}

		if ( sloc->start() > _origin->time().value() ) continue;

		if ( sloc->streamCount() == 0 )
		    _query->loadStreams(sloc.get());

		for (size_t k = 0; k < sloc->streamCount(); ++k) {
			StreamPtr stream = sloc->stream(k);

			try {
				if ( stream->end() <= _origin->time().value() ) continue;
			} catch ( ... ) {}

			if ( stream->start() > _origin->time().value() ) continue;

			ActiveStream as;
			as.networkCode = networkCode.c_str();
			as.stationCode = stationCode.c_str();
			as.locationCode = sloc->code().c_str();
			as.channelCode = stream->code().c_str();
			as.gainUnit = stream->gainUnit().c_str();
			if ( !channelCode.empty() && channelCode == stream->code().substr(0, 2) ) {
				as.inUse = true;
				defaultChannel = as.channelCode;
				defaultGain = as.gainUnit;
			}
			else
				as.inUse = false;
			_activeStreams.append(as);
		}
	}

	QStringList streamsToFetch;
	if ( _autoFetchRecords ) {
		for (int i = 0; i < _activeStreams.size(); ++i) {
			QString s = QString("%1.%2.%3.%4").arg(_activeStreams.at(i).networkCode)
			        .arg(_activeStreams.at(i).stationCode)
			        .arg(_activeStreams.at(i).locationCode)
			        .arg(_activeStreams.at(i).channelCode);
			RecordPtr rec = _cache->get(s.toStdString());
			if ( !rec )
			    streamsToFetch.append(s);
		}
	}

	if ( !defaultGain.isEmpty() ) {
		_activeGainUnit = defaultGain;
		_usedStream->setText(QString("%1?").arg(defaultChannel.left(2)));
	}
	else {
		_usedStream->setText(QString("%1?").arg(_activeStreams.at(0).channelCode.left(2)));
		_activeGainUnit = _activeStreams.at(0).gainUnit;
		_activeStreams[0].inUse = true;
	}
	_usedStream->setCheckable(true);
	_usedStream->setChecked(true);

	_networkCode = networkCode.c_str();
	_stationCode = stationCode.c_str();

	fetchWaveForms(streamsToFetch);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginRecordViewer::drawRecordTraces() {

	if ( !_origin ) return;

	//! Reset streams
	recordEntity("Z").reset("Z");
	recordEntity("1").reset("1");
	recordEntity("2").reset("2");

	//! Fill background rects with no data color, this way if an entity doesn't
	//! have any data, it will show up with the appropriate background and not
	//! the one available by default (light yellow) when resetting the recordEntity
	recordEntity("Z").rect()->setBackground(Misc::noStreamDataColor);
	recordEntity("1").rect()->setBackground(Misc::noStreamDataColor);
	recordEntity("2").rect()->setBackground(Misc::noStreamDataColor);

	recordEntity("Z").setSequenceFilter(_filter);
	recordEntity("1").setSequenceFilter(_filter);
	recordEntity("2").setSequenceFilter(_filter);

	if ( _sv & Frequency ) {
		recordEntity("Zf").reset("Zf");
		recordEntity("1f").reset("1f");
		recordEntity("2f").reset("2f");

		recordEntity("Zf").rect()->setBackground(Misc::noStreamDataColor);
		recordEntity("1f").rect()->setBackground(Misc::noStreamDataColor);
		recordEntity("2f").rect()->setBackground(Misc::noStreamDataColor);

		recordEntity("Zf").setSequenceFilter(_filter);
		recordEntity("1f").setSequenceFilter(_filter);
		recordEntity("2f").setSequenceFilter(_filter);
	}

	for (int i = 0; i < _activeStreams.size(); ++i) {

		if ( !_activeStreams.at(i).inUse ) continue;

		QString sid = QString("%1.%2.%3.%4")
		        .arg(_activeStreams.at(i).networkCode)
		        .arg(_activeStreams.at(i).stationCode)
		        .arg(_activeStreams.at(i).locationCode)
		        .arg(_activeStreams.at(i).channelCode);

		std::vector<RecordPtr> recV = _cache->getRecords(sid.toStdString());
		RingBuffer* buffer = new RingBuffer((int) recV.size());

		for (size_t v = 0; v < recV.size(); ++v) {
			RecordPtr rec = recV.at(v);
			if ( !rec ) continue;
			buffer->feed(Record::ConstCast(rec));
		}

		//! See SEED manual for more information about component naming system,
		//! it can be quite entertaining...

		if ( _activeStreams.at(i).channelCode.right(1) == "Z"
		        or _activeStreams.at(i).channelCode.right(1) == "A" ) {
			recordEntity("Z").setSequence(buffer);
			recordEntity("Z").setName(sid);
			_picker->component("Z").setLabel(sid);

			if ( _sv & Frequency ) {
				recordEntity("Zf").setSequence(buffer);
				_picker->component("Zf").setLabel(QString("%1 spect.").arg(sid));
			}
		}

		if ( _activeStreams.at(i).channelCode.right(1) == "N"
		        or _activeStreams.at(i).channelCode.right(1) == "B"
		        or _activeStreams.at(i).channelCode.right(1) == "1" ) {
			recordEntity("1").setSequence(buffer);
			recordEntity("1").setName(sid);
			_picker->component("1").setLabel(sid);

			if ( _sv & Frequency ) {
				recordEntity("1f").setSequence(buffer);
				_picker->component("1f").setLabel(QString("%1 spect.").arg(sid));
			}
		}

		if ( _activeStreams.at(i).channelCode.right(1) == "E"
		        or _activeStreams.at(i).channelCode.right(1) == "C"
		        or _activeStreams.at(i).channelCode.right(1) == "2" ) {
			recordEntity("2").setSequence(buffer);
			recordEntity("2").setName(sid);
			_picker->component("2").setLabel(sid);

			if ( _sv & Frequency ) {
				_picker->component("2f").setLabel(QString("%1 spect.").arg(sid));
				recordEntity("2f").setSequence(buffer);
			}
		}

		delete buffer;
		buffer = NULL;
	}
	//! Make sure component names are up to date
	_picker->updateComponents();

	recordEntity("Z").rescale(true);
	recordEntity("1").rescale(true);
	recordEntity("2").rescale(true);

	if ( _sv & Frequency ) {
		recordEntity("Zf").rescale(true);
		recordEntity("1f").rescale(true);
		recordEntity("2f").rescale(true);
	}

	qApp->processEvents();

	closeStream();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
