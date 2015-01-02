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

#include <ipgp/gui/datamodel/plottingwidget.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <QtGui>


using namespace Seiscomp::DataModel;

using namespace IPGP::Core;


namespace IPGP {
namespace Gui {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PlottingWidget::
PlottingWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		QWidget(parent, f), _query(query), _events(NULL), _cache(NULL),
		_listIsACopy(false), _cacheIsACopy(false), _blinkFrequency(300) {

	_toolBox = new ToolBox;
	_toolBox->hide();
	connect(_toolBox, SIGNAL(printRequested(const ToolBox::ExportConfig&)), this, SLOT(print(const ToolBox::ExportConfig&)));
	connect(_toolBox->ui()->pushButtonReplot, SIGNAL(clicked()), this, SLOT(replot()));

	_color1 = _toolBox->ui()->pushButtonReplot->palette().color(QPalette::Button);
	_color2 = QColor::fromRgb(229, 130, 9, 255);

	_wb = WB_Default;
	_timer = NULL;
	_buttonState = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PlottingWidget::~PlottingWidget() {

	if ( _toolBox )
		delete _toolBox;
	_toolBox = NULL;

	if ( _listIsACopy && _events )
		delete _events;
	_events = NULL;

	if ( _cacheIsACopy && _cache )
		delete _cache;
	_cache = NULL;

	if ( _timer )
		delete _timer;
	_timer = NULL;

	emit nullifyQObject(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::setEvents(OriginList* list, const bool& copy) {

	if ( copy ) {

		if ( _events )
			delete _events, _events = NULL;

		_events = new OriginList;
		*_events = *list;
		_listIsACopy = true;

		emit eventListModified();

		return;
	}

	_events = list;
	_listIsACopy = false;

	emit eventListModified();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::setCache(ObjectCache* cache, const bool& copy) {

	if ( copy ) {

		if ( _cache )
			delete _cache, _cache = NULL;

		_cache = new ObjectCache;
		*_cache = *cache;
		_cacheIsACopy = true;

		return;
	}

	_cache = cache;
	_cacheIsACopy = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::closeEvent(QCloseEvent* event) {

	if ( _wb & WB_Default )
		event->accept();
	else if ( _wb & WB_WidgetCloses )
		this->close();
	else if ( _wb & WB_WidgetHidesOnClose )
		event->ignore(), this->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickPtr PlottingWidget::getPick(const std::string& publicID,
                                const bool& loadArrivals) {

	for (OriginList::const_iterator i = _events->begin();
	        i != _events->end(); ++i) {

		OriginPtr org = i->first;

		if ( org->arrivalCount() == 0 && loadArrivals )
			_query->loadArrivals(org.get());

		for (size_t j = 0; j < org->arrivalCount(); ++j) {

			ArrivalPtr ar = org->arrival(j);
			if ( !ar )
				continue;

			PickPtr pick = Pick::Find(ar->pickID());
			if ( !pick ) {
				pick = Pick::Cast(_query->getObject(Pick::TypeInfo(), ar->pickID()));
				if ( !pick )
					continue;
			}

			if ( pick->publicID() == publicID )
				return pick;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickPtr PlottingWidget::getPick(const std::string& networkCode,
                                const std::string& stationCode,
                                const std::string& phaseCode,
                                const bool& loadArrivals) {

	for (OriginList::const_iterator i = _events->begin();
	        i != _events->end(); ++i) {

		OriginPtr org = i->first;

		if ( org->arrivalCount() == 0 && loadArrivals )
			_query->loadArrivals(org.get());

		for (size_t j = 0; j < org->arrivalCount(); ++j) {

			ArrivalPtr ar = org->arrival(j);
			if ( !ar )
				continue;

			PickPtr pick = Pick::Find(ar->pickID());
			if ( !pick ) {
				pick = Pick::Cast(_query->getObject(Pick::TypeInfo(), ar->pickID()));
				if ( !pick )
					continue;
			}

			if ( pick->waveformID().networkCode() == networkCode
			        && pick->waveformID().stationCode() == stationCode
			        && pick->phaseHint().code().find(phaseCode) != std::string::npos )
				return pick;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<PickPtr> PlottingWidget::getPicks(Seiscomp::DataModel::OriginPtr object,
                                        const std::string& phaseCode,
                                        const bool& loadArrivals) {

	PickList list;

	if ( !object )
		return list;

	if ( object->arrivalCount() == 0 && loadArrivals )
		_query->loadArrivals(object.get());

	for (OriginList::const_iterator i = _events->begin();
	        i != _events->end(); ++i) {

		OriginPtr org = i->first;

		if ( org != object )
			continue;

		for (size_t j = 0; j < org->arrivalCount(); ++j) {

			ArrivalPtr ar = org->arrival(j);
			if ( !ar )
				continue;

			PickPtr pick = Pick::Find(ar->pickID());
			if ( !pick ) {
				pick = Pick::Cast(_query->getObject(Pick::TypeInfo(), ar->pickID()));
				if ( !pick )
					continue;
			}

			if ( pick->phaseHint().code().find(phaseCode) != std::string::npos )
				list << pick;
		}
	}

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::setMagnitudeTypes(const Core::MagnitudeTypes& magnitudes) {
	_magnitudes = magnitudes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::setBlinkFrequency(const int& freq) {
	_blinkFrequency = freq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::startBlinking() {

	if ( _timer )
		delete _timer, _timer = NULL;

	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(updateBlinker()));
	_timer->start(_blinkFrequency);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::updateBlinker() {

	Q_ASSERT(_toolBox!=NULL);

	_buttonState ? _buttonState = false : _buttonState = true;

	QPalette palette(_toolBox->ui()->pushButtonReplot->palette());
	palette.setColor(QPalette::Button, (_buttonState) ? _color1 : _color2);

	_toolBox->ui()->pushButtonReplot->setPalette(palette);
	_toolBox->ui()->pushButtonReplot->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::stopBlinking() {

	if ( !_timer )
		return;

	_timer->stop();

	delete _timer, _timer = NULL;

	_buttonState = true;
	QPalette palette(_toolBox->ui()->pushButtonReplot->palette());
	palette.setColor(QPalette::Button, _color1);

	_toolBox->ui()->pushButtonReplot->setPalette(palette);
	_toolBox->ui()->pushButtonReplot->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PlottingWidget::setPrintOptionVisible(bool) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}// namespace Gui
} // namespace IPGP
