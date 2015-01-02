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

#include <ipgp/gui/datamodel/magnitudevariance/magnitudevariancewidget.h>
#include <ipgp/gui/datamodel/magnitudevariance/ui_magnitudevariancewidget.h>
#include <ipgp/core/string/string.h>
#include <ipgp/core/math/math.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/types.h>
#include <QtGui>




using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::String;

namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeVarianceWidget::
MagnitudeVarianceWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::MagnitudeVarianceWidget) {

	_ui->setupUi(this);

	setObjectName("MagnitudeVariance");

	_showLegend = true;
	_ui->checkBox_legend->setChecked(_showLegend);

	_plot = new SquarreZoomPlot(this);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
	_plot->axisRect()->setBackground(Qt::white);
	_plot->axisRect()->setRangeDrag(Qt::Horizontal);
	_plot->axisRect()->setRangeZoom(Qt::Horizontal);
	_plot->xAxis->setDateTimeSpec(Qt::UTC);
#ifndef __APPLE__
	_plot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
	_plot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
#endif
	_plot->yAxis->setLabel(QString("Number of events"));
	_plot->yAxis->setAutoTicks(true);
	_plot->yAxis->setAutoTickLabels(true);
	_plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
	_plot->yAxis->grid()->setSubGridVisible(false);
	_plot->xAxis2->setVisible(true);
	_plot->yAxis2->setVisible(true);
	_plot->xAxis2->setTicks(false);
	_plot->yAxis2->setTicks(false);
	_plot->xAxis2->setTickLabels(false);
	_plot->yAxis2->setTickLabels(false);
	_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);


	QFont legendFont = font();
	legendFont.setPointSize(10);

	_plot->legend->setFont(legendFont);
	_plot->legend->setSelectedFont(legendFont);

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
	connect(_ui->toolButtonAddMag, SIGNAL(clicked()), this, SLOT(addMagnitude()));
	connect(_ui->toolButtonRemoveMag, SIGNAL(clicked()), this, SLOT(removeMagnitude()));
	connect(_ui->comboBox_style, SIGNAL(currentIndexChanged(int)), this, SLOT(showHideNormalizeBox(int)));

	_ui->checkBox_normalize->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeVarianceWidget::~MagnitudeVarianceWidget() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::showHideNormalizeBox(int idx) {

	Q_UNUSED(idx);

	if ( _ui->comboBox_style->currentText() == "Piled curves" )
		_ui->checkBox_normalize->show();
	else
		_ui->checkBox_normalize->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::showLegend(bool clicked) {
	_showLegend = clicked;
	replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::
setMagnitudeTypes(const Core::MagnitudeTypes& magnitudes) {

	_magnitudes = magnitudes;

	_ui->listWidgetAvailable->clear();
	_ui->listWidgetSelected->clear();

	for (size_t i = 0; i < magnitudes.size(); ++i)
		_ui->listWidgetSelected->addItem(magnitudes.at(i).c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::addMagnitude() {
	for (int i = _ui->listWidgetAvailable->count() - 1; i > -1; --i)
		if ( _ui->listWidgetAvailable->item(i)->isSelected() )
		    _ui->listWidgetSelected->addItem(_ui->listWidgetAvailable->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::removeMagnitude() {
	for (int i = _ui->listWidgetSelected->count() - 1; i > -1; --i)
		if ( _ui->listWidgetSelected->item(i)->isSelected() )
		    _ui->listWidgetAvailable->addItem(_ui->listWidgetSelected->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::print(const ToolBox::ExportConfig& ec) {

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
void MagnitudeVarianceWidget::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("the event list is empty");
		return;
	}

	if ( _ui->listWidgetSelected->count() == 0 ) {
		emit plottingError("no magnitude(s) selected, nothing plotted");
		return;
	}

	emit working();

	if ( _timer )
	    stopBlinking();

	if ( _ui->comboBox_style->currentText() == "Piled bars" )
		plotPiledBarsGraph();
	else if ( _ui->comboBox_style->currentText() == "Piled curves" )
		plotPiledCurvesGraph();
	else if ( _ui->comboBox_style->currentText() == "Curves" )
	    plotCurvesGraph();

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool MagnitudeVarianceWidget::
checkMagnitude(const std::string& magtype) {

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i)
		if ( _ui->listWidgetSelected->item(i)->text() == magtype.c_str() )
		    return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::plotPiledBarsGraph() {

	_plot->clearPlottables();

	QMap<Time, float> range0to1;
	QMap<Time, float> range1to2;
	QMap<Time, float> range2to3;
	QMap<Time, float> range3to4;
	QMap<Time, float> range4to5;
	QMap<Time, float> rangeAbove5;
	QMap<Time, float> count;

	for (OriginList::const_iterator j = _events->begin(); j != _events->end();
	        ++j) {

		OriginPtr org = j->first;
		MagnitudePtr mag = Magnitude::Find(j->second->preferredMagnitudeID());

		if ( !mag ) continue;

		if ( !checkMagnitude(mag->type()) ) continue;

		float magnitude = -1.;
		try {
			magnitude = mag->magnitude().value();
		}
		catch ( ... ) {}

		if ( magnitude == -1. || magnitude == .0 ) continue;

		Time t;
		if ( _ui->comboBox_sensibility->currentText() == "Months" )
			t.set(stringToInt(org->time().value().toString("%Y")),
			    stringToInt(org->time().value().toString("%m")), 1, 0, 0, 0, 0);
		else if ( _ui->comboBox_sensibility->currentText() == "Days" )
		    t.set(stringToInt(org->time().value().toString("%Y")),
		        stringToInt(org->time().value().toString("%m")),
		        stringToInt(org->time().value().toString("%d")), 0, 0, 0, 0);

		if ( count[t] )
			count[t] += 1.;
		else
			count.insert(t, 1.);

		if ( magnitude > .0 && magnitude < 1. ) {
			if ( range0to1.value(t) )
				range0to1[t] += 1.;
			else
				range0to1.insert(t, 1.);
		}

		if ( 1. < magnitude && magnitude < 2. ) {
			if ( range1to2.value(t) )
				range1to2[t] += 1.;
			else
				range1to2.insert(t, 1.);
		}

		if ( 2. < magnitude && magnitude < 3. ) {
			if ( range2to3.value(t) )
				range2to3[t] += 1.;
			else
				range2to3.insert(t, 1.);
		}

		if ( 3. < magnitude && magnitude < 4. ) {
			if ( range3to4.value(t) )
				range3to4[t] += 1.;
			else
				range3to4.insert(t, 1.);
		}

		if ( 4. < magnitude && magnitude < 5. ) {
			if ( range4to5.value(t) )
				range4to5[t] += 1.;
			else
				range4to5.insert(t, 1.);
		}

		if ( magnitude > 5. ) {
			if ( rangeAbove5.value(t) )
				rangeAbove5[t] += 1.;
			else
				rangeAbove5.insert(t, 1.);
		}
	}

	QPen pen;
	pen.setStyle(Qt::SolidLine);

	QCPBars* above0 = new QCPBars(_plot->xAxis, _plot->yAxis);
	QCPBars* above1 = new QCPBars(_plot->xAxis, _plot->yAxis);
	QCPBars* above2 = new QCPBars(_plot->xAxis, _plot->yAxis);
	QCPBars* above3 = new QCPBars(_plot->xAxis, _plot->yAxis);
	QCPBars* above4 = new QCPBars(_plot->xAxis, _plot->yAxis);
	QCPBars* above5 = new QCPBars(_plot->xAxis, _plot->yAxis);

	_plot->addPlottable(above0);
	_plot->addPlottable(above1);
	_plot->addPlottable(above2);
	_plot->addPlottable(above3);
	_plot->addPlottable(above4);
	_plot->addPlottable(above5);

	above0->setName("0 < Mag < 1");
	above0->setPen(pen);
	above0->setBrush(QColor(134, 134, 134));

	above1->setName("1 < Mag < 2");
	above1->setPen(pen);
	above1->setBrush(QColor(99, 173, 99));

	above2->setName("2 < Mag < 3");
	above2->setPen(pen);
	above2->setBrush(QColor(171, 219, 99));

	above3->setName("3 < Mag < 4");
	above3->setPen(pen);
	above3->setBrush(QColor(255, 76, 76));

	above4->setName("4 < Mag < 5");
	above4->setPen(pen);
	above4->setBrush(QColor(251, 166, 156));

	above5->setName("5 < Mag");
	above5->setPen(pen);
	above5->setBrush(QColor(173, 124, 89));

	above1->moveAbove(above0);
	above2->moveAbove(above1);
	above3->moveAbove(above2);
	above4->moveAbove(above3);
	above5->moveAbove(above4);

	double minX = .0, maxX = .0, maxY = .0;
	bool firstIt = true;
	QVector<double> ticks(count.size());
	QVector<QString> labels(count.size());
	QString maxYDate;

	QMap<Time, float>::iterator it = count.begin();
	while ( it != count.end() ) {

		if ( firstIt ) {
			minX = maxX = it.key().seconds();
			firstIt = false;
		}

		if ( minX > it.key().seconds() )
		    minX = it.key().seconds();

		if ( maxX < it.key().seconds() )
		    maxX = it.key().seconds();

		if ( it == count.begin() )
		    minX = it.key().seconds(), maxX = it.key().seconds();

		if ( it.key().seconds() < minX )
		    minX = it.key().seconds();

		if ( it.key().seconds() > maxX )
		    maxX = it.key().seconds();

		ticks << it.key().seconds();

		if ( _ui->comboBox_sensibility->currentText() == "Months" ) {
			labels << QDateTime().fromMSecsSinceEpoch(it.key().seconds() * 1000)
			    .toString("yyyy\nMMMM");
			if ( it.value() > maxY )
			    maxY = it.value(), maxYDate = QDateTime().fromMSecsSinceEpoch(
			        it.key().seconds() * 1000).toString("yyyy-MMMM");
		}
		else if ( _ui->comboBox_sensibility->currentText() == "Days" ) {
			labels << it.key().toString("%Y-%m-%d").c_str();

			if ( it.value() > maxY )
			    maxY = it.value(), maxYDate = QDateTime().fromMSecsSinceEpoch(
			        it.key().seconds() * 1000).toString("yyyy-MMMM-dd");
		}

		++it;
	}

	QMap<Time, float>::iterator it0 = range0to1.begin();
	while ( it0 != range0to1.end() ) {
		above0->addData(it0.key().seconds(), it0.value());
		++it0;
	}

	QMap<Time, float>::iterator it1 = range1to2.begin();
	while ( it1 != range1to2.end() ) {
		above1->addData(it1.key().seconds(), it1.value());
		++it1;
	}

	QMap<Time, float>::iterator it2 = range2to3.begin();
	while ( it2 != range2to3.end() ) {
		above2->addData(it2.key().seconds(), it2.value());
		++it2;
	}

	QMap<Time, float>::iterator it3 = range3to4.begin();
	while ( it3 != range3to4.end() ) {
		above3->addData(it3.key().seconds(), it3.value());
		++it3;
	}

	QMap<Time, float>::iterator it4 = range4to5.begin();
	while ( it4 != range4to5.end() ) {
		above4->addData(it4.key().seconds(), it4.value());
		++it4;
	}

	QMap<Time, float>::iterator it5 = rangeAbove5.begin();
	while ( it5 != rangeAbove5.end() ) {
		above5->addData(it5.key().seconds(), it5.value());
		++it5;
	}

	_plot->legend->setVisible(_showLegend);
	_plot->xAxis2->setTicks(false);
	_plot->xAxis2->setTickLabels(false);
	_plot->yAxis->setRange(.0, maxY + 2.);
	_plot->xAxis->setTickVector(ticks);
	_plot->xAxis->setTickVectorLabels(labels);
	_plot->xAxis->setSubTickCount(0);

	_ui->label_mEvent->setText(maxYDate);

	if ( _ui->comboBox_sensibility->currentText() == "Months" ) {
		above0->setWidth(3600 * 24 * 30);
		above1->setWidth(3600 * 24 * 30);
		above2->setWidth(3600 * 24 * 30);
		above3->setWidth(3600 * 24 * 30);
		above4->setWidth(3600 * 24 * 30);
		above5->setWidth(3600 * 24 * 30);
		_plot->xAxis->setRange(minX - 2628000., maxX + 2628000.);
		_plot->xAxis->setTickLabelRotation(0);
		_plot->xAxis->setLabelPadding(0);
	}

	else if ( _ui->comboBox_sensibility->currentText() == "Days" ) {
		above0->setWidth(3600 * 24);
		above1->setWidth(3600 * 24);
		above2->setWidth(3600 * 24);
		above3->setWidth(3600 * 24);
		above4->setWidth(3600 * 24);
		above5->setWidth(3600 * 24);
		_plot->xAxis->setRange(minX - 86400., maxX + 86400.);
		_plot->xAxis->setTickLabelRotation(90);
		_plot->xAxis->setLabelPadding(15);
	}
	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::plotPiledCurvesGraph() {

	_plot->clearPlottables();

	QMap<Time, float> range0to1;
	QMap<Time, float> range1to2;
	QMap<Time, float> range2to3;
	QMap<Time, float> range3to4;
	QMap<Time, float> range4to5;
	QMap<Time, float> rangeAbove5;
	QMap<Time, float> count;

	for (OriginList::const_iterator j = _events->begin(); j != _events->end();
	        ++j) {

		OriginPtr org = j->first;
		MagnitudePtr mag = Magnitude::Find(j->second->preferredMagnitudeID());

		if ( !mag ) continue;

		if ( !checkMagnitude(mag->type()) ) continue;

		float magnitude = -1.;
		try {
			magnitude = mag->magnitude().value();
		} catch ( ... ) {}

		if ( magnitude == -1. || magnitude == .0 )
		    continue;

		Time t;
		if ( _ui->comboBox_sensibility->currentText() == "Months" )
			t.set(stringToInt(org->time().value().toString("%Y")),
			    stringToInt(org->time().value().toString("%m")), 1, 0, 0, 0, 0);
		else if ( _ui->comboBox_sensibility->currentText() == "Days" )
		    t.set(stringToInt(org->time().value().toString("%Y")),
		        stringToInt(org->time().value().toString("%m")),
		        stringToInt(org->time().value().toString("%d")), 0, 0, 0, 0);

		if ( count[t] )
			count[t] += 1.;
		else
			count.insert(t, 1.);

		if ( magnitude > .0 && magnitude < 1. ) {
			if ( range0to1.value(t) )
				range0to1[t] += 1.;
			else
				range0to1.insert(t, 1.);
		}

		if ( 1. < magnitude && magnitude < 2. ) {
			if ( range1to2.value(t) )
				range1to2[t] += 1.;
			else
				range1to2.insert(t, 1.);
		}

		if ( 2. < magnitude && magnitude < 3. ) {
			if ( range2to3.value(t) )
				range2to3[t] += 1.;
			else
				range2to3.insert(t, 1.);
		}

		if ( 3. < magnitude && magnitude < 4. ) {
			if ( range3to4.value(t) )
				range3to4[t] += 1.;
			else
				range3to4.insert(t, 1.);
		}

		if ( 4. < magnitude && magnitude < 5. ) {
			if ( range4to5.value(t) )
				range4to5[t] += 1.;
			else
				range4to5.insert(t, 1.);
		}

		if ( magnitude > 5. ) {
			if ( rangeAbove5.value(t) )
				rangeAbove5[t] += 1.;
			else
				rangeAbove5.insert(t, 1.);
		}
	}

	QPen pen;
	pen.setStyle(Qt::SolidLine);

	pen.setColor(QColor(83, 83, 83));
	QCPGraph* aboveZero = _plot->addGraph();
	aboveZero->setSelectable(true);
	aboveZero->setName(tr("0 < Mag < 1"));
	aboveZero->setPen(pen);
	aboveZero->setBrush(QColor(134, 134, 134));
	aboveZero->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(34, 139, 34));
	QCPGraph* aboveOne = _plot->addGraph();
	aboveOne->setSelectable(false);
	aboveOne->setName(tr("1 < Mag < 2"));
	aboveOne->setPen(pen);
	aboveOne->setBrush(QColor(99, 173, 99));
	aboveOne->setChannelFillGraph(aboveZero);
	aboveOne->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(136, 204, 34));
	QCPGraph* aboveTwo = _plot->addGraph();
	aboveTwo->setSelectable(false);
	aboveTwo->setName(tr("2 < Mag < 3"));
	aboveTwo->setPen(pen);
	aboveTwo->setBrush(QColor(171, 219, 99));
	aboveTwo->setChannelFillGraph(aboveOne);
	aboveTwo->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(255, 0, 0));
	QCPGraph* aboveThree = _plot->addGraph();
	aboveThree->setSelectable(false);
	aboveThree->setName(tr("3 < Mag < 4"));
	aboveThree->setPen(pen);
	aboveThree->setBrush(QColor(255, 76, 76));
	aboveThree->setChannelFillGraph(aboveTwo);
	aboveThree->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(250, 128, 114));
	QCPGraph* aboveFour = _plot->addGraph();
	aboveFour->setSelectable(false);
	aboveFour->setName(tr("4 < Mag < 5"));
	aboveFour->setPen(pen);
	aboveFour->setBrush(QColor(251, 166, 156));
	aboveFour->setChannelFillGraph(aboveThree);
	aboveFour->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(139, 69, 19));
	QCPGraph* aboveFive = _plot->addGraph();
	aboveFive->setSelectable(false);
	aboveFive->setName(tr("Mag > 5"));
	aboveFive->setPen(pen);
	aboveFive->setBrush(QColor(173, 124, 89));
	aboveFive->setChannelFillGraph(aboveFour);
	aboveFive->setLineStyle(QCPGraph::lsLine);

	float maxValue = .0;
	QMap<Time, float> plotValue;
	QVector<double> ticks(count.size());
	QVector<QString> labels(count.size());
	Time rt;

	QMap<Time, float>::iterator it = count.begin();
	while ( it != count.end() ) {

		if ( maxValue < it.value() )
		    maxValue = it.value(), rt = it.key();

		ticks << it.key().seconds();

		if ( _ui->comboBox_sensibility->currentText() == "Months" )
			labels << QDateTime().fromMSecsSinceEpoch(it.key().seconds() * 1000)
			    .toString("yyyy\nMMMM");
		else if ( _ui->comboBox_sensibility->currentText() == "Days" )
		    labels << it.key().toString("%Y-%m-%d").c_str();

		plotValue.insert(it.key(), .0);

		++it;
	}

	QMap<Time, float>::iterator it0 = range0to1.begin();
	while ( it0 != range0to1.end() ) {

		if ( _ui->checkBox_normalize->isChecked() ) {
			float v = (100. * it0.value()) / count[it0.key()];
			aboveZero->addData(it0.key().seconds(), v);
			plotValue[it0.key()] = v;
		}
		else {
			plotValue[it0.key()] += it0.value();
			aboveZero->addData(it0.key().seconds(), plotValue[it0.key()]);
		}

		++it0;
	}

	// Fill data holes with curve last it value
	QMap<Time, float>::iterator itf1 = count.begin();
	while ( itf1 != count.end() ) {
		if ( !range0to1[itf1.key()] )
		    aboveZero->addData(itf1.key().seconds(), plotValue[itf1.key()]);
		++itf1;
	}


	QMap<Time, float>::iterator it1 = range1to2.begin();
	while ( it1 != range1to2.end() ) {

		if ( _ui->checkBox_normalize->isChecked() ) {
			float v = (100. * it1.value()) / count[it1.key()];
			v += plotValue[it1.key()];
			plotValue[it1.key()] = v;
			aboveOne->addData(it1.key().seconds(), v);
		}
		else {
			plotValue[it1.key()] += it1.value();
			aboveOne->addData(it1.key().seconds(), plotValue[it1.key()]);
		}
		++it1;
	}

	// Fill data holes with curve last it value
	QMap<Time, float>::iterator itf2 = count.begin();
	while ( itf2 != count.end() ) {
		if ( !range1to2[itf2.key()] )
		    aboveOne->addData(itf2.key().seconds(), plotValue[itf2.key()]);
		++itf2;
	}


	QMap<Time, float>::iterator it2 = range2to3.begin();
	while ( it2 != range2to3.end() ) {

		if ( _ui->checkBox_normalize->isChecked() ) {
			float v = (100. * it2.value()) / count[it2.key()];
			v += plotValue[it2.key()];
			plotValue[it2.key()] = v;
			aboveTwo->addData(it2.key().seconds(), v);
		}
		else {
			plotValue[it2.key()] += it2.value();
			aboveTwo->addData(it2.key().seconds(), plotValue[it2.key()]);
		}
		++it2;
	}

	// Fill data holes with curve last it value
	QMap<Time, float>::iterator itf3 = count.begin();
	while ( itf3 != count.end() ) {
		if ( !range2to3[itf3.key()] )
		    aboveTwo->addData(itf3.key().seconds(), plotValue[itf3.key()]);
		++itf3;
	}

	QMap<Time, float>::iterator it3 = range3to4.begin();
	while ( it3 != range3to4.end() ) {

		if ( _ui->checkBox_normalize->isChecked() ) {
			float v = (100. * it3.value()) / count[it3.key()];
			v += plotValue[it3.key()];
			plotValue[it3.key()] = v;
			aboveThree->addData(it3.key().seconds(), v);
		}
		else {
			plotValue[it3.key()] += it3.value();
			aboveThree->addData(it3.key().seconds(), plotValue[it3.key()]);
		}
		++it3;
	}

	// Fill data holes with curve last it value
	QMap<Time, float>::iterator itf4 = count.begin();
	while ( itf4 != count.end() ) {
		if ( !range3to4[itf4.key()] )
		    aboveThree->addData(itf4.key().seconds(), plotValue[itf4.key()]);
		++itf4;
	}

	QMap<Time, float>::iterator it4 = range4to5.begin();
	while ( it4 != range4to5.end() ) {

		if ( _ui->checkBox_normalize->isChecked() ) {
			float v = (100. * it4.value()) / count[it4.key()];
			v += plotValue[it4.key()];
			plotValue[it4.key()] = v;
			aboveFour->addData(it4.key().seconds(), v);
		}
		else {
			plotValue[it4.key()] += it4.value();
			aboveFour->addData(it4.key().seconds(), plotValue[it4.key()]);
		}

		++it4;
	}

	// Fill data holes with curve last it value
	QMap<Time, float>::iterator itf5 = count.begin();
	while ( itf5 != count.end() ) {
		if ( !range4to5[itf5.key()] )
		    aboveFour->addData(itf5.key().seconds(), plotValue[itf5.key()]);
		++itf5;
	}

	QMap<Time, float>::iterator it5 = rangeAbove5.begin();
	while ( it5 != rangeAbove5.end() ) {

		if ( _ui->checkBox_normalize->isChecked() ) {
			float v = (100. * it5.value()) / count[it5.key()];
			v += plotValue[it5.key()];
			plotValue[it5.key()] = v;
			aboveFive->addData(it5.key().seconds(), v);
		}
		else {
			plotValue[it5.key()] += it5.value();
			aboveFive->addData(it5.key().seconds(), plotValue[it5.key()]);
		}
		++it5;
	}

	// Fill data holes with curve last it value
	QMap<Time, float>::iterator itf6 = count.begin();
	while ( itf6 != count.end() ) {
		if ( !rangeAbove5[itf6.key()] )
		    aboveFive->addData(itf6.key().seconds(), plotValue[itf6.key()]);
		++itf6;
	}

	_plot->legend->setVisible(_showLegend);
	_plot->xAxis->setAutoTicks(false);
	_plot->xAxis->setAutoTickLabels(false);
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
	_plot->xAxis->setSubTickCount(0);


	if ( _ui->comboBox_sensibility->currentText() == "Months" ) {
		_plot->xAxis->setTickLabelRotation(0);
		_plot->xAxis->setLabelPadding(0);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    rt.seconds() * 1000).toString("yyyy-MMMM"));
	}

	else if ( _ui->comboBox_sensibility->currentText() == "Days" ) {
		_plot->xAxis->setTickLabelRotation(90);
		_plot->xAxis->setLabelPadding(15);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    rt.seconds() * 1000).toString("yyyy-MMMM-dd"));
	}

	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeVarianceWidget::plotCurvesGraph() {

	_plot->clearPlottables();

	QMap<Time, float> range0to1;
	QMap<Time, float> range1to2;
	QMap<Time, float> range2to3;
	QMap<Time, float> range3to4;
	QMap<Time, float> range4to5;
	QMap<Time, float> rangeAbove5;
	QMap<Time, float> count;

	for (OriginList::const_iterator j = _events->begin(); j != _events->end();
	        ++j) {

		OriginPtr org = j->first;
		MagnitudePtr mag = Magnitude::Find(j->second->preferredMagnitudeID());

		if ( !mag ) continue;

		if ( !checkMagnitude(mag->type()) ) continue;

		float magnitude = -1.;
		try {
			magnitude = mag->magnitude().value();
		} catch ( ... ) {}

		if ( magnitude == -1. || magnitude == .0 )
		    continue;

		Time t;
		if ( _ui->comboBox_sensibility->currentText() == "Months" )
			t.set(stringToInt(org->time().value().toString("%Y")),
			    stringToInt(org->time().value().toString("%m")), 1, 0, 0, 0, 0);
		else if ( _ui->comboBox_sensibility->currentText() == "Days" )
		    t.set(stringToInt(org->time().value().toString("%Y")),
		        stringToInt(org->time().value().toString("%m")),
		        stringToInt(org->time().value().toString("%d")), 0, 0, 0, 0);

		if ( count[t] )
			count[t] += 1.;
		else
			count.insert(t, 1.);

		if ( magnitude > .0 && magnitude < 1. ) {
			if ( range0to1.value(t) )
				range0to1[t] += 1.;
			else
				range0to1.insert(t, 1.);
		}

		if ( 1. < magnitude && magnitude < 2. ) {
			if ( range1to2.value(t) )
				range1to2[t] += 1.;
			else
				range1to2.insert(t, 1.);
		}

		if ( 2. < magnitude && magnitude < 3. ) {
			if ( range2to3.value(t) )
				range2to3[t] += 1.;
			else
				range2to3.insert(t, 1.);
		}

		if ( 3. < magnitude && magnitude < 4. ) {
			if ( range3to4.value(t) )
				range3to4[t] += 1.;
			else
				range3to4.insert(t, 1.);
		}

		if ( 4. < magnitude && magnitude < 5. ) {
			if ( range4to5.value(t) )
				range4to5[t] += 1.;
			else
				range4to5.insert(t, 1.);
		}

		if ( magnitude > 5. ) {
			if ( rangeAbove5.value(t) )
				rangeAbove5[t] += 1.;
			else
				rangeAbove5.insert(t, 1.);
		}
	}

	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(2.);

	pen.setColor(QColor(134, 134, 134));
	QCPGraph* aboveZero = _plot->addGraph();
	aboveZero->setSelectable(true);
	aboveZero->setName(tr("0 < Mag < 1"));
	aboveZero->setPen(pen);
	aboveZero->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(99, 173, 99));
	QCPGraph* aboveOne = _plot->addGraph();
	aboveOne->setSelectable(false);
	aboveOne->setName(tr("1 < Mag < 2"));
	aboveOne->setPen(pen);
	aboveOne->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(171, 219, 99));
	QCPGraph* aboveTwo = _plot->addGraph();
	aboveTwo->setSelectable(false);
	aboveTwo->setName(tr("2 < Mag < 3"));
	aboveTwo->setPen(pen);
	aboveTwo->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(255, 76, 76));
	QCPGraph* aboveThree = _plot->addGraph();
	aboveThree->setSelectable(false);
	aboveThree->setName(tr("3 < Mag < 4"));
	aboveThree->setPen(pen);
	aboveThree->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(251, 166, 156));
	QCPGraph* aboveFour = _plot->addGraph();
	aboveFour->setSelectable(false);
	aboveFour->setName(tr("4 < Mag < 5"));
	aboveFour->setPen(pen);
	aboveFour->setLineStyle(QCPGraph::lsLine);

	pen.setColor(QColor(173, 124, 89));
	QCPGraph* aboveFive = _plot->addGraph();
	aboveFive->setSelectable(false);
	aboveFive->setName(tr("Mag > 5"));
	aboveFive->setPen(pen);
	aboveFive->setLineStyle(QCPGraph::lsLine);


	Time rt;

	QMap<Time, float>::iterator it0 = range0to1.begin();
	while ( it0 != range0to1.end() ) {
		aboveZero->addData(it0.key().seconds(), it0.value());
		++it0;
	}

	QMap<Time, float>::iterator it1 = range1to2.begin();
	while ( it1 != range1to2.end() ) {
		aboveOne->addData(it1.key().seconds(), it1.value());
		++it1;
	}

	QMap<Time, float>::iterator it2 = range2to3.begin();
	while ( it2 != range2to3.end() ) {
		aboveTwo->addData(it2.key().seconds(), it2.value());
		++it2;
	}

	QMap<Time, float>::iterator it3 = range3to4.begin();
	while ( it3 != range3to4.end() ) {
		aboveThree->addData(it3.key().seconds(), it3.value());
		++it3;
	}

	QMap<Time, float>::iterator it4 = range4to5.begin();
	while ( it4 != range4to5.end() ) {
		aboveFour->addData(it4.key().seconds(), it4.value());
		++it4;
	}

	QMap<Time, float>::iterator it5 = rangeAbove5.begin();
	while ( it5 != rangeAbove5.end() ) {
		aboveFive->addData(it5.key().seconds(), it5.value());
		++it5;
	}

	QVector<double> ticks(count.size());
	QVector<QString> labels(count.size());
	float maxValue = .0;

	QMap<Time, float>::iterator it6 = count.begin();
	while ( it6 != count.end() ) {

		ticks << it6.key().seconds();

		if ( maxValue < it6.value() )
		    maxValue = it6.value(), rt = it6.key();

		if ( _ui->comboBox_sensibility->currentText() == "Months" )
			labels << QDateTime().fromMSecsSinceEpoch(it6.key().seconds() * 1000)
			    .toString("yyyy\nMMMM");
		else if ( _ui->comboBox_sensibility->currentText() == "Days" )
		    labels << it6.key().toString("%Y-%m-%d").c_str();

		if ( !range0to1[it6.key()] )
		    aboveZero->addData(it6.key().seconds(), .0);

		if ( !range1to2[it6.key()] )
		    aboveOne->addData(it6.key().seconds(), .0);

		if ( !range2to3[it6.key()] )
		    aboveTwo->addData(it6.key().seconds(), .0);

		if ( !range3to4[it6.key()] )
		    aboveThree->addData(it6.key().seconds(), .0);

		if ( !range4to5[it6.key()] )
		    aboveFour->addData(it6.key().seconds(), .0);

		if ( !rangeAbove5[it6.key()] )
		    aboveFive->addData(it6.key().seconds(), .0);

		++it6;
	}

	_plot->legend->setVisible(_showLegend);
	_plot->legend->setSelectableParts(QCPLegend::spItems);
	_plot->xAxis->setAutoTicks(false);
	_plot->xAxis->setAutoTickLabels(false);
	_plot->xAxis->setDateTimeFormat("MMMM\nyyyy");
	_plot->xAxis->rescale(true);
	_plot->yAxis->rescale(true);
	_plot->xAxis->setTickVector(ticks);
	_plot->xAxis->setTickVectorLabels(labels);
	_plot->xAxis->setSubTickCount(0);


	if ( _ui->comboBox_sensibility->currentText() == "Months" ) {
		_plot->xAxis->setTickLabelRotation(0);
		_plot->xAxis->setLabelPadding(0);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    rt.seconds() * 1000).toString("yyyy-MMMM"));
	}

	else if ( _ui->comboBox_sensibility->currentText() == "Days" ) {
		_plot->xAxis->setTickLabelRotation(90);
		_plot->xAxis->setLabelPadding(15);
		_ui->label_mEvent->setText(QDateTime().fromMSecsSinceEpoch(
		    rt.seconds() * 1000).toString("yyyy-MMMM-dd"));
	}

	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
