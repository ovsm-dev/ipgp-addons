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


#include <ipgp/core/string/string.h>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <iostream>



namespace IPGP {
namespace Core {
namespace String {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string addWhiteSpace(std::string str, unsigned int nb, int pos) {

	if ( str.size() > nb )
		return "";
	unsigned int count;
	std::string blank;
	count = nb - str.size();

	if ( count > 0 ) {
		while ( blank.size() < count )
			blank += " ";
		switch ( pos ) {
			case 0:
				//! add blank space before
				str = blank + str;
			break;
			case 1:
				//! add blank space after
				str += blank;
			break;
			default:
				break;
		}
	}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string stripWhiteSpace(std::string str) {

	for (unsigned int i = 0; i < str.length(); i++)
		if ( str[i] == ' ' ) {
			str.erase(i, 1);
			i--;
		}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double stringToDouble(const std::string& str) {

	std::stringstream ss(str);
	double f;
	ss >> f;

	return f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int stringToInt(const std::string& str) {

	int value;
	std::istringstream iss(str);
	iss >> value;

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void explodeString(std::string& str, char c, std::vector<std::string>& v) {

	std::string::size_type i = 0;
	std::string::size_type j = str.find(c);
	while ( j != std::string::npos ) {
		v.push_back(str.substr(i, j - i));
		i = ++j;
		j = str.find(c, j);
		if ( j == std::string::npos ) {
			if ( stripWhiteSpace(str.substr(i, str.length())) != "" )
				v.push_back(str.substr(i, str.length()));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool stringType(std::string& str, int type) {

	int i;
	float f;
	switch ( type ) {
		case 0:
			if ( sscanf(str.c_str(), "%f", &f) != 0 )
				return true;
		break;
		case 1:
			if ( sscanf(str.c_str(), "%d", &i) != 0 )
				return true;
		break;
		default:
			return false;
		break;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string blankReplace(std::string& str, std::string& str1) {

	for (unsigned int i = 0; i < str.length(); i++)
		if ( str[i] == ' ' ) {
			str.replace(i, str1.length(), str1);
			i--;
		}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//#if SC_API_VERSION < SC_API_VERSION_CHECK(10, 0, 0)
#if SC_API_VERSION < 0x0A0000
std::string stringify(const char* fmt, ...) {

	int size = 512;
	char* buffer = 0;
	buffer = new char[size];
	va_list vl;
	va_start(vl, fmt);
	int nsize = vsnprintf(buffer, size, fmt, vl);
	if ( size <= nsize ) { //fail delete buffer and try again
		delete buffer;
		buffer = 0;
		buffer = new char[nsize + 1]; //+1 for /0
		nsize = vsnprintf(buffer, size, fmt, vl);
	}
	std::string ret(buffer);
	va_end(vl);
	delete buffer;

	return ret;
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



template<class T> T fromString(std::string s) {

	T result;
	std::stringstream str;
	str << s;
	str >> result;

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// end String
} // end Core
} // end IPGP
