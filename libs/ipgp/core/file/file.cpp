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


#include <ipgp/core/file/file.h>
#include <iostream>
#include <string>
#include <sys/stat.h>


namespace IPGP {
namespace Core {
namespace File {




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fileExists(std::string file) {

	//'Nothing' obviously doesn't exist
	if ( file.empty() ) {
		return false;
	}

	struct stat info;

	//Stat returns 0 on success
	if ( stat(file.c_str(), &info) ) {
		return false;
	}
	else if ( info.st_size == 0 ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool folderExists(std::string folder) {

	//'Nothing' obviously doesn't exist
	if ( folder.empty() ) {
		return false;
	}

	struct stat info;

	//Stat returns 0 on success
	if ( stat(folder.c_str(), &info) ) {
		return false;
	}

	//Return whether or not it's a folder
	return (S_ISDIR(info.st_mode));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int makeFolder(std::string folder) {

	//A null folder is obviously created just fine, as is the case if it already exists
	if ( folder.empty() || folderExists(folder) ) {
		return 0;
	}

	std::string par = parentFolder(folder);

	//If the parent doesn't exist, try to make it
	if ( !folderExists(par) ) {
		//If the parent can't be made, the requested folder obviously can't be
		if ( makeFolder(par) != 0 ) {
			return 1;
		}
	}

	//The parent must now exist, so we can make the folder; if this fails, we still do
	if ( mkdir(folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ) {
		return 1;
	}

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string parentFolder(const std::string& file) {

	if ( file.empty() ) {
		return "";
	}

	//Find position of last '/' that is not the final character (length - 1 = final character)
	int last = file.find_last_of('/', file.length() - 2);

	//We want at least 1 character
	if ( last == 0 ) {
		last++;
	}

	return file.substr(0, last);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}
}
}
