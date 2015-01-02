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

#ifndef __IPGP_GUI_CLIENT_MISC_H__
#define __IPGP_GUI_CLIENT_MISC_H__

#include <ipgp/gui/api.h>
#include <QString>
#include <QMetaType>


namespace IPGP {
namespace Gui {
namespace Client {

//! Application State
enum SC_IPGP_GUI_API State {
	AppInit = 0x001,
	AppLoadingConfiguration = 0x002,
	AppLoadingConnections = 0x004,
	AppIdling = 0x008,
	AppWorking = 0x010
};

//! Log's message type
enum SC_IPGP_GUI_API LogMessage {
	LM_OK = 0x01,
	LM_WARNING = 0x02,
	LM_ERROR = 0x04,
	LM_INFO = 0x08
};

/**
 * @brief Application
 */
struct SC_IPGP_GUI_API ApplicationDescriptor {

		ApplicationDescriptor() {}

		explicit ApplicationDescriptor(const QString& n, const QString& s,
		                               const QString& f, const QString& v) :
				name(n), shortDescription(s), fullDescription(f), version(v) {}

		ApplicationDescriptor& operator=(const ApplicationDescriptor& ad) {

			name = ad.name;
			shortDescription = ad.shortDescription;
			fullDescription = ad.fullDescription;
			version = ad.version;
			return *this;
		}

		QString name;
		QString shortDescription;
		QString fullDescription;
		QString version;
};

} // namespace Client
} // namespace Gui
} // namespace IPGP

Q_DECLARE_METATYPE(IPGP::Gui::Client::LogMessage)

#endif

