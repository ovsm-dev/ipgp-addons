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


#ifndef __IPGP_COREAPP_WEBOBSTOSEISCOMP3_H__
#define __IPGP_COREAPP_WEBOBSTOSEISCOMP3_H__

#include <string>
#include <vector>
#include <map>
#include <seiscomp3/client/application.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>


namespace Seiscomp {
namespace Logging {
class Channel;
class Output;
}
}


class WebobsToSeisComP3 : public Seiscomp::Client::Application {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum EventDeletionMethod {
			EDM_Hide = 0x01,
			EDM_Delete = 0x02,
			EDM_None = 0x04
		};
		typedef std::map<std::string, std::string> MappedHosts;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit WebobsToSeisComP3(int argc, char** argv);
		~WebobsToSeisComP3();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool initConfiguration();
		bool run();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		int handleData(int*);
		void handleError(int*);
		void dataHandler(int*);
		void runDataHandler(int*);
		bool clientIsAllowed(const char*);

		bool removeEvent(xmlXPathObject*);
		bool createOrigin(xmlDoc*, xmlXPathObject*);

	private:
		// ------------------------------------------------------------------
		//  Private members
		// ------------------------------------------------------------------
		int _port;
		int _module_id;
		int _module_type;

		bool _enableXMLArchive;
		bool _enableDatabaseStorage;
		bool _enableEventDeletion;

		std::string _defaultPath;
		std::string _errorLog;
		std::string _regionTag;

		EventDeletionMethod _edm;

		MappedHosts _authorizedHosts;

		Seiscomp::Logging::Channel* _infoChannel;
		Seiscomp::Logging::Output* _infoOutput;
		Seiscomp::Logging::Channel* _errorChannel;
		Seiscomp::Logging::Output* _errorOutput;

	protected:
		// ------------------------------------------------------------------
		//  Protected members
		// ------------------------------------------------------------------
		boost::thread* _streamThread;
};

#endif


