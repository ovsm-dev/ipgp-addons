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


#include <ipgp/core/bulletin/isf.h>
#include <iostream>

namespace ISF {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void print_double(std::string* stringStream, double x, int width, int max_prec) {

	int prec, spare;

	if ( x < 0 ) {
		spare = width - 2.5 - (int) log10(fabs(x));
	}
	else if ( x > 0 ) {
		spare = width - 1.5 - (int) log10(fabs(x));
	}
	else {
		spare = max_prec;
	}

	if ( spare > max_prec ) {
		prec = max_prec;
	}
	else if ( spare < 0 ) {
		prec = 0;
	}
	else {
		prec = spare;
	}
	char buffer[width];
	sprintf(buffer, "%*.*f", width, prec, x);
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int is_null(int i) {

	if ( i == ISF_NULL ) {
		return 1;
	}
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_data_type(std::string* stringStream, std::string data_type,
                     std::string subtype, std::string data_format,
                     std::string subformat) {

	int len = 10; /* line length so far ("DATA_TYPE ") */
	char buffer[ISF_LINE_LEN];

	/* Check and write data_type. */
	if ( len += data_type.size() > ISF_LINE_LEN ) {
		std::cout << "line too long : data type" << std::endl;
		return;
	}
	sprintf(buffer, "DATA_TYPE %s", data_type.c_str());
	*stringStream += buffer;

	/* Check and write subtype - if there is one. */
	if ( len += subtype.size() > ISF_LINE_LEN ) {
		std::cout << "line too long : subtype" << std::endl;
		return;
	}
	sprintf(buffer, ":%s", subtype.c_str());
	*stringStream += buffer;


	/* Check and write data_format - if there is one. */
	if ( len += data_format.size() > ISF_LINE_LEN ) {
		std::cout << "line too long : data format" << std::endl;
		return;
	}
	sprintf(buffer, " %s", data_format.c_str());
	*stringStream += buffer;

	/* Check and write subformat - if there is one. */
	if ( len += subformat.size() > ISF_LINE_LEN ) {
		std::cout << "line too long : subformat" << std::endl;
		return;
	}
	sprintf(buffer, ":%s", subformat.c_str());
	*stringStream += buffer;
	sprintf(buffer, "\n");
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_event_id(std::string* stringStream, std::string evid,
                    std::string region) {

	char buffer[ISF_LINE_LEN];
	/* Chars 1-5: the word "Event". Chars 7-14: event ID. */
	if ( evid.size() > ISF_EVID_LEN ) {
		std::cout << "evid too long" << std::endl;
		return;
	}
	sprintf(buffer, "\nEvent %-*s", ISF_EVID_LEN, evid.c_str());
	*stringStream += buffer;

	/* Chars 16-80: geographic region if given. */
	if ( region.size() > ISF_REGION_LEN ) {
		std::cout << "region too long" << std::endl;
		return;
	}
	sprintf(buffer, " %-s", region.c_str());
	*stringStream += buffer;
	sprintf(buffer, "\n");
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_origin_head(std::string* stringStream) {
	std::string head = "   Date       Time        Err   RMS Latitude Longitude  Smaj  Smin  Az Depth   Err Ndef Nsta Gap  mdist  Mdist Qual   Author      OrigID";
	*stringStream += head + "\n";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_origin(std::string* stringStream, std::string date, std::string time,
                  std::string timfix, double stime, double sdobs, double lat,
                  double lon, std::string epifix, double smaj, double smin,
                  int strike, double depth, std::string depfix, double sdepth,
                  int ndef, int nsta, int gap, double mindist, double maxdist,
                  std::string antype, std::string loctype, std::string etype,
                  std::string author, std::string origid, std::vector<
                          std::string>* log) {

	char buffer[ISF_LINE_LEN];
	std::string prefix = "[" + (std::string) __func__ + "]";
	std::string msg;

	/* Chars 1-10: date. Char 11: space. */
	sprintf(buffer, "%s ", date.c_str());
	*stringStream += buffer;

	/* Chars 12-22: time. */
	sprintf(buffer, "%s", time.c_str());
	*stringStream += buffer;

	/* Char 23: fixed time flag. Char 24: space */
	if ( timfix == "F" ) {
		timfix = "f";
	}
	if ( timfix != " " && timfix != "f" ) {
		msg = prefix + " - bad timfix";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%s ", timfix.c_str());
	*stringStream += buffer;

	/* Chars 25-29: optional origin time error. Char 30: space. */
	/* printf gives at least 2 decimal places but less if number > 99. */
	if ( is_null(stime) ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( stime < 0 || stime > 99999 ) {
			msg = prefix + " - bad stime";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, stime, 5, 2);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 31-35: optional rms (sdobs). Char 36: space. */
	/* printf gives at least 2 decimal places but less if number > 99. */
	if ( is_null(sdobs) ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( sdobs < 0 || sdobs > 99999 ) {
			msg = prefix + " - bad sdobs";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, sdobs, 5, 2);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 37-44: lattitude. Char 45: space. */
	if ( is_null(lat) ) {
		msg = prefix + " - missing latitude";
		log->push_back(msg);
		return;
	}
	if ( lat <= -90 || lat > 90 ) {
		msg = prefix + " - bad latitude";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%8.4f ", lat);
	*stringStream += buffer;

	/* Chars 46-54: longitude. */
	if ( is_null(lon) ) {
		msg = prefix + " - bad missing longitude";
		log->push_back(msg);
		return;
	}
	if ( lon < -180 || lon > 180 ) {
		msg = prefix + "bad longitude";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%9.4f", lon);
	*stringStream += buffer;

	/* Char 55: fixed epicentre flag.*/
	if ( epifix == "F" ) {
		epifix = "f";
	}
	if ( !(epifix == " " || epifix == "f") ) {
		msg = prefix + " - bad epifix";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%s", epifix.c_str());
	*stringStream += buffer;

	/* Char 56 should be a space but then can"t have 5 digit smaj. */
	/* Chars 56-60: optional semi-major axis. Char 61: space. */
	/* printf gives at least 1 decimal place but 0 if number > 999. */
	if ( is_null(smaj) ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( smaj < 0 || smaj > 99999 ) {
			msg = prefix + " - bad smaj";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, smaj, 5, 1);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 62-66: optional semi-minor axis. Char 67: space. */
	/* printf gives at least 1 decimal place but 0 if number > 999. */
	if ( is_null(smin) ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( smin < 0 || smin > 99999 ) {
			msg = prefix + " - bad smin";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, smin, 5, 1);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 68-70: optional strike. Char 71: space. */
	/* Strike can be -1, when it"s a flag to signify that smaj,smin */
	/* are really slat,slon. */
	if ( is_null(strike) ) {
		sprintf(buffer, "    ");
		*stringStream += buffer;
	}
	else {
		if ( strike < -1 || strike > 360 ) {
			msg = prefix + " - bad strike";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%3d ", strike);
		*stringStream += buffer;
	}

	/* Chars 72-76: optional depth. */
	if ( is_null(depth) ) {
		sprintf(buffer, "     ");
		*stringStream += buffer;
	}
	else {
		if ( depth > 999 ) {
			msg = prefix + " - bad depth";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%5.1f", depth);
		*stringStream += buffer;
	}

	/* Char 77: fixed depth flag. Char 78: space.*/
	if ( depfix == "F" ) {
		depfix = "f";
	}
	if ( depfix != " " && depfix != "f" && depfix != "d" ) {
		msg = prefix + " - bad depfix";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%s ", depfix.c_str());
	*stringStream += buffer;

	/* Chars 79-82: optional depth error. Char 83: space. */
	/* printf gives 1 decimal place or 0 if number > 99 */
	if ( is_null(sdepth) ) {
		sprintf(buffer, "     ");
		*stringStream += buffer;
	}
	else {
		if ( sdepth < 0 || sdepth > 9999 ) {
			msg = prefix + " - bad sdepth";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, sdepth, 4, 1);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 84-87: optional ndef. Char 88: space. */
	if ( is_null(ndef) ) {
		sprintf(buffer, "     ");
		*stringStream += buffer;
	}
	else {
		if ( ndef < 0 || ndef > 9999 ) {
			msg = prefix + " - bad ndef";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%4d ", ndef);
		*stringStream += buffer;
	}

	/* Chars 89-92: optional nsta. Char 93: space. */
	if ( is_null(nsta) ) {
		sprintf(buffer, "     ");
		*stringStream += buffer;
	}
	else {
		if ( nsta < 0 || nsta > 9999 ) {
			msg = prefix + " - bad nsta";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%4d ", nsta);
		*stringStream += buffer;
	}

	/* Chars 94-96: optional gap. Char 97: space. */
	if ( is_null(gap) ) {
		sprintf(buffer, "    ");
		*stringStream += buffer;
	}
	else {
		if ( gap < 0 || gap > 360 ) {
			msg = prefix + " - bad gap";
			log->push_back(msg);
			sprintf(buffer, "    ");
		}
		else {
			sprintf(buffer, "%3d ", gap);
		}
		*stringStream += buffer;
	}

	/* Chars 98-103: optional minimum distance. Char 104: space. */
	/* printf gives at least 2 decimal places but less if number > 999. */
	if ( is_null(mindist) ) {
		sprintf(buffer, "       ");
		*stringStream += buffer;
	}
	else {
		if ( mindist < 0 || mindist > 999999 ) {
			msg = prefix + " - bad mindist";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, mindist, 6, 2);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 105-110: optional maximum distance. Char 111: space. */
	/* printf gives at least 2 decimal places but less if number > 999. */
	if ( is_null(maxdist) ) {
		sprintf(buffer, "       ");
		*stringStream += buffer;
	}
	else {
		if ( maxdist < 0 || maxdist > 999999 ) {
			msg = prefix + " - bad maxdist";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, maxdist, 6, 2);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Char 112: analysis type. Char 113 space. */
	if ( antype != " " && antype != "a" && antype != "m" && antype != "g" ) {
		msg = prefix + " - bad antype";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%s ", antype.c_str());

	/* Char 114: location method. Char 115 space. */
	if ( loctype != " " && loctype != "i" && loctype != "p" &&
	        loctype != "g" && loctype != "o" ) {
		msg = prefix + " - bad loctype";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%s ", loctype.c_str());
	*stringStream += buffer;

	/* Chars 116-117: event type. Char 118 space. */
	if ( etype.size() != ISF_ETYPE_LEN ) {
		msg = prefix + " - etype wrong length";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%*s ", ISF_ETYPE_LEN, etype.c_str());
	*stringStream += buffer;


	/* Chars 119-127: author. Char 128: space. */
	if ( author.size() > ISF_AUTHOR_LEN ) {
		msg = prefix + " - author name is too long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s ", ISF_AUTHOR_LEN, author.c_str());
	*stringStream += buffer;

	/* Chars 129-136: origid. */
	if ( origid.size() > ISF_ORIGID_LEN ) {
		msg = prefix + " - originid is too long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s\n", ISF_ORIGID_LEN, origid.c_str());
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_netmag_head(std::string* stringStream) {

	char buffer[ISF_LINE_LEN];
	std::string head = "Magnitude  Err Nsta Author      OrigID";
	sprintf(buffer, "\n%s\n", head.c_str());
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_netmag(std::string* stringStream, std::string magtype,
                  std::string magind, double mag, double magerr, int nsta,
                  std::string author, std::string origid,
                  std::vector<std::string>* log) {

	char buffer[ISF_LINE_LEN];
	std::string prefix = "[" + (std::string) __func__ + "]";
	std::string msg;

	/* Chars 1-5: magtype. */
	if ( magtype.size() > ISF_MAGTYPE_LEN ) {
		msg = prefix + " - magtype is too long and has been truncated [" + magtype + "]";
		log->push_back(msg);
		magtype = magtype.substr(0, 2);
	}
	sprintf(buffer, "%-*s", ISF_MAGTYPE_LEN, magtype.c_str());
	*stringStream += buffer;

	/* Char 6: less than or greater than indicator */
	if ( magind != " " && magind != "<" && magind != ">" ) {
		msg = prefix + " - bad magind";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%s", magind.c_str());
	*stringStream += buffer;

	/* Chars 7-10: magnitude value. Char 11 space. */
	if ( is_null(mag) ) {
		msg = prefix + " - magnitude value is missing";
		log->push_back(msg);
		return;
	}
	if ( mag < -1 || mag > 12 ) {
		msg = prefix + " - bag magnitude value";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%4.1f ", mag);
	*stringStream += buffer;

	/* Chars 12-14: optional magnitude error. Char 15: space. */
	if ( is_null(magerr) ) {
		sprintf(buffer, "    ");
		*stringStream += buffer;
	}
	else {
		if ( magerr < 0 || magerr > 9.9 ) {
			msg = prefix + " - bad magnitude error";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%3.1f ", magerr);
		*stringStream += buffer;
	}

	/* Chars 16-19 optional number of stations. Char 20: space. */
	if ( is_null(nsta) ) {
		sprintf(buffer, "     ");
		*stringStream += buffer;
	}
	else {
		if ( nsta < 0 || nsta > 9999 ) {
			msg = prefix + " - bad number of station";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%4d ", nsta);
		*stringStream += buffer;
	}
	/* Chars 21-29 author. Char 30 space. */
	if ( author.size() > ISF_AUTHOR_LEN ) {
		msg = prefix + " - author name is too long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s ", ISF_AUTHOR_LEN, author.c_str());
	*stringStream += buffer;

	/* Chars 31-38 origid. */
	if ( origid.size() > ISF_ORIGID_LEN ) {
		msg = prefix + " - originid is too long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s\n", ISF_ORIGID_LEN, origid.c_str());
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_phase_head(std::string *stringStream) {

	char buffer[ISF_LINE_LEN];
	std::string head = "Sta     Dist  EvAz Phase        Time      TRes  Azim AzRes   Slow   SRes Def   SNR       Amp   Per Qual Magnitude    ArrID";
	sprintf(buffer, "\n%s\n", head.c_str());
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_phase(std::string* stringStream, std::string sta, double dist,
                 double esaz, std::string phase, std::string time, double timeres,
                 double azim, double azimres, double slow, double slowres,
                 std::string timedef, std::string azimdef, std::string slowdef,
                 double snr, double amp, double per, std::string picktype,
                 std::string sp_fm, std::string detstring, std::string magtype,
                 std::string magind, double mag, std::string arrid,
                 std::vector<std::string>* log) {

	char buffer[ISF_LINE_LEN];
	std::string prefix = "[" + (std::string) __func__ + "] - " + sta;
	std::string msg;

	/* Chars 1-5: station code. Char 6: space  */
	if ( sta.size() > ISF_STA_LEN ) {
		msg = prefix + " - station code is too long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s ", ISF_STA_LEN, sta.c_str());
	*stringStream += buffer;

	/* Chars 7-12: distance. Char 13: space */
	if ( is_null(dist) ) {
		sprintf(buffer, "       ");
		*stringStream += buffer;
	}
	else {
		if ( dist < 0 || dist > 999.99 ) {
			msg = prefix + " - bad distance";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%6.2f ", dist);
		*stringStream += buffer;
	}

	/* Chars 14-18: event to sta azimuth. Char 19: space */
	if ( is_null(esaz) ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( esaz < 0 || esaz > 360 ) {
			msg = prefix + " - bad esaz";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%5.1f ", esaz);
		*stringStream += buffer;
	}

	/* Chars 20-27: phase code - can be null. Char 28: space */
	if ( phase.size() > ISF_PHASE_LEN ) {
		msg = prefix + " - phase is too long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s ", ISF_PHASE_LEN, phase.c_str());
	*stringStream += buffer;


	/* Chars 29-40: time. Char 41: space. */
	/* Time can be completely null. */
	sprintf(buffer, "%s", time.c_str());
	*stringStream += buffer;



	/* Chars 42-46: time residual. Char 47: space */
	if ( timeres == .00 ) {
		sprintf(buffer, "       ");
		*stringStream += buffer;
	}
	else {
		if ( timeres < -9999 || timeres > 9999 ) {
			msg = prefix + " - bad timeres";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, timeres, 5, 1);
		sprintf(buffer, "  ");
		*stringStream += buffer;
	}

	/* Chars 48-52: observed azimuth. Char 53: space */
	if ( azim == .00 ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( azim < 0 || azim > 360 ) {
			msg = prefix + " - bad observed azimuth";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%5.1f ", azim);
		*stringStream += buffer;
	}

	/* Chars 54-58: azimuth residual. Char 59: space */
	if ( azimres == .00 ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( azimres < -360 || azimres > 360 ) {
			msg = prefix + " - bad azimuth residual";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, azimres, 5, 1);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 60-65: slowness. Char 66: space */
	if ( slow == .00 ) {
		sprintf(buffer, "       ");
		*stringStream += buffer;
	}
	else {
		if ( slow < 0 || slow > 999.99 ) {
			msg = prefix + " - bad slowness";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%6.2f ", slow);
		*stringStream += buffer;
	}

	/* Chars 67-72: slowness residual. Char 73: space */
	if ( slowres == .00 ) {
		sprintf(buffer, "       ");
		*stringStream += buffer;
	}
	else {
		if ( slowres < -99999 || slowres > 99999 ) {
			msg = prefix + " - bad slowness residual";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, slowres, 6, 1);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Char 74: time defining flag */
	if ( timedef == "T" || timedef == "_" ) {
		sprintf(buffer, "%s", timedef.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad time defining flag";
		log->push_back(msg);
		return;
	}

	/* Char 75: azimuth defining flag */
	if ( azimdef == "A" || azimdef == "_" ) {
		sprintf(buffer, "%s", azimdef.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad azimuth defining flag";
		log->push_back(msg);
		return;
	}

	/* Char 76: slowness defining flag. Char 77: space. */
	if ( slowdef == "S" || slowdef == "_" ) {
		sprintf(buffer, "%s ", slowdef.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad slowness definig flag";
		log->push_back(msg);
		return;
	}

	/* Chars 78-82: signal-to noise. Char 83: space */
	//if (is_null(snr)) {
	if ( snr == .00 ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( snr < 0 || snr > 999 ) {
			msg = prefix + " - bad signal to noise ratio";
			log->push_back(msg);
			sprintf(buffer, "      ");
		}
		else {
			sprintf(buffer, "%5.1f ", snr);
		}
		*stringStream += buffer;
	}

	/* Chars 84-92: amplitude. Char 93: space */
	if ( amp == .00 ) {
		sprintf(buffer, "          ");
		*stringStream += buffer;
	}
	else {
		if ( amp < 0 || amp > 999999999 ) {
			msg = prefix + " - bad amplitude";
			log->push_back(msg);
			return;
		}
		print_double(stringStream, amp, 9, 1);
		sprintf(buffer, " ");
		*stringStream += buffer;
	}

	/* Chars 94-98: period. Char 99: space */
	if ( per == .00 ) {
		sprintf(buffer, "      ");
		*stringStream += buffer;
	}
	else {
		if ( per < 0 || per > 99.99 ) {
			msg = prefix + " - period value has been changed to 5.1f instead of 5.2f";
			log->push_back(msg);
			sprintf(buffer, "%5.1f ", per);
		}
		else {
			sprintf(buffer, "%5.2f ", per);
		}
		*stringStream += buffer;
	}

	/* Char 100: picktype. */
	if ( picktype == "a" || picktype == "m" || picktype == "_" ) {
		sprintf(buffer, "%s", picktype.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad picktype";
		log->push_back(msg);
		return;
	}

	/* Char 101: sp_fm. */
	if ( sp_fm == "C" ) {
		sp_fm = "c";
	}
	if ( sp_fm == "D" ) {
		sp_fm = "d";
	}
	if ( sp_fm == "c" || sp_fm == "d" || sp_fm == "_" ) {
		sprintf(buffer, "%s", sp_fm.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad sp_fm";
		log->push_back(msg);
		return;
	}

	/* Char 102: detstring. Char 103: space. */
	if ( detstring == "I" ) {
		detstring = "i";
	}
	if ( detstring == "E" ) {
		detstring = "e";
	}
	if ( detstring == "Q" ) {
		detstring = "q";
	}
	if ( detstring == "i" || detstring == "e" || detstring == "q" || detstring == "_" ) {
		sprintf(buffer, "%s ", detstring.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad detstring";
		log->push_back(msg);
		return;
	}

	/* Chars 104-108: magnitude type */
	if ( magtype.size() > ISF_MAGTYPE_LEN ) {
		msg = prefix + " - magtype is too long and has been truncated ("
		        + magtype + " -> " + magtype.substr(0, 2) + ")";
		log->push_back(msg);
		magtype = magtype.substr(0, 2);
	}
	sprintf(buffer, "%-*s", ISF_MAGTYPE_LEN, magtype.c_str());
	*stringStream += buffer;


	/* Char 109: magnitude indicator. */
	if ( magind == " " || magind == "<" || magind == ">" ) {
		sprintf(buffer, "%s", magind.c_str());
		*stringStream += buffer;
	}
	else {
		msg = prefix + " - bad magnitude indicator";
		log->push_back(msg);
		return;
	}

	/* Chars 110-113: magnitude. Char 114: space */
	if ( is_null(mag) ) {
		sprintf(buffer, "     ");
		*stringStream += buffer;
	}
	else {
		if ( mag < 0 || mag > 10 ) {
			msg = prefix + " - bad magnitude";
			log->push_back(msg);
			return;
		}
		sprintf(buffer, "%4.1f ", mag);
		*stringStream += buffer;
	}

	/* Chars 115-122: arrival ID */
	if ( arrid.size() > ISF_ARRID_LEN ) {
		msg = prefix + " - arrivalId is to long";
		log->push_back(msg);
		return;
	}
	sprintf(buffer, "%-*s\n", ISF_ARRID_LEN, arrid.c_str());
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void write_stop(std::string* stringStream) {

	char buffer[ISF_LINE_LEN];
	sprintf(buffer, "\nSTOP\n");
	*stringStream += buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace ISF

