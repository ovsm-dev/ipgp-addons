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

#ifndef __IPGP_GUI_DEFS_H__
#define __IPGP_GUI_DEFS_H__

#include <QSharedPointer>
#include <string>

namespace IPGP {
namespace Gui {

template<typename T>
struct SmartPointer {
		typedef QSharedPointer<T> Impl;
};

}
}

#define initPtr(classname) \
	QSharedPointer<classname>(new classname)

#define initCPtr(classname) \
	QSharedPointer<classname>(classname)

#define TYPEDEF_IPGPSMARTPOINTER(classname) \
    typedef IPGP::Gui::SmartPointer<classname>::Impl classname##Ptr

#define TYPEDEF_CONST_IPGPSMARTPOINTER(classname) \
    typedef IPGP::Gui::SmartPointer<const classname>::Impl classname##CPtr

#define DEFINE_IPGP_SMARTPOINTER(classname) \
    class classname; \
    TYPEDEF_IPGPSMARTPOINTER(classname); \
    TYPEDEF_CONST_IPGPSMARTPOINTER(classname)



#endif

