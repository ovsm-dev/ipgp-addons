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


#include <ipgp/core/api.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <iosfwd>
#include <vector>


namespace ISF {

#define ISF_NULL			9999999
#define ISF_LINE_LEN        140
#define ISF_COMM_LEN        80
#define ISF_EVID_LEN        8
#define ISF_REGION_LEN      65
#define ISF_ETYPE_LEN       2
#define ISF_AUTHOR_LEN      9
#define ISF_ORIGID_LEN      8
#define ISF_MAGTYPE_LEN     5
#define ISF_STA_LEN         5
#define ISF_NET_LEN         9
#define ISF_CHAN_LEN        3
#define ISF_PHASE_LEN       8
#define ISF_ARRID_LEN       8
#define ISF_F_TYPE_LEN      3
#define ISF_F_PLANE_LEN     5
#define ISF_I_LOCTYPE_LEN   6
#define ISF_COUNTRY_LEN     3
#define ISF_POSTCODE_LEN    10
#define ISF_I_SCALE_LEN     5
#define ISF_AUXID_LEN       4
#define ISF_GROUPID_LEN     8
#define ISF_NUM_STA     200
#define ISF_NUM_PARAM   100


/**
 * Writes a double a bit more flexible than what can be achieved with printf.
 * If a number is too big for the ideal precision it is printed with less
 * precision until it fills the field width without a decimal point at all.
 * For example might want 99.9999 => 99.99 but 999.9999 => 999.9.
 * @param stringStream the string to fill
 * @param x the number
 * @param width the width to acheive
 * @param max_prec the precision to acheive
 */
void SC_IPGP_CORE_API
print_double(std::string* stringStream, double x, int width, int max_prec);

/**
 * Checks whether a number is null or not
 * @param i the number to checkout
 * @return 1 if it is or 0 if it is not
 */
int SC_IPGP_CORE_API is_null(int i);

/**
 * Writes the data type line at the top of a GSE report.
 * Format is:  DATA_TYPE data_type:subtype data_format:subformat
 * Only data_type is required.  Only other limitation is that a subformat
 * is not allowed without a data_format.
 * @param stringStream the string's pointer to write into
 * @param type
 * @param subtype
 * @param format
 * @param subformat
 */
void SC_IPGP_CORE_API
write_data_type(std::string* stringStream, std::string type, std::string subtype,
                std::string format, std::string subformat);

/**
 * Writes an event title line with a preceding blank line.
 * Requires event ID but will write a line without a region if required.
 * @param stringStream
 * @param evid
 * @param region
 */
void SC_IPGP_CORE_API
write_event_id(std::string* stringStream, std::string evid, std::string region);

/**
 * Writes an origin header line.
 * @param stringStream
 */
void SC_IPGP_CORE_API write_origin_head(std::string* stringStream);

/**
 * Writes an origin line.
 * @param stringStream
 * @param date
 * @param time
 * @param timfix
 * @param stime
 * @param sdobs
 * @param lat
 * @param lon
 * @param epifix
 * @param smaj
 * @param smin
 * @param strike
 * @param depth
 * @param depfix
 * @param sdepth
 * @param ndef
 * @param nsta
 * @param gap
 * @param mindist
 * @param maxdist
 * @param antype
 * @param loctype
 * @param etype
 * @param author
 * @param origid
 */
void SC_IPGP_CORE_API
write_origin(std::string* stringStream, std::string date, std::string time,
             std::string timfix, double stime, double sdobs, double lat, double lon,
             std::string epifix, double smaj, double smin, int strike, double depth,
             std::string depfix, double sdepth, int ndef, int nsta, int gap,
             double mindist, double maxdist, std::string antype, std::string loctype,
             std::string etype, std::string author, std::string origid,
             std::vector<std::string>* log);

/**
 * Writes a  magnitude data line.
 * @param stringStream
 */
void SC_IPGP_CORE_API write_netmag_head(std::string* stringStream);

/**
 *
 * @param stringStream
 * @param magtype
 * @param magind
 * @param mag
 * @param magerr
 * @param nsta
 * @param author
 * @param origid
 */
void SC_IPGP_CORE_API
write_netmag(std::string* stringStream, std::string magtype, std::string magind,
             double mag, double magerr, int nsta, std::string author, std::string origid,
             std::vector<std::string>* log);


/**
 * Writes  phase header complete with preceding blank line.
 * @param stringStream
 */
void SC_IPGP_CORE_API write_phase_head(std::string* stringStream);


/**
 * Writes a phase block data line.
 * @param fp
 * @param sta
 * @param dist
 * @param esaz
 * @param phase
 * @param time
 * @param timeres
 * @param azim
 * @param azimres
 * @param slow
 * @param slowres
 * @param timedef
 * @param azimdef
 * @param slowdef
 * @param snr
 * @param amp
 * @param per
 * @param picktype
 * @param sp_fm
 * @param detstring
 * @param magtype
 * @param magind
 * @param mag
 * @param arrid
 */
void SC_IPGP_CORE_API
write_phase(std::string* stringStream, std::string sta, double dist,
            double esaz, std::string phase, std::string time, double timeres,
            double azim, double azimres, double slow, double slowres,
            std::string timedef, std::string azimdef, std::string slowdef,
            double snr, double amp, double per, std::string picktype,
            std::string sp_fm, std::string detstring, std::string magtype,
            std::string magind, double mag, std::string arrid,
            std::vector<std::string>* log);

/**
 * Writes STOP line with a preceding blank line.
 * @param stringStream
 */
void SC_IPGP_CORE_API write_stop(std::string* stringStream);


}

