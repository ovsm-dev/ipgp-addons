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

#include <ipgp/gui/datamodel/eventtypevariance/eventtypevariancewidget.h>
#include <ipgp/gui/datamodel/eventtypevariance/ui_eventtypevariancewidget.h>
#include <ipgp/core/string/string.h>
#include <ipgp/core/math/math.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <QtGui>



using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::String;
using namespace IPGP::Core::Math;



namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTypeVarianceWidget::
EventTypeVarianceWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::EventTypeVarianceWidget) {

	_ui->setupUi(this);
	setObjectName("EventTypeVariance");

	_showLegend = true;
	_ui->checkBox_legend->setChecked(_showLegend);

	_plot = new SquarreZoomPlot(this);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
	_plot->axisRect()->setBackground(Qt::white);
#ifndef __APPLE__
	_plot->yAxis->setLabelFont(QFont(QFont().family(), 8));
#endif
	_plot->yAxis->setLabel(QString("Number of events"));

	QBoxLayout* l = new QVBoxLayout(_ui->framePlot);
	_ui->framePlot->setLayout(l);
	l->addWidget(_plot);
	l->setMargin(0);

	QBoxLayout* l2 = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l2);
	l2->addWidget(_toolBox);
	l2->setMargin(0);
	_toolBox->show();

	_ui->listWidgetAvailable->setSelectionMode(QAbstractItemView::MultiSelection);
	_ui->listWidgetSelected->setSelectionMode(QAbstractItemView::MultiSelection);

	connect(_ui->checkBox_legend, SIGNAL(clicked(bool)), this, SLOT(showLegend(bool)));
	connect(_ui->toolButtonAddType, SIGNAL(clicked()), this, SLOT(addEventType()));
	connect(_ui->toolButtonRemoveType, SIGNAL(clicked()), this, SLOT(removeEventType()));

	connect(_ui->comboBox_style, SIGNAL(currentIndexChanged(int)), this, SLOT(showHideNormalizeBox(int)));
	connect(_ui->comboBox_method, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSeismicTypes()));
	connect(this, SIGNAL(eventListModified()), this, SLOT(updateSeismicTypes()));

	_ui->checkBox_normalize->hide();

	_brushes << QColor(134, 134, 134);
	_brushes << QColor(99, 173, 99);
	_brushes << QColor(171, 219, 99);
	_brushes << QColor(255, 76, 76);
	_brushes << QColor(251, 166, 156);
	_brushes << QColor(173, 124, 89);
	_brushes << QColor(223, 198, 63);

	_pens << QColor(83, 83, 83);
	_pens << QColor(34, 139, 34);
	_pens << QColor(136, 204, 34);
	_pens << QColor(255, 0, 0);
	_pens << QColor(250, 128, 114);
	_pens << QColor(139, 69, 19);
	_pens << QColor(255, 215, 0);

	_plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
	_plot->xAxis->setAutoTicks(false);
	_plot->xAxis->setAutoTickLabels(false);
	_plot->xAxis->setDateTimeSpec(Qt::UTC);
	_plot->yAxis->setAutoTicks(true);
	_plot->yAxis->setAutoTickLabels(true);
#ifndef __APPLE__
	_plot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
	_plot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
#endif
	_plot->xAxis->grid()->setVisible(true);
	_plot->xAxis2->setVisible(true);
	_plot->xAxis2->setTicks(false);
	_plot->xAxis2->setTickLabels(false);
	_plot->yAxis->grid()->setSubGridVisible(false);
	_plot->yAxis2->setVisible(true);
	_plot->yAxis2->setTicks(false);
	_plot->yAxis2->setTickLabels(false);
	_plot->axisRect()->setRangeDrag(Qt::Horizontal);
	_plot->axisRect()->setRangeZoom(Qt::Horizontal);
	_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	_plot->xAxis->setSubTickCount(0);

	QFont legendFont = font();
	legendFont.setPointSize(10);
	_plot->legend->setFont(legendFont);
	_plot->legend->setSelectedFont(legendFont);
	_plot->legend->setSelectableParts(QCPLegend::spItems);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTypeVarianceWidget::~EventTypeVarianceWidget() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::updateSeismicTypes() {

	_types.clear();
	_ui->listWidgetAvailable->clear();
	_ui->listWidgetSelected->clear();

	for (OriginList::iterator j = _events->begin(); j != _events->end(); ++j) {

		EventPtr event = j->second;

		if ( _ui->comboBox_method->currentText().contains("SCODE") ) {

			if ( event->commentCount() == 0 )
			    _query->loadComments(event.get());

			for (size_t i = 0; i < event->commentCount(); ++i) {
				if ( !event->comment(i) ) continue;

				//! SCODE is stored after the first pipe of the comment, we
				//! are to recover its value and then add it to the list.
				if ( i == 0 ) {
					QStringList v = QString(event->comment(i)->text().c_str()).split("|");
					if ( v.size() > 1 && !_types.contains(v.at(1).left(2)) && !v.at(1).isEmpty() )
					    _types << v.at(1).left(2);
				}
			}
		}
		else {

			QString type;
			try {
				type = event->type().toString();
			} catch ( ... ) {}
			if ( !_types.contains(type) && !type.isEmpty() )
			    _types << type;
		}
	}

	_ui->listWidgetSelected->addItems(_types);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::showHideNormalizeBox(int idx) {

	Q_UNUSED(idx);

	if ( _ui->comboBox_style->currentText() == "Piled curves" )
		_ui->checkBox_normalize->show();
	else
		_ui->checkBox_normalize->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::showLegend(bool clicked) {
	_showLegend = clicked;
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::addEventType() {
	for (int i = _ui->listWidgetAvailable->count() - 1; i > -1; --i)
		if ( _ui->listWidgetAvailable->item(i)->isSelected() )
		    _ui->listWidgetSelected->addItem(_ui->listWidgetAvailable->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::removeEventType() {
	for (int i = _ui->listWidgetSelected->count() - 1; i > -1; --i)
		if ( _ui->listWidgetSelected->item(i)->isSelected() )
		    _ui->listWidgetAvailable->addItem(_ui->listWidgetSelected->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::print(const ToolBox::ExportConfig& ec) {

	QString outputFile = QFileDialog::getSaveFileName(this, tr("Save S-P Duration graphic"),
	    QDir::currentPath(), tr("%1 (*.%1)").arg(ec.format.toString()), 0,
	    QFileDialog::DontUseNativeDialog);

	if ( outputFile.isEmpty() )
	    return;

	if ( (outputFile.right(4) != QString(".%1").arg(ec.format.toString())) )
	    outputFile.append(QString(".%1").arg(ec.format.toString()).toLower());

	if ( ec.format == IPGP::Gui::PDF )
		_plot->savePdf(outputFile, 0, ec.printSize.width(), ec.printSize.height());
	else if ( ec.format == IPGP::Gui::PNG )
		_plot->savePng(outputFile, ec.printSize.width(), ec.printSize.height(), 1, -1);
	else if ( ec.format == IPGP::Gui::JPG )
		_plot->saveJpg(outputFile, ec.printSize.width(), ec.printSize.height(), 1, -1);
	else if ( ec.format == IPGP::Gui::BMP )
		_plot->saveBmp(outputFile, ec.printSize.width(), ec.printSize.height(), 1);
	else if ( ec.format == IPGP::Gui::PS )
	    _plot->saveRastered(outputFile, ec.printSize.width(), ec.printSize.height(), 1., "PS", 100);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("the event list is empty");
		return;
	}

	if ( _ui->listWidgetSelected->count() == 0 ) {
		QMessageBox::critical(this, tr("Error"),
		    QString("At least one event type should be available for plotting."));
		emit plottingError("no event type(s) selected, nothing plotted");
		return;
	}

	emit working();

	if ( _timer )
	    stopBlinking();

	if ( _ui->comboBox_style->currentText().contains("Piled bars") )
		plotPiledBarsGraph();
	else if ( _ui->comboBox_style->currentText().contains("Piled curves") )
		plotPiledCurvesGraph();
	else if ( _ui->comboBox_style->currentText().contains("Curves") )
	    plotCurvesGraph();

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool EventTypeVarianceWidget::checkType(const QString& evttype) {

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i)
		if ( _ui->listWidgetSelected->item(i)->text() == evttype )
		    return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::prepareData(TimedOccurrence& count) {

	_data.clear();

	for (OriginList::iterator j = _events->begin(); j != _events->end(); ++j) {

		OriginPtr org = j->first;
		EventPtr event = j->second;

		QString type;
		if ( _ui->comboBox_method->currentText().contains("SCODE") ) {

			if ( event->commentCount() == 0 )
			    _query->loadComments(event.get());

			for (size_t i = 0; i < event->commentCount(); ++i) {

				if ( !event->comment(i) ) continue;

				//! SCODE is stored after the first pipe of the comment, we
				//! are to recover its value and then add it to the list.
				if ( i == 0 ) {
					QStringList v = QString(event->comment(i)->text().c_str()).split("|");
					if ( v.size() > 1 && !v.at(1).isEmpty() )
					    type = v.at(1).left(2);
				}
			}
		}
		else {
			try {
				type = j->second->type().toString();
			} catch ( ... ) {}
		}

		if ( !checkType(type) )
		    continue;

		Time t;
		if ( _ui->comboBox_sensibility->currentText().contains("Months") )
			t.set(stringToInt(org->time().value().toString("%Y")),
			    stringToInt(org->time().value().toString("%m")), 1, 0, 0, 0, 0);
		else if ( _ui->comboBox_sensibility->currentText().contains("Days") )
		    t.set(stringToInt(org->time().value().toString("%Y")),
		        stringToInt(org->time().value().toString("%m")),
		        stringToInt(org->time().value().toString("%d")), 0, 0, 0, 0);

		if ( count[t] )
			count[t] += 1.;
		else
			count.insert(t, 1.);

		for (int i = 0; i < _types.size(); ++i) {

			if ( _types.at(i) != type ) continue;

			if ( !_data.contains(type) )
			    _data.insert(type, TimedOccurrence());

			if ( _data.contains(type) ) {

				for (OccurrenceMap::iterator it = _data.begin();
				        it != _data.end(); ++it) {

					if ( it.key() != type ) continue;

					if ( it.value()[t] )
						it.value()[t] += 1.;
					else
						it.value().insert(t, 1.);
				}
			}
		}
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::plotPiledBarsGraph() {

	_plot->clearPlottables();

	TimedOccurrence count;
	prepareData(count);

	qreal barSize = .0;
	if ( _ui->comboBox_sensibility->currentText() == "Months" )
		barSize = 3600 * 24 * 30;
	else if ( _ui->comboBox_sensibility->currentText() == "Days" )
	    barSize = 3600 * 24;

	QPen pen;
	pen.setStyle(Qt::SolidLine);

	//! Bars get ordered accordingly to the types in list and pointers will be
	//! destroyed by the plot methods.
	QCPBars** bars = new QCPBars*[_ui->listWidgetSelected->count()];
	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i) {

		int r = qrand() % 255;
		int g = qrand() % 255;

		bars[i] = new QCPBars(_plot->xAxis, _plot->yAxis);
		bars[i]->setName(_ui->listWidgetSelected->item(i)->text());

		(i < _brushes.size()) ? bars[i]->setBrush(_brushes.at(i)) : bars[i]->setBrush(QColor(r, g, 190));

		bars[i]->setPen(pen);
		bars[i]->setWidth(barSize);
		_plot->addPlottable(bars[i]);

		//! Pile bars up on top of the first one
		if ( i > 0 )
		    bars[i]->moveAbove(bars[i - 1]);
	}

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i) {
		const TimedOccurrence map = _data.value(_ui->listWidgetSelected->item(i)->text());
		TimedOccurrence::const_iterator it = map.constBegin();
		while ( it != map.constEnd() ) {
			bars[i]->addData(it.key().seconds(), it.value());
			++it;
		}
	}

	double maxY = .0;
	Numbers<double> keys;
	QVector<double> ticks(count.size());
	QVector<QString> labels(count.size());
	QString maxYDate;

	TimedOccurrence::const_iterator it = count.begin();
	while ( it != count.end() ) {

		keys.add((double) it.key());
		ticks << it.key().seconds();

		if ( _ui->comboBox_sensibility->currentText().contains("Months") ) {
			labels << QDateTime().fromMSecsSinceEpoch(it.key().seconds() * 1000)
			    .toString("yyyy\nMMMM");
			if ( it.value() > maxY )
			    maxY = it.value(), maxYDate = QDateTime().fromMSecsSinceEpoch(
			        it.key().seconds() * 1000).toString("yyyy-MMMM");
		}
		else if ( _ui->comboBox_sensibility->currentText().contains("Days") ) {
			labels << it.key().toString("%Y-%m-%d").c_str();

			if ( it.value() > maxY )
			    maxY = it.value(), maxYDate = QDateTime().fromMSecsSinceEpoch(
			        it.key().seconds() * 1000).toString("yyyy-MMMM-dd");
		}

		++it;
	}

	_plot->legend->setVisible(_showLegend);

	_plot->yAxis->setRange(.0, maxY + 2.);
	_plot->xAxis->setTickVector(ticks);
	_plot->xAxis->setTickVectorLabels(labels);

	_ui->label_mEvent->setText(maxYDate);

	if ( _ui->comboBox_sensibility->currentText().contains("Months") ) {
		try {
			_plot->xAxis->setRange(keys.min() - 2628000., keys.max() + 2628000.);
		} catch ( ... ) {
			_plot->xAxis->rescale();
		}
		_plot->xAxis->setTickLabelRotation(0);
		_plot->xAxis->setLabelPadding(0);
	}

	else if ( _ui->comboBox_sensibility->currentText().contains("Days") ) {
		try {
			_plot->xAxis->setRange(keys.min() - 86400., keys.max() + 86400.);
		} catch ( ... ) {
			_plot->xAxis->rescale();
		}
		_plot->xAxis->setTickLabelRotation(90);
		_plot->xAxis->setLabelPadding(15);
	}

	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::plotPiledCurvesGraph() {

	_plot->clearPlottables();

	TimedOccurrence count;
	prepareData(count);

	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setColor(QColor(134, 134, 134));

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i) {

		int r = qrand() % 255;
		int g = qrand() % 255;

		QCPGraph* graph = _plot->addGraph();
		graph->setSelectable(true);
		graph->setName(_ui->listWidgetSelected->item(i)->text());
		if ( i < _brushes.size() ) {
			graph->setPen(_pens.at(i));
			graph->setBrush(_brushes.at(i));
		}
		else {
			graph->setPen(pen);
			graph->setBrush(QColor(r, g, 190));
		}
		graph->setLineStyle(QCPGraph::lsLine);

		if ( i > 0 )
		    graph->setChannelFillGraph(_plot->graph(i - 1));
	}

	TimedOccurrence plotValue;
	QVector<double> ticks(count.size());
	QVector<QString> labels(count.size());
	Time firstMaxOcc;

	for (TimedOccurrence::const_iterator it = count.begin();
	        it != count.end(); ++it) {

		ticks << it.key().seconds();

		if ( _ui->comboBox_sensibility->currentText().contains("Months") )
			labels << QDateTime().fromMSecsSinceEpoch(it.key().seconds() * 1000)
			    .toString("yyyy\nMMMM");
		else if ( _ui->comboBox_sensibility->currentText().contains("Days") )
		    labels << it.key().toString("%Y-%m-%d").c_str();

		plotValue.insert(it.key(), .0);
	}

	//! TODO: normalize values by using the max it at a position for the day,
	//!       do not use the global max as the maximum peek value

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i) {

		const TimedOccurrence map = _data.value(_ui->listWidgetSelected->item(i)->text());
		TimedOccurrence::const_iterator it = map.constBegin();
		while ( it != map.constEnd() ) {

			if ( _ui->checkBox_normalize->isChecked() ) {
				double v = percentageOfSomething<double>(count[it.key()], it.value());
				_plot->graph(i)->addData(it.key().seconds(), v);
				plotValue[it.key()] += v;
			}
			else {
				plotValue[it.key()] += it.value();
				_plot->graph(i)->addData(it.key().seconds(), plotValue[it.key()]);
			}

			++it;
		}

		//! Fill data holes with curve last it value
		TimedOccurrence::const_iterator itf = count.begin();
		while ( itf != count.end() ) {
			if ( !map[itf.key()] )
			    _plot->graph(i)->addData(itf.key().seconds(), plotValue[itf.key()]);
			++itf;
		}
	}

	_plot->legend->setVisible(_showLegend);

	_plot->xAxis->setDateTimeFormat("MMMM\nyyyy");

	if ( _ui->checkBox_normalize->isChecked() ) {
		_plot->yAxis->setAutoTicks(false);
		_plot->yAxis->setAutoTickLabels(false);
		_plot->yAxis->setTickVector(QVector<double>() << .0 << 25. << 50. << 75. << 100.);
		_plot->yAxis->setTickVectorLabels(QVector<QString>() << "" << "25%" << "50%" << "75%" << "100%");
		_plot->yAxis->setRange(.0, 100.);
	}
	else {
		_plot->yAxis->setAutoTicks(true);
		_plot->yAxis->setAutoTickLabels(true);
		_plot->yAxis->rescale(true);
	}
	_plot->xAxis->rescale(true);
	_plot->xAxis->setTickVector(ticks);
	_plot->xAxis->setTickVectorLabels(labels);

	if ( _ui->comboBox_sensibility->currentText().contains("Months") ) {
		_plot->xAxis->setTickLabelRotation(0);
		_plot->xAxis->setLabelPadding(0);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    firstMaxOcc.seconds() * 1000).toString("yyyy-MMMM"));
	}

	else if ( _ui->comboBox_sensibility->currentText().contains("Days") ) {
		_plot->xAxis->setTickLabelRotation(90);
		_plot->xAxis->setLabelPadding(15);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    firstMaxOcc.seconds() * 1000).toString("yyyy-MMMM-dd"));
	}

	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventTypeVarianceWidget::plotCurvesGraph() {

	_plot->clearPlottables();

	TimedOccurrence count;
	prepareData(count);

	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(2.);

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i) {

		int r = qrand() % 255;
		int g = qrand() % 255;

		QCPGraph* graph = _plot->addGraph();
		graph->setSelectable(true);
		graph->setName(_ui->listWidgetSelected->item(i)->text());
		(i < _pens.size()) ? pen.setColor(_pens.at(i)) : pen.setColor(QColor(r, g, 190));
		graph->setPen(pen);
		graph->setLineStyle(QCPGraph::lsLine);
	}

	QVector<double> ticks(count.size());
	QVector<QString> labels(count.size());
	Time rt;

	TimedOccurrence::const_iterator it = count.begin();
	while ( it != count.end() ) {

		ticks << it.key().seconds();

		if ( _ui->comboBox_sensibility->currentText().contains("Months") )
			labels << QDateTime().fromMSecsSinceEpoch(it.key().seconds() * 1000)
			    .toString("yyyy\nMMMM");
		else if ( _ui->comboBox_sensibility->currentText().contains("Days") )
		    labels << it.key().toString("%Y-%m-%d").c_str();

		for (int i = 0; i < _plot->graphCount(); ++i)
			_plot->graph(i)->addData(it.key().seconds(), .0);

		++it;
	}


	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i) {
		const TimedOccurrence map = _data.value(_ui->listWidgetSelected->item(i)->text());
		TimedOccurrence::const_iterator it = map.constBegin();
		while ( it != map.constEnd() ) {
			_plot->graph(i)->data()->remove(it.key());
			_plot->graph(i)->addData(it.key().seconds(), it.value());
			++it;
		}
	}

	_plot->legend->setVisible(_showLegend);

	_plot->xAxis->setDateTimeFormat("MMMM\nyyyy");

	_plot->xAxis->setAutoTicks(false);
	_plot->xAxis->setAutoTickLabels(false);
	_plot->yAxis->setAutoTicks(true);
	_plot->yAxis->setAutoTickLabels(true);

	_plot->yAxis->rescale(true);
	_plot->xAxis->rescale(true);
	_plot->xAxis->setTickVector(ticks);
	_plot->xAxis->setTickVectorLabels(labels);

	if ( _ui->comboBox_sensibility->currentText().contains("Months") ) {
		_plot->xAxis->setTickLabelRotation(.0);
		_plot->xAxis->setLabelPadding(0);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    rt.seconds() * 1000).toString("yyyy-MMMM"));
	}

	else if ( _ui->comboBox_sensibility->currentText().contains("Days") ) {
		_plot->xAxis->setTickLabelRotation(90.);
		_plot->xAxis->setLabelPadding(15);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    rt.seconds() * 1000).toString("yyyy-MMMM-dd"));
	}

	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
