#############################################################################
##                                                                         ##
##  Copyright (C) 2012 OVSM/IPGP                                           ##
##                                                                         ##
##  This program is free software: you can redistribute it and/or modify   ##
##  it under the terms of the GNU General Public License as published by   ##
##  the Free Software Foundation, either version 3 of the License, or      ##
##  (at your option) any later version.                                    ##
##                                                                         ##
##  This program is distributed in the hope that it will be useful,        ##
##  but WITHOUT ANY WARRANTY; without even the implied warranty of         ##
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          ##
##  GNU General Public License for more details.                           ##
##                                                                         ##
##  This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.       ##
##  It has been co-financed by the European Union and le Ministère de      ##
##  l'Ecologie, du Développement Durable, des Transports et du Logement.   ##
##                                                                         ##
#############################################################################


SET(IPGP_CORE_SOURCES "")
SET(IPGP_CORE_HEADERS "")

SC_ADD_SUBDIR_SOURCES(IPGP_CORE 3rd-party)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE bulletin)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE datamodel)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE file)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE geo)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE math)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE misc)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE network)
SC_ADD_SUBDIR_SOURCES(IPGP_CORE string)

SET(PROJECT_API_H api.h)
SET(IPGP_CORE_HEADERS "")

SC_ADD_LIBRARY(IPGP_CORE ipgp_core)

SC_LIB_INSTALL_HEADERS(IPGP_CORE)

SC_LIB_LINK_LIBRARIES_INTERNAL(ipgp_core core client config)


