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




#include <ipgp/gui/datamodel/eventenergy/eventenergy.h>
#include <ipgp/gui/datamodel/eventenergy/ui_eventenergy.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <QtGui>


using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace IPGP::Core;
using namespace IPGP::Core::Math;



namespace IPGP {
namespace Gui {

typedef QPair<double, double> MagMo;
//	typedef QPair<Time, MagMo> TimedMo;
typedef QMap<double, MagMo> MagMap;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventEnergy::EventEnergy(Seiscomp::DataModel::DatabaseQuery* query,
                         QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::EventEnergy) {

	_ui->setupUi(this);
	setObjectName("EventEnergy");

	_ui->listWidgetAvailable->setSelectionMode(QAbstractItemView::MultiSelection);
	_ui->listWidgetSelected->setSelectionMode(QAbstractItemView::MultiSelection);

	_ui->checkBoxPlotBaseTen->setChecked(true);
	_ui->checkBoxShowCumulativeValues->setChecked(true);
	_ui->checkBoxShowIncrementalValues->setChecked(true);
	_ui->checkBoxShowLegend->setChecked(true);

	_plot = new SquarreZoomPlot(this, f);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
	_plot->plotLayout()->setAutoMargins(QCP::msNone);
	_plot->plotLayout()->setMargins(QMargins(0, 0, 0, 0));
//	_plot->axisRect(0)->addAxis(QCPAxis::atRight)->setTickLabelColor(Qt::darkGreen);
	_plot->axisRect()->setBackground(Qt::white);
	_plot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_plot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
	_plot->xAxis->setDateTimeFormat("yyyy-MM-dd\nhh:mm:ss");
	_plot->xAxis->setTickLabelPadding(0);
	_plot->xAxis->setSubTickCount(5);
	_plot->xAxis->setTickLength(0, _plot->xAxis->axisRect()->axis(QCPAxis::atBottom)->tickLengthIn() + 3);
	_plot->xAxis->setSubTickLength(0, _plot->xAxis->axisRect()->axis(QCPAxis::atBottom)->subTickLengthIn() + 2);
#ifndef __APPLE__
	_plot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
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
#ifndef __APPLE__
	_plot->yAxis->setLabelFont(QFont(QFont().family(), 8));
#endif
	_plot->yAxis->setLabel(QString("Cumulated Energy [Joule]"));
	_plot->yAxis->setNumberPrecision(0); // makes sure "1*10^4" is displayed only as "10^4"
	_plot->yAxis->setSubTickCount(10);
	_plot->yAxis->setPadding(5); // a bit more space to the left border
	_plot->yAxis->grid()->setSubGridVisible(true);

	QBoxLayout* l1 = new QVBoxLayout(_ui->framePlot);
	_ui->framePlot->setLayout(l1);
	l1->addWidget(_plot);
	l1->setMargin(0);

	QBoxLayout* l2 = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l2);
	l2->addWidget(_toolBox);
	l2->setMargin(0);
	_toolBox->show();

	connect(_ui->toolButtonAdd, SIGNAL(clicked()), this, SLOT(addMagnitude()));
	connect(_ui->toolButtonRemove, SIGNAL(clicked()), this, SLOT(removeMagnitude()));
	connect(_ui->checkBoxPlotBaseTen, SIGNAL(clicked()), this, SLOT(replot()));
	connect(_ui->checkBoxShowCumulativeValues, SIGNAL(clicked()), this, SLOT(replot()));
	connect(_ui->checkBoxShowIncrementalValues, SIGNAL(clicked()), this, SLOT(replot()));
	connect(_ui->checkBoxShowLegend, SIGNAL(clicked()), this, SLOT(showHideLegend()));
	connect(_plot, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventEnergy::~EventEnergy() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEnergy::print(const ToolBox::ExportConfig& ec) {

	QString outputFile = QFileDialog::getSaveFileName(this, tr("Save S-P Duration graphic"),
	    QDir::currentPath(), tr("%1 (*.%2)").arg(ec.format.toString())
	        .arg(QString(ec.format.toString()).toLower()), 0,
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
void EventEnergy::setMagnitudeTypes(const Core::MagnitudeTypes& magnitudes) {

	_magnitudes = magnitudes;

	_ui->listWidgetAvailable->clear();
	_ui->listWidgetSelected->clear();

	for (size_t i = 0; i < magnitudes.size(); ++i)
		_ui->listWidgetSelected->addItem(magnitudes.at(i).c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool EventEnergy::checkMagnitude(const std::string& magtype) {

	for (int i = 0; i < _ui->listWidgetSelected->count(); ++i)
		if ( _ui->listWidgetSelected->item(i)->text() == magtype.c_str() )
		    return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEnergy::addMagnitude() {
	for (int i = _ui->listWidgetAvailable->count() - 1; i > -1; --i)
		if ( _ui->listWidgetAvailable->item(i)->isSelected() )
		    _ui->listWidgetSelected->addItem(_ui->listWidgetAvailable->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEnergy::removeMagnitude() {
	for (int i = _ui->listWidgetSelected->count() - 1; i > -1; --i)
		if ( _ui->listWidgetSelected->item(i)->isSelected() )
		    _ui->listWidgetAvailable->addItem(_ui->listWidgetSelected->takeItem(i));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEnergy::plottableClicked(QCPAbstractPlottable* plottable,
                                   QMouseEvent* event) {
	Q_UNUSED(event);
	emit elementClicked(plottable->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEnergy::showHideLegend() {
	_plot->legend->setVisible(_ui->checkBoxShowLegend->isChecked());
	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEnergy::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("The event list is empty");
		return;
	}

	if ( _timer )
	    stopBlinking();

	if ( _ui->listWidgetSelected->count() == 0 ) {
		QMessageBox::critical(this, tr("Magnitude"),
		    QString("At least one magnitude should be available for plotting."));
		emit plottingError("No magnitude(s) selected, nothing plotted");
		return;
	}

	emit working();

	// Ordered map
	MagMap map;
	// Un-ordered list
	MagnitudeList m;

	//! Store all valid magnitude values and origins time, calculate mo (in joule)
	for (OriginList::const_iterator j = _events->begin();
	        j != _events->end(); ++j) {

		MagnitudePtr mag = Magnitude::Find(j->second->preferredMagnitudeID());

		if ( !mag ) continue;

		if ( !checkMagnitude(mag->type()) ) continue;

		double magnitude;
		try {
			magnitude = mag->magnitude().value();
		} catch ( ... ) {
			//! Ignore event without magnitude!!
			continue;
		}

		// Gutenber-Richter formula...
		//! TODO Make this configurable?!
		double mo = pow(10, 2.9 + 1.92 * magnitude - .024 * pow(magnitude, 2));
		map.insert((double) j->first->time().value(), MagMo(magnitude, mo));

		EventMagnitude e;
		e.time = j->first->time().value();
		e.publicID = j->first->publicID();
		e.magnitude = magnitude;
		e.magnitudeEnergy = mo;
		e.magnitudeSize = (4.9 * (magnitude - 1.2)) / 2.;
		if ( e.magnitudeSize < 2. )
		    e.magnitudeSize = 2.;

		try {
			if ( j->first->evaluationMode() == AUTOMATIC )
				e.isAuto = true;
			else if ( j->first->evaluationMode() == MANUAL )
			    e.isAuto = false;
		} catch ( ... ) {
			e.isAuto = false;
		}

		try {
			e.depth = j->first->depth().value();
			e.hasDepth = true;
		} catch ( ... ) {
			e.hasDepth = false;
		}

		m.append(e);
	}

	//! Determine cumulative values of magnitudes values and mo values
	QVector<double> moCum(map.size());
	QVector<double> times(map.size());
	QVector<double> moInc(map.size());

	double lastMo = .0;
	int i = 0;
	for (MagMap::const_iterator it = map.constBegin();
	        it != map.constEnd(); ++it, ++i) {
		moCum[i] = it.value().second + lastMo;
		times[i] = it.key();
		moInc[i] = it.value().second;
		lastMo += it.value().second;
	}

	_plot->clearPlottables();
	_plot->clearGraphs();

	if ( _ui->checkBoxPlotBaseTen->isChecked() ) {
		_plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
		_plot->yAxis->setScaleLogBase(10);
	}
	else
		_plot->yAxis->setScaleType(QCPAxis::stLinear);

	// Draw the cumulated values
	if ( _ui->checkBoxShowCumulativeValues->isChecked() ) {
		QCPGraph* a = _plot->addGraph();
		a->setData(times, moCum);
		a->setPen(QPen(QColor("#6050F8")));
		a->setName(QString("Cumulated energy"));
		a->setLineStyle(QCPGraph::lsLine);
		a->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
	}


	// Draw the incremental values
	if ( _ui->checkBoxShowIncrementalValues->isChecked() ) {

		// Pen offset
		double offset = 2.;

		// Draw empty shapes first
		for (MagnitudeList::const_iterator it = m.constBegin();
		        it != m.constEnd(); ++it) {

			QCPGraph* b = _plot->addGraph();
			b->addData((double) (*it).time, (*it).magnitudeEnergy);
			b->setName((*it).publicID.c_str());
			b->setLineStyle(QCPGraph::lsNone);
			b->setPen(QPen());
			b->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
			    QCPScatterStyle::ssSquare : QCPScatterStyle::ssCircle,
			    (*it).magnitudeSize + offset));
			b->setTooltip(QString("%1\n%2")
			    .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
			    .arg((*it).publicID.c_str()));

			_plot->legend->removeItem(_plot->legend->itemCount() - 1);
		}

		// Fill up shapes with colors
		for (MagnitudeList::const_iterator it = m.constBegin();
		        it != m.constEnd(); ++it) {

			QColor c = Misc::getDepthColoration((*it).depth);

			QCPGraph* b = _plot->addGraph();
			b->addData((double) (*it).time, (*it).magnitudeEnergy);
			b->setName((*it).publicID.c_str());
			b->setLineStyle(QCPGraph::lsNone);
			b->setPen(c);
			b->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
			    QCPScatterStyle::ssFilledSquare : QCPScatterStyle::ssDisc, c, (*it).magnitudeSize));
			b->setTooltip(QString("%1\n%2")
			    .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
			    .arg((*it).publicID.c_str()));

			_plot->legend->removeItem(_plot->legend->itemCount() - 1);
		}
	}

	if ( _ui->checkBoxShowIncrementalRegression->isChecked() ) {

		LinearRegression reg;
		try {
			reg = leastMeanSquareRegression(times.toStdVector(), moInc.toStdVector());
		} catch ( ... ) {}

		QPen p;
		p.setWidthF(2.);

		QVector<double> lrInc(times.size());
		for (int i = 0; i < times.size(); ++i)
			lrInc[i] = reg.slope * times.at(i) + reg.y_intercept;

		QCPGraph* g = _plot->addGraph();
		g->setData(times, lrInc);
		g->setPen(p);
		g->setName(QString("Inc. a = %1, b = %2").arg(reg.slope).arg(reg.y_intercept));
		g->setLineStyle(QCPGraph::lsLine);
		g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
	}


	if ( _ui->checkBoxShowCumulativeRegression->isChecked() ) {

		LinearRegression reg;
		try {
			reg = leastMeanSquareRegression(times.toStdVector(), moCum.toStdVector());
		} catch ( ... ) {}

		QPen p(QColor(86, 124, 181));
		p.setWidthF(2.);

		QVector<double> lrCum(times.size());
		for (int i = 0; i < times.size(); ++i)
			lrCum[i] = reg.slope * times.at(i) + reg.y_intercept;

		QCPGraph* g = _plot->addGraph();
		g->setData(times, lrCum);
		g->setPen(p);
		g->setName(QString("Cum. a = %1, b = %2").arg(reg.slope).arg(reg.y_intercept));
		g->setLineStyle(QCPGraph::lsLine);
		g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
	}

	// Add legend
	if ( _ui->checkBoxShowLegend->isChecked() ) {
		QFont font = this->font();
#ifndef __APPLE__
		font.setPointSize(8);
#endif
		_plot->legend->setVisible(true);
		_plot->legend->setFont(font);
		_plot->legend->setSelectedFont(font);
		_plot->legend->setSelectableParts(QCPLegend::spItems);
	}
	else
		_plot->legend->setVisible(false);

	_plot->rescaleAxes();
	_plot->replot();

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
