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

#ifndef __SC_IPGP_CORE_API_H__
#define __SC_IPGP_CORE_API_H__

#if defined(WIN32) && (defined(SC_IPGP_CORE_SHARED) || defined(SC_ALL_SHARED))
# if defined(SC_IPGP_CORE_EXPORTS)
#  define SC_IPGP_CORE_API __declspec(dllexport)
#  define SC_IPGP_CORE_TEMPLATE_EXPORT
# else
#  define SC_IPGP_CORE_API __declspec(dllimport)
#  define SC_IPGP_CORE_TEMPLATE_EXPORT extern
# endif
#else
# define SC_IPGP_CORE_API
# define SC_IPGP_CORE_TEMPLATE_EXPORT
#endif

#endif


