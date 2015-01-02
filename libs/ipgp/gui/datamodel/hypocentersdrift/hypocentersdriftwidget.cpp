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

#include <ipgp/gui/datamodel/hypocentersdrift/hypocentersdriftwidget.h>
#include <ipgp/gui/datamodel/hypocentersdrift/ui_hypocentersdriftwidget.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <ipgp/gui/client/application.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/math/geo.h>
#include <QtGui>


using namespace Seiscomp;
using namespace Seiscomp::DataModel;

using namespace IPGP::Core;


static double const MINITEMSIZE = 2.;
static double const MAXITEMSIZE = 20.;


namespace {

enum ETypes {
	eLatitude, eLongitude, eDepth
};
const char* STypes[] = { "Latitude", "Longitude", "Depth" };
const std::vector<const char*> EString(STypes, STypes + sizeof(STypes) / sizeof(STypes[0]));
const size_t getType(const ETypes& e) {
	char* str;
	for (size_t i = 0; i < EString.size(); ++i) {
		str = (char*) EString.at(i);
		if ( str == STypes[e] )
		    return i;
	}
	return -1;
}

}



namespace IPGP {
namespace Gui {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HypocentersDriftWidget::
HypocentersDriftWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::HypocentersDriftWidget) {

	setObjectName("HypocentersDrift");

	_ui->setupUi(this);

	QStringList list;
	for (size_t i = 0; i < ::EString.size(); ++i)
		list << ::EString[i];
	_ui->comboBoxType->addItems(list);

	QBoxLayout* l = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l);
	l->addWidget(_toolBox);
	l->setMargin(0);
	_toolBox->show();

	_plot = new SquarreZoomPlot(_ui->framePlot);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
	_plot->axisRect()->setBackground(Qt::white);
	_plot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_plot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_plot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_plot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
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
#ifndef __APPLE__
	_plot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
#endif
	_plot->yAxis->grid()->setSubGridVisible(false);
	_plot->xAxis2->setVisible(true);
	_plot->xAxis2->setTicks(false);
	_plot->xAxis2->setTickLabels(false);
	_plot->yAxis2->setVisible(true);
	_plot->yAxis2->setTicks(false);
	_plot->yAxis2->setTickLabels(false);
	_plot->axisRect()->setRangeDrag(Qt::Horizontal);
	_plot->axisRect()->setRangeZoom(Qt::Horizontal);
	_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

	QBoxLayout* l2 = new QVBoxLayout(_ui->framePlot);
	_ui->framePlot->setLayout(l2);
	l2->addWidget(_plot);
	l2->setMargin(0);

	_ui->radioButtonFormula->setChecked(true);
	_ui->frameItemSize->hide();
	_ui->doubleSpinBoxMinSize->setValue(MINITEMSIZE);
	_ui->doubleSpinBoxMaxSize->setValue(MAXITEMSIZE);

	connect(_plot, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));
	connect(_ui->radioButtonFormula, SIGNAL(clicked()), this, SLOT(showFormulaBox()));
	connect(_ui->radioButtonNormalize, SIGNAL(clicked()), this, SLOT(showCustomSizeBox()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HypocentersDriftWidget::~HypocentersDriftWidget() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HypocentersDriftWidget::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("The event list is empty, nothing to plot");
		return;
	}

	if ( _timer ) stopBlinking();

	emit working();

	ItemList items;
	Core::Math::Numbers<double> mags;
	for (OriginList::const_iterator i = _events->begin();
	        i != _events->end(); ++i) {

		OriginPtr org = i->first;
		MagnitudePtr mag = Magnitude::Find(i->second->preferredMagnitudeID());

		bool hasMag = true;
		if ( !mag )
		    hasMag = false;

		double magnitude;
		if ( hasMag )
		    try {
			    magnitude = mag->magnitude().value();
		    } catch ( ... ) {}

		EventItem e;
		e.time = org->time().value();
		e.publicID = org->publicID();
		if ( AppInstance->scheme().distanceInKM() ) {
			e.latitude = org->latitude();
			e.longitude = org->longitude();
		}
		else {
			e.latitude = Seiscomp::Math::Geo::km2deg(org->latitude());
			e.longitude = Seiscomp::Math::Geo::km2deg(org->longitude());
		}

		if ( hasMag ) {

			mags.add(magnitude);
			e.magnitude = magnitude;
			e.hasMagnitude = true;
			e.magnitudeSize = (4.9 * (magnitude - 1.2)) / 2.;
			if ( e.magnitudeSize < 2. )
			    e.magnitudeSize = 2.;
		}
		else {
			e.magnitudeSize = 2.;
			e.hasMagnitude = false;
		}

		try {
			if ( org->evaluationMode() == AUTOMATIC )
				e.isAuto = true;
			else if ( org->evaluationMode() == MANUAL )
			    e.isAuto = false;
		} catch ( ... ) {
			e.isAuto = false;
		}

		try {
			e.depth = org->depth().value();
			e.hasDepth = true;
		} catch ( ... ) {
			e.hasDepth = false;
		}

		items << e;
	}

	_plot->clearPlottables();
	_plot->clearGraphs();

	::ETypes t = ::ETypes(_ui->comboBoxType->currentIndex());

	_plot->yAxis->setLabel(QString(::STypes[_ui->comboBoxType->currentIndex()]));

	// Pen offset
	double offset = 2.;

	// Draw empty shapes first
	for (ItemList::const_iterator it = items.constBegin();
	        it != items.constEnd(); ++it) {

		if ( t == ::eDepth && !(*it).hasDepth ) continue;

		double itmSize = (*it).magnitudeSize + offset;
		if ( _ui->radioButtonNormalize->isChecked() ) {
			if ( (*it).hasMagnitude )
			    itmSize = Core::Math::remap<double>((*it).magnitude, mags.min(), mags.max(),
			        _ui->doubleSpinBoxMinSize->value(), _ui->doubleSpinBoxMaxSize->value()) + offset;
		}

		QCPGraph* g = _plot->addGraph();

		if ( t == ::eLatitude )
			g->addData((double) (*it).time, (*it).latitude);
		else if ( t == ::eLongitude )
			g->addData((double) (*it).time, (*it).longitude);
		else if ( t == ::eDepth )
		    g->addData((double) (*it).time, (*it).depth);

		g->setName((*it).publicID.c_str());
		g->setLineStyle(QCPGraph::lsNone);
		g->setPen(QPen());
		g->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssCircle, itmSize));
		g->setTooltip(QString("%1\n%2")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).publicID.c_str()));

		_plot->legend->removeItem(_plot->legend->itemCount() - 1);
	}

	// Fill up shapes with colors
	for (ItemList::const_iterator it = items.constBegin();
	        it != items.constEnd(); ++it) {

		if ( t == ::eDepth && !(*it).hasDepth )
		    continue;

		double itmSize = (*it).magnitudeSize;
		if ( _ui->radioButtonNormalize->isChecked() ) {
			if ( (*it).hasMagnitude )
			    itmSize = Core::Math::remap<double>((*it).magnitude, mags.min(), mags.max(),
			        _ui->doubleSpinBoxMinSize->value(), _ui->doubleSpinBoxMaxSize->value());
		}

		QColor c = Misc::getDepthColoration((*it).depth);

		QCPGraph* g = _plot->addGraph();

		if ( t == ::eLatitude )
			g->addData((double) (*it).time, (*it).latitude);
		else if ( t == ::eLongitude )
			g->addData((double) (*it).time, (*it).longitude);
		else if ( t == ::eDepth )
		    g->addData((double) (*it).time, (*it).depth);

		g->setName((*it).publicID.c_str());
		g->setLineStyle(QCPGraph::lsNone);
		g->setPen(c);
		g->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssDisc, c, itmSize));
		g->setTooltip(QString("%1\n%2")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).publicID.c_str()));

		_plot->legend->removeItem(_plot->legend->itemCount() - 1);
	}

	_plot->rescaleAxes();
	_plot->replot();

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HypocentersDriftWidget::print(const ToolBox::ExportConfig& ec) {

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
void HypocentersDriftWidget::plottableClicked(QCPAbstractPlottable* plottable,
                                              QMouseEvent* event) {
	emit elementClicked(plottable->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HypocentersDriftWidget::showFormulaBox() {
	_ui->frameItemSize->hide();
	_ui->labelMagnitudeFormula->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HypocentersDriftWidget::showCustomSizeBox() {
	_ui->frameItemSize->show();
	_ui->labelMagnitudeFormula->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
