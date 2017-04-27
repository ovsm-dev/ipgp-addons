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

#define SEISCOMP_COMPONENT IPGPBULLETIN

#include <ipgp/core/bulletin/bulletin.h>
#include <ipgp/core/bulletin/isf.h>
#include <ipgp/core/string/string.h>
#include <ipgp/core/misc/misc.h>
#include <ipgp/core/geo/geo.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/math/geo.h>

#if SC_API_VERSION >= SC_API_VERSION_CHECK(10, 0, 0)
#include <seiscomp3/core/strings.h>
#endif

#include <boost/algorithm/string.hpp>

using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::IO;
using namespace Seiscomp::Math::Geo;

using namespace ISF;
using namespace IPGP::Core::String;
using namespace IPGP::Core::Geo;


namespace IPGP {
namespace Core {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bulletin::Bulletin(Event* event, DatabaseQuery* query) :
		_event(event), _query(query), _originCount(0), _pickCount(0),
		_magnitudeCount(0) {

	_author = "Bulletin";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bulletin::~Bulletin() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bulletin::getObjects() {

	if ( !_query || !_event ) return false;

	// Clear objects collections first
	_arrivalList.clear();
	_pickList.clear();
	_amplitudeList.clear();
	_stationMagnitudeList.clear();
	_commentList.clear();

	// Preferred magnitude
	PublicObjectPtr obj = _query->getObject(Magnitude::TypeInfo(), _event->preferredMagnitudeID());
	_magnitude = Magnitude::Cast(obj);

	// Preferred origin
	obj = _query->getObject(Origin::TypeInfo(), _event->preferredOriginID());
	_origin = Origin::Cast(obj);

	if ( _origin ) {

		if ( _origin->arrivalCount() == 0 )
		    _query->loadArrivals(_origin.get());

		if ( _origin->stationMagnitudeCount() == 0 )
		    _query->loadStationMagnitudes(_origin.get());

		if ( _origin->magnitudeCount() == 0 )
		    _query->loadMagnitudes(_origin.get());

		// Origin picks
		DatabaseIterator di = _query->getPicks(_origin->publicID());
		for (; *di; ++di) {
			PickPtr pick = Pick::Cast(*di);
			if ( pick )
			    _pickList.push_back(pick);
		}

		// Origin amplitudes
		di = _query->getAmplitudesForOrigin(_origin->publicID());
		for (; *di; ++di) {
			AmplitudePtr amplitude = Amplitude::Cast(*di);
			if ( amplitude )
			    _amplitudeList.push_back(amplitude);
		}

		// Origin station magnitude
		for (size_t j = 0; j < _origin->stationMagnitudeCount(); ++j) {
			StationMagnitude* sta = _origin->stationMagnitude(j);
			if ( sta )
			    _stationMagnitudeList.push_back(sta);
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Bulletin::setAuthor(const std::string& str) {
	_author = str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getHypo71Bulletin() {
	std::string stringStream = "Empty!";
	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getIMSBulletin() {

	std::string stringStream;

	char origID[8];
	Misc::randomID(origID, 8);

	std::string region = "";
	std::string evtType;

	int gap = 0;
	int usedPhaseCount = 0;
	int usedStationCount = 0;
	double err = .00;
	double rms = .00;
	double depth = .00;
	double depthErr = .00;
	double minDist = .00;
	double maxDist = .00;

	try {
		err = _origin->time().uncertainty();
	} catch ( ... ) {}
	try {
		rms = _origin->quality().standardError();
	} catch ( ... ) {}
	try {
		depth = _origin->depth().value();
	} catch ( ... ) {}
	try {
		depthErr = _origin->depth().uncertainty();
	} catch ( ... ) {}
	try {
		gap = (int) _origin->quality().azimuthalGap();
	} catch ( ... ) {}
	try {
		minDist = _origin->quality().minimumDistance();
	} catch ( ... ) {}
	try {
		maxDist = _origin->quality().maximumDistance();
	} catch ( ... ) {}
	try {
		usedPhaseCount = _origin->quality().usedPhaseCount();
	} catch ( ... ) {}
	try {
		usedStationCount = _origin->quality().usedStationCount();
	} catch ( ... ) {}
	try {
		evtType = getEventType(_event->type());
	} catch ( ... ) {
		evtType = "uk";
	}

	Regions reg;
	region = reg.getRegionName(_origin->latitude().value(), _origin->longitude().value());

	stringStream += "EVENT " + _event->publicID() + " " + region + "\n";
	stringStream += "   Date       Time        Err   RMS Latitude Longitude  Smaj"
			"  Smin  Az Depth   Err Ndef Nsta Gap  mdist  Mdist Qual   Author      OrigID\n";

	write_origin(&stringStream, _origin->time().value().toString("%Y/%m/%d"),
	    _origin->time().value().toString("%H:%M:%S.%2f"), " ", err, rms,
	    _origin->latitude().value(), _origin->longitude().value(),
	    " ", .0, .0, 0, depth, " ", depthErr, usedPhaseCount, usedStationCount,
	    gap, minDist, maxDist, "m", "g", evtType, _author, toString(origID), &_log);

	write_netmag_head(&stringStream);

	for (size_t j = 0; j < _origin->magnitudeCount(); ++j) {
		double magErr;
		try {
			magErr = _origin->magnitude(j)->magnitude().uncertainty();
		} catch ( ... ) {}
		write_netmag(&stringStream,
		    _origin->magnitude(j)->type(),
		    " ",
		    _origin->magnitude(j)->magnitude().value(),
		    magErr,
		    _origin->magnitude(j)->stationCount(),
		    _author,
		    toString(origID),
		    &_log);
		_magnitudeCount++;
	}

	stringStream += "Sta     Dist  EvAz Phase        Time      TRes  Azim AzRes"
			"   Slow   SRes Def   SNR       Amp   Per Qual Magnitude    ArrID\n";

	for (size_t k = 0; k < _origin->arrivalCount(); ++k) {

		PickPtr pick = Misc::getPick(_origin->arrival(k)->pickID(), _pickList);
		AmplitudePtr amp = Misc::getAmplitude(_origin->arrival(k)->pickID(), _amplitudeList);

		char arrivalID[8];
		Misc::randomID(arrivalID, 8);

		if ( !pick )
		    continue;

		_pickCount++;
		StationMagnitude* station = NULL;

		double snr = .00;
		double period = .00;
		double amplitude = .00;
		double azimuth = .00;
		double distance = .00;
		double magValue = .00;
		double horizSlow = .00;
		double horizSlowRes = .00;
		double backAzimuth = .00;
		double backAzRes = .00;
		double timeRes = .00;
		std::string magType = "";
		std::string eval = "";

		if ( amp ) {

			try {
				snr = amp->snr();
			} catch ( ... ) {}
			try {
				amplitude = amp->amplitude().value();
			} catch ( ... ) {}
			try {
				period = amp->period().value();
			} catch ( ... ) {}

			station = Misc::getStationMagnitude(amp->publicID(), _stationMagnitudeList);
			if ( station ) {
				try {
					magType = station->type();
				} catch ( ... ) {}
				try {
					magValue = station->magnitude().value();
				} catch ( ... ) {}
			}
		}

		try {
			timeRes = _origin->arrival(k)->timeResidual();
		} catch ( ... ) {}
		try {
			backAzimuth = _origin->arrival(k)->takeOffAngle();
		} catch ( ... ) {}
		try {
			backAzRes = _origin->arrival(k)->backazimuthResidual();
		} catch ( ... ) {}
		try {
			horizSlow = pick->horizontalSlowness().value();
		} catch ( ... ) {}
		try {
			horizSlowRes = pick->horizontalSlowness().confidenceLevel();
		} catch ( ... ) {}
		try {
			eval = pick->evaluationMode().toString();
			if ( eval == "automatic" )
				eval = "a";
			else
			if ( eval == "manual" )
			    eval = "m";
		} catch ( ... ) {}
		try {
			azimuth = _origin->arrival(k)->azimuth();
		} catch ( ... ) {}
		try {
			distance = _origin->arrival(k)->distance();
		} catch ( ... ) {}
		write_phase(&stringStream,
		    pick->waveformID().stationCode(), // Station code
		    distance, // Station distance
		    azimuth, // Station azimuth
		    pick->phaseHint().code(), // Phase code
		    pick->time().value().toString("%H:%M:%S.%3f"), // Time
		    timeRes, // Time residuals
		    backAzimuth, // Observed azimuth
		    backAzRes, // Observed azimuth residuals
		    horizSlow, // Horizontal slowness
		    horizSlowRes, // Horizontal slowness residuals
		    "_", // Time defining
		    "_", // Azimuth defining
		    "_", // Azimuth slowness defining
		    snr, // Signal noise ratio
		    amplitude, // Amplitude
		    period, // Period
		    eval, // Pick type
		    "_", // Short period motion
		    "_", // Detection character
		    magType, // Magnitude type
		    " ", // Magnitude indicator
		    magValue, // Magnitude value
		    toString(arrivalID), // Arrival ID
		    &_log);
	}

	stringStream.append("\n\n");

	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getHypo2000EventString(const IDMethod& idmethod,
                                             const std::string& idstr,
                                             const std::string& seismicCode) {

	std::string stringStream = "";

	// H71SUMM2K Format
	// DATE     TIME       LAT N     LONG W    DEPTH   MAG  NO GAP DMIN RMS   ERH  ERZ Q  SCode File/OriginID
	// 20091108 0340 38.30 16-20.67  61-26.82  87.20   3.48 21 283 41.4 0.15  5.3  2.8 D1 TE2GP 11080340_00
	// 20111201 0713 44.92 15-34.03  61-08.68  99.00   2.63  8 170 82.9 0.11  1.5  1.6    VM4SX 20111201_071300_a.mq0
	// 20111227 0814 25.21 15-52.96  60-55.52  28.21         8 203 49.9      19.7 16.5    TE2GM 20111227_081400_a.mq0


	std::string date = _origin->time().value().toString("%Y%m%d");
	std::string hour = _origin->time().value().toString("%H%M");
	std::string sec = _origin->time().value().toString("%S.%2f");
	std::string lat = decimalToSexagesimal(_origin->latitude().value());
	std::string lon = decimalToSexagesimal(_origin->longitude().value());


	// Linker ID of an origin to her picks
	std::string id;
	switch ( idmethod ) {
		case FILEID:
			id = date + "_" + hour + "00_a." + idstr;
		break;
		case ORIGINID:
			id = _origin->publicID();
		break;
		default:
			SEISCOMP_INFO("No IDMethod specified, will go with OriginID");
			id = _origin->publicID();
		break;
	}

	std::string depth;
	try {
		depth = stringify("%7.2f", _origin->depth().value());
	} catch ( ... ) {}

	std::string magnitude;
	if ( _magnitude )
	    try {
		    magnitude = stringify("%5.2f", _magnitude->magnitude().value());
	    } catch ( ... ) {}

	std::string dMin;
	try {
		dMin = stringify("%5.1f", Seiscomp::Math::Geo::deg2km(_origin->quality().minimumDistance()));
	} catch ( ... ) {}

	// Teleseismic fix: ensure nothing comes and perturb the 5chars rule
	if ( dMin.length() > 5 )
	    dMin = "";

	std::string rms;
	try {
		rms = stringify("%5.2f", _origin->quality().standardError());
	} catch ( ... ) {}

	// Teleseismic fix: ensure nothing comes and perturb the 5 chars rule
	if ( rms.length() > 5 )
	    rms = "";

	std::string erh;
	try {
		erh = stringify("%5.1f", _origin->latitude().uncertainty());
	} catch ( ... ) {}

	std::string erz;
	try {
		erz = stringify("%5.1f", _origin->depth().uncertainty());
	} catch ( ... ) {}

	std::string quality;
	try {
		quality = _origin->quality().groundTruthLevel();
	} catch ( ... ) {}

	if ( seismicCode.size() != 5 ) {
		SEISCOMP_ERROR("Specified seismic code is not correct");
		return "";
	}

	std::string azigap;
	try {
		azigap = stringify("%4.0f", _origin->quality().azimuthalGap());
	} catch ( ... ) {}

	// Date YYYYMMDD col 1-9
	stringStream.append(date);

	// Hour Minute col 10-14
	stringStream.append(addWhiteSpace(hour, 5, 0));

	// Seconds col 14-20 F6.2
	stringStream.append(addWhiteSpace(sec, 6, 0));

	// Latitude col 20-29
	stringStream.append(addWhiteSpace(lat, 9, 0));

	// Longitude col 30-39
	stringStream.append(addWhiteSpace(lon, 10, 0));

	// Depth col 39-46
	stringStream.append(addWhiteSpace(depth, 7, 0));

	// Blank col 46-47
	stringStream.append(addWhiteSpace("", 1, 0));

	// Magnitude code col 47-48
	if ( _magnitude ) {
		std::string magLetter = _magnitude->type().substr(1, 1);
		boost::to_upper(magLetter);
		stringStream.append(addWhiteSpace(magLetter, 1, 0));
	}
	else {
		stringStream.append(addWhiteSpace("", 1, 0));
	}

	// Magnitude col 48-53
	stringStream.append(addWhiteSpace(magnitude, 5, 0));

	// Number of P & S times with weights greater than 0.1.
	// becomes here the number of phases col 53-56
	stringStream.append(addWhiteSpace(toString(_origin->arrivalCount()), 3, 0));

	// Azimuthal gap col 56-60
	stringStream.append(addWhiteSpace(azigap, 4, 0));

	// Distance to nearest station col 60-65
	stringStream.append(addWhiteSpace(dMin, 5, 0));

	// RMS travel time residual col 65-70
	stringStream.append(addWhiteSpace(rms, 5, 0));

	// Horizontal error (km) col 70-75
	stringStream.append(addWhiteSpace(erh, 5, 0));

	// Vertical error (km) col 75-80
	stringStream.append(addWhiteSpace(erz, 5, 0));

	// Remark assigned by analyst (i.e. Q for quarry blast)
	stringStream.append(addWhiteSpace("", 1, 0));

	// Quality code A-D
	stringStream.append(addWhiteSpace(quality, 1, 0));

	// Most common data source (i.e. W= earthworm)
	stringStream.append(addWhiteSpace("", 1, 0));

	// Auxiliary remark from program (i.e. “-“ for depth fixed, etc.)
	stringStream.append(addWhiteSpace("", 1, 0));

	stringStream.append(addWhiteSpace(seismicCode, 6, 1));
	stringStream.append(id);


	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getHypo71Picks() {
	std::string stringStream = "Empty!";
	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getHypo2000Bulletin() {
	std::string stringStream = "Empty!";
	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getGSEBulletin() {

	std::string stringStream;

	char origID[8];
	Misc::randomID(origID, 8);

	std::string region = "";
	std::string evtType;

	int gap = 0;
	int usedPhaseCount = 0;
	int usedStationCount = 0;
	double err = .00;
	double rms = .00;
	double depth = .00;
	double depthErr = .00;
	double minDist = .00;
	double maxDist = .00;

	try {
		err = _origin->time().uncertainty();
	} catch ( ... ) {}
	try {
		rms = _origin->quality().standardError();
	} catch ( ... ) {}
	try {
		depth = _origin->depth().value();
	} catch ( ... ) {}
	try {
		depthErr = _origin->depth().uncertainty();
	} catch ( ... ) {}
	try {
		gap = (int) _origin->quality().azimuthalGap();
	} catch ( ... ) {}
	try {
		minDist = _origin->quality().minimumDistance();
	} catch ( ... ) {}
	try {
		maxDist = _origin->quality().maximumDistance();
	} catch ( ... ) {}
	try {
		evtType = getEventType(_event->type());
	} catch ( ... ) {
		evtType = "uk";
	}
	try {
		usedPhaseCount = _origin->quality().usedPhaseCount();
	} catch ( ... ) {}
	try {
		usedStationCount = _origin->quality().usedStationCount();
	} catch ( ... ) {}

	Regions reg;
	region = reg.getRegionName(_origin->latitude().value(),
	    _origin->longitude().value());

	// GSE2.0 formalism differs from GSE2.1 or IMS1.0 (understandable)
	stringStream += "EVENT " + _event->publicID() + " " + region + "\n";
	stringStream += "    Date      Time       Latitude Longitude    Depth    Ndef"
			" Nsta Gap    Mag1  N    Mag2  N    Mag3  N  Author          ID\n";
	stringStream += "       rms   OT_Error      Smajor Sminor Az        Err   mdist"
			"  Mdist     Err        Err        Err     Quality\n\n";

	// Origin line
	stringStream += _origin->time().value().toString("%Y/%m/%d");
	stringStream += " "; // 1 blank char
	stringStream += _origin->time().value().toString("%H:%M:%S.%1f");
	stringStream += "  "; // 2 blank chars
	stringStream += " "; // f
	stringStream += stringify("%8.4f ", _origin->latitude().value());
	stringStream += stringify("%9.4f ", _origin->longitude().value());
	stringStream += " "; // 1 blank char
	stringStream += " "; // f
	stringStream += "  "; // 2 blank chars
	stringStream += stringify("%5.1f", depth);
	stringStream += " "; // 1 blank char
	stringStream += " "; // f
	stringStream += "  "; // 2 blank chars
	stringStream += stringify("%4d", usedPhaseCount);
	stringStream += " "; // 1 blank char
	stringStream += stringify("%4d", usedStationCount);
	stringStream += " "; // 1 blank chars
	stringStream += stringify("%3d", gap);
	stringStream += "  "; // 2 blank chars

	if ( _origin->magnitudeCount() != 0 ) {
		for (size_t j = 0; j < _origin->magnitudeCount(); ++j) {
			MagnitudePtr mag = _origin->magnitude(j);
			if ( !mag ) continue;
			if ( j != 3 ) {
				if ( mag->type().size() < 3 ) {
					if ( (j == 0) || (j == 1) ) {
						stringStream += stringify("%2s%4.1f %2d  ", mag->type().c_str(),
						    mag->magnitude().value(), mag->stationCount());
					}
					else
					if ( j == 2 ) {
						stringStream += stringify("%2s%4.1f %2d ", mag->type().c_str(),
						    mag->magnitude().value(), mag->stationCount());
					}
				}
				else {
					if ( (j == 0) || (j == 1) ) {
						stringStream += "           "; // 11 blank chars
					}
					else
					if ( j == 2 ) {
						stringStream += "          "; // 10 blank chars
					}
				}
			}
			_magnitudeCount++;
		}
	}
	else {
		stringStream += "                                ";
	}

	stringStream += " "; // 1 blank char
	stringStream += stringify("%8s", _author.c_str());
	stringStream += "  "; // 2 blank chars
	stringStream += stringify("%8s", origID);
	stringStream += "\n"; // newline
	stringStream += "     "; // 5 blank chars
	stringStream += stringify("%5.2f", rms);
	stringStream += "   "; // 3 blank chars
	stringStream += "+-";
	stringStream += stringify("%6.2f", err);
	stringStream += "    "; // 4 blank chars
	stringStream += "      "; // semi-major axis
	stringStream += " "; // 1 blank char
	stringStream += "      "; // semi-minor axis
	stringStream += "  "; // 2 blank chars
	stringStream += stringify("%3d", gap);
	stringStream += "    "; // 4 blank chars
	stringStream += "+-";
	stringStream += stringify("%5.1f", depthErr);
	stringStream += "  "; // 2 blank chars
	stringStream += stringify("%6.2f", minDist);
	stringStream += " "; // 1 blank char
	stringStream += stringify("%6.2f", maxDist);
	stringStream += "   "; // 3 blank chars

	if ( _origin->magnitudeCount() != 0 ) {
		for (size_t j = 0; j < _origin->magnitudeCount(); ++j) {
			MagnitudePtr mag = _origin->magnitude(j);
			if ( !mag ) continue;
			if ( j != 3 ) {
				if ( mag->type().size() < 3 ) {
					double magErr;
					try {
						magErr = mag->magnitude().uncertainty();
					} catch ( ... ) {}
					if ( (j == 0) || (j == 1) ) {
						stringStream += stringify("+-%3.1f      ", magErr);
					}
					else
					if ( j == 2 ) {
						stringStream += stringify("+-%3.1f     ", magErr);
					}
				}
				else {
					if ( (j == 0) || (j == 1) ) {
						stringStream += "+-         ";
					}
					else
					if ( j == 2 ) {
						stringStream += "+-        ";
					}
				}
			}
		}
	}
	else {
		stringStream += "                                ";
	}

	stringStream += "m";
	stringStream += " "; // 1 blank char
	stringStream += "i";
	stringStream += " "; // 1 blank char
	stringStream += stringify("%2s", evtType.c_str());
	stringStream += "\n\n";

	// Arrival line
	stringStream += "Sta     Dist  EvAz     Phase      Date       Time     TRes"
			"  Azim  AzRes  Slow  SRes Def   SNR       Amp   Per   Mag1   Mag2       ID\n";

	for (size_t k = 0; k < _origin->arrivalCount(); ++k) {

		PickPtr pick = Misc::getPick(_origin->arrival(k)->pickID(), _pickList);
		AmplitudePtr amp = Misc::getAmplitude(_origin->arrival(k)->pickID(), _amplitudeList);

		char arrivalID[8];
		Misc::randomID(arrivalID, 8);

		if ( !pick )
		    continue;

		_pickCount++;
		StationMagnitudePtr station;

		double snr = .00;
		double period = .00;
		double amplitude = .00;
		double azimuth = .00;
		double distance = .00;
		double magValue = .00;
		double horizSlow = .00;
		double horizSlowRes = .00;
		double backAzimuth = .00;
		double backAzRes = .00;
		double timeRes = .00;
		std::string magType = "";
		std::string eval = "";

		if ( amp ) {

			try {
				snr = amp->snr();
			} catch ( ... ) {}
			try {
				amplitude = amp->amplitude().value();
			} catch ( ... ) {}
			try {
				period = amp->period().value();
			} catch ( ... ) {}

			station = Misc::getStationMagnitude(amp->publicID(), _stationMagnitudeList);
			if ( station ) {
				try {
					if ( station->type().size() < 3 )
						magType = station->type();
					else
						magType = station->type().substr(0, 2);
				} catch ( ... ) {}
				try {
					magValue = station->magnitude().value();
				} catch ( ... ) {}
			}
		}

		try {
			timeRes = _origin->arrival(k)->timeResidual();
		} catch ( ... ) {}
		try {
			backAzimuth = _origin->arrival(k)->takeOffAngle();
		} catch ( ... ) {}
		try {
			backAzRes = _origin->arrival(k)->backazimuthResidual();
		} catch ( ... ) {}
		try {
			horizSlow = pick->horizontalSlowness().value();
		} catch ( ... ) {}
		try {
			horizSlowRes = pick->horizontalSlowness().confidenceLevel();
		} catch ( ... ) {}
		try {
			eval = pick->evaluationMode().toString();
			if ( eval == "automatic" )
				eval = "a";
			else
			if ( eval == "manual" )
			    eval = "m";
		} catch ( ... ) {}
		try {
			azimuth = _origin->arrival(k)->azimuth();
		} catch ( ... ) {}
		try {
			distance = _origin->arrival(k)->distance();
		} catch ( ... ) {}

		stringStream += addWhiteSpace(pick->waveformID().stationCode(), 5, 1);
		stringStream += " "; // blank
		stringStream += stringify("%6.2f ", distance);
		stringStream += stringify("%5.1f ", azimuth);
		stringStream += stringify("%s", eval.c_str());
		stringStream += " "; // direction of short period
		stringStream += "i "; // detection character
		stringStream += addWhiteSpace(pick->phaseHint().code(), 7, 1);
		stringStream += " "; // blank
		stringStream += stringify("%s ", pick->time().value().toString("%Y/%m/%d").c_str());
		stringStream += stringify("%s ", pick->time().value().toString("%H:%M:%S.%1f").c_str());

		if ( timeRes != .00 )
			stringStream += stringify("%5.1f ", timeRes);
		else
			stringStream += "      ";

		if ( backAzimuth != .00 )
			stringStream += stringify("%5.1f ", backAzimuth);
		else
			stringStream += "      ";

		if ( backAzRes != .00 )
			stringStream += stringify("%6.1f ", backAzRes);
		else
			stringStream += "       ";

		if ( horizSlow != .00 )
			stringStream += stringify("%5.1f ", horizSlow);
		else
			stringStream += "      ";

		if ( horizSlowRes != .00 )
			stringStream += stringify("%5.1f ", horizSlowRes);
		else
			stringStream += "      ";

		stringStream += " "; // time defining flag
		stringStream += " "; // azimuth defining flag
		stringStream += " "; // slowness defining flag
		stringStream += " "; // blank

		if ( snr != .00 )
			stringStream += stringify("%5.1f ", snr);
		else
			stringStream += "      ";

		if ( amplitude != .00 )
			stringStream += stringify("%9.1f ", amplitude);
		else
			stringStream += "          ";

		if ( period != .00 ) {
			if ( floor(period) < 99 )
				stringStream += stringify("%5.2f ", period);
			else
				stringStream += stringify("%5.1f ", period);
		}
		else {
			stringStream += "      ";
		}

		stringStream += addWhiteSpace(magType, 2, 1);

		if ( magValue != .00 )
			stringStream += stringify("%4.1f ", magValue);
		else
			stringStream += "     ";

		stringStream += addWhiteSpace("", 2, 1); // Mag 2
		stringStream += addWhiteSpace("", 4, 1); // Mag 2
		stringStream += " "; // blank

		stringStream += stringify("%s", arrivalID);
		stringStream += "\n";
	}

	stringStream.append("\n\n");

	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Bulletin::getSC3MLBulletin() {

	EventParametersPtr ep = new EventParameters;

	if ( _origin ) {

		ep->add(_origin.get());

		// Adding picks
		for (size_t a = 0; a < _origin->arrivalCount(); ++a) {
			PickPtr pick = Pick::Cast(PublicObjectPtr(
			    _query->getObject(Pick::TypeInfo(), _origin->arrival(a)->pickID())));
			if ( !pick ) {
				SEISCOMP_WARNING("Pick with id '%s' not found",
				    _origin->arrival(a)->pickID().c_str());
				continue;
			}

			if ( !pick->eventParameters() )
			    ep->add(pick.get());
		}

		// Adding magnitudes
		for (size_t m = 0; m < _origin->magnitudeCount(); ++m) {
			MagnitudePtr netmag = _origin->magnitude(m);

			for (size_t s = 0; s < netmag->stationMagnitudeContributionCount();
			        ++s) {

				StationMagnitudePtr stamag = StationMagnitude::Find(
				    netmag->stationMagnitudeContribution(s)->stationMagnitudeID());
				if ( !stamag ) {
					SEISCOMP_WARNING("StationMagnitude with id '%s' not found",
					    netmag->stationMagnitudeContribution(s)->stationMagnitudeID().c_str());
					continue;
				}

				AmplitudePtr staamp = Amplitude::Cast(PublicObjectPtr(
				    _query->getObject(Amplitude::TypeInfo(), stamag->amplitudeID())));
				if ( !staamp ) {
					SEISCOMP_WARNING("Amplitude with id '%s' not found", stamag->amplitudeID().c_str());
					continue;
				}

				if ( !staamp->eventParameters() )
				    ep->add(staamp.get());
			}
		}
	}

	XMLArchive ar;
	std::stringbuf buf;
	if ( !ar.create(&buf) ) {
		return "";
	}

	ar.setFormattedOutput(true);
	ar << ep;
	ar.close();

	return buf.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string Bulletin::getBulletin(const Type& type) {

	std::string stringStream = "Wrong export format type, nothing will be exported";

	_originCount = 0;
	_pickCount = 0;
	_magnitudeCount = 0;

	switch ( type ) {
		case GSE2_0:
			stringStream = getGSEBulletin();
		break;
		case HYPO71:
			stringStream = getHypo71Bulletin();
		break;
		case HYPO2000:
			stringStream = getHypo2000Bulletin();
		break;
		case IMS1_0:
			stringStream = getIMSBulletin();
		break;
		case QUAKEML:
			stringStream = getSC3MLBulletin();
		break;
	}

	return stringStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string Bulletin::getEventType(EventType type) const {

	std::string evtType;

	if ( type == EARTHQUAKE )
		evtType = "ke";
	else
	if ( type == INDUCED_EARTHQUAKE )
		evtType = "si";
	else
	if ( type == CHEMICAL_EXPLOSION )
		evtType = "kh";
	else
	if ( type == NUCLEAR_EXPLOSION )
		evtType = "kn";
	else
	if ( type == LANDSLIDE )
		evtType = "ls";
	else
	if ( type == ROCKSLIDE )
		evtType = "kr";
	else
		evtType = "uk";

	return evtType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Core
} // namespace IPGP
