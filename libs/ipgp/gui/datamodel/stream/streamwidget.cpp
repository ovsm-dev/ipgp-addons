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

#define SEISCOMP_COMPONENT IPGP_STREAMWIDGET

#include <ipgp/gui/datamodel/stream/streamwidget.h>
#include <ipgp/gui/datamodel/stream/ui_streamwidget.h>
#include <ipgp/gui/datamodel/streamdelegate.h>
#include <ipgp/core/string/string.h>
#include <ipgp/gui/misc/misc.h>


#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/inventory.h>
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

#include <QtGui>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::String;



namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpectrogramPlot::SpectrogramPlot(QWidget* parent, Qt::WFlags f) :
		QCustomPlot(parent) {

	setWindowFlags(f);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMinimumWidth(50);
	resize(50, 100);

	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	setBackground(Qt::transparent);

	plotLayout()->setAutoMargins(QCP::msNone);
	plotLayout()->setMargins(QMargins(0, 0, 0, 0));

	axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	axisRect()->setMargins(QMargins(0, 0, 0, 0));
	axisRect()->setBackground(Qt::white);
	axisRect()->setRangeDrag(Qt::Horizontal);
	axisRect()->setRangeZoom(Qt::Horizontal);
	axisRect()->setupFullAxesBox();

	yAxis->setLabel("Frequency [Hz]");
	yAxis->setRange(.0, 10.);
	xAxis->setTickLabelType(QCPAxis::ltDateTimeAuto);
	xAxis->setDateTimeSpec(Qt::UTC);
	xAxis->setTickLabelPadding(0);
	xAxis->setDateTimeFormat("hh:mm:ss");
	xAxis->setSubTickCount(5);
	xAxis->setTickLength(0, xAxis->tickLengthIn() + 3);
	xAxis->setSubTickLength(0, xAxis->subTickLengthIn() + 2);

	_gradient.clearColorStops();
	_gradient.setColorInterpolation(QCPColorGradient::ciRGB);
	_gradient.setColorStopAt(1, QColor(132, 0, 0));
	_gradient.setColorStopAt(.8, QColor(255, 255, 0));
	_gradient.setColorStopAt(.5, QColor(0, 255, 0));
	_gradient.setColorStopAt(0, QColor(0, 0, 155));

	_rm = new RecordManager(this);
	connect(_rm, SIGNAL(newEntity(Entity)), this, SLOT(processEntity(Entity)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpectrogramPlot::~SpectrogramPlot() {
	clearPlottables();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramPlot::setData(const QString& streamID,
                              Seiscomp::RecordSequence* seq) {

	if ( !seq ) return;

	if ( streamID != _streamID ) {
		// Clear the graph of old color map(s) from previous stream
		for (CCMap::iterator it = _cmap.begin(); it != _cmap.end(); ++it)
			removePlottable(it.value());
		_cmap.clear();
		_streamID = streamID;
	}

	for (size_t i = 0; i < seq->size(); ++i)
		if ( !_cmap.contains(fabs((double) seq->at(i)->startTime())) )
		    _rm->feed(seq->at(i).get());

	_rm->prepareFFT();
	_rm->computeFFT();

	/*
	 RecordManager::EntityVector v = _rm->entities();

	 for (int i = 0; i < v.size(); ++i) {

	 if ( _cmap.contains(v.at(i).time) ) continue;

	 QCPColorMap* map = new QCPColorMap(xAxis, yAxis);
	 addPlottable(map);
	 map->setGradient(QCPColorGradient::gpSpectrum);
	 map->data()->setSize(1., 10.);
	 map->data()->setRange(QCPRange(v.at(i).time, v.at(i).time + 1.), QCPRange(.0, 10.));

	 for (size_t j = 0; j < v.at(i).freqs.size(); ++j)
	 map->data()->setData(v.at(i).time, v.at(i).freqs.at(j), v.at(i).amps.at(j));

	 map->rescaleDataRange();
	 _cmap.insert(v.at(i).time, map);
	 }
	 */

	_rm->clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramPlot::setTimeWindow(const double& start, const double& end) {

	xAxis->setRange(start, end);

	QList<double> l;
	for (CCMap::iterator it = _cmap.begin(); it != _cmap.end(); ++it)
		if ( it.key() < start || it.key() > end )
		    l << it.key();

	for (int i = 0; i < l.size(); ++i) {
		removePlottable(_cmap.value(l.at(i)));
		_cmap.remove(l.at(i));
	}

	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramPlot::processEntity(Entity e) {

	if ( _cmap.contains(e.time) ) return;

	QCPColorMap* map = new QCPColorMap(xAxis, yAxis);
	addPlottable(map);

//	map->setGradient(QCPColorGradient::gpHot);
	map->setGradient(_gradient);
	map->data()->setSize(1., 100.);
	map->data()->setRange(QCPRange(e.time, e.time + _rm->getComputingResolution()), QCPRange(.0, 10.));

//	Core::Math::Numbers<double> amps;
//	double maxAmp = e.amps.max();
//	for (size_t j = 0; j < e.amps.size(); ++j)
//		amps.add(Core::Math::remap<double>(e.amps.at(j), -1 * fabs(maxAmp), fabs(maxAmp), .0, 1.));

	for (size_t j = 0; j < e.freqs.size(); ++j)
		map->data()->setData(e.time, e.freqs.at(j), e.amps.at(j));

	map->rescaleDataRange();
	_cmap.insert(e.time, map);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamWidget::StreamWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		QFrame(parent, f), _query(query), _inventory(NULL),
		_ui(new Ui::StreamWidget), _delegateThread(NULL), _filter(NULL) {

	_ui->setupUi(this);

	_viewer = new RecordViewer(this, 0, RecordViewer::onTheFly, RecordViewer::Scroll);
	_viewer->setStreamsSelectionPolicy(RecordViewer::StreamsSelectable);

	_spectro = new SpectrogramPlot(this);

	QLayout* l = new QHBoxLayout(_ui->framePlot);
	l->setMargin(0);
	_ui->framePlot->setLayout(l);

	QSplitter* splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Horizontal);
	splitter->setContentsMargins(0, 0, 0, 0);
	l->addWidget(splitter);

	splitter->addWidget(_viewer);
	splitter->addWidget(_spectro);
	splitter->setSizes(QList<int>() << 100 << 0);

	_filterBox = _ui->comboBoxFilter;
	_filterBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	_filterBox->setToolTip("Select stream filter");

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(_filterBox->sizePolicy().hasHeightForWidth());
	_filterBox->setSizePolicy(sizePolicy);

	connect(_ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(stationSelected(QListWidgetItem*)));
	connect(&_timer, SIGNAL(timeout()), this, SLOT(updateStreams()));

	_ui->checkBox_autoRescale->setChecked(true);

	_recordStreamUrl = "slink://localhost:18000";
	_recordInputHint = Record::DATA_ONLY;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamWidget::~StreamWidget() {

	closeStream();

	resetSequences();

	if ( _inventory ) delete _inventory;
	_inventory = NULL;

	if ( _filter ) delete _filter;
	_filter = NULL;

	delete _ui;
	_ui = NULL;

	SEISCOMP_DEBUG("StreamWidget is terminated");

	emit nullifyQObject(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamWidget::addStation(const std::string& networkCode,
                              const std::string& stationCode) {

	if ( _recordStream )
	    return _recordStream->addStream(networkCode, stationCode, "??", "???");

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::setStartTime(const Seiscomp::Core::Time& time) {
	if ( _recordStream )
	    _recordStream->setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::setEndTime(const Seiscomp::Core::Time& time) {
	if ( _recordStream )
	    _recordStream->setEndTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamWidget::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {

	if ( !_recordStream )
	    return false;

	return _recordStream->setTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamWidget::addStream(const std::string& networkCode,
                             const std::string& stationCode,
                             const std::string& locationCode,
                             const std::string& channelCode) {

	if ( _recordStream )
	    return _recordStream->addStream(networkCode, stationCode, locationCode, channelCode);

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamWidget::openStream() {

	if ( _recordStream ) return false;

	_recordStream = IO::RecordStream::Open(_recordStreamUrl.c_str());

	return _recordStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::closeStream() {

	if ( _delegate ) {
		_delegate->stopReading();
		_timer.stop();
	}

	if ( _recordStream )
	    _recordStream->close();

	_recordStream = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::startReading() {

	if ( !_recordStream ) return;

	if ( _delegate && _delegate->isRunning() ) {
		// Chances are it's still reading some bits
		_delegate->stopReading();
	}

// Let the smart pointer delete the delegate instance, which will trigger
// the deletion of the previous QThread instance, therefore, we can
// allocate a new block for this pointer...
	_delegateThread = new QThread;

	_delegate = QSharedPointer<StreamDelegate>(new StreamDelegate(
	    _recordStream.get(), _recordInputHint, NULL, StreamDelegate::RealTimeRecords));
	_delegate->moveToThread(_delegateThread);

	connect(_delegateThread, SIGNAL(started()), _delegate.data(), SLOT(readRecords()));
	connect(_delegate.data(), SIGNAL(acquisitionTerminated()), _delegateThread, SLOT(quit()));
	connect(this, SIGNAL(stopDelegate()), _delegateThread, SLOT(quit()));
	connect(_delegateThread, SIGNAL(finished()), _delegateThread, SLOT(deleteLater()));
	connect(_delegate.data(), SIGNAL(newRecord(Seiscomp::Record*)),
	    this, SLOT(storeNewRecord(Seiscomp::Record*)));

	_delegateThread->start();
	_timer.start(_settings.refreshRate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::stopReading() {
	emit stopDelegate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::resetSequences() {

	for (SequenceMap::iterator it = _sequences.begin();
	        it != _sequences.end(); ++it) {
		if ( !it.value() ) continue;
		it.value()->clear();
		delete it.value();
	}
	_sequences.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::initFilters(const QMap<QString, QString>& filters) {

	_filterBox->clear();
	_filterBox->insertItem(0, "No filter");

	_filters.clear();

	QString activeFilter;
	int i = 1;
	for (StringMap::const_iterator it = filters.begin();
	        it != filters.end(); ++it, ++i) {

		QString name;
		(it.key().left(1) == "@") ? name = activeFilter = it.key().mid(1, it.key().size()) : name = it.key();

		_filters.insert(name, it.value());
		_filterBox->insertItem(i, name);
	}

	connect(_filterBox, SIGNAL(currentIndexChanged(QString)),
	    this, SLOT(filterChanged(const QString&)));

	if ( !activeFilter.isEmpty() )
	    filterChanged(activeFilter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::filterChanged(const QString& filter) {

	if ( filter == "No filter" ) {
		if ( _filter ) delete _filter;
		_filter = NULL;
	}
	else {

		QString filterName;
		for (StringMap::const_iterator it = _filters.begin();
		        it != _filters.end(); ++it) {
			if ( it.key() == filter )
			    filterName = it.value();
		}

		if ( filterName.isEmpty() ) return;

		std::string error;
		Seiscomp::Math::Filtering::InPlaceFilter<double>* f = NULL;
		f = Seiscomp::Math::Filtering::InPlaceFilter<double>::Create(filterName.toStdString(), &error);

		if ( f )
			_filter = f;
		else
			QMessageBox::warning(this, tr("Seiscomp filter exception"), error.c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::stationSelected(QListWidgetItem* current) {

	QList<QString> list = _sensors.values(current->text());

	_selectedStation = current->text();

	_viewer->removeAllRecordEntities();
	_spectro->clearPlottables();

	resetSequences();

	if ( _delegate ) closeStream();

	openStream();

	Time st1 = Time::GMT();
	double stime = (double) st1 - _settings.buffer;
	setStartTime(Time(stime));

	QStringList item = current->text().split(".");

	for (int i = 0; i < list.size(); ++i) {

		QStringList l = list.at(i).split(".");

		if ( l.at(1).right(1) == "Z" && !_settings.visibleSensors.isSet(Settings::Z) )
		    continue;

		if ( (l.at(1).right(1) == "N" || l.at(1).right(1) == "1")
		        && !_settings.visibleSensors.isSet(Settings::NorthSouth) )
		    continue;

		if ( (l.at(1).right(1) == "E" || l.at(1).right(1) == "2")
		        && !_settings.visibleSensors.isSet(Settings::EastWest) )
		    continue;

		addStream(item.at(0).toStdString(), item.at(1).toStdString(),
		    l.at(0).toStdString(), l.at(1).toStdString());

		_viewer->addRecordEntity(QString("%1.%2").arg(current->text()).arg(list.at(i)));
		_sequences.insert(QString("%1.%2").arg(current->text()).arg(list.at(i)), new RingBuffer(TimeSpan(_settings.buffer)));
	}

	_viewer->setupRecordEntities();

	startReading();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::updateStreams() {

	for (SequenceMap::iterator it = _sequences.begin();
	        it != _sequences.end(); ++it) {
		try {
			_viewer->recordEntity(it.key()).setSequenceFilter(_filter);
			_viewer->recordEntity(it.key()).setSequence(it.value());
			_viewer->recordEntity(it.key()).rescaleItems();

			if ( _viewer->recordEntity(it.key()).isSelected() )
			    _spectro->setData(it.key(), it.value());
		} catch ( ... ) {}
	}

	if ( _ui->checkBox_autoRescale->isChecked() ) {
		QDateTime t = QDateTime::currentDateTimeUtc();
		double now = t.toMSecsSinceEpoch() / 1000;
		_viewer->timeLine()->setTimeWindow(Time(now - _settings.buffer), Time(now));
		_spectro->setTimeWindow(now - _settings.buffer, now);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::storeNewRecord(Seiscomp::Record* r) {

	if ( !r ) return;

	RecordSequence* rs = _sequences[r->streamID().c_str()];

	if ( rs )
	    rs->feed(Record::ConstCast(r));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::setupInventory() {

	_sensors.clear();
	if ( !_inventory ) {
		SEISCOMP_INFO("Trying to acquire inventory from query instance");

		if ( !_query ) {
			QMessageBox::critical(this, tr("Query"), tr("There is no database query interface available!"));
			SEISCOMP_ERROR("No query interface available for inventory fetching");
			return;
		}

		emit working();

		try {
			emit loadingPercentage(10, objectName(), "Loading inventory...");
		} catch ( ... ) {}

		_inventory = new Inventory();

		int objects = 0;
		QMutex m;
		m.lock();
		objects = _query->load(_inventory);
		m.unlock();
		SEISCOMP_DEBUG("Loaded inventory: %d objects fetched", objects);
		emit idling();
	}

	if ( !_inventory ) {
		SEISCOMP_ERROR("No inventory acquired from query instance");
		return;
	}

	Time now = Time::GMT();
	QStringList items;

	for (size_t i = 0; i < _inventory->networkCount(); ++i) {
		NetworkPtr network = _inventory->network(i);

		for (size_t j = 0; j < network->stationCount(); ++j) {
			StationPtr station = network->station(j);

			if ( station->start() > now ) continue;

			try {
				if ( station->end() <= now ) continue;
			} catch ( ... ) {}

			QString item = QString("%1.%2").arg(network->code().c_str()).arg(station->code().c_str());
			if ( !items.contains(item) )
			    items.append(item);

			for (size_t l = 0; l < station->sensorLocationCount(); ++l) {
				SensorLocationPtr sloc = station->sensorLocation(l);

				try {
					if ( sloc->end() <= now ) continue;
				} catch ( ... ) {}

				if ( sloc->start() > now ) continue;

				for (size_t k = 0; k < sloc->streamCount(); ++k) {
					StreamPtr stream = sloc->stream(k);

					try {
						if ( stream->end() <= now ) continue;
					} catch ( ... ) {}

					if ( stream->start() > now ) continue;

					QString streamID = QString("%1.%2").arg(sloc->code().c_str()).arg(stream->code().c_str());
					_sensors.insertMulti(item, streamID);
				}
			}
		}

		try {
			emit loadingPercentage(Core::Math::percentageOfSomething<int>(_inventory->networkCount(), i),
			    objectName(), "Loading inventory...");
		} catch ( ... ) {}
	}

	_ui->listWidget->clear();
	_ui->listWidget->addItems(items);

	emit loadingPercentage(-1, objectName(), "");
	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
