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



#ifndef __IPGP_CORE_STRING_H__
#define __IPGP_CORE_STRING_H__


#include <ipgp/core/api.h>
#include <iosfwd>
#include <string>
#include <vector>


namespace IPGP {
namespace Core {
namespace String {

/**
 * @brief Blank characters generator
 * @param toFormat string value to format
 * @param nb characters to achieve
 * @param pos 0 or 1 to add blank before or after passed string
 * @return formated string
 */
std::string SC_IPGP_CORE_API addWhiteSpace(std::string str, unsigned int nb, int pos);

/**
 * @brief Removes white spaces from passed string
 * @param str string to strip whites paces
 * @return stripped string of passed string
 */
std::string SC_IPGP_CORE_API stripWhiteSpace(std::string str);

/**
 * @brief Converts string to double
 * @param s string to convert
 * @return double value of passed string
 */
double SC_IPGP_CORE_API stringToDouble(const std::string& str);

/**
 * @brief Converts passed string number into int number
 * @param str string to convert
 * @return int value of passed string
 */
int SC_IPGP_CORE_API stringToInt(const std::string& str);

/**
 * @brief Explodes a string using char as delimiter into given std::vector
 * @param s std::string to explode
 * @param c reference character
 * @param v vector to hydrate
 */
void SC_IPGP_CORE_API explodeString(std::string& str, char c,
                                    std::vector<std::string>& v);

/**
 * @brief File type identifier\n
 * Tells if a string is originally a double or a integer
 * @param str the string to analyze
 * @param type 0 for double detection | 1 for integer detection
 * @return true/false pending on detection's success
 */
bool SC_IPGP_CORE_API stringType(std::string& str, int type);

/**
 * @brief Blank character filler\n
 * Replaces blank character in string1 by string1
 * @param str the original string
 * @param str1 the character to insert in replacement
 * @return a new string filled with string2
 */
std::string SC_IPGP_CORE_API blankReplace(std::string& str, std::string& str1);


/**
 * @brief Modern sprintf...
 * @param fmt the output format e.g. "%02.f"
 * @param ...
 * @return the std::string of requested format
 */
//#if SC_API_VERSION < SC_API_VERSION_CHECK(10, 0, 0)
#if SC_API_VERSION < 0x0A0000
std::string SC_IPGP_CORE_API stringify(const char* fmt, ...);
#endif


/**
 * @brief std::string to * converter
 * @param s the std::string to convert
 * @return the * value of inputted std::string
 */
template<class T> T SC_IPGP_CORE_API fromString(std::string s);


} // end String
} // end Core
} // end IPGP


#endif
