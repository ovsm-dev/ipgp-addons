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
 * Parts of this code are extracted from Seisan-Explorer code source    *
 * published under GPL v.3 and are not the property of OVSM/IPGP        *
 * http://seis.geus.net/software/seisan/                                *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/


#include <ipgp/gui/datamodel/gutenbergrichter/gutenbergrichterwidget.h>
#include <ipgp/gui/datamodel/gutenbergrichter/ui_gutenbergrichterwidget.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <QtGui>



using namespace Seiscomp;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;
using namespace IPGP::Core::Math;




namespace {

inline bool numberIsNan(const double& x) {
	return x != x;
}



}

namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GutenbergRichterWidget::
GutenbergRichterWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::GutenbergRichterWidget) {

	_ui->setupUi(this);

	setObjectName("GutenbergRichter");

	_ui->checkBoxShowInc->setChecked(true);
	_ui->checkBoxShowCum->setChecked(true);
	_ui->checkBoxIncLinearRegression->setChecked(true);
	_ui->checkBoxCumLinearRegression->setChecked(true);

	_ui->doubleSpinBoxStartMag->setValue(.0);
	_ui->doubleSpinBoxStartMag->setSingleStep(.1);
	_ui->doubleSpinBoxStartMag->setRange(-5., 11.);

	_ui->doubleSpinBoxInterval->setValue(.2);
	_ui->doubleSpinBoxInterval->setSingleStep(.1);
	_ui->doubleSpinBoxInterval->setRange(.01, 4.9);

	_ui->doubleSpinBoxBStart->setValue(2.);
	_ui->doubleSpinBoxBStart->setSingleStep(.1);
	_ui->doubleSpinBoxBStart->setRange(-5., 11.);

	_ui->doubleSpinBoxBEnd->setValue(9.9);
	_ui->doubleSpinBoxBEnd->setSingleStep(.1);
	_ui->doubleSpinBoxBEnd->setRange(-5., 11.);

	_ui->listWidgetAvailable->setSelectionMode(QAbstractItemView::MultiSelection);
	_ui->listWidgetSelected->setSelectionMode(QAbstractItemView::MultiSelection);

	_plot = new QCustomPlot(this);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
	_plot->axisRect()->setBackground(Qt::white);
	_plot->axisRect()->setRangeDrag(Qt::Horizontal);
	_plot->axisRect()->setRangeZoom(Qt::Horizontal);
	_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	_plot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_plot->axisRect()->setMargins(QMargins(2, 2, 2, 2));
	_plot->xAxis->grid()->setVisible(true);
	_plot->xAxis2->setVisible(true);
	_plot->xAxis2->setTicks(false);
	_plot->xAxis2->setTickLabels(false);
	_plot->yAxis->grid()->setSubGridVisible(false);
	_plot->yAxis2->setVisible(true);
	_plot->yAxis2->setTicks(false);
	_plot->yAxis2->setTickLabels(false);
	_plot->xAxis->setSubTickCount(0);
	_plot->xAxis->setLabel("Earthquake magnitude");
	_plot->yAxis->setNumberPrecision(0);
	_plot->yAxis->setSubTickCount(10);
	_plot->yAxis->setPadding(5);
	_plot->yAxis->setLabel("Number of earthquakes");
	_plot->yAxis->grid()->setSubGridVisible(true);
	_plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
	_plot->yAxis->setScaleLogBase(10);

	QFont font = this->font();
#ifndef __APPLE__
	font.setPointSize(8);
#endif
	_plot->legend->setVisible(true);
	_plot->legend->setFont(font);
	_plot->legend->setSelectedFont(font);
	_plot->legend->setSelectableParts(QCPLegend::spItems);

	_plot->setWindowTitle("Gutenberg-Richter relation");

	QBoxLayout* l1 = new QVBoxLayout(_ui->framePlot);
	_ui->framePlot->setLayout(l1);
	l1->addWidget(_plot);
	l1->setMargin(0);

	QBoxLayout* l2 = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l2);
	l2->addWidget(_toolBox);
	l2->setMargin(0);
	_toolBox->show();

	connect(_ui->toolButtonAddMag, SIGNAL(clicked()), this, SLOT(addMagnitude()));
	connect(_ui->toolButtonRemoveMag, SIGNAL(clicked()), this, SLOT(removeMagnitude()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GutenbergRichterWidget::~GutenbergRichterWidget() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GutenbergRichterWidget::addMagnitude() {
	for (int i = _ui->listWidgetAvailable->count() - 1; i > -1; --i)
		if ( _ui->listWidgetAvailable->item(i)->isSelected() )
		    _ui->listWidgetSelected->addItem(_ui->listWidgetAvailable->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GutenbergRichterWidget::removeMagnitude() {
	for (int i = _ui->listWidgetSelected->count() - 1; i > -1; --i)
		if ( _ui->listWidgetSelected->item(i)->isSelected() )
		    _ui->listWidgetAvailable->addItem(_ui->listWidgetSelected->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GutenbergRichterWidget::print(const ToolBox::ExportConfig& ec) {

	QString outputFile = QFileDialog::getSaveFileName(this, tr("Save S-P Duration graphic"),
	    QDir::currentPath(), tr("%1 (*.%1)").arg(ec.format.toString()), 0,
	    QFileDialog::DontUseNativeDialog);

	if ( outputFile.isEmpty() ) return;

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
void GutenbergRichterWidget::
setMagnitudeTypes(const Core::MagnitudeTypes& magnitudes) {

	_magnitudes = magnitudes;

	_ui->listWidgetAvailable->clear();
	_ui->listWidgetSelected->clear();

	for (size_t i = 0; i < magnitudes.size(); ++i)
		_ui->listWidgetSelected->addItem(magnitudes.at(i).c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool GutenbergRichterWidget::
checkMagnitude(const std::string& magtype) {

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i)
		if ( _ui->listWidgetSelected->item(i)->text() == magtype.c_str() )
		    return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GutenbergRichterWidget::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("the event list is empty");
		return;
	}

	if ( _timer )
	    stopBlinking();

	if ( _ui->listWidgetSelected->count() == 0 ) {
		QMessageBox::critical(this, tr("Magnitude"),
		    QString("At least one magnitude should be available for plotting."));
		emit plottingError("no magnitude(s) selected, nothing plotted");
		return;
	}

	emit working();

	double bStart = _ui->doubleSpinBoxBStart->value();
	if ( bStart < _ui->doubleSpinBoxStartMag->value() )
	    _ui->doubleSpinBoxBStart->setValue(_ui->doubleSpinBoxStartMag->value());
	bStart = _ui->doubleSpinBoxBStart->value();

	int bStartNum = 0;

	double bMax = _ui->doubleSpinBoxBEnd->value();
	if ( bMax < bStart )
	    _ui->doubleSpinBoxBEnd->setValue(9.9);
	bMax = _ui->doubleSpinBoxBEnd->value();

	Numbers<double> mags;
	for (OriginList::const_iterator j = _events->begin();
	        j != _events->end(); ++j) {

		MagnitudePtr mag = Magnitude::Find(j->second->preferredMagnitudeID());

		if ( !mag ) continue;
		if ( !checkMagnitude(mag->type()) ) continue;

		try {
			mags.add(mag->magnitude().value());
		} catch ( ... ) {}
	}

	// Count number of intervals
	double magLow = _ui->doubleSpinBoxStartMag->value();
	double magHigh = _ui->doubleSpinBoxStartMag->value() + _ui->doubleSpinBoxInterval->value();
	int nbInterval = 0;
	while ( magLow < mags.max() ) {
		magLow = magHigh;
		magHigh = magHigh + _ui->doubleSpinBoxInterval->value();
		++nbInterval;
	}

	// Determine interval number for bStart and bMax
	int count = 0;
	int bMaxNum = nbInterval;
	magLow = _ui->doubleSpinBoxStartMag->value();
	magHigh = _ui->doubleSpinBoxStartMag->value() + _ui->doubleSpinBoxInterval->value();
	while ( magLow < mags.max() ) {

		if ( bStart >= magLow && bStart < magHigh )
		    bStartNum = count;

		if ( bMax >= magLow && bMax < magHigh )
		    bMaxNum = count;

		magLow = magHigh;
		magHigh = magHigh + _ui->doubleSpinBoxInterval->value();
		++count;
	}

	Numbers<double> values;

	// Determine incremental values yinc
	QVector<double> yinc(nbInterval);
	QVector<double> magVector(nbInterval);
	magLow = _ui->doubleSpinBoxStartMag->value();
	magHigh = _ui->doubleSpinBoxStartMag->value() + _ui->doubleSpinBoxInterval->value();

	for (int i = 0; i < nbInterval; ++i) {

		yinc[i] = .0;
		magVector[i] = magLow + _ui->doubleSpinBoxInterval->value() / 2.;

		for (size_t j = 0; j < mags.size(); ++j)
			if ( mags.at(j) >= magLow && mags.at(j) < magHigh )
			    yinc[i] += 1.;

		values.add(yinc.at(i));

		magLow = magHigh;
		magHigh += _ui->doubleSpinBoxInterval->value();
	}

	//! Determine cumulative values
	QVector<double> ycum(nbInterval);
	QVector<double> lycum(nbInterval);
	QVector<double> lyinc(nbInterval);
	double lastInc = .0;
	for (int i = nbInterval - 1; i > -1; --i) {
		ycum[i] = yinc.at(i) + lastInc;
		lycum[i] = log10(ycum.at(i));
		lyinc[i] = log10(yinc.at(i));
		lastInc += yinc.at(i);
	}

	//! Evaluate incremental values linear regression for selected interval
	std::vector<double> incX, incY;
	for (int i = bStartNum; i < bMaxNum; ++i) {
		incX.push_back(magVector.at(i));
		incY.push_back(lyinc.at(i));
	}
	LinearRegression incRegression;
	try {
		incRegression = leastMeanSquareRegression(incX, incY);
	} catch ( ... ) {}

	//! Evaluate incremental values from a and b
	QVector<double> yie(bMaxNum - bStartNum);
	QVector<double> magVector_b(bMaxNum - bStartNum);
	for (int i = 0; i < (bMaxNum - bStartNum); ++i) {
		magVector_b[i] = magVector.at(i + bStartNum);
		yie[i] = pow(10, incRegression.y_intercept - (-1. * incRegression.slope) * magVector_b.at(i));
	}


	//! Evaluate cumulative values linear regression for selected interval
	std::vector<double> cumY;
	for (int i = bStartNum; i < bMaxNum; ++i)
		cumY.push_back(lycum.at(i));
	LinearRegression cumRegression;
	try {
		cumRegression = leastMeanSquareRegression(incX, cumY);
	} catch ( ... ) {}

	//! Evaluate cumulative values from a and b
	QVector<double> yce(bMaxNum - bStartNum);
	for (int i = 0; i < (bMaxNum - bStartNum); ++i)
		yce[i] = pow(10, cumRegression.y_intercept - (-1. * cumRegression.slope) * magVector_b.at(i));


	_plot->clearPlottables();
	_plot->xAxis->setRange(_ui->doubleSpinBoxStartMag->value(), 10);
	_plot->yAxis->setRange(.5, (100. * values.max()));

	QPen pen;

	// Add histogram
	QCPBars* magHistogram = new QCPBars(_plot->xAxis, _plot->yAxis);
	magHistogram->setName("Number of earthquakes");
	magHistogram->setPen(pen);
	magHistogram->setBrush(QColor(255, 76, 76));
	magHistogram->setWidth(_ui->doubleSpinBoxInterval->value());
	magHistogram->setData(magVector, yinc);
	_plot->addPlottable(magHistogram);

	// plot incremental values
	if ( _ui->checkBoxShowInc->isChecked() ) {

		pen.setWidth(2.);
		pen.setColor(Qt::black);
		_plot->addGraph();
		_plot->graph()->setPen(pen);
		_plot->graph()->setData(magVector, yinc);
		_plot->graph()->setName("Incremental values");
		_plot->graph()->setLineStyle(QCPGraph::lsNone);
		_plot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 6));
	}

	if ( _ui->checkBoxIncLinearRegression->isChecked() ) {

		pen.setWidth(2.);
		pen.setColor(Qt::black);

		QVector<double> magV, magVCum;
		for (int i = 0; i < magVector_b.size(); ++i) {
			(::numberIsNan(magVector_b.at(i))) ? magV.append(1.) : magV.append(magVector_b.at(i));
			(::numberIsNan(yie.at(i))) ? magVCum.append(1.) : magVCum.append(yie.at(i));
		}

		_plot->addGraph();
		_plot->graph()->setPen(pen);
		_plot->graph()->setData(magV, magVCum);
		_plot->graph()->setName(QString("a = %1, b = %2")
		        .arg(incRegression.y_intercept).arg(-1. * incRegression.slope));
		_plot->graph()->setLineStyle(QCPGraph::lsLine);
		_plot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
	}

	// Plot Cumulative values
	if ( _ui->checkBoxShowCum->isChecked() ) {

		pen.setWidth(2.);
		pen.setColor(QColor(86, 124, 181));
		_plot->addGraph();
		_plot->graph()->setPen(pen);
		_plot->graph()->setData(magVector, ycum);
		_plot->graph()->setName("Cumulative values");
		_plot->graph()->setLineStyle(QCPGraph::lsNone);
		_plot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 8));
	}

	if ( _ui->checkBoxCumLinearRegression->isChecked() ) {

		pen.setWidth(2.);
		pen.setColor(QColor(86, 124, 181));
		_plot->addGraph();
		_plot->graph()->setPen(pen);
		_plot->graph()->setData(magVector_b, yce);
		_plot->graph()->setName(QString("a = %1, b = %2")
		        .arg(cumRegression.y_intercept).arg(-1. * cumRegression.slope));
		_plot->graph()->setLineStyle(QCPGraph::lsLine);
		_plot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
	}

	_plot->replot();

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
