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


#include <ipgp/gui/datamodel/shakemapscity/shakemapscitywidget.h>
#include <ipgp/gui/datamodel/shakemapscity/ui_shakemapscitywidget.h>
#include <ipgp/gui/client/application.h>
#include <ipgp/gui/map/widgets/originwidget.h>
#include <ipgp/gui/datamodel/qcustomstandarditem.h>
#include <ipgp/gui/map/drawables/city.h>
#include <ipgp/gui/map/drawables/pin.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/math/math.h>
#include <ipgp/core/geo/geo.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/math.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/seismology/regions.h>
#include <fstream>
#include <QtGui>



using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Math::Geo;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Processing;

using namespace IPGP::Core;
using namespace IPGP::Core::Geo;
using namespace IPGP::Gui::Math;
using namespace IPGP::Gui::Misc;


// Limit value in degrees when drawing curve
static double const DRAWLIMIT = 3.;


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ShakemapsCityWidget::ShakemapsCityWidget(Origin* origin,
                                         Magnitude* magnitude,
                                         QWidget* parent, Qt::WFlags f) :
		QWidget(parent, f), _ui(new Ui::ShakemapsCityWidget), _origin(origin),
		_magnitude(magnitude) {

	_ui->setupUi(this);
	_ui->shakemaps_tableView->setAlternatingRowColors(true);
	_ui->shakemaps_tableView->setSortingEnabled(true);
	_ui->shakemaps_tableView->setFocusPolicy(Qt::NoFocus);
	_ui->shakemaps_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//	_ui->shakemaps_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_ui->shakemaps_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->shakemaps_tableView->setWordWrap(false);

	QHeaderView* horizontalHeader;
	horizontalHeader = _ui->shakemaps_tableView->horizontalHeader();
	horizontalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(horizontalHeader, SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(tableViewHeaderMenu(const QPoint&)));

	_pgaMinAction = new QAction(tr("&PGA min"), this);
	_pgaMinAction->setCheckable(true);
	_pgaMinAction->setChecked(true);
	_pgaMaxAction = new QAction(tr("&PGA max"), this);
	_pgaMaxAction->setCheckable(true);
	_pgaMaxAction->setChecked(true);
	_mskMinAction = new QAction(tr("&MSK min"), this);
	_mskMinAction->setCheckable(true);
	_mskMinAction->setChecked(true);
	_mskMaxAction = new QAction(tr("&MSK max"), this);
	_mskMaxAction->setCheckable(true);
	_mskMaxAction->setChecked(true);
	_epidistAction = new QAction(QString::fromUtf8("&Dist. épi"), this);
	_epidistAction->setCheckable(true);
	_epidistAction->setChecked(false);
	_azimuthAction = new QAction(tr("&Azimuth"), this);
	_azimuthAction->setCheckable(true);
	_azimuthAction->setChecked(true);

	connect(_pgaMinAction, SIGNAL(triggered()), this, SLOT(showHideTableViewHeaderItems()));
	connect(_pgaMaxAction, SIGNAL(triggered()), this, SLOT(showHideTableViewHeaderItems()));
	connect(_mskMinAction, SIGNAL(triggered()), this, SLOT(showHideTableViewHeaderItems()));
	connect(_mskMaxAction, SIGNAL(triggered()), this, SLOT(showHideTableViewHeaderItems()));
	connect(_epidistAction, SIGNAL(triggered()), this, SLOT(showHideTableViewHeaderItems()));
	connect(_azimuthAction, SIGNAL(triggered()), this, SLOT(showHideTableViewHeaderItems()));

	_standardItemModel = new QStandardItemModel(_ui->shakemaps_tableView);

	updateViewModel(ST_MSKPGA);
	_ui->shakemaps_tableView->setModel(_standardItemModel);

	MapDescriptor md = AppInstance->mapDescriptor();
	_map = new OriginWidget(md.names(), md.paths(), md.defaultLongitude(),
	    md.defaultLatitude(), this);
	_map->setTilePattern(md.tilePattern().c_str());
	_map->setScheme(AppInstance->scheme());
	_map->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(_map, SIGNAL(cityClicked(const QString&)), this, SLOT(indicateCityInList(const QString&)));

	QBoxLayout* lm = new QVBoxLayout(_ui->frameMap);
	_ui->frameMap->setLayout(lm);
	lm->addWidget(_map);
	lm->setMargin(0);

	_plot = new QCustomPlot(this);
	_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_plot->setBackground(Qt::transparent);
	_plot->axisRect()->setBackground(Qt::white);
	_plot->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_plot->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_plot->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_plot->yAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_plot->xAxis->grid()->setVisible(true);
	_plot->xAxis2->setVisible(true);
	_plot->xAxis2->setTicks(false);
	_plot->xAxis2->setTickLabels(false);
	_plot->yAxis->grid()->setSubGridVisible(false);
	_plot->yAxis2->setVisible(true);
	_plot->yAxis2->setTicks(false);
	_plot->yAxis2->setTickLabels(false);
	_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	_plot->yAxis->grid()->setVisible(true);
	_plot->xAxis->setLabel(QString("Distance hypocentrale [km]"));

	connect(_plot, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));

	QBoxLayout* lp = new QVBoxLayout(_ui->framePlot);
	_ui->framePlot->setLayout(lp);
	lp->addWidget(_plot);
	lp->setMargin(0);

	connect(_ui->shakemaps_gButton, SIGNAL(clicked()), this, SLOT(updateView()));
	connect(_ui->shakemaps_mgButton, SIGNAL(clicked()), this, SLOT(updateView()));
	connect(_ui->shakemaps_pgaLawBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateView()));
	connect(_ui->shakemaps_mskLawBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateView()));
	connect(_ui->shakemaps_tableView, SIGNAL(clicked(const QModelIndex&)),
	    this, SLOT(moveOnCity(const QModelIndex&)));

	_ui->shakemaps_mgButton->setChecked(true);
	_ui->shakemaps_gButton->setChecked(false);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(erraseCityIndicators()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ShakemapsCityWidget::~ShakemapsCityWidget() {
	if ( _ui )
	    delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::tableViewHeaderMenu(const QPoint& point) {

	Q_UNUSED(point);
	QMenu menu(this);
	if ( _shakeMode & ST_MSKPGA ) {
		menu.addAction(_pgaMinAction);
		menu.addAction(_pgaMaxAction);
	}
	menu.addAction(_mskMinAction);
	menu.addAction(_mskMaxAction);
	menu.addAction(_epidistAction);
	menu.addAction(_azimuthAction);
	menu.exec(QCursor::pos());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::showHideTableViewHeaderItems() {

	if ( _shakeMode & ST_MSKPGA ) {

		_pgaMinAction->isChecked() ? _ui->shakemaps_tableView->showColumn(3)
		    : _ui->shakemaps_tableView->hideColumn(3);
		_pgaMaxAction->isChecked() ? _ui->shakemaps_tableView->showColumn(5)
		    : _ui->shakemaps_tableView->hideColumn(5);
		_mskMinAction->isChecked() ? _ui->shakemaps_tableView->showColumn(6)
		    : _ui->shakemaps_tableView->hideColumn(6);
		_mskMaxAction->isChecked() ? _ui->shakemaps_tableView->showColumn(8)
		    : _ui->shakemaps_tableView->hideColumn(8);
		_epidistAction->isChecked() ? _ui->shakemaps_tableView->showColumn(9)
		    : _ui->shakemaps_tableView->hideColumn(9);
		_azimuthAction->isChecked() ? _ui->shakemaps_tableView->showColumn(11)
		    : _ui->shakemaps_tableView->hideColumn(11);
	}
	else {

		_mskMinAction->isChecked() ? _ui->shakemaps_tableView->showColumn(3)
		    : _ui->shakemaps_tableView->hideColumn(3);
		_mskMaxAction->isChecked() ? _ui->shakemaps_tableView->showColumn(5)
		    : _ui->shakemaps_tableView->hideColumn(5);
		_epidistAction->isChecked() ? _ui->shakemaps_tableView->showColumn(6)
		    : _ui->shakemaps_tableView->hideColumn(6);
		_azimuthAction->isChecked() ? _ui->shakemaps_tableView->showColumn(8)
		    : _ui->shakemaps_tableView->hideColumn(8);
	}

	for (int i = 0; i < _standardItemModel->columnCount(); ++i)
		_ui->shakemaps_tableView->resizeColumnToContents(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::init(const std::string& cityFile,
                               const int& islandID) {

	if ( !_origin )
	    return;

	std::ifstream file(cityFile.c_str());
	std::string line;
	QStringList shakemapsBox;
	shakemapsBox << QString(" - Toutes les régions - ");

	_islandList.clear();

	int id = 0, idx = 0;
	while ( file.good() ) {

		getline(file, line);

		if ( line.substr(0, 1) == "#" || line == "" )
		    continue;

		QString content = tr(line.c_str());
		QStringList tmp = content.split("|");

		ShakemapsCity sc;
		for (int i = 0; i < tmp.size(); ++i) {
			if ( i == 0 )
			    sc.code = tmp.at(i).toInt();
			if ( i == 1 )
			    sc.longitude = tmp.at(i).toDouble();
			if ( i == 2 )
			    sc.latitude = tmp.at(i).toDouble();
			if ( i == 3 )
			    sc.cityName = tmp.at(i).toStdString();
			if ( i == 4 )
			    sc.islandName = tmp.at(i).toStdString();
			if ( i == 5 )
			    sc.factor = tmp.at(i).toInt();
		}
		_cityList.push_back(sc);

		if ( sc.code == islandID )
		    idx = id;

		if ( !_islandList.contains(sc.islandName.c_str()) ) {
			_islandList.push_back(sc.islandName.c_str());
			shakemapsBox << sc.islandName.c_str();
			id++;
		}
	}

	_townRadioButtons = new QCheckBox*[_islandList.size()];
	QGridLayout* townGrid;
	townGrid = new QGridLayout(_ui->shakemaps_cities);
	townGrid->setContentsMargins(0, 0, 0, 0);

	QFont radioFont;
	radioFont.setPointSize(9);

	for (int i = 0; i < _islandList.size(); ++i) {

		_townRadioButtons[i] = new QCheckBox(_ui->shakemaps_tableView);
		_townRadioButtons[i]->setText(_islandList.at(i));
		_townRadioButtons[i]->setFont(radioFont);

		townGrid->addWidget(_townRadioButtons[i], i, 0, 1, 1);

		int j = i + 1;
		if ( j == idx )
		    _townRadioButtons[i]->setChecked(true);

		connect(_townRadioButtons[i], SIGNAL(clicked()), this, SLOT(updateView()));
	}



	// Locating the nearest city from epicenter
	for (int x = 0; x < _cityList.size(); ++x) {

		double dist, az, baz;
		delazi(_origin->latitude().value(), _origin->longitude().value(),
		    _cityList.at(x).latitude, _cityList.at(x).longitude, &dist, &az, &baz);
		double epidist = deg2km(dist);

		if ( x == 0 )
			_nearestCityDistKM = epidist, _nearestCityDistDEG = dist;
		else {
			if ( epidist < _nearestCityDistKM )
			    if ( _cityList.at(x).code == islandID ) {
				    _nearestCityName = _cityList.at(x).cityName.c_str();
				    _nearestCityDistKM = epidist;
				    _nearestCityDistDEG = dist;
			    }
		}
	}

	QStringList smstr;
	for (int i = 0; i < _shakeProcessors.size(); ++i) {
		if ( _shakeProcessors.at(i) )
		    smstr << tr(_shakeProcessors.at(i)->getType().c_str());
	}
	_ui->shakemaps_mskLawBox->clear();
	_ui->shakemaps_pgaLawBox->insertItems(0, smstr);


	QStringList p2mstr;
	for (int i = 0; i < _p2mProcessors.size(); ++i) {
		if ( _p2mProcessors.at(i) )
		    p2mstr << tr(_p2mProcessors.at(i)->getType().c_str());
	}
	_ui->shakemaps_mskLawBox->clear();
	_ui->shakemaps_mskLawBox->insertItems(0, p2mstr);

	Regions reg;
	std::string region = reg.getRegionName(_origin->latitude().value(), _origin->longitude().value());
	_ui->shakemaps_mapLabel->setText(region.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::setLayoutMargin(int left, int top, int right,
                                          int bottom) {
	_ui->mainLayout->setContentsMargins(left, top, right, bottom);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::setShakemapsProcessors(const QList<
        ShakemapProcessorPtr>& processors) {
	_shakeProcessors = processors;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::setPGA2MSKProcessors(const QList<
        PGA2MSKProcessorPtr>& processors) {
	_p2mProcessors = processors;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::updateView() {

	if ( !_magnitude ) {
		_ui->shakemaps_plotLabel->setText(tr(""));
		_ui->shakemaps_pgaLabel->setText(tr(""));
		_ui->shakemaps_mskLabel->setText(tr(""));
		return;
	}

	ShakemapProcessorPtr ShakeProcessor = NULL;
	for (int i = 0; i < _shakeProcessors.size(); ++i) {
		if ( !_shakeProcessors.at(i) ) continue;
		if ( _shakeProcessors.at(i)->getType() == _ui->shakemaps_pgaLawBox->currentText().toStdString() )
		    ShakeProcessor = _shakeProcessors.at(i);
	}

	if ( !ShakeProcessor )
	    return;

	PGA2MSKProcessorPtr P2MProcessor = NULL;
	for (int i = 0; i < _p2mProcessors.size(); ++i) {
		if ( !_p2mProcessors.at(i) ) continue;
		if ( _p2mProcessors.at(i)->getType() == _ui->shakemaps_mskLawBox->currentText().toStdString() )
		    P2MProcessor = _p2mProcessors.at(i);
	}

	if ( !P2MProcessor )
	    return;

	std::vector<ShakemapsCity> _shakemapsCustomCity;
	for (int z = 0; z < _islandList.size(); ++z) {
		if ( _townRadioButtons[z]->isChecked() )
		    for (int j = 0; j < _cityList.size(); ++j)
			    if ( _cityList.at(j).islandName == _islandList.at(z).toStdString().c_str() )
			        _shakemapsCustomCity.push_back(_cityList.at(j));
	}

	_map->setOrigin(_origin.get());
	_map->setArrivalsVisible(false);
	_map->setStationsVisible(false);

	if ( _plot->graphCount() > 0 )
	    _plot->clearGraphs(), _plot->clearItems(), _plot->clearPlottables();

	_shakemapsPlotList.clear();

	QVector<double> pgaDot;
	QVector<double> mskDot;
	QVector<double> epiDistDeg;
	QVector<double> hypoDistDot;


	if ( ShakeProcessor->isConvertible() ) {

		// Shakemaps processor returns value in mg/g
		// and allows conversion into MSK/intensity scale

		// Showing off elements we need
		_ui->label_24->show();
		_ui->label_25->show();
		_ui->label_28->show();
		_ui->shakemaps_mskLawBox->show();
		_ui->shakemaps_pgaLabel->show();
		_ui->shakemaps_mskLabel->show();
		_ui->widget_3->show();

		// Updating the model with legit columns headers
		updateViewModel(ST_MSKPGA);

		QString unit;
		if ( _ui->shakemaps_gButton->isChecked() )
			unit = "g";
		else
			unit = "mg";

		double medianPGA = .0, medianMSK = .0;
		for (size_t x = 0; x < _shakemapsCustomCity.size(); ++x) {

			double dist, az, baz;
			delazi(_origin->latitude().value(),
			    _origin->longitude().value(), _shakemapsCustomCity.at(x).latitude,
			    _shakemapsCustomCity.at(x).longitude, &dist, &az, &baz);

			double epidist = deg2km(dist);
			double hypodist = sqrt((pow(epidist, 2) + pow(_origin->depth().value(), 2)));

			double pga, pgamin, pgamax, msk, mskmin, mskmax;
			ShakemapProcessor::Status stat
			= ShakeProcessor->computeShakemap(_magnitude->magnitude().value(),
			    dist, _origin->depth().value(), &pga, _shakemapsCustomCity[x].factor,
			    &pgamin, &pgamax);

			// Since laws can't always be returning values in mg or cm/sec2
			// we need this kind of hack
			if ( ShakeProcessor->getUnit() == "mg" )
			    pga /= 1000, pgamin /= 1000, pgamax /= 1000;

			if ( stat == ShakemapProcessor::OK ) {

				// PGA2MSK only works with PGA in mg or cm/sec2
				double pgaV = pga * 1000;
				double pgaminV = pgamin * 1000;
				double pgamaxV = pgamax * 1000;

				P2MProcessor->computePGA2MSK(pgaV, &msk);
				P2MProcessor->computePGA2MSK(pgaminV, &mskmin);
				P2MProcessor->computePGA2MSK(pgamaxV, &mskmax);
			}
			else
				continue;

			pgaPlot current;
			current.epiDistKm = epidist;
			current.hypoDistKm = hypodist;
			current.siteName = _shakemapsCustomCity.at(x).islandName + "/"
			        + _shakemapsCustomCity.at(x).cityName;
			current.uncertaintyMax = pgamax;
			current.uncertaintyMin = pgamin;
			current.value = pga;
			current.mskValue = msk;
			try {
				current.magUncertainty = _magnitude->magnitude().uncertainty();
			} catch ( ... ) {
				current.magUncertainty = .0;
			}
			_shakemapsPlotList.push_back(current);

			hypoDistDot.push_back(hypodist);
			pgaDot.push_back(pga);
			epiDistDeg.push_back(dist);


			if ( !_ui->shakemaps_gButton->isChecked() ) {
				pga *= 1000;
				pgamin *= 1000;
				pgamax *= 1000;
			}

			medianPGA += pga;
			medianMSK += msk;

			// Hydrating items with data
			QStandardItem* region = new QStandardItem(QString::fromUtf8(_shakemapsCustomCity.at(x).islandName.c_str()));
			QStandardItem* town = new QStandardItem(QString::fromUtf8(_shakemapsCustomCity.at(x).cityName.c_str()));
			QStandardItem* siteEffect = new QStandardItem(QString::number(_shakemapsCustomCity.at(x).factor));

			QCustomStandardItem* epi;

			if ( AppInstance->scheme().distanceInKM() )
				epi = new QCustomStandardItem(
				    QString("%1 km").arg(QString::number(epidist, 'f',
				    AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);
			else
				epi = new QCustomStandardItem(
				    QString::fromUtf8("%1°").arg(QString::number(km2deg(epidist),
				        'f', AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);

			QCustomStandardItem* hyp;
			if ( AppInstance->scheme().distanceInKM() )
				hyp = new QCustomStandardItem(
				    QString("%1 km").arg(QString::number(hypodist, 'f',
				    AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);
			else
				hyp = new QCustomStandardItem(
				    QString::fromUtf8("%1°").arg(QString::number(km2deg(hypodist),
				        'f', AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);

			QCustomStandardItem* azimuth = new QCustomStandardItem(
			    QString::fromUtf8("%1°").arg(QString::number(baz, 'f',
			    AppInstance->scheme().azimuthPrecision())), QCustomStandardItem::NumberUnitRole);

			QStandardItem* pgav = new QStandardItem(QString("%1 %2")
			        .arg(QString::number(pga, 'f', AppInstance->scheme().pgaPrecision())).arg(unit));
			QStandardItem *pgaminv = new QStandardItem(QString("%1 %2")
			        .arg(QString::number(pgamin, 'f', AppInstance->scheme().pgaPrecision())).arg(unit));
			QStandardItem* pgamaxv = new QStandardItem(QString("%1 %2")
			        .arg(QString::number(pgamax, 'f', AppInstance->scheme().pgaPrecision())).arg(unit));
			QStandardItem* mskv = new QStandardItem(tr(msk2str(msk).c_str()));
			QStandardItem* mskminv = new QStandardItem(tr(msk2str(mskmin).c_str()));
			QStandardItem* mskmaxv = new QStandardItem(tr(msk2str(mskmax).c_str()));

			region->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			town->setTextAlignment(Qt::AlignRight);
			siteEffect->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			epi->setTextAlignment(Qt::AlignRight);
			hyp->setTextAlignment(Qt::AlignRight);
			azimuth->setTextAlignment(Qt::AlignRight);
			pgav->setTextAlignment(Qt::AlignRight);
			pgaminv->setTextAlignment(Qt::AlignRight);
			pgamaxv->setTextAlignment(Qt::AlignRight);
			mskv->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			mskminv->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			mskmaxv->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);


			QColor color = getMSKColor(msk);
			region->setBackground(color);
			town->setBackground(color);
			siteEffect->setBackground(color);
			mskv->setBackground(color);
			mskminv->setBackground(color);
			mskmaxv->setBackground(color);
			pgav->setBackground(color);
			pgaminv->setBackground(color);
			pgamaxv->setBackground(color);
			epi->setBackground(color);
			hyp->setBackground(color);
			azimuth->setBackground(color);

			QList<QStandardItem*> row;
			row << region;
			row << town;
			row << siteEffect;
			row << pgaminv;
			row << pgav;
			row << pgamaxv;
			row << mskminv;
			row << mskv;
			row << mskmaxv;
			row << epi;
			row << hyp;
			row << azimuth;
			_standardItemModel->appendRow(row);

			Map::City* city = new Map::City;
			city->setName(QString("%1/%2")
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).islandName.c_str()))
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).cityName.c_str())));
			city->setGeoPosition(QPointF(_shakemapsCustomCity.at(x).longitude,
			    _shakemapsCustomCity.at(x).latitude));
			city->setBrush(color);
			city->setSize(QSizeF(3.1, 3.1));
			city->setShape(Map::City::is_ellipse);
			city->setToolTip(QString("<h3>%1&nbsp;(%2)</h3><p><b>Effect</b>:&nbsp;%3")
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).cityName.c_str()))
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).islandName.c_str()))
			        .arg(QString::number(_shakemapsCustomCity.at(x).factor)));

			if ( !_map->addCity(city) ) {
				delete city;
				city = NULL;
			}
		}

		if ( _shakemapsCustomCity.size() != 0 ) {
			medianMSK /= _shakemapsCustomCity.size();
			medianPGA /= _shakemapsCustomCity.size();
			_ui->shakemaps_pgaLabel->setText(QString("%1 %2")
			        .arg(QString::number(medianPGA, 'f', AppInstance->scheme().pgaPrecision())).arg(unit));
			_ui->shakemaps_mskLabel->setText(QString(msk2str(medianMSK).c_str()));
		}
		else {
			_ui->shakemaps_pgaLabel->setText("not calculated");
			_ui->shakemaps_mskLabel->setText("not calculated");
		}


		// Theory data
		QVector<double> curveDist, curvePGA, curvePGAMIN, curvePGAMAX;
		for (double x = .0; x < getMax(epiDistDeg) + DRAWLIMIT; x += .005) {

			double pga, pgamin, pgamax;
			ShakemapProcessor::Status stat
			= ShakeProcessor->computeShakemap(_magnitude->magnitude().value(),
			    x, _origin->depth().value(), &pga, 1, &pgamin, &pgamax);

			if ( ShakeProcessor->getUnit() == "mg" )
			    pga /= 1000, pgamin /= 1000, pgamax /= 1000;

			if ( stat == ShakemapProcessor::OK ) {

				double epi = deg2km(x);
				double hypodist = sqrt((pow(epi, 2) + pow(_origin->depth().value(), 2)));

				curveDist.push_back(hypodist);
				curvePGA.push_back(pga);
				curvePGAMIN.push_back(pgamin);
				curvePGAMAX.push_back(pgamax);
			}
		}


		// Confidence band
		QPen errorPen;
		errorPen.setStyle(Qt::SolidLine);
		errorPen.setWidth(2);
		errorPen.setColor(QColor(180, 180, 180));

		_plot->addGraph();
		_plot->graph(0)->setSelectable(false);
		_plot->graph(0)->setName(tr("Incertitude [+]"));
		_plot->graph(0)->setPen(errorPen);
		_plot->graph(0)->setBrush(QColor(255, 50, 30, 20));
		_plot->graph(0)->setData(curveDist, curvePGAMAX);

		_plot->addGraph();
		_plot->graph(1)->setSelectable(false);
		_plot->graph(1)->setPen(errorPen);
		_plot->graph(1)->setName(tr("Incertitude [-]"));
		_plot->graph(1)->setData(curveDist, curvePGAMIN);
		_plot->graph(0)->setChannelFillGraph(_plot->graph(1));


		// Theory curve
		QPen theoryPen;
		theoryPen.setStyle(Qt::SolidLine);
		theoryPen.setWidth(1);
		theoryPen.setColor(Qt::red);
		_plot->addGraph();
		_plot->graph(2)->setSelectable(false);
		_plot->graph(2)->setPen(theoryPen);
		_plot->graph(2)->setName(QString::fromUtf8("Courbe théorique"));
		_plot->graph(2)->setData(curveDist, curvePGA);


		for (int i = 0; i < _shakemapsPlotList.size(); ++i) {

			QColor color = getMSKColor(_shakemapsPlotList.at(i).mskValue);

			QPixmap pixm(7, 7);
			pixm.fill(Qt::transparent);

			QPainter painter(&pixm);
			painter.setBrush(color);
			painter.drawEllipse(0, 0, 6, 6);

			QCPGraph* g = _plot->addGraph();
			g->setSelectable(true);
			g->setScatterStyle(QCPScatterStyle(pixm));
			g->setName(QString::fromUtf8(_shakemapsPlotList.at(i).siteName.c_str()));
			g->addData(_shakemapsPlotList.at(i).hypoDistKm, _shakemapsPlotList.at(i).value);
		}

		_plot->yAxis->setRange(.0, getMax(pgaDot) + .001);
		_plot->yAxis->setLabel(tr("PGA (g)"));

	}
	else {

		// Shakemaps processor returns value in MSK/intensity scale
		// and no conversion into mg/g is possible

		// Hiding elements we don't need
		_ui->label_24->hide();
		_ui->label_25->hide();
		_ui->label_15->hide();
		_ui->shakemaps_pgaLabel->hide();
		_ui->shakemaps_mskLawBox->hide();
		_ui->widget_3->hide();

		// Updating the model with legit columns headers
		updateViewModel(ST_MSKONLY);

		double medianMSK = .0;
		for (size_t x = 0; x < _shakemapsCustomCity.size(); ++x) {

			double dist, az, baz;
			delazi(_origin->latitude().value(),
			    _origin->longitude().value(), _shakemapsCustomCity.at(x).latitude,
			    _shakemapsCustomCity.at(x).longitude, &dist, &az, &baz);

			double epidist = deg2km(dist);
			double hypodist = sqrt((pow(epidist, 2) + pow(_origin->depth().value(), 2)));

			double msk, mskmin, mskmax;
			ShakemapProcessor::Status stat = ShakeProcessor->computeShakemap(
			    _magnitude->magnitude().value(), dist, _origin->depth().value(),
			    &msk, _shakemapsCustomCity.at(x).factor, &mskmin, &mskmax);

			pgaPlot current;
			current.epiDistKm = epidist;
			current.hypoDistKm = hypodist;
			current.siteName = _shakemapsCustomCity.at(x).islandName + "/" + _shakemapsCustomCity.at(x).cityName;
			current.uncertaintyMax = mskmax;
			current.uncertaintyMin = mskmin;
			current.value = msk;
			try {
				current.magUncertainty = _magnitude->magnitude().uncertainty();
			} catch ( ... ) {
				current.magUncertainty = .0;
			}
			_shakemapsPlotList.push_back(current);

			mskDot.push_back(msk);
			hypoDistDot.push_back(hypodist);
			epiDistDeg.push_back(dist);

			medianMSK += msk;

			// Hydrating items with data
			QStandardItem* region = new QStandardItem(QString::fromUtf8(_shakemapsCustomCity.at(x).islandName.c_str()));
			QStandardItem* town = new QStandardItem(QString::fromUtf8(_shakemapsCustomCity.at(x).cityName.c_str()));
			QStandardItem* siteEffect = new QStandardItem(QString::number(_shakemapsCustomCity.at(x).factor));

			QCustomStandardItem* epi;
			if ( AppInstance->scheme().distanceInKM() )
				epi = new QCustomStandardItem(
				    QString("%1 km").arg(QString::number(epidist, 'f',
				    AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);
			else
				epi = new QCustomStandardItem(
				    QString::fromUtf8("%1°").arg(QString::number(
				        km2deg(epidist), 'f', AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);

			QCustomStandardItem* hyp;
			if ( AppInstance->scheme().distanceInKM() )
				hyp = new QCustomStandardItem(
				    QString("%1 km").arg(QString::number(hypodist, 'f',
				    AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);
			else
				hyp = new QCustomStandardItem(
				    QString::fromUtf8("%1°").arg(QString::number(km2deg(hypodist), 'f',
				    AppInstance->scheme().distancePrecision())),
				    QCustomStandardItem::NumberUnitRole);

			QCustomStandardItem* azimuth = new QCustomStandardItem(
			    QString::fromUtf8("%1°").arg(QString::number(baz, 'f',
			    AppInstance->scheme().azimuthPrecision())), QCustomStandardItem::NumberUnitRole);

			QStandardItem* mskminv = new QStandardItem(tr(msk2str(mskmin).c_str()));
			QStandardItem* mskmaxv = new QStandardItem(tr(msk2str(mskmax).c_str()));
			QStandardItem* mskv = new QStandardItem(tr(msk2str(msk).c_str()));

			region->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			town->setTextAlignment(Qt::AlignRight);
			epi->setTextAlignment(Qt::AlignRight);
			hyp->setTextAlignment(Qt::AlignRight);
			azimuth->setTextAlignment(Qt::AlignRight);
			siteEffect->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			mskminv->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			mskmaxv->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			mskv->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

			QColor color = getMSKColor(msk);
			town->setBackground(color);
			region->setBackground(color);
			mskv->setBackground(color);
			mskminv->setBackground(color);
			mskmaxv->setBackground(color);
			siteEffect->setBackground(color);
			epi->setBackground(color);
			hyp->setBackground(color);
			azimuth->setBackground(color);

			QList<QStandardItem*> row;
			row << region;
			row << town;
			row << siteEffect;
			row << mskminv;
			row << mskv;
			row << mskmaxv;
			row << epi;
			row << hyp;
			row << azimuth;
			_standardItemModel->appendRow(row);

			Map::City* city = new Map::City;
			city->setName(QString("%1/%2")
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).islandName.c_str()))
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).cityName.c_str())));
			city->setGeoPosition(QPointF(_shakemapsCustomCity.at(x).longitude, _shakemapsCustomCity.at(x).latitude));
			city->setBrush(color);
			city->setSize(QSizeF(3.1, 3.1));
			city->setShape(Map::City::is_ellipse);
			city->setToolTip(QString("<h3>%1&nbsp;(%2)</h3><p><b>Effect</b>:&nbsp;%3")
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).cityName.c_str()))
			        .arg(QString::fromUtf8(_shakemapsCustomCity.at(x).islandName.c_str()))
			        .arg(QString::number(_shakemapsCustomCity.at(x).factor)));

			if ( !_map->addCity(city) ) {
				delete city;
				city = NULL;
			}
		}

		if ( _shakemapsCustomCity.size() != 0 ) {
			medianMSK /= _shakemapsCustomCity.size();
			_ui->shakemaps_mskLabel->setText(msk2str(medianMSK).c_str());
		}
		else
			_ui->shakemaps_mskLabel->setText("");


		// Theory data
		QVector<double> curveDist, curvePGA, curvePGAMIN, curvePGAMAX;

		for (double d = 0; d < getMax(epiDistDeg) + DRAWLIMIT; d += .005) {

			double pga, pgamin, pgamax;
			ShakemapProcessor::Status
			stat = ShakeProcessor->computeShakemap(_magnitude->magnitude().value(), d,
			    _origin->depth().value(), &pga, 1, &pgamin, &pgamax);

			if ( stat == ShakemapProcessor::OK ) {

				double epi = deg2km(d);
				double hypodist = sqrt((pow(epi, 2) + pow(_origin->depth().value(), 2)));

				curveDist.push_back(hypodist);
				curvePGA.push_back(pga);
				curvePGAMIN.push_back(pgamin);
				curvePGAMAX.push_back(pgamax);
			}
		}


		// Confidence band
		QPen errorPen;
		errorPen.setStyle(Qt::SolidLine);
		errorPen.setWidth(1);
		errorPen.setColor(QColor(180, 180, 180));
		_plot->addGraph();
		_plot->graph(0)->setSelectable(false);
		_plot->graph(0)->setName(tr("Incertitude [+]"));
		_plot->graph(0)->setPen(errorPen);
		_plot->graph(0)->setBrush(QBrush(QColor(255, 50, 30, 20)));
		_plot->graph(0)->setData(curveDist, curvePGAMAX);

		_plot->addGraph();
		_plot->graph(1)->setSelectable(false);
		_plot->graph(1)->setPen(errorPen);
		_plot->graph(1)->setName(tr("Incertitude [-]"));
		_plot->graph(1)->setData(curveDist, curvePGAMIN);
		_plot->graph(0)->setChannelFillGraph(_plot->graph(1));


		// Theory curve
		QPen theoryPen;
		theoryPen.setStyle(Qt::SolidLine);
		theoryPen.setWidth(2);
		theoryPen.setColor(Qt::red);
		_plot->addGraph();
		_plot->graph(2)->setSelectable(false);
		_plot->graph(2)->setPen(theoryPen);
		_plot->graph(2)->setName(QString::fromUtf8("Courbe théorique"));
		_plot->graph(2)->setData(curveDist, curvePGA);


		for (int i = 0; i < _shakemapsPlotList.size(); ++i) {

			QColor color = getMSKColor(_shakemapsPlotList.at(i).value);
			QPixmap pixm(7, 7);
			pixm.fill(Qt::transparent);

			QPainter painter(&pixm);
			painter.setBrush(color);
			painter.drawEllipse(0, 0, 6, 6);

			QCPGraph* g = _plot->addGraph();
			g->setSelectable(true);
			g->setScatterStyle(QCPScatterStyle(pixm));
			g->setName(QString::fromUtf8(_shakemapsPlotList.at(i).siteName.c_str()));
			g->addData(_shakemapsPlotList.at(i).hypoDistKm, _shakemapsPlotList.at(i).value);
		}

		_plot->yAxis->setRange(.0, getMax(mskDot) + .5);
		_plot->yAxis->setLabel(QString::fromUtf8("Intensité"));
	}

	_ui->shakemaps_tableView->setModel(_standardItemModel);

	if ( !_epidistAction->isChecked() ) {
		if ( _shakeMode & ST_MSKONLY )
			_ui->shakemaps_tableView->hideColumn(6);
		else
			_ui->shakemaps_tableView->hideColumn(9);
	}

	_ui->shakemaps_tableView->resizeColumnsToContents();
	_ui->shakemaps_tableView->resizeRowsToContents();
	_ui->shakemaps_tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	_ui->shakemaps_plotLabel->setText(ShakeProcessor->getType().c_str());
	_plot->xAxis->setRange(getMin(hypoDistDot) - 10., getMax(hypoDistDot) + 10.);
//	_plot->rescaleAxes();
	_plot->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::updateViewModel(const ShakemapsType& type) {

	_standardItemModel->clear();
	_shakeMode = type;

	switch ( type ) {
		case ST_MSKONLY:
			_standardItemModel->setHorizontalHeaderItem(0, new QStandardItem(QString::fromUtf8("Région")));
			_standardItemModel->setHorizontalHeaderItem(1, new QStandardItem(tr("Ville")));
			_standardItemModel->setHorizontalHeaderItem(2, new QStandardItem(tr("Effet")));
			_standardItemModel->setHorizontalHeaderItem(3, new QStandardItem(tr("MSK min")));
			_standardItemModel->setHorizontalHeaderItem(4, new QStandardItem(tr("MSK")));
			_standardItemModel->setHorizontalHeaderItem(5, new QStandardItem(tr("MSK max")));
			_standardItemModel->setHorizontalHeaderItem(6, new QStandardItem(QString::fromUtf8("Dist. épi.")));
			_standardItemModel->setHorizontalHeaderItem(7, new QStandardItem(tr("Dist. hyp.")));
			_standardItemModel->setHorizontalHeaderItem(8, new QStandardItem(tr("Azimuth")));
		break;
		case ST_MSKPGA:
			_standardItemModel->setHorizontalHeaderItem(0, new QStandardItem(QString::fromUtf8("Région")));
			_standardItemModel->setHorizontalHeaderItem(1, new QStandardItem(tr("Ville")));
			_standardItemModel->setHorizontalHeaderItem(2, new QStandardItem(tr("Effet")));
			_standardItemModel->setHorizontalHeaderItem(3, new QStandardItem(tr("PGA min")));
			_standardItemModel->setHorizontalHeaderItem(4, new QStandardItem(tr("PGA")));
			_standardItemModel->setHorizontalHeaderItem(5, new QStandardItem(tr("PGA max")));
			_standardItemModel->setHorizontalHeaderItem(6, new QStandardItem(tr("MSK min")));
			_standardItemModel->setHorizontalHeaderItem(7, new QStandardItem(tr("MSK")));
			_standardItemModel->setHorizontalHeaderItem(8, new QStandardItem(tr("MSK max")));
			_standardItemModel->setHorizontalHeaderItem(9, new QStandardItem(QString::fromUtf8("Dist. épi.")));
			_standardItemModel->setHorizontalHeaderItem(10, new QStandardItem(tr("Dist. hyp.")));
			_standardItemModel->setHorizontalHeaderItem(11, new QStandardItem(tr("Azimuth")));
		break;
		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::plottableClicked(QCPAbstractPlottable* plottable,
                                           QMouseEvent* event) {
	Q_UNUSED(event);
	indicateCityInList(plottable->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::moveOnCity(const QModelIndex& index) {

	QString island = _standardItemModel->data(_standardItemModel->index(index.row(), 0)).toString();
	QString city = _standardItemModel->data(_standardItemModel->index(index.row(), 1)).toString();

	for (int j = 0; j < _cityList.size(); ++j)
		if ( _cityList.at(j).cityName == city.toUtf8().constData()
		        && _cityList.at(j).islandName == island.toUtf8().constData() ) {

			_map->centerOn(_cityList.at(j).latitude, _cityList.at(j).longitude);

			Map::Pin* pin = new Map::Pin;
			pin->setGeoPosition(QPointF(_cityList.at(j).longitude, _cityList.at(j).latitude));
			pin->setPix(QPixmap(":images/redpin.png"), QSize(30, 24));
			pin->setLegendEnabled(true);
			pin->setBrush(Qt::black);
			pin->pen().setColor(Qt::white);
			pin->setOpacity(1.);
			pin->setToolTip(QString("<p style=\"color:white;\"><b>Region</b>:&nbsp;%1<br/>"
				"<b>City</b>:&nbsp;%2<br/><b>Effect</b>:&nbsp;%3</p>")
			        .arg(QString::fromUtf8(_cityList.at(j).islandName.c_str()))
			        .arg(QString::fromUtf8(_cityList.at(j).cityName.c_str()))
			        .arg(QString::number(_cityList.at(j).factor)));

			if ( !_map->foregroundCanvas().addGeometry(pin) ) {
				delete pin;
				pin = NULL;
			}

			_timer.start(2000);
		}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::indicateCityInList(const QString& name) {

	QStringList list = name.split("/");

	if ( list.size() < 1 ) return;

	for (int i = 0; i < _standardItemModel->rowCount(); ++i) {

		QString islandName = _standardItemModel->data(_standardItemModel->index(i, 0)).toString();
		QString cityName = _standardItemModel->data(_standardItemModel->index(i, 1)).toString();
		if ( islandName == list.at(0) && cityName == list.at(1) )
		    _ui->shakemaps_tableView->setCurrentIndex(_ui->shakemaps_tableView->model()->index(i, 0));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ShakemapsCityWidget::erraseCityIndicators() {
	_map->foregroundCanvas().removeGeometries(Map::Geometry::d_Pin);
	_timer.stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
