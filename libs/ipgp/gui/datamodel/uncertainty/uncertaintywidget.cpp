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

#include <ipgp/gui/datamodel/uncertainty/uncertaintywidget.h>
#include <ipgp/gui/datamodel/uncertainty/ui_uncertaintywidget.h>
#include <ipgp/core/string/string.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/core/datetime.h>
#include <ipgp/core/math/math.h>
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
UncertaintyWidget::
UncertaintyWidget(DatabaseQuery* query, QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::UncertaintyWidget) {

	_ui->setupUi(this);

	setObjectName("Uncertainty");

	QBoxLayout* l1 = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l1);
	l1->addWidget(_toolBox);
	l1->setMargin(0);
	_toolBox->show();

	QFont legendFont = font();
	legendFont.setBold(true);
	legendFont.setPointSize(11);

	_latPlot = new QCustomPlot(this);
	_latPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_latPlot->setBackground(Qt::transparent);
	_latPlot->axisRect()->setBackground(Qt::white);
	_latPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_latPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_latPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_latPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_latPlot->xAxis->grid()->setVisible(true);
	_latPlot->xAxis2->setVisible(true);
	_latPlot->xAxis2->setTicks(false);
	_latPlot->xAxis2->setTickLabels(false);
	_latPlot->yAxis->grid()->setSubGridVisible(false);
	_latPlot->yAxis2->setVisible(true);
	_latPlot->yAxis2->setTicks(false);
	_latPlot->yAxis2->setTickLabels(false);
	_latPlot->plotLayout()->insertRow(0);
	QCPPlotTitle* latTitle = new QCPPlotTitle(_latPlot, "Latitudes");
	latTitle->setFont(legendFont);
	_latPlot->plotLayout()->addElement(0, 0, latTitle);

	_lonPlot = new QCustomPlot(this);
	_lonPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_lonPlot->setBackground(Qt::transparent);
	_lonPlot->axisRect()->setBackground(Qt::white);
	_lonPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_lonPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_lonPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_lonPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_lonPlot->xAxis->grid()->setVisible(true);
	_lonPlot->xAxis2->setVisible(true);
	_lonPlot->xAxis2->setTicks(false);
	_lonPlot->xAxis2->setTickLabels(false);
	_lonPlot->yAxis->grid()->setSubGridVisible(false);
	_lonPlot->yAxis2->setVisible(true);
	_lonPlot->yAxis2->setTicks(false);
	_lonPlot->yAxis2->setTickLabels(false);
	_lonPlot->plotLayout()->insertRow(0);
	QCPPlotTitle* lonTitle = new QCPPlotTitle(_lonPlot, "Longitudes");
	lonTitle->setFont(legendFont);
	_lonPlot->plotLayout()->addElement(0, 0, lonTitle);

	_depthPlot = new QCustomPlot(this);
	_depthPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_depthPlot->setBackground(Qt::transparent);
	_depthPlot->axisRect()->setBackground(Qt::white);
	_depthPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_depthPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_depthPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_depthPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_depthPlot->xAxis->grid()->setVisible(true);
	_depthPlot->xAxis2->setVisible(true);
	_depthPlot->xAxis2->setTicks(false);
	_depthPlot->xAxis2->setTickLabels(false);
	_depthPlot->yAxis->grid()->setSubGridVisible(false);
	_depthPlot->yAxis2->setVisible(true);
	_depthPlot->yAxis2->setTicks(false);
	_depthPlot->yAxis2->setTickLabels(false);
	_depthPlot->plotLayout()->insertRow(0);
	QCPPlotTitle* depthTitle = new QCPPlotTitle(_depthPlot, "Depths");
	depthTitle->setFont(legendFont);
	_depthPlot->plotLayout()->addElement(0, 0, depthTitle);

	_rmsPlot = new QCustomPlot(this);
	_rmsPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_rmsPlot->setBackground(Qt::transparent);
	_rmsPlot->axisRect()->setBackground(Qt::white);
	_rmsPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_rmsPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_rmsPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_rmsPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_rmsPlot->xAxis->grid()->setVisible(true);
	_rmsPlot->xAxis2->setVisible(true);
	_rmsPlot->xAxis2->setTicks(false);
	_rmsPlot->xAxis2->setTickLabels(false);
	_rmsPlot->yAxis->grid()->setSubGridVisible(false);
	_rmsPlot->yAxis2->setVisible(true);
	_rmsPlot->yAxis2->setTicks(false);
	_rmsPlot->yAxis2->setTickLabels(false);
	_rmsPlot->plotLayout()->insertRow(0);
	QCPPlotTitle* rmsTitle = new QCPPlotTitle(_rmsPlot, "RMS");
	rmsTitle->setFont(legendFont);
	_rmsPlot->plotLayout()->addElement(0, 0, rmsTitle);

	_phasePlot = new QCustomPlot(this);
	_phasePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_phasePlot->setBackground(Qt::transparent);
	_phasePlot->axisRect()->setBackground(Qt::white);
	_phasePlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_phasePlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_phasePlot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_phasePlot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_phasePlot->xAxis->grid()->setVisible(true);
	_phasePlot->xAxis2->setVisible(true);
	_phasePlot->xAxis2->setTicks(false);
	_phasePlot->xAxis2->setTickLabels(false);
	_phasePlot->yAxis->grid()->setSubGridVisible(false);
	_phasePlot->yAxis2->setVisible(true);
	_phasePlot->yAxis2->setTicks(false);
	_phasePlot->yAxis2->setTickLabels(false);
	_phasePlot->plotLayout()->insertRow(0);
	QCPPlotTitle* phaseTitle = new QCPPlotTitle(_phasePlot, "Phases");
	phaseTitle->setFont(legendFont);
	_phasePlot->plotLayout()->addElement(0, 0, phaseTitle);

	_magPlot = new QCustomPlot(this);
	_magPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_magPlot->setBackground(Qt::transparent);
	_magPlot->axisRect()->setBackground(Qt::white);
	_magPlot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_magPlot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_magPlot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_magPlot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_magPlot->xAxis->grid()->setVisible(true);
	_magPlot->xAxis2->setVisible(true);
	_magPlot->xAxis2->setTicks(false);
	_magPlot->xAxis2->setTickLabels(false);
	_magPlot->yAxis->grid()->setSubGridVisible(false);
	_magPlot->yAxis2->setVisible(true);
	_magPlot->yAxis2->setTicks(false);
	_magPlot->yAxis2->setTickLabels(false);
	_magPlot->plotLayout()->insertRow(0);
	QCPPlotTitle* magTitle = new QCPPlotTitle(_magPlot, "Magnitudes");
	magTitle->setFont(legendFont);
	_magPlot->plotLayout()->addElement(0, 0, magTitle);

	QBoxLayout* l2 = new QVBoxLayout(_ui->frameLatPlot);
	_ui->frameLatPlot->setLayout(l2);
	l2->addWidget(_latPlot);
	l2->setMargin(0);

	QBoxLayout* l3 = new QVBoxLayout(_ui->frameLonPlot);
	_ui->frameLonPlot->setLayout(l3);
	l3->addWidget(_lonPlot);
	l3->setMargin(0);

	QBoxLayout* l4 = new QVBoxLayout(_ui->frameDepthPlot);
	_ui->frameDepthPlot->setLayout(l4);
	l4->addWidget(_depthPlot);
	l4->setMargin(0);

	QBoxLayout* l5 = new QVBoxLayout(_ui->frameRmsPlot);
	_ui->frameRmsPlot->setLayout(l5);
	l5->addWidget(_rmsPlot);
	l5->setMargin(0);

	QBoxLayout* l6 = new QVBoxLayout(_ui->framePhasePlot);
	_ui->framePhasePlot->setLayout(l6);
	l6->addWidget(_phasePlot);
	l6->setMargin(0);

	QBoxLayout* l7 = new QVBoxLayout(_ui->frameMagPlot);
	_ui->frameMagPlot->setLayout(l7);
	l7->addWidget(_magPlot);
	l7->setMargin(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
UncertaintyWidget::~UncertaintyWidget() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UncertaintyWidget::print(const ToolBox::ExportConfig& ec) {

	QString outputFile = QFileDialog::getSaveFileName(this, tr("Save uncertainty graphics"),
	    QDir::currentPath(), tr("%1 (*.%1)").arg(ec.format.toString()), 0,
	    QFileDialog::DontUseNativeDialog);

	if ( outputFile.isEmpty() )
	    return;

	QString out1, out2, out3, out4, out5, out6;

	if ( outputFile.right(3) != QString(".%1").arg(ec.format.toString()) ) {

		out1.append(outputFile);
		out1.append(".lat");
		out1.append(QString(".%1").arg(ec.format.toString()).toLower());

		out2.append(outputFile);
		out2.append(".lon");
		out2.append(QString(".%1").arg(ec.format.toString()).toLower());

		out3.append(outputFile);
		out3.append(".depth");
		out3.append(QString(".%1").arg(ec.format.toString()).toLower());

		out4.append(outputFile);
		out4.append(".rms");
		out4.append(QString(".%1").arg(ec.format.toString()).toLower());

		out5.append(outputFile);
		out5.append(".phases");
		out5.append(QString(".%1").arg(ec.format.toString()).toLower());

		out6.append(outputFile);
		out6.append(".mag");
		out6.append(QString(".%1").arg(ec.format.toString()).toLower());
	}
	else {
		out1.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out1.append(".hp");
		out1.append(QString(".%1").arg(ec.format.toString()).toLower());

		out2.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out2.append(".vp");
		out2.append(QString(".%1").arg(ec.format.toString()).toLower());

		out3.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out3.append(".depth");
		out3.append(QString(".%1").arg(ec.format.toString()).toLower());

		out4.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out4.append(".rms");
		out4.append(QString(".%1").arg(ec.format.toString()).toLower());

		out5.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out5.append(".phases");
		out5.append(QString(".%1").arg(ec.format.toString()).toLower());

		out6.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out6.append(".mag");
		out6.append(QString(".%1").arg(ec.format.toString()).toLower());
	}


	if ( ec.format == IPGP::Gui::PDF ) {
		_latPlot->savePdf(out1, 0, ec.printSize.width(), ec.printSize.height());
		_lonPlot->savePdf(out2, 0, ec.printSize.width(), ec.printSize.height());
		_depthPlot->savePdf(out3, 0, ec.printSize.width(), ec.printSize.height());
		_rmsPlot->savePdf(out4, 0, ec.printSize.width(), ec.printSize.height());
		_phasePlot->savePdf(out5, 0, ec.printSize.width(), ec.printSize.height());
		_magPlot->savePdf(out6, 0, ec.printSize.width(), ec.printSize.height());
	}
	else if ( ec.format == IPGP::Gui::PS ) {
		_latPlot->saveRastered(out1, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
		_lonPlot->saveRastered(out2, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
		_depthPlot->saveRastered(out3, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
		_rmsPlot->saveRastered(out4, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
		_phasePlot->saveRastered(out5, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
		_magPlot->saveRastered(out6, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
	}
	else if ( ec.format == IPGP::Gui::PNG ) {
		_latPlot->savePng(out1, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_lonPlot->savePng(out2, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_depthPlot->savePng(out3, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_rmsPlot->savePng(out4, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_phasePlot->savePng(out5, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_magPlot->savePng(out6, ec.printSize.width(), ec.printSize.height(), 1, -1);
	}
	else if ( ec.format == IPGP::Gui::JPG ) {
		_latPlot->saveJpg(out1, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_lonPlot->saveJpg(out2, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_depthPlot->saveJpg(out3, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_rmsPlot->saveJpg(out4, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_phasePlot->saveJpg(out5, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_magPlot->saveJpg(out6, ec.printSize.width(), ec.printSize.height(), 1, -1);
	}
	else if ( ec.format == IPGP::Gui::BMP ) {
		_latPlot->saveBmp(out1, ec.printSize.width(), ec.printSize.height(), 1);
		_lonPlot->saveBmp(out2, ec.printSize.width(), ec.printSize.height(), 1);
		_depthPlot->saveBmp(out3, ec.printSize.width(), ec.printSize.height(), 1);
		_rmsPlot->saveBmp(out4, ec.printSize.width(), ec.printSize.height(), 1);
		_phasePlot->saveBmp(out5, ec.printSize.width(), ec.printSize.height(), 1);
		_magPlot->saveBmp(out6, ec.printSize.width(), ec.printSize.height(), 1);

	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UncertaintyWidget::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("the event list is empty");
		return;
	}

	emit working();

	if ( _timer ) stopBlinking();

	int latErr10 = 0, latErr20 = 0, latErr30 = 0, latErr40 = 0, latErr50 = 0,
	        latErr60 = 0, latErrOther = 0;
	int lonErr10 = 0, lonErr20 = 0, lonErr30 = 0, lonErr40 = 0, lonErr50 = 0,
	        lonErr60 = 0, lonErrOther = 0;
	int depthErr10 = 0, depthErr20 = 0, depthErr30 = 0, depthErr40 = 0,
	        depthErr50 = 0, depthErr60 = 0, depthErrOther = 0;
	int magErr01 = 0, magErr02 = 0, magErr03 = 0, magErr04 = 0, magErr05 = 0,
	        magErr06 = 0, magErrOther = 0;
	int rmsErr01 = 0, rmsErr02 = 0, rmsErr03 = 0, rmsErr04 = 0, rmsErr05 = 0,
	        rmsErr06 = 0, rmsErrOther = 0;
	int phases2 = 0, phases4 = 0, phases6 = 0, phases8 = 0, phases10 = 0,
	        phases12 = 0, phasesOther = 0;

	size_t idx = 0;
	for (OriginList::const_iterator j = _events->begin();
	        j != _events->end(); ++j, ++idx) {

		emit loadingPercentage(percentageOfSomething<int>(_events->size(), idx),
		    objectName(), "Sorting events");

		OriginPtr org = j->first;

		if ( org->arrivalCount() == 0 )
		    _query->loadArrivals(org.get());

		if ( org->magnitudeCount() == 0 )
		    _query->loadMagnitudes(org.get());

		// Phase
		if ( org->arrivalCount() < 3 )
			phases2++;
		else if ( (org->arrivalCount() > 2) && (org->arrivalCount() < 5) )
			phases4++;
		else if ( (org->arrivalCount() > 4) && (org->arrivalCount() < 7) )
			phases6++;
		else if ( (org->arrivalCount() > 6) && (org->arrivalCount() < 9) )
			phases8++;
		else if ( (org->arrivalCount() > 8) && (org->arrivalCount() < 11) )
			phases10++;
		else if ( (org->arrivalCount() > 10) && (org->arrivalCount() < 13) )
			phases12++;
		else if ( org->arrivalCount() > 12 )
		    phasesOther++;


		// Longitude
		try {
			double lonErrKM = org->longitude().uncertainty();
			if ( lonErrKM < 10. )
				lonErr10++;
			else if ( (lonErrKM < 20.) && (lonErrKM > 10.) )
				lonErr20++;
			else if ( (lonErrKM < 30.) && (lonErrKM > 20.) )
				lonErr30++;
			else if ( (lonErrKM < 40.) && (lonErrKM > 30.) )
				lonErr40++;
			else if ( (lonErrKM < 50.) && (lonErrKM > 40.) )
				lonErr50++;
			else if ( (lonErrKM < 60.) && (lonErrKM > 50.) )
				lonErr60++;
			else if ( lonErrKM > 60. )
			    lonErrOther++;
		} catch ( ... ) {}

		// Latitude
		try {
			double latErrKM = org->latitude().uncertainty();
			if ( latErrKM < 10. )
				latErr10++;
			else if ( (latErrKM < 20.) && (latErrKM > 10.) )
				latErr20++;
			else if ( (latErrKM < 30.) && (latErrKM > 20.) )
				latErr30++;
			else if ( (latErrKM < 40.) && (latErrKM > 30.) )
				latErr40++;
			else if ( (latErrKM < 50.) && (latErrKM > 40.) )
				latErr50++;
			else if ( (latErrKM < 60.) && (latErrKM > 50.) )
				latErr60++;
			else if ( latErrKM > 60. )
			    latErrOther++;
		} catch ( ... ) {}

		// Depth
		try {
			double depthErrKM = org->depth().uncertainty();
			if ( depthErrKM < 10. )
				depthErr10++;
			else if ( (depthErrKM < 20.) && (depthErrKM > 10.) )
				depthErr20++;
			else if ( (depthErrKM < 30.) && (depthErrKM > 20.) )
				depthErr30++;
			else if ( (depthErrKM < 40.) && (depthErrKM > 30.) )
				depthErr40++;
			else if ( (depthErrKM < 50.) && (depthErrKM > 40.) )
				depthErr50++;
			else if ( (depthErrKM < 60.) && (depthErrKM > 50.) )
				depthErr60++;
			else if ( depthErrKM > 60. )
			    depthErrOther++;
		} catch ( ... ) {}

		// RMS
		try {
			double rmsOrg = org->quality().standardError();
			if ( rmsOrg < .1 )
				rmsErr01++;
			else if ( (rmsOrg < .2) && (rmsOrg > .1) )
				rmsErr02++;
			else if ( (rmsOrg < .3) && (rmsOrg > .2) )
				rmsErr03++;
			else if ( (rmsOrg < .4) && (rmsOrg > .3) )
				rmsErr04++;
			else if ( (rmsOrg < .5) && (rmsOrg > .4) )
				rmsErr05++;
			else if ( (rmsOrg < .6) && (rmsOrg > .5) )
				rmsErr06++;
			else if ( rmsOrg > .6 )
			    rmsErrOther++;

		} catch ( ... ) {}

		MagnitudePtr mag = Magnitude::Find(j->second->preferredMagnitudeID());

		if ( !mag ) continue;

		bool hasMag = false;
		float magErrKM;
		try {
			magErrKM = mag->magnitude().value();
			hasMag = true;
		} catch ( ... ) {}

		if ( !hasMag ) continue;

		if ( magErrKM < .1 )
			magErr01++;
		else if ( (magErrKM < .2) && (magErrKM > .1) )
			magErr02++;
		else if ( (magErrKM < .3) && (magErrKM > .2) )
			magErr03++;
		else if ( (magErrKM < .4) && (magErrKM > .3) )
			magErr04++;
		else if ( (magErrKM < .5) && (magErrKM > .4) )
			magErr05++;
		else if ( (magErrKM < .6) && (magErrKM > .5) )
			magErr06++;
		else if ( magErrKM > .6 )
		    magErrOther++;
	}

	errPlotPaint(_latPlot, latErr10, latErr20, latErr30, latErr40, latErr50, latErr60, latErrOther);
	errPlotPaint(_lonPlot, lonErr10, lonErr20, lonErr30, lonErr40, lonErr50, lonErr60, lonErrOther);
	errPlotPaint(_depthPlot, depthErr10, depthErr20, depthErr30, depthErr40, depthErr50, depthErr60, depthErrOther);
	errPlotPaint(_magPlot, magErr01, magErr02, magErr03, magErr04, magErr05, magErr06, magErrOther, ptMagnitude);
	errPlotPaint(_rmsPlot, rmsErr01, rmsErr02, rmsErr03, rmsErr04, rmsErr05, rmsErr06, rmsErrOther, ptMagnitude);
	errPlotPaint(_phasePlot, phases2, phases4, phases6, phases8, phases10, phases12, phasesOther, ptPhase);

	emit loadingPercentage(-1, objectName(), "");
	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UncertaintyWidget::errPlotPaint(QCustomPlot* plot, const int& err10,
                                     const int& err20, const int& err30,
                                     const int& err40, const int& err50,
                                     const int& err60, const int& errOther,
                                     PlotType type) {

	plot->clearGraphs();
	plot->clearItems();
	plot->clearPlottables();

	std::vector<int> tmp;
	tmp.push_back(err10);
	tmp.push_back(err20);
	tmp.push_back(err30);
	tmp.push_back(err40);
	tmp.push_back(err50);
	tmp.push_back(err60);
	tmp.push_back(errOther);

	QVector<double> errTicks;
	QVector<QString> errLabels;
	errTicks << 1 << 2 << 3 << 4 << 5 << 6 << 7;

	switch ( type ) {
		case ptMagnitude:
			errLabels << "+/- 0.1" << "+/- 0.2" << "+/- 0.3" << "+/- 0.4"
			    << "+/- 0.5"
			    << "+/- 0.6" << "+ 0.6";
			break;
		case ptPhase:
			errLabels << "+/- 2" << "+/- 4" << "+/- 6" << "+/- 8"
			    << "+/- 10"
			    << "+/- 12" << "+ 12";
			break;
		default:
			errLabels << "+/- 10km" << "+/- 20km" << "+/- 30km"
			    << "+/- 40km"
			    << "+/- 50km" << "+/- 60km" << "+ 60km";
			break;
	}

	QFont labelFont = font();
	labelFont.setPointSize(8);

	QCPBars* err10Bar = new QCPBars(plot->xAxis, plot->yAxis);
	QCPBars* err20Bar = new QCPBars(plot->xAxis, plot->yAxis);
	QCPBars* err30Bar = new QCPBars(plot->xAxis, plot->yAxis);
	QCPBars* err40Bar = new QCPBars(plot->xAxis, plot->yAxis);
	QCPBars* err50Bar = new QCPBars(plot->xAxis, plot->yAxis);
	QCPBars* err60Bar = new QCPBars(plot->xAxis, plot->yAxis);
	QCPBars* errOtherBar = new QCPBars(plot->xAxis, plot->yAxis);

	plot->addPlottable(err10Bar);
	plot->addPlottable(err20Bar);
	plot->addPlottable(err30Bar);
	plot->addPlottable(err40Bar);
	plot->addPlottable(err50Bar);
	plot->addPlottable(err60Bar);
	plot->addPlottable(errOtherBar);

	QPen pen;
	pen.setWidthF(1.2);

	pen.setColor(QColor(150, 222, 0));
	err10Bar->setPen(pen);
	err10Bar->setBrush(QColor(150, 222, 0, 70));

	pen.setColor(QColor(255, 222, 0));
	err20Bar->setPen(pen);
	err20Bar->setBrush(QColor(255, 222, 0, 70));

	pen.setColor(QColor(255, 174, 0));
	err30Bar->setPen(pen);
	err30Bar->setBrush(QColor(255, 174, 0, 70));

	pen.setColor(QColor(255, 104, 0));
	err40Bar->setPen(pen);
	err40Bar->setBrush(QColor(255, 104, 0, 70));

	pen.setColor(QColor(255, 29, 0));
	err50Bar->setPen(pen);
	err50Bar->setBrush(QColor(255, 29, 0, 70));

	pen.setColor(QColor(255, 0, 28));
	err60Bar->setPen(pen);
	err60Bar->setBrush(QColor(255, 0, 28, 70));

	pen.setColor(QColor(255, 0, 91));
	errOtherBar->setPen(pen);
	errOtherBar->setBrush(QColor(255, 0, 91, 70));

	plot->xAxis->setAutoTicks(false);
	plot->xAxis->setAutoTickLabels(false);
	plot->xAxis->setTickVector(errTicks);
	plot->xAxis->setTickLabelFont(labelFont);
	plot->xAxis->setTickVectorLabels(errLabels);
//plot->xAxis->setTickLabelRotation(60);
	plot->xAxis->setSubTickCount(0);
	plot->xAxis->setTickLength(0, 7);
	plot->xAxis->setRange(0, 9);
	plot->yAxis->setRange(0, 15);

	err10Bar->addData(1, err10);
	err20Bar->addData(2, err20);
	err30Bar->addData(3, err30);
	err40Bar->addData(4, err40);
	err50Bar->addData(5, err50);
	err60Bar->addData(6, err60);
	errOtherBar->addData(7, errOther);

	plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	plot->axisRect()->setRangeDrag(Qt::Vertical);
	plot->axisRect()->setRangeZoom(Qt::Vertical);

	try {
		plot->yAxis->setRange(.0, (double) getVectorMaxValue(tmp) + 1.);
	} catch ( ... ) {}

	plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
