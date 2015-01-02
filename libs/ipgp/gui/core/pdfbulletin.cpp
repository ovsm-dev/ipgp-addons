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


#include <ipgp/gui/core/pdfbulletin.h>
#include <ipgp/core/geo/geo.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <ipgp/gui/map/widgets/originwidget.h>

#include <fstream>



using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace IPGP::Core::Geo;



namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PDFBulletin::PDFBulletin(Seiscomp::DataModel::DatabaseQuery* query,
                         const QString& pdfConverter) :
		_query(query), _pdfConverter(pdfConverter) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PDFBulletin::~PDFBulletin() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PDFBulletin::publishBulletin(Seiscomp::DataModel::OriginPtr origin,
                                  Seiscomp::DataModel::MagnitudePtr magnitude,
                                  const QString& scode,
                                  const std::vector<std::string>& mapsNames,
                                  const std::vector<std::string>& mapsPaths) {

	if ( !_query ) {
		emit errorOcurred("No query interface");
		log(false, "no query interface");
		emit logMessage(0x04, __func__, "no query interface");
		return;
	}
	if ( !origin ) {
		emit errorOcurred("Origin is NULL");
		log(false, "Origin is NULL");
		emit logMessage(0x04, __func__, "Seiscomp::DataModel::Origin objet is NULL");
		return;
	}
	if ( _tmpFolder.empty() ) {
		emit errorOcurred("Tmp folder not set");
		log(false, "no tmp folder set");
		emit logMessage(0x04, __func__, "no tmp folder set");
		return;
	}
	if ( _bulletinExportPath.empty() ) {
		emit errorOcurred("Bulletin export folder not set");
		log(false, "no export folder set");
		emit logMessage(0x04, __func__, "no export folder set");
		return;
	}
	if ( _bulletinModelFile.empty() ) {
		emit errorOcurred("Bulletin model not set");
		log(false, "no HTML model set");
		emit logMessage(0x04, __func__, "no HTML template set");
		return;
	}

	if ( origin->arrivalCount() == 0 )
	    _query->loadArrivals(origin.get());

	if ( origin->stationMagnitudeCount() == 0 )
	    _query->loadStationMagnitudes(origin.get());

	QString htmlText;
	std::string line;
	std::ifstream file(_bulletinModelFile.c_str());
	while ( file.good() ) {
		getline(file, line);
		htmlText.append(line.c_str());
	}


	double latU = .0;
	try {
		latU = origin->latitude().uncertainty();
	} catch ( ... ) {}

	double depthU = .0;
	try {
		depthU = origin->depth().uncertainty();
	} catch ( ... ) {}

	double rms = .0;
	try {
		rms = origin->quality().standardError();
	} catch ( ... ) {}

	double depth = .0;
	try {
		depth = origin->depth().value();
	} catch ( ... ) {}

	double aziGap = .0;
	try {
		aziGap = origin->quality().azimuthalGap();
	} catch ( ... ) {}

	QString lat = QString::fromStdString(getStringPosition(origin->latitude().value(), Latitude))
	        + " (" + QString::fromStdString(decimalToSexagesimal(origin->latitude().value())) + ")";
	QString lon = QString::fromStdString(getStringPosition(origin->longitude().value(), Longitude))
	        + " (" + QString::fromStdString(decimalToSexagesimal(origin->longitude().value())) + ")";

	QString erh = QString("%1 km").arg(QString::number(latU, 'f', 2));
	QString erz = QString("%1 km").arg(QString::number(depthU, 'f', 2));
	QString depthStr = QString("%1 km").arg(QString::number(depth, 'f', 2));
	QString rmsStr = QString("%1 sec").arg(QString::number(rms, 'f', 2));
	QString gazi = QString("%1°").arg(QString::number(aziGap));
	QString quality = QString::fromUtf8(" -");
	try {
		quality = QString::fromStdString(origin->quality().groundTruthLevel());
	} catch ( ... ) {}

	htmlText.replace("%title%", "New seismic event");
	htmlText.replace("%eventDate%", origin->time().value().toString("%d/%m/%Y").c_str());
	htmlText.replace("%eventTime%", origin->time().value().toString("%H:%M:%S.%2f").c_str());
	htmlText.replace("%latitude%", lat);
	htmlText.replace("%longitude%", lon);
	htmlText.replace("%methodID%", origin->methodID().c_str());
	htmlText.replace("%earthModelID%", origin->earthModelID().c_str());
	htmlText.replace("%erh%", erh);
	htmlText.replace("%erz%", erz);
	htmlText.replace("%depth%", depthStr);
	htmlText.replace("%rms%", rmsStr);
	htmlText.replace("%gazi%", gazi);
	htmlText.replace("%phases%", QString::number(origin->arrivalCount()));
	if ( magnitude ) {
		QString magn = QString("%1 (%2)")
		        .arg(QString::number(magnitude->magnitude().value(), 'f', 2))
		        .arg(magnitude->type().c_str());
		htmlText.replace("%mag%", magn);
	}
	else
		htmlText.replace("%mag%", "-");
	htmlText.replace("%scode%", scode);
	htmlText.replace("%quality%", quality);
	htmlText.replace("%author%", origin->creationInfo().author().c_str());


	QString phases;
	for (size_t i = 0; i < origin->arrivalCount(); ++i) {

		ArrivalPtr arrival = origin->arrival(i);

		if ( arrival->phase().code() == "P" ) {

			PickPtr pick = getPick(arrival->pickID());

			if ( !pick ) continue;

			QString dist = QString("%1 km (%2°)")
			        .arg(QString::number(Seiscomp::Math::Geo::deg2km(arrival->distance()), 'f', 2))
			        .arg(QString::number(arrival->distance(), 'f', 2));

			QString azi = QString("%1°").arg(QString::number(arrival->azimuth()));

			QString stime;
			QString pminuss;
			QString srms;

			PickPtr spick = getPick(origin, pick->waveformID().stationCode(), "S");

			if ( spick ) {

				try {
					if ( spick->time().value().valid() ) {
						stime = spick->time().value().toString("%d-%m-%Y %H:%M:%S.%2f").c_str();
						pminuss = QString("%1 sec").arg(QString::number(
						    spick->time().value().length() - pick->time().value().length(), 'f', 2));
					}
				} catch ( ... ) {}

				ArrivalPtr sarrival = getArrival(origin, spick->publicID());
				if ( sarrival ) {
					try {
						srms = QString::number(sarrival->timeResidual(), 'f', 2);
					} catch ( ... ) {}
				}

			}


			QString magn;
			for (size_t j = 0; j < origin->stationMagnitudeCount(); ++j) {
				StationMagnitudePtr sm = origin->stationMagnitude(j);

				if ( !sm ) continue;

				if ( (sm->waveformID().networkCode() == pick->waveformID().networkCode())
				        && sm->waveformID().stationCode() == pick->waveformID().stationCode() ) {
					magn = QString::number(sm->magnitude().value(), 'f', 2);
				}
			}

			QString timeRes;
			try {
				timeRes = QString::number(arrival->timeResidual(), 'f', 2);
			}
			catch ( std::exception& e ) {
				timeRes = e.what();
			}

			QString str = QString("<tr>"
				"  <td align=\"center\">%1</td>"
				"  <td align=\"right\">%2</td>"
				"  <td align=\"right\">%3</td>"
				"  <td align=\"right\">%4</td>"
				"  <td align=\"right\">%5</td>"
				"  <td align=\"right\">%6</td>"
				"  <td align=\"right\">%7</td>"
				"  <td align=\"right\">%8</td>"
				"  <td align=\"right\">%9</td>"
				"</tr>")
			        .arg(pick->waveformID().stationCode().c_str())
			        .arg(dist)
			        .arg(azi)
			        .arg(pick->time().value().toString("%d-%m-%Y %H:%M:%S.%2f").c_str())
			        .arg(timeRes)
			        .arg(stime)
			        .arg(srms)
			        .arg(pminuss)
			        .arg(magn);
			phases.append(str);
		}
	}
	htmlText.replace("%picks%", phases);

	std::string outFile = _tmpFolder + "tmp.html";
	std::ofstream out(outFile.c_str());
	out << htmlText.toStdString() << std::endl;
	out.close();
	log(true, QString("stored tmp html file in folder %1").arg(_tmpFolder.c_str()));
	emit logMessage(0x01, __func__, QString("stored tmp html file in folder %1").arg(_tmpFolder.c_str()));

	OriginWidgetPtr pdfmap = OriginWidgetPtr(new OriginWidget(mapsNames, mapsPaths,
	    origin->longitude().value(), origin->latitude().value()));
	pdfmap->centerOn(origin->latitude().value(), origin->longitude().value());
	pdfmap->setDatabase(_query);
	pdfmap->setOrigin(origin.get());
	pdfmap->setGeometry(0, 0, 400, 540);
	pdfmap->setZoomLevel(6);
	pdfmap->takeSnapshot(QString("%1/bigmap.png").arg(_tmpFolder.c_str()));
	log(true, QString("stored bigmap.png in folder %1").arg(_tmpFolder.c_str()));
	emit logMessage(0x01, __func__, QString("stored bigmap.png in folder %1").arg(_tmpFolder.c_str()));

	pdfmap->setGeometry(0, 0, 390, 270);
	pdfmap->setZoomLevel(8);
	pdfmap->takeSnapshot(QString("%1/smallmap.png").arg(_tmpFolder.c_str()));
	log(true, QString("stored smallmap.png in folder %1").arg(_tmpFolder.c_str()));
	emit logMessage(0x01, __func__, QString("stored smallmap.png in folder %1").arg(_tmpFolder.c_str()));

	QString cmd = QString("%1 %2 %3").arg(_pdfConverter)
	        .arg(QString("%1%2").arg(_tmpFolder.c_str()).arg("tmp.html"))
	        .arg(QString("%1%2").arg(_bulletinExportPath.c_str())
	        .arg(origin->time().value().toString("%Y%m%d_%H%M%S.pdf").c_str()));

	QProcess process;
	process.start(cmd);

	log(true, QString("waiting for converter %1 to produce PDF").arg(_pdfConverter));
	emit logMessage(0x08, __func__, QString("waiting for converter %1 to produce PDF").arg(_pdfConverter));

	if ( !process.waitForStarted() ) {
		log(false, "process couldn't start");
		emit logMessage(0x04, __func__, "process couldn't start");
		return;
	}

	if ( !process.waitForFinished() ) {
		log(false, "process couldn't finish");
		emit logMessage(0x04, __func__, "process couldn't finish");
		return;
	}

	log(true, QString("successfully generated document %1")
	        .arg(origin->time().value().toString("%Y%m%d_%H%M%S.pdf").c_str()));
	emit logMessage(0x01, __func__, QString("successfully generated document %1")
	        .arg(origin->time().value().toString("%Y%m%d_%H%M%S.pdf").c_str()));

	emit bulletinFileReady(outFile.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickPtr PDFBulletin::getPick(const std::string& publicID) {

	PickPtr pick = NULL;
	PublicObjectPtr obj = _query->getObject(Pick::TypeInfo(), publicID);
	pick = Pick::Cast(obj);

	return pick;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickPtr PDFBulletin::getPick(OriginPtr origin, const std::string& station,
                             const std::string& phase) {

	for (size_t i = 0; i < origin->arrivalCount(); ++i) {
		ArrivalPtr arrival = origin->arrival(i);
		if ( arrival->phase().code() == phase ) {
			PickPtr pick = getPick(arrival->pickID());
			if ( pick->waveformID().stationCode() == station )
			    return pick;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArrivalPtr PDFBulletin::getArrival(Seiscomp::DataModel::OriginPtr origin,
                                   const std::string pickID) {

	for (size_t i = 0; i < origin->arrivalCount(); ++i) {
		ArrivalPtr arrival = origin->arrival(i);
		if ( arrival->pickID() == pickID )
		    return arrival;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PDFBulletin::log(bool status, const QString& msg) {

	if ( status )
		emit logHtml(QString("<font color=\"#006600\"><b>OK</b></font>&nbsp;"
			"<font color=\"#003366\"><b>PDF Bulletin</b></font>&nbsp;%1").arg(msg));
	else
		emit logHtml(QString("<font color=\"#660000\"><b>ERROR</b></font>&nbsp;"
			"<font color=\"#003366\"><b>PDF Bulletin</b></font>&nbsp;%1").arg(msg));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
