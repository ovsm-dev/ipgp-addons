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

#include <ipgp/gui/datamodel/crosssection/crosssection.h>
#include <ipgp/gui/datamodel/crosssection/ui_crosssection.h>
#include <ipgp/gui/client/application.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/math/polygon.h>
#include <seiscomp3/math/geo.h>
#include <ipgp/core/math/math.h>
#include <ipgp/gui/math/math.h>
#include <ipgp/gui/misc/misc.h>
#include <QtGui>



using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math::Geo;
using namespace IPGP::Gui::Math;
using namespace IPGP::Gui::Misc;

using namespace IPGP::Core;
using namespace IPGP::Core::Math;

namespace IPGP {
namespace Gui {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection::Profile::Profile() :
		name("Unknown"), depthMin(.0), depthMax(.0), widthKM(.0), latitudeA(.0),
		longitudeA(.0), latitudeB(.0), longitudeB(.0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection::CrossSection(DatabaseQuery* query,
                           QWidget* parent, Qt::WFlags f) :
		PlottingWidget(query, parent, f), _ui(new Ui::CrossSection) {

	_ui->setupUi(this);
	setObjectName("CrossSection");

	_ui->checkBox_enableCrossSection->setChecked(false);

	_horizontalProjection = new SquarreZoomPlot(_ui->framePlotH);
	_horizontalProjection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_horizontalProjection->setBackground(Qt::transparent);
	_horizontalProjection->axisRect()->setBackground(Qt::white);
	_horizontalProjection->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_horizontalProjection->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_horizontalProjection->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	_horizontalProjection->axisRect()->setRangeZoom(Qt::Vertical);
	_horizontalProjection->axisRect()->setRangeDrag(Qt::Vertical);
	_horizontalProjection->xAxis->setLabel(QString("Longitude"));
	_horizontalProjection->yAxis->setLabel(QString("Depth"));
	_horizontalProjection->xAxis->grid()->setVisible(true);
	_horizontalProjection->xAxis2->setVisible(true);
	_horizontalProjection->xAxis2->setTicks(false);
	_horizontalProjection->xAxis2->setTickLabels(false);
	_horizontalProjection->yAxis->grid()->setSubGridVisible(false);
	_horizontalProjection->yAxis2->setVisible(true);
	_horizontalProjection->yAxis2->setTicks(false);
	_horizontalProjection->yAxis2->setTickLabels(false);
	_horizontalProjection->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_horizontalProjection->yAxis->grid()->setZeroLinePen(QPen(Qt::red));

	QBoxLayout* l1 = new QVBoxLayout(_ui->framePlotH);
	_ui->framePlotH->setLayout(l1);
	l1->addWidget(_horizontalProjection);
	l1->setMargin(0);

	_verticalProjection = new SquarreZoomPlot(_ui->framePlotV);
	_verticalProjection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_verticalProjection->setBackground(Qt::transparent);
	_verticalProjection->axisRect()->setBackground(Qt::white);
	_verticalProjection->axisRect()->setMinimumMargins(QMargins(0, 0, 0, 0));
	_verticalProjection->axisRect()->setMargins(QMargins(0, 0, 0, 0));
	_verticalProjection->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	_verticalProjection->axisRect()->setRangeZoom(Qt::Vertical);
	_verticalProjection->axisRect()->setRangeDrag(Qt::Vertical);
	_verticalProjection->xAxis->setLabel(QString("Latitude"));
	_verticalProjection->yAxis->setLabel(QString("Depth"));
	_verticalProjection->xAxis->grid()->setVisible(true);
	_verticalProjection->xAxis2->setVisible(true);
	_verticalProjection->xAxis2->setTicks(false);
	_verticalProjection->xAxis2->setTickLabels(false);
	_verticalProjection->yAxis->grid()->setSubGridVisible(false);
	_verticalProjection->yAxis2->setVisible(true);
	_verticalProjection->yAxis2->setTicks(false);
	_verticalProjection->yAxis2->setTickLabels(false);
	_verticalProjection->xAxis->grid()->setZeroLinePen(QPen(Qt::red));
	_verticalProjection->yAxis->grid()->setZeroLinePen(QPen(Qt::red));

	QBoxLayout* l2 = new QVBoxLayout(_ui->framePlotV);
	_ui->framePlotV->setLayout(l2);
	l2->addWidget(_verticalProjection);
	l2->setMargin(0);

	QBoxLayout* l3 = new QVBoxLayout(_ui->frameToolBox);
	_ui->frameToolBox->setLayout(l3);
	l3->addWidget(_toolBox);
	l3->setMargin(0);
	_toolBox->show();

	_profiles.clear();
	loadSettings();

	handleCrossSectionState();

	connect(_ui->checkBox_enableCrossSection, SIGNAL(clicked()), this, SLOT(handleCrossSectionState()));
	connect(_horizontalProjection, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));
	connect(_verticalProjection, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)),
	    this, SLOT(plottableClicked(QCPAbstractPlottable*, QMouseEvent*)));
	connect(_ui->checkBoxReverseDepthRange, SIGNAL(clicked(bool)), this, SLOT(reverseRangeClicked(const bool&)));
	connect(_ui->comboBoxProfile, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentProfile(const int&)));

	_ui->checkBoxReverseDepthRange->setChecked(true);
	reverseRangeClicked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection::~CrossSection() {
	emit disableCrossSectionPainting();
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CrossSection::loadSettings() {

	std::vector<std::string> tmpProfiles;
	try {
		tmpProfiles = AppInstance->configGetStrings("crossSection.profiles");
	} catch ( ... ) {
		return false;
	}

	for (std::vector<std::string>::const_iterator it = tmpProfiles.begin();
	        it != tmpProfiles.end(); ++it) {

		Profile p;
		p.name = (*it).c_str();

		std::string prefix = "crossSection.profile." + (*it);

		try {
			p.widthKM = AppInstance->configGetInt(prefix + ".width");
		} catch ( ... ) {}
		try {
			p.depthMin = AppInstance->configGetInt(prefix + ".depthMin");
		} catch ( ... ) {}
		try {
			p.depthMax = AppInstance->configGetInt(prefix + ".depthMax");
		} catch ( ... ) {}
		try {
			p.latitudeA = AppInstance->configGetDouble(prefix + ".latitude.a");
		} catch ( ... ) {}
		try {
			p.longitudeA = AppInstance->configGetDouble(prefix + ".longitude.a");
		} catch ( ... ) {}
		try {
			p.latitudeB = AppInstance->configGetDouble(prefix + ".latitude.b");
		} catch ( ... ) {}
		try {
			p.longitudeB = AppInstance->configGetDouble(prefix + ".longitude.b");
		} catch ( ... ) {}

		appendProfile(p);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::appendProfile(const Profile& profile) {

	_profiles.insert(_profiles.size(), profile);
	_ui->comboBoxProfile->addItem(profile.name);

	if ( _profiles.size() == 1 )
	    updateCurrentProfile(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::removeProfile(const int& idx) {

	ProfileMap::iterator i = _profiles.find(idx);

	if ( i == _profiles.end() ) return;

	_profiles.remove(idx);
	_ui->comboBoxProfile->removeItem(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::removeProfile(const QString& name) {

	ProfileMap::iterator f = _profiles.end();
	for (ProfileMap::iterator it = _profiles.begin();
	        it != _profiles.end(); ++it) {
		if ( it.value().name == name )
		    f = it;
	}

	if ( f == _profiles.end() ) return;

	_profiles.remove(f.key());
	_ui->comboBoxProfile->removeItem(f.key());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::reverseRangeClicked(const bool& selected) {
	_horizontalProjection->yAxis->setRangeReversed(selected);
	_horizontalProjection->replot();
	_verticalProjection->yAxis->setRangeReversed(selected);
	_verticalProjection->replot();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::handleCrossSectionState() {

	if ( !_ui->checkBox_enableCrossSection->isChecked() ) {
		_ui->frameMenu->setEnabled(false);
		_ui->framePlots->setEnabled(false);
		emit disableCrossSectionPainting();
	}
	else {
		_ui->frameMenu->setEnabled(true);
		_ui->framePlots->setEnabled(true);
		replot();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::print(const ToolBox::ExportConfig& ec) {

	QString outputFile = QFileDialog::getSaveFileName(this, tr("Save cross-section graphics"),
	    QDir::currentPath(), tr("%1 (*.%1)").arg(ec.format.toString()), 0,
	    QFileDialog::DontUseNativeDialog);

	if ( outputFile.isEmpty() ) return;

	QString ext = QString(".%1").arg(ec.format.toString());
	QString out1, out2;

	if ( outputFile.right(3) != ext ) {
		out1.append(outputFile);
		out1.append(".hp");
		out1.append(ext);

		out2.append(outputFile);
		out2.append(".vp");
		out2.append(ext);
	}
	else {
		out1.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out1.append(".hp");
		out1.append(ext);

		out2.append(outputFile.toStdString().substr(0, outputFile.size() - 4).c_str());
		out2.append(".vp");
		out2.append(ext);
	}

	if ( ec.format == IPGP::Gui::PDF ) {
		_horizontalProjection->savePdf(out1, 0, ec.printSize.width(), ec.printSize.height());
		_verticalProjection->savePdf(out2, 0, ec.printSize.width(), ec.printSize.height());
	}
	else if ( ec.format == IPGP::Gui::PS ) {
		_horizontalProjection->saveRastered(out1, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
		_verticalProjection->saveRastered(out2, ec.printSize.width(), ec.printSize.height(), 1., "SVG", 100);
	}
	else if ( ec.format == IPGP::Gui::PNG ) {
		_horizontalProjection->savePng(out1, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_verticalProjection->savePng(out2, ec.printSize.width(), ec.printSize.height(), 1, -1);
	}
	else if ( ec.format == IPGP::Gui::JPG ) {
		_horizontalProjection->saveJpg(out1, ec.printSize.width(), ec.printSize.height(), 1, -1);
		_verticalProjection->saveJpg(out2, ec.printSize.width(), ec.printSize.height(), 1, -1);
	}
	else if ( ec.format == IPGP::Gui::BMP ) {
		_horizontalProjection->saveBmp(out1, ec.printSize.width(), ec.printSize.height(), 1);
		_verticalProjection->saveBmp(out2, ec.printSize.width(), ec.printSize.height(), 1);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::replot() {

	if ( _events->size() == 0 ) {
		emit plottingError("The event list is empty, nothing to plot");
		return;
	}

	emit working();

	if ( _timer ) stopBlinking();

	_horizontalProjection->clearGraphs();
	_verticalProjection->clearGraphs();

	emit enableCrossSection(QPointF(_ui->longitudeA->value(), _ui->latitudeA->value()),
	    QPointF(_ui->longitudeB->value(), _ui->latitudeB->value()),
	    _ui->depth_min->value(), _ui->depth_max->value(), _ui->trench->value());

	double trenchDeg = km2deg((double) _ui->trench->value());
	double latAMax = _ui->latitudeA->value() + trenchDeg;
	double latAMin = _ui->latitudeA->value() - trenchDeg;
	double latBMax = _ui->latitudeB->value() + trenchDeg;
	double latBMin = _ui->latitudeB->value() - trenchDeg;

	PolygonD poly;
	poly.addVertex(latAMax, _ui->longitudeA->value());
	poly.addVertex(latBMax, _ui->longitudeB->value());
	poly.addVertex(latBMin, _ui->longitudeB->value());
	poly.addVertex(latAMin, _ui->longitudeA->value());

	MagnitudeList list;
	QStringList objects;
	size_t idx = 0;
	for (OriginList::const_iterator i = _events->begin();
	        i != _events->end(); ++i, ++idx) {

		OriginPtr org = i->first;

		EventMagnitude e;
		e.publicID = org->publicID();
		e.time = org->time().value();

		if ( AppInstance->scheme().distanceInKM() ) {
			e.latitude = org->latitude();
			e.longitude = org->longitude();
		}
		else {
			e.latitude = Seiscomp::Math::Geo::km2deg(org->latitude());
			e.longitude = Seiscomp::Math::Geo::km2deg(org->longitude());
		}

		MagnitudePtr mag = Magnitude::Find(i->second->preferredMagnitudeID());
		if ( mag ) {
			try {
				e.magnitude = mag->magnitude().value();
				e.magnitudeSize = (4.9 * (mag->magnitude().value() - 1.2)) / 2.;
				e.hasMagnitude = true;
			} catch ( ... ) {
				e.hasMagnitude = false;
			}
		}

		if ( e.magnitudeSize < 2. )
		    e.magnitudeSize = 2.;

		try {
			e.depth = org->depth().value();
			e.hasDepth = true;
		} catch ( ... ) {
			e.hasDepth = false;
		}

		try {
			if ( org->evaluationMode() == AUTOMATIC )
				e.isAuto = true;
			else if ( org->evaluationMode() == MANUAL )
			    e.isAuto = false;
		} catch ( ... ) {}

		//! Ignore object without depth, it will misrepresent the cross section
		if ( !e.hasDepth ) continue;

		if ( poly.pointInPolygon(org->latitude().value(), org->longitude().value())
		        && e.depth > _ui->depth_min->value()
		        && e.depth < _ui->depth_max->value() ) {
			list << e;
			objects << org->publicID().c_str();
		}

		emit loadingPercentage(percentageOfSomething<int>(_events->size(), idx),
		    objectName(), "Sorting events");
	}

	// Inform parent of objects to be highlighted
	emit objectsInSection(objects);

	// Pen offset
	double offset = 2.;
	idx = 0;

	// Draw objects shapes
	for (MagnitudeList::const_iterator it = list.constBegin();
	        it != list.constEnd(); ++it, ++idx) {

		QCPGraph* hg = _horizontalProjection->addGraph();
		hg->setName((*it).publicID.c_str());
		hg->addData((*it).longitude, (*it).depth);
		hg->setLineStyle(QCPGraph::lsNone);
		hg->setPen(QPen());
		hg->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssCircle, (*it).magnitudeSize + offset));
		hg->setTooltip(QString("%1\n%2")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).publicID.c_str()));
		_horizontalProjection->legend->removeItem(_horizontalProjection->legend->itemCount() - 1);


		QCPGraph* vg = _verticalProjection->addGraph();
		vg->setName((*it).publicID.c_str());
		vg->addData((*it).latitude, (*it).depth);
		vg->setLineStyle(QCPGraph::lsNone);
		vg->setPen(QPen());
		vg->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssCircle, (*it).magnitudeSize + offset));
		vg->setTooltip(QString("%1\n%2")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).publicID.c_str()));
		_verticalProjection->legend->removeItem(_verticalProjection->legend->itemCount() - 1);

		emit loadingPercentage(percentageOfSomething<int>(list.size(), idx),
		    objectName(), "First painting");
	}

	idx = 0;
	// Fill up shapes with colors
	for (MagnitudeList::const_iterator it = list.constBegin();
	        it != list.constEnd(); ++it, ++idx) {

		QColor c;
		if ( (*it).hasDepth )
			c = getDepthColoration((*it).depth);
		else
			c = QColor(0, 0, 0, 150);

		QCPGraph* hg = _horizontalProjection->addGraph();
		hg->setName((*it).publicID.c_str());
		hg->addData((*it).longitude, (*it).depth);
		hg->setLineStyle(QCPGraph::lsNone);
		hg->setPen(c);
		hg->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssSquare : QCPScatterStyle::ssDisc, c, (*it).magnitudeSize));
		hg->setTooltip(QString("%1\n%2")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).publicID.c_str()));
		_horizontalProjection->legend->removeItem(_horizontalProjection->legend->itemCount() - 1);


		QCPGraph* vg = _verticalProjection->addGraph();
		vg->setName((*it).publicID.c_str());
		vg->addData((*it).latitude, (*it).depth);
		vg->setLineStyle(QCPGraph::lsNone);
		vg->setPen(c);
		vg->setScatterStyle(QCPScatterStyle(((*it).isAuto) ?
		        QCPScatterStyle::ssFilledSquare : QCPScatterStyle::ssDisc, c, (*it).magnitudeSize));
		vg->setTooltip(QString("%1\n%2")
		        .arg((*it).time.toString("%Y-%m-%d %H:%M:%S").c_str())
		        .arg((*it).publicID.c_str()));
		_verticalProjection->legend->removeItem(_verticalProjection->legend->itemCount() - 1);

		emit loadingPercentage(percentageOfSomething<int>(list.size(), idx),
		    objectName(), "Second painting");
	}

	_horizontalProjection->rescaleAxes();
	_verticalProjection->rescaleAxes();

	_horizontalProjection->replot();
	_verticalProjection->replot();

	emit loadingPercentage(-1, objectName(), "");

	emit idling();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::updateCurrentProfile(const int& idx) {

	_ui->latitudeA->setValue(_profiles.value(idx).latitudeA);
	_ui->longitudeA->setValue(_profiles.value(idx).longitudeA);
	_ui->latitudeB->setValue(_profiles.value(idx).latitudeB);
	_ui->longitudeB->setValue(_profiles.value(idx).longitudeB);
	_ui->depth_min->setValue(_profiles.value(idx).depthMin);
	_ui->depth_max->setValue(_profiles.value(idx).depthMax);
	_ui->trench->setValue(_profiles.value(idx).widthKM);

	double aziAB, aziBA, distAB;
	delazi(_ui->latitudeA->value(), _ui->longitudeA->value(),
	    _ui->latitudeB->value(), _ui->longitudeB->value(),
	    &distAB, &aziAB, &aziBA);

	_ui->distanceAB->setText(QString("%1 km").arg(deg2km(distAB)));
	_ui->azimuthAB->setText(QString::fromUtf8("%1°").arg(aziAB));
	_ui->azimuthBA->setText(QString::fromUtf8("%1°").arg(aziBA));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::plottableClicked(QCPAbstractPlottable* plottable,
                                    QMouseEvent* event) {
	Q_UNUSED(event);
	emit elementClicked(plottable->name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP
