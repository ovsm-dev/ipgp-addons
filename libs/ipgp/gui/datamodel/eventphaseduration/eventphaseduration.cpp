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




#include <ipgp/gui/datamodel/eventphaseduration/eventphaseduration.h>
#include <ipgp/gui/datamodel/eventphaseduration/ui_eventphaseduration.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <QtGui>



using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace IPGP::Core;
using namespace IPGP::Core::Math;


static double const MINITEMSIZE = 2.;
static double const MAXITEMSIZE = 20.;


namespace {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickPtr getPick(const QList<PickPtr>& list, const std::string& networkCode,
                const std::string& stationCode, const std::string& phaseCode) {
	for (QList<PickPtr>::const_iterator it = list.constBegin();
	        it != list.constEnd(); ++it) {
		if ( (*it)->waveformID().networkCode() == networkCode
		        && (*it)->waveformID().stationCode() == stationCode
		        && (*it)->phaseHint().code().find(phaseCode) != std::string::npos )
		    return (*it);
	}
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickPtr findPick(const QList<PickPtr>& list, const std::string& publicID) {
	for (QList<PickPtr>::const_iterator it = list.constBegin();
	        it != list.constEnd(); ++it) {
		if ( (*it)->publicID() == publicID )
		    return (*it);
	}
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}




namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventPhaseDuration::
EventPhaseDuration(Seiscomp::DataModel::DatabaseQuery* query,
                   QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::EventPhaseDuration) {

	_ui->setupUi(this);
	setObjectName("EventPhaseDuration");

	_ui->tableWidgetStations->setColumnCount(4);
	_ui->tableWidgetStations->setHorizontalHeaderLabels(
	    QStringList() << "Network" << "Station" << "Color" << "Used");
	_ui->tableWidgetStations->verticalHeader()->setVisible(false);
	_ui->tableWidgetStations->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	_ui->tableWidgetStations->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->tableWidgetStations->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidgetStations->setSelectionMode(QAbstractItemView::SingleSelection);
	_ui->tableWidgetStations->resizeColumnsToContents();
	_ui->tableWidgetStations->resizeRowsToContents();
	_ui->tableWidgetStations->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_ui->tableWidgetStations->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidgetStations->setAlternatingRowColors(true);
//	_ui->tableWidgetStations->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui->tableWidgetStations->setSortingEnabled(true);


	_plot = new SquarreZoomPlot(this, f);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
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
#ifndef __APPLE__
	_plot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
#endif
	_plot->yAxis->setLabel(QString("Duration [second]"));
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

	QBoxLayout* l = new QVBoxLayout(_ui->framePlot);
	_ui->framePlot->setLayout(l);
	l->addWidget(_plot);
	l->setMargin(0);

	QBoxLayout* l2 = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l2);
	l2->addWidget(_toolBox);
	l2->setMargin(0);
	_toolBox->show();

	_ui->radioButtonFormula->setChecked(true);
	_ui->frameItemSize->hide();
	_ui->doubleSpinBoxMinSize->setValue(MINITEMSIZE);
	_ui->doubleSpinBoxMaxSize->setValue(MAXITEMSIZE);

	connect(_plot, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));
	connect(_ui->checkBoxShowLegend, SIGNAL(clicked()), this, SLOT(showHideLegend()));
	connect(_ui->checkBoxShowLinearRegression, SIGNAL(clicked()), this, SLOT(replot()));
	connect(_ui->radioButtonFormula, SIGNAL(clicked()), this, SLOT(showFormulaBox()));
	connect(_ui->radioButtonNormalize, SIGNAL(clicked()), this, SLOT(showCustomSizeBox()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventPhaseDuration::~EventPhaseDuration() {
	pickList.clear();
	colorMap.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::setEvents(Core::OriginList* list, const bool& copy) {
	PlottingWidget::setEvents(list, copy);
	updateStationList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool EventPhaseDuration::checkStation(const QString& name) {

	for (int i = 0; i < _ui->tableWidgetStations->rowCount(); ++i)
		if ( name == QString("%1.%2").arg(_ui->tableWidgetStations->item(i, 0)->text())
		        .arg(_ui->tableWidgetStations->item(i, 1)->text())
		        && _ui->tableWidgetStations->item(i, 3)->checkState() == Qt::Checked )
		    return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("The event list is empty");
		return;
	}

	if ( _timer )
	    stopBlinking();

	size_t checked = 0;
	for (int i = 0; i < _ui->tableWidgetStations->rowCount(); ++i)
		if ( _ui->tableWidgetStations->item(i, 3)->checkState() == Qt::Checked )
		    ++checked;

	if ( checked == 0 ) {
		QMessageBox::critical(this, tr("Station list error"),
		    QString("At least one Station should be available for plotting."));
		emit plottingError("No station(s) selected, nothing plotted");
		return;
	}

	emit working();

	StationList stations;
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
		    } catch ( ... ) {
			//				continue;
		}

		//! At this point, picks have been fetched and stored in the local
		//! pick list, which makes them immediately available, but we can use
		//! the parent's methods to fetch them thru the PO cache/map
		PickList ppicks = getPicks(org, "P");
		PickList spicks = getPicks(org, "S");

		for (PickList::const_iterator it = ppicks.constBegin();
		        it != ppicks.constEnd(); ++it) {

			if ( !(*it) ) continue;

			if ( !checkStation(QString("%1.%2").arg((*it)->waveformID().networkCode().c_str())
			        .arg((*it)->waveformID().stationCode().c_str())) )
			    continue;

			PickPtr spick = ::getPick(spicks, (*it)->waveformID().networkCode(), (*it)->waveformID().stationCode(), "S");

			if ( !spick ) continue;

			EventStation e;
			e.networkCode = (*it)->waveformID().networkCode();
			e.stationCode = (*it)->waveformID().stationCode();
			e.time = org->time().value();
			e.phasediff = (double) spick->time().value() - (double) (*it)->time().value();
			e.originID = org->publicID();

			if ( hasMag ) {

				mags.add(magnitude);
				e.magnitude = magnitude;
				e.hasMag = true;
				e.magnitudeSize = (4.9 * (magnitude - 1.2)) / 2.;
				if ( e.magnitudeSize < 2. )
				    e.magnitudeSize = 2.;
			}
			else {
				e.magnitudeSize = 2.;
				e.hasMag = false;
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

			stations << e;
		}
	}


	_plot->clearPlottables();
	_plot->clearGraphs();

	// Pen offset
	double offset = 2.;
	QVector<double> xs, ys;

	// Draw empty shapes first
	for (StationList::const_iterator it = stations.constBegin();
	        it != stations.constEnd(); ++it) {

		double itmSize = (*it).magnitudeSize + offset;
		if ( _ui->radioButtonNormalize->isChecked() ) {
			if ( (*it).hasMag )
			    itmSize = Core::Math::remap<double>((*it).magnitude, mags.min(), mags.max(),
			        _ui->doubleSpinBoxMinSize->value(), _ui->doubleSpinBoxMaxSize->value()) + offset;
		}

		QCPGraph* g = _plot->addGraph();
		g->addData((double) (*it).time, (*it).phasediff);
		g->setName((*it).originID.c_str());
		g->setLineStyle(QCPGraph::lsNone);
		g->setPen(QPen());
		g->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssCircle, itmSize));
		g->setTooltip(QString("%1\n%2\n%3.%4 - %5 sec")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).originID.c_str())
		        .arg((*it).networkCode.c_str())
		        .arg((*it).stationCode.c_str())
		        .arg(QString::number((*it).phasediff, 'f', 2)));

		xs << (double) (*it).time;
		ys << (*it).phasediff;

		_plot->legend->removeItem(_plot->legend->itemCount() - 1);
	}

	// Fill up shapes with colors
	for (StationList::const_iterator it = stations.constBegin();
	        it != stations.constEnd(); ++it) {

		QColor c = colorMap.value(QString("%1.%2").arg((*it).networkCode.c_str())
		        .arg((*it).stationCode.c_str()));

		double itmSize = (*it).magnitudeSize;
		if ( _ui->radioButtonNormalize->isChecked() ) {
			if ( (*it).hasMag )
			    itmSize = Core::Math::remap<double>((*it).magnitude, mags.min(), mags.max(),
			        _ui->doubleSpinBoxMinSize->value(), _ui->doubleSpinBoxMaxSize->value());
		}

		QCPGraph* g = _plot->addGraph();
		g->addData((double) (*it).time, (*it).phasediff);
		g->setName((*it).originID.c_str());
		g->setLineStyle(QCPGraph::lsNone);
		g->setPen(c);
		g->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssDisc, c, itmSize));
		g->setTooltip(QString("%1\n%2\n%3.%4 - %5 sec")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).originID.c_str())
		        .arg((*it).networkCode.c_str())
		        .arg((*it).stationCode.c_str())
		        .arg(QString::number((*it).phasediff, 'f', 2)));

		_plot->legend->removeItem(_plot->legend->itemCount() - 1);
	}


	if ( _ui->checkBoxShowLinearRegression->isChecked() ) {

		LinearRegression reg;
		try {
			reg = leastMeanSquareRegression(xs.toStdVector(), ys.toStdVector());
		} catch ( ... ) {}

		QPen p(QColor(86, 124, 181));
		p.setWidthF(2.);

		QVector<double> lrInc(xs.size());
		for (int i = 0; i < xs.size(); ++i)
			lrInc[i] = reg.slope * xs.at(i) + reg.y_intercept;

		QCPGraph* g = _plot->addGraph();
		g->setData(xs, lrInc);
		g->setPen(p);
		g->setName(QString("Inc. a = %1, b = %2").arg(reg.slope).arg(reg.y_intercept));
		g->setTooltip(QString("Incremental values regression line.\na=%1, b=%2").arg(reg.slope).arg(reg.y_intercept));
		g->setLineStyle(QCPGraph::lsLine);
		g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
		g->setSelectable(false);
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




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::print(const ToolBox::ExportConfig& ec) {

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
void EventPhaseDuration::updateStationList() {

	emit working();

	_ui->tableWidgetStations->clearSelection();
	_ui->tableWidgetStations->model()->removeRows(0, _ui->tableWidgetStations->rowCount());

	colorMap.clear();


	QStringList stations;
	QList<QColor> colors;

	size_t count = 0;
	for (OriginList::iterator i = _events->begin();
	        i != _events->end(); ++i, ++count) {

		OriginPtr org = i->first;

		//! Makes sure we don't end up with and empty station's list
		if ( org->arrivalCount() == 0 )
		    _query->loadArrivals(org.get());

		for (size_t j = 0; j < org->arrivalCount(); ++j) {

			ArrivalPtr ar = org->arrival(j);
			if ( !ar ) continue;

			PickPtr pick = ::findPick(pickList, ar->pickID());
			if ( !pick )
			    pick = Pick::Find(ar->pickID());
			if ( !pick ) {
				pick = Pick::Cast(_query->getObject(Pick::TypeInfo(), ar->pickID()));
				if ( !pick ) continue;
			}

			//! Store an instance of the pointer in a list to make sure it
			//! is available for a next use
			pickList.append(pick);

			QString name = QString("%1.%2").arg(pick->waveformID().networkCode().c_str())
			        .arg(pick->waveformID().stationCode().c_str());

			if ( !stations.contains(name) ) {
				stations.append(name);

				int idx = _ui->tableWidgetStations->rowCount();
				_ui->tableWidgetStations->insertRow(idx);

				QColor c = QColor(rand() % (254 + 1), rand() % (254 + 1), rand() % (254 + 1));
				while ( colors.contains(c) )
					c = QColor(rand() % (254 + 1), rand() % (254 + 1), rand() % (254 + 1));

				colorMap.insert(name, c);
				colors << c;

				QTableWidgetItem* netCode = new QTableWidgetItem(pick->waveformID().networkCode().c_str());
				netCode->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
				_ui->tableWidgetStations->setItem(idx, 0, netCode);

				QTableWidgetItem* staCode = new QTableWidgetItem(pick->waveformID().stationCode().c_str());
				staCode->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
				_ui->tableWidgetStations->setItem(idx, 1, staCode);

				QTableWidgetItem* color = new QTableWidgetItem;
				color->setBackground(c);
				_ui->tableWidgetStations->setItem(idx, 2, color);

				QTableWidgetItem* check = new QTableWidgetItem;
				check->setCheckState(Qt::Checked);
				_ui->tableWidgetStations->setItem(idx, 3, check);
			}

			try {
				emit loadingPercentage(Core::Math::percentageOfSomething<int>(_events->size(), count),
				    objectName(), "Loading origins picks...");
			} catch ( ... ) {}
		}
	}

//	_ui->tableWidgetStations->resizeColumnsToContents();
	_ui->tableWidgetStations->resizeRowsToContents();

	emit loadingPercentage(-1, objectName(), "");
	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::plottableClicked(QCPAbstractPlottable* plottable,
                                          QMouseEvent* event) {
	Q_UNUSED(event);

	//! Only emits the signal if object is a sc3 entity like origin or event
	//! @note the user must have specified the selectability of the object
	//! in a proper way first
	if ( plottable->selectable() )
	    emit elementClicked(plottable->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::showHideLegend() {
	_plot->legend->setVisible(_ui->checkBoxShowLegend->isChecked());
	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::showFormulaBox() {
	_ui->frameItemSize->hide();
	_ui->labelMagnitudeFormula->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventPhaseDuration::showCustomSizeBox() {
	_ui->frameItemSize->show();
	_ui->labelMagnitudeFormula->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
