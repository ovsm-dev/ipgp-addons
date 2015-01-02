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


#ifndef __IPGP_CORE_FILE_H__
#define __IPGP_CORE_FILE_H__

#include <ipgp/core/api.h>
#include <string>


namespace IPGP {
namespace Core {
namespace File {

/**
 * @brief Look over of transmitted file/folder/symlink/socket/wtv exists
 *        at the given path
 * @param file the thing to search
 * @return true if whatever it is your looking for exists or false if not
 */
bool SC_IPGP_CORE_API fileExists(std::string file);

/**
 * @brief Verify if folder exists at a given path
 * @param folder the folder to look out for
 * @return true if exists or false otherwise
 */
bool SC_IPGP_CORE_API folderExists(std::string folder);

/**
 * @brief Folder creator.\nCreate any folder including parents if necessary
 * @param folder the folder to create
 * @return 0 on succes or 1 otherwise
 */
int SC_IPGP_CORE_API makeFolder(std::string folder);

/**
 * @brief Hierarchy researcher!
 * @param file the file or folder to look out for
 * @return the parent directory of the file/folder
 */
std::string SC_IPGP_CORE_API parentFolder(const std::string& file);


}
}
}


#endif

