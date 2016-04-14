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


#define SEISCOMP_COMPONENT WO2SC3

#include "wo2sc3.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/logging/filerotator.h>
#include <seiscomp3/logging/channel.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/math/geo.h>

#if SC_API_VERSION < 0x020000
#include <seiscomp3/processing/parameters.h>
#include <seiscomp3/datamodel/parameterset.h>
#else
#include <seiscomp3/utils/files.h>
#include <seiscomp3/utils/keyvalues.h>
#endif

#include <boost/thread.hpp>
#include <boost/bind.hpp>

// Socket libs
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ipgp/core/string/string.h>
#include <iostream>
#include <libxml/xmlstring.h>



using namespace Seiscomp;
using namespace Seiscomp::Client;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Logging;
using namespace IPGP;
using namespace IPGP::Core::String;



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebobsToSeisComP3::WebobsToSeisComP3(int argc, char** argv) :
		Application(argc, argv), _streamThread(NULL) {

	_port = -1;
	_module_id = -1;
	_module_type = -1;
	_enableXMLArchive = false;
	_enableDatabaseStorage = false;
	_enableEventDeletion = false;

	_defaultPath = "";
	_edm = EDM_None;
	_errorLog = "";

	setLoadInventoryEnabled(false);
	setLoadStationsEnabled(true);
	setLoadConfigModuleEnabled(true);
	setPrimaryMessagingGroup("EVENT");
	addMessagingSubscription("LOCATION");
	addMessagingSubscription("PICK");

	_infoChannel = SEISCOMP_DEF_LOGCHANNEL("info", LL_INFO);
	_infoOutput = new FileRotatorOutput(Environment::Instance()->
	        logFile("WebobsToSeisComP3-processing-info").c_str(), 60 * 60 * 24, 30);
	_infoOutput->subscribe(_infoChannel);

	_errorChannel = SEISCOMP_DEF_LOGCHANNEL("error", LL_INFO);
	_errorOutput = new FileRotatorOutput(Environment::Instance()->
	        logFile("WebobsToSeisComP3-processing-error").c_str(), 60 * 60 * 24, 30);
	_errorOutput->subscribe(_errorChannel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebobsToSeisComP3::~WebobsToSeisComP3() {

	if ( _infoChannel )
		delete _infoChannel, _infoChannel = NULL;

	if ( _infoOutput )
		delete _infoOutput, _infoOutput = NULL;

	if ( _errorChannel )
		delete _errorChannel, _errorChannel = NULL;

	if ( _errorOutput )
		delete _errorOutput, _errorOutput = NULL;

	SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d",
	    BaseObject::ObjectCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebobsToSeisComP3::initConfiguration() {

	if ( !Application::initConfiguration() )
		return false;

	int error = 0;

	try {
		_port = configGetInt("wo2sc3.socket.port");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.socket.port variable is not set");
		error++;
	}

	try {
		_regionTag = configGetString("wo2sc3.region.tag");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.region.tag variable is not set");
		error++;
	}

	std::vector<std::string> allowedHosts;
	try {
		allowedHosts = configGetStrings("wo2sc3.socket.authorizedHosts");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.socket.authorizedHosts variable is not set");
		error++;
	}

	if ( allowedHosts.size() > 0 ) {

		_authorizedHosts.clear();
		for (size_t i = 0; i < allowedHosts.size(); ++i) {
			struct hostent* info = gethostbyname(allowedHosts.at(i).c_str());
			if ( info )
				for (int j = 0; info->h_addr_list[j] != NULL; ++j)
					_authorizedHosts.insert(std::pair<std::string, std::string>(
					    allowedHosts.at(i),
					    std::string(inet_ntoa(*((struct in_addr*) info->h_addr_list[j])))));
		}
	}


	try {
		_module_id = configGetInt("wo2sc3.module.ID");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.moduleId variable is not set");
		error++;
	}

	try {
		_module_type = configGetInt("wo2sc3.module.type");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.module.type variable is not set");
		error++;
	}

	try {
		_enableXMLArchive = configGetBool("wo2sc3.enable.XMLArchive");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.enable.XMLArchive variable is not set");
		error++;
	}

	try {
		_enableDatabaseStorage = configGetBool("wo2sc3.enable.databaseStorage");
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.enable.databaseStorage variable is not set");
		error++;
	}

	try {
		_enableEventDeletion = configGetBool("wo2sc3.enable.eventDeletion");
	} catch ( ... ) {
		SEISCOMP_WARNING("wo2sc3.enable.eventDeletion variable is not set, FALSE by default");
		_enableEventDeletion = false;
	}

	try {
		if ( _enableEventDeletion ) {
			std::string deletionMethod = configGetString("wo2sc3.eventDeletionMethod");
			if ( (deletionMethod.compare("hide") != 0)
			        && (deletionMethod.compare("HIDE") != 0)
			        && (deletionMethod.compare("delete") != 0)
			        && (deletionMethod.compare("DELETE") != 0) ) {
				SEISCOMP_ERROR("Event deletion enabled but not configured properly...");
				error++;
			}
			else if ( deletionMethod.compare("hide") == 0
			        or deletionMethod.compare("HIDE") == 0 ) {
				_edm = EDM_Hide;
			}
			else if ( deletionMethod.compare("delete") == 0
			        or deletionMethod.compare("DELETE") == 0 ) {
				_edm = EDM_Delete;
			}
		}
	} catch ( ... ) {}

	try {
		_defaultPath = configGetString("wo2sc3.defaultPath");
		if ( _defaultPath.size() > 0 ) {
			if ( _defaultPath[_defaultPath.size() - 1] != '/' )
				_defaultPath += '/';
		}
	} catch ( ... ) {
		SEISCOMP_ERROR("wo2sc3.defaultPath variable is not net");
		error++;
	}

	if ( error > 0 ) {
		SEISCOMP_LOG(_errorChannel, "Initialization failed... Check configuration!");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebobsToSeisComP3::runDataHandler(int* csock) {

	int retCode = handleData(csock);

	switch ( retCode ) {
		case 0:
			SEISCOMP_LOG(_infoChannel, "WebObs origin processed without warning");
		break;
		case 1:
			SEISCOMP_LOG(_errorChannel, "Data stream error");
		break;
		case 2:
			SEISCOMP_LOG(_errorChannel, "Station not found in inventory, nothing to do");
		break;
		case 3:
			SEISCOMP_LOG(_errorChannel, "IO error while saving file");
		break;
		case -1:
			SEISCOMP_LOG(_infoChannel, "WebObs origin processed with warning(s)");
		break;
		default:
			SEISCOMP_LOG(_infoChannel, "WebobsToSeisComP3 OK!");
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebobsToSeisComP3::dataHandler(int* csock) {
	_streamThread = new boost::thread(boost::bind(&WebobsToSeisComP3::runDataHandler, this, csock));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebobsToSeisComP3::run() {

	struct sockaddr_in my_addr;
	int hsock;
	int* p_int;

	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if ( hsock == -1 ) {
		SEISCOMP_LOG(_errorChannel, "Error initializing socket %d", errno);
		return false;
	}

	p_int = (int*) malloc(sizeof(int));
	*p_int = 1;

	if ( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*) p_int, sizeof(int)) == -1)
	        || (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*) p_int, sizeof(int)) == -1) ) {
		SEISCOMP_LOG(_errorChannel, "Error setting options %d", errno);
		free(p_int);
		return false;
	}
	free(p_int);

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(_port);

	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	if ( bind(hsock, (sockaddr*) &my_addr, sizeof(my_addr)) == -1 ) {
		SEISCOMP_LOG(_errorChannel, "Socket's binding failed, make sure nothing "
			"else is listening on this port %d [code %d]", _port, errno);
		return false;
	}

	if ( listen(hsock, 10) == -1 ) {
		SEISCOMP_LOG(_errorChannel, "Socket's listening failed [code %d]", errno);
		return false;
	}

	int* csock;
	sockaddr_in sadr;
	socklen_t addr_size = 0;
	addr_size = sizeof(sockaddr_in);

	for (MappedHosts::iterator it = _authorizedHosts.begin();
	        it != _authorizedHosts.end(); ++it)
		SEISCOMP_DEBUG("Allowed host: %s | %s", it->first.c_str(), it->second.c_str());

	SEISCOMP_LOG(_infoChannel, "Server is up and running... Now awaiting clients connections...");

	while ( !isExitRequested() ) {

		csock = (int*) malloc(sizeof(int));
		if ( (*csock = accept(hsock, (sockaddr*) &sadr, &addr_size)) != -1 ) {

			SEISCOMP_LOG(_infoChannel, "-START-----------------------------------------------");
			SEISCOMP_LOG(_infoChannel, "Received connection from %s", inet_ntoa(sadr.sin_addr));

			if ( clientIsAllowed(inet_ntoa(sadr.sin_addr)) )
				runDataHandler(csock);
			else {
				std::string error = "You're not allowed to send data in...";
				SEISCOMP_LOG(_infoChannel, "Client from host %s is not allowed to send in data and has been ignored", inet_ntoa(sadr.sin_addr));
				::send(*csock, error.c_str(), error.size(), 0);
				close(*csock);
			}
			SEISCOMP_LOG(_infoChannel, "------------------------------------------------STOP-");
		}
	}

	SEISCOMP_LOG(_infoChannel, "Closing connection(s) thru socket on port %d", _port);
	::shutdown(hsock, SHUT_RDWR);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebobsToSeisComP3::clientIsAllowed(const char* address) {

	std::map<std::string, std::string>::iterator it = _authorizedHosts.begin();
	for (; it != _authorizedHosts.end(); ++it) {
		if ( (std::string) address == it->first or (std::string) address == it->second )
			return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int WebobsToSeisComP3::handleData(int* csock) {

	int retCode = 0;
	_errorLog = "";

	std::string data;
	char buffer[1024];
	int buffer_len = 1024;
	int bytecount;

	memset(buffer, 0, buffer_len);
	if ( (bytecount = recv(*csock, buffer, buffer_len, 0)) == -1 ) {
		SEISCOMP_LOG(_errorChannel, "Error receiving data %d, exiting.", errno);
		return 1;
	}

	SEISCOMP_LOG(_infoChannel, "Incoming stream [%d bytes]", bytecount);

	data = toString(buffer);

	// We store the stream into a tmp file so it could actually be opened
	// later by the XML parser
	std::string tmpFile = _defaultPath + "/event.tmp";
	try {
		std::ofstream event(tmpFile.c_str());
		event << data;
		event.close();
	}
	catch ( std::exception &e ) {
		SEISCOMP_LOG(_errorChannel, "WebobsToSeisComP3 failed to save tmp file : %s", e.what());
		return 3;
	}


	xmlInitParser();
	LIBXML_TEST_VERSION


	// When comes time to storing the stream as a human readable XML file,
	// we'll want LibXML2 to auto-indent the nodes but this can only
	// happen if whitespace is discarded when the XML is read
	xmlKeepBlanksDefault(0);
	xmlDoc* doc = xmlParseFile(tmpFile.c_str());


	// It's time to check out if the stream is a true XML
	if ( !doc ) {
		SEISCOMP_LOG(_errorChannel, "Failed to read data stream (stream is empty) [file: %s]", tmpFile.c_str());
		_errorLog += "Failed to read data stream (stream is empty)\n";
		return 1;
	}
	xmlXPathContext* xpathCtx = xmlXPathNewContext(doc);

	if ( !xpathCtx ) {
		SEISCOMP_LOG(_errorChannel, "Failed to create the xpath context");
		_errorLog += "[error] Failed to create the xpath context\n";
		return 1;
	}


	// Let's see if it contains the "module description" node.
	// It allows us to verify if this stream is a legit WebobsToSeisComP3 message
	// using "type" and "id" as identifiers
	xmlXPathObject* xpathObj1 = xmlXPathEvalExpression((xmlChar*) "/webObs/moduleDescription", xpathCtx);
	if ( !xpathObj1 ) {
		SEISCOMP_LOG(_errorChannel, "Identification phase failed : wrong file format");
		_errorLog += "[error] Identification phase failed : wrong file format\n";
		return 1;
	}

	xmlNode* node1 = NULL;
	if ( xpathObj1->nodesetval && xpathObj1->nodesetval->nodeTab ) {
		node1 = xpathObj1->nodesetval->nodeTab[0];
	}
	else {
		SEISCOMP_LOG(_errorChannel, "Identification phase failed : can't find node /webObs/moduleDescription");
		_errorLog += "[error] Identification phase failed : can't find node /webObs/moduleDescription\n";
	}

	xmlNode* module;
	xmlChar* value;
	int error = 0;
	for (module = node1->children; module != NULL; module = module->next) {

		// Module type variable
		if ( node1->type == XML_ELEMENT_NODE && !xmlStrcmp(module->name, (const xmlChar *) "type") ) {
			value = xmlNodeGetContent(module);
			if ( !value && (xmlStrEqual(value, (xmlChar*) toString(_module_type).c_str()) != 0) )
				error++;
			xmlFree(value);
		}

		// Module variable
		if ( node1->type == XML_ELEMENT_NODE && !xmlStrcmp(module->name, (const xmlChar *) "id") ) {
			value = xmlNodeGetContent(module);
			if ( !value && (xmlStrEqual(value, (xmlChar*) toString(_module_id).c_str()) != 0) )
				error++;
			xmlFree(value);
		}
	}

	// If no error reported, then it's a legit WebObs to SeisComP3 stream
	// and we can proceed and analyze its content
	if ( error == 0 ) {

		// The stream announces an event creation
		xmlXPathObject* xpathObjAdd = xmlXPathEvalExpression((xmlChar*) "/webObs/eventDescription", xpathCtx);
		if ( xpathObjAdd ) {
			if ( xpathObjAdd->nodesetval && xpathObjAdd->nodesetval->nodeTab ) {
				SEISCOMP_LOG(_infoChannel, "Node /webObs/eventDescription found...");
				if ( createOrigin(doc, xpathObjAdd) )
					retCode = 0;
				else
					retCode = 1;
			}
		}
		else {
			SEISCOMP_LOG(_errorChannel, "Failed to evaluate /webObs/eventDescription");
			_errorLog += "Failed to evaluate /webObs/eventDescription\n";
		}

		// The stream announces an event removal
		xmlXPathObject* xpathObjDel = xmlXPathEvalExpression((xmlChar*) "/webObs/objectRemoval", xpathCtx);
		if ( xpathObjDel ) {
			if ( xpathObjDel->nodesetval && xpathObjDel->nodesetval->nodeTab ) {
				SEISCOMP_LOG(_infoChannel, "Node /webObs/objectRemoval found...");
				if ( removeEvent(xpathObjDel) )
					retCode = 0;
				else
					retCode = 1;
			}
		}
		else {
			SEISCOMP_LOG(_errorChannel, "Failed to evaluate /webObs/objectRemoval");
			_errorLog += "Failed to evaluate /webObs/objectRemoval\n";
		}

	}
	else {
		SEISCOMP_LOG(_errorChannel, "Bad module configuration. nothing to do");
		_errorLog += "[error] Bad module configuration. nothing to do\n";
		retCode = 1;
	}

	xmlXPathFreeContext(xpathCtx);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	handleError(csock);

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebobsToSeisComP3::removeEvent(xmlXPathObject* object) {

	// The data stream is basically a XML file containing a module id,
	// a message id and an event removal instruction.
	// e.g.:
	//		<webObs>
	//		     <moduleDescription>
	//			     <type>1</type>
	//			     <id>1</id>
	//		     </moduleDescription>
	//		     <objectRemoval>
	//			     <eventID>ovsm2012zxyd</eventID>
	//		     </objectRemoval>
	//		</webObs>

	xmlNode* node = NULL;
	if ( object->nodesetval && object->nodesetval->nodeTab )
		node = object->nodesetval->nodeTab[0];


	if ( !node ) {
		xmlXPathFreeObject(object);
		return false;
	}

	std::string eventID;
	xmlNode* child_node;
	xmlChar* field;

	for (child_node = node->children; child_node != NULL;
	        child_node = child_node->next)
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "eventID") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventID = toString(field);
			xmlFree(field);
		}

	if ( eventID.empty() ) {
		SEISCOMP_LOG(_infoChannel, "Received empty objectID, nothing will be deleted");
		xmlXPathFreeObject(object);
		return false;
	}

	PublicObjectPtr obj = query()->getObject(Event::TypeInfo(), eventID);
	EventPtr evt = Event::Cast(obj.get());

	if ( !evt ) {
		SEISCOMP_LOG(_infoChannel, "Object with publicID %s is to be removed, "
			"but has not been found in database", eventID.c_str());
		_errorLog += "Object with publicID " + eventID + " is to be removed, "
				"but has not been found in database";
		return false;
	}

	if ( !_enableEventDeletion ) {
		SEISCOMP_LOG(_infoChannel, "Object with publicID %s has been found but "
			"delete credential is set to FALSE", eventID.c_str());
		_errorLog += "Object with publicID " + eventID + " has been found but "
				"delete credential is set to FALSE";
		xmlXPathFreeObject(object);
		return true;
	}


	if ( _edm & EDM_Delete ) {

		int count = 0;
		if ( evt && query()->removeTree(evt.get(), eventID, &count) ) {
			SEISCOMP_LOG(_infoChannel, "Event %s deleted with %d child(ren)", eventID.c_str(), count);
			_errorLog += "OK";
		}
	}


	if ( _edm & EDM_Hide ) {

		const EventType t = NOT_EXISTING;
		evt->setType(t);
		evt->update();

		NotifierPtr n = new Notifier("EventParameters", DataModel::OP_UPDATE, evt.get());
		NotifierMessagePtr m = new NotifierMessage;
		m->attach(n.get());
		connection()->send(m.get());

		_errorLog += "OK";
	}

	if ( _edm & EDM_None )
		SEISCOMP_LOG(_infoChannel, "Event %s found in database but deletion method is missing", eventID.c_str());

	xmlXPathFreeObject(object);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebobsToSeisComP3::createOrigin(xmlDoc* doc, xmlXPathObject* object) {

	// The data stream is basically a XML file containing a module id,
	// a message id and an event/origin description.
	// e.g.:
	//		<webObs>
	//		     <moduleDescription>
	//			     <type>1</type>
	//			     <id>1</id>
	//		     </moduleDescription>
	//		     <eventDescription>
	//			     <date>2011-12-06</date>
	//			     <time>12:12:12</time>
	//			     <station>MVM</station>
	//			     <files>5</files>
	//			     <occurrences>MVM</occurrences>
	//			     <duration>35</duration>
	//			     <sMinusP>25</sMinusP>
	//			     <amplitude>??</amplitude>
	//			     <type>Tectonique</type>
	//			     <comment>??</comment>
	//		     </eventDescription>
	//		</webObs>

	xmlNode* node = NULL;
	if ( object->nodesetval && object->nodesetval->nodeTab ) {
		node = object->nodesetval->nodeTab[0];
	}
	else {
		SEISCOMP_LOG(_errorChannel, "Failed to find the node /webObs/eventDescription");
		_errorLog += "Failed to find the node /webObs/eventDescription\n";
	}

	if ( !node ) {
		xmlXPathFreeObject(object);
		return false;
	}

	std::string eventYear, eventMonth, eventDay, eventHour, eventMin,
	        eventSec, eventComment, eventOperator, eventSminusp,
	        eventDuration, eventStation, eventNetwork, eventMCID,
	        eventEarthModel, eventAmplitude;

	xmlNode* child_node;
	xmlChar* field;

	for (child_node = node->children; child_node != NULL;
	        child_node = child_node->next) {

		// Date variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "date") ) {
			field = xmlNodeGetContent(child_node);
			if ( field ) {
				try {
					eventYear = toString(field).substr(0, 4);
					eventMonth = toString(field).substr(5, 2);
					eventDay = toString(field).substr(8, 2);
				}
				catch ( std::exception& e ) {
					SEISCOMP_LOG(_errorChannel, "Wrong date format. It should be YEAR/MONTH/DAY");
					_errorLog += "[error] Wrong date format. It should be YEAR/MONTH/DAY\n";
				}
			}
			xmlFree(field);
		}

		// Operator variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "operator") ) {
			field = xmlNodeGetContent(child_node);
			if ( field ) {
				try {
					eventOperator = toString(field);
				}
				catch ( std::exception& e ) {
					SEISCOMP_LOG(_errorChannel, "Operator value is missing. It should not be!");
					_errorLog += "[error] Operator value is missing. It should not be !\n";
				}
			}
			xmlFree(field);
		}

		// Time variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "time") ) {
			field = xmlNodeGetContent(child_node);
			if ( field ) {
				try {
					eventHour = toString(field).substr(0, 2);
					eventMin = toString(field).substr(3, 2);
					eventSec = toString(field).substr(6, 2);
				}
				catch ( std::exception& e ) {
					SEISCOMP_LOG(_errorChannel, "Wrong time format. It should be H:M:S");
					_errorLog += "[error] Wrong time format. It should be H:M:S\n";
				}
			}
			xmlFree(field);
		}

		// Station variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "station") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventStation = toString(field);
			xmlFree(field);
		}

		// Network variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "network") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventNetwork = toString(field);
			xmlFree(field);
		}

		//Type variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "type") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventEarthModel = toString(field);
			xmlFree(field);
		}

		// Comment variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "comment") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventComment = toString(field);
			xmlFree(field);
		}

		// MCID variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "mcid") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventMCID = toString(field);
			else {
				SEISCOMP_LOG(_errorChannel, "No MCID set, there should be one");
				_errorLog += "[error] No MCID set, there should be one\n";
			}
			xmlFree(field);
		}

		// Amplitude variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "amplitude") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventAmplitude = toString(field);
			xmlFree(field);
		}

		// S-P variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "sminusp") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventSminusp = toString(field);
			xmlFree(field);
		}

		// Duration variable
		if ( node->type == XML_ELEMENT_NODE && !xmlStrcmp(child_node->name, (const xmlChar *) "duration") ) {
			field = xmlNodeGetContent(child_node);
			if ( field )
				eventDuration = toString(field);
			xmlFree(field);
		}
	}

	if ( eventYear.empty() || eventHour.empty() ) {
		SEISCOMP_LOG(_errorChannel, "Event time information in stream is malformed, operation ignored");
		xmlXPathFreeObject(object);
		return false;
	}

	SEISCOMP_LOG(_infoChannel, "Stream identified as legit origin from WebObs...");
	SEISCOMP_LOG(_infoChannel, "      Date: %s/%s/%s at %s:%s:%s by %s",
	    eventYear.c_str(), eventMonth.c_str(), eventDay.c_str(),
	    eventHour.c_str(), eventMin.c_str(), eventSec.c_str(),
	    eventOperator.c_str());

	if ( !eventEarthModel.empty() )
		SEISCOMP_LOG(_infoChannel, "      Type: %s", eventEarthModel.c_str());

	if ( !eventComment.empty() )
		SEISCOMP_LOG(_infoChannel, "   Comment: %s", eventComment.c_str());

	SEISCOMP_LOG(_infoChannel, "      MCID: %s", eventMCID.c_str());


	Time time = Time::GMT();
	StationPtr station = NULL;
	std::string locCode, chaCode;

	// Try and get active component (loc+chan) from the bindings if there is
	// any configured. Otherwise, use the first available vertical one.
	ConfigModule* module = configModule();
	if ( module ) {
		for (size_t j = 0; j < module->configStationCount(); ++j) {

			if ( !module->configStation(j) )
				continue;

			if ( module->configStation(j)->networkCode() == eventNetwork
			        && module->configStation(j)->stationCode() == eventStation ) {

				Setup* setup = findSetup(module->configStation(j), name());
				if ( setup ) {
					ParameterSet* ps = ParameterSet::Find(setup->parameterSetID());
					if ( !ps ) {
						SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
						continue;
					}

#if SC_API_VERSION < 0x020000
					Processing::Parameters params;
					params.readFrom(ps);
#else
					Util::KeyValues params;
					params.init(ps);
#endif

					std::string loc, cha;

					params.getString(loc, "detecLocid");
					params.getString(cha, "detecStream");

					locCode = loc;
					if ( !cha.empty() ) {
						bool isFixedChannel = cha.size() > 2;

						if ( !isFixedChannel ) {
							SensorLocation* sloc = Client::Inventory::Instance()->getSensorLocation(eventNetwork, eventStation, loc, time);

							if ( sloc ) {
								Stream* stream = getVerticalComponent(sloc, cha.c_str(), time);
								if ( stream )
									chaCode = stream->code();
								else
									chaCode = cha + 'Z';
							}
							else
								chaCode = cha + 'Z';
						}
						else
							chaCode = cha;
					}
				}
			}
		}
	}

	if ( query() )
		station = query()->getStation(eventNetwork, eventStation, time);

	if ( !station ) {

		SEISCOMP_LOG(_errorChannel, "Station %s/%s not found in inventory, no origin can be created",
		    eventNetwork.c_str(), eventStation.c_str());
		_errorLog += "Station " + eventNetwork + "/" + eventStation +
		        " not found in inventory, no origin can be created";

		xmlXPathFreeObject(object);
		return false;
	}

	if ( eventSminusp.empty() || eventSminusp == "NA" )
		eventSminusp = "not set";

	std::string originComment;

	originComment += "File: " + eventYear + eventMonth + eventDay + "_" +
	        eventHour + eventMin + "00." + _regionTag;

	if ( !eventDuration.empty() )
		originComment += " - Dur: " + eventDuration;

	originComment += " - S-P: " + eventSminusp;
	originComment += " - MCID: " + eventMCID;
	originComment += " - Amplitude: " + eventAmplitude;


	if ( _enableDatabaseStorage ) {

		CreationInfo oci;
		oci.setCreationTime(Time::GMT());
		oci.setAuthor(eventOperator);
		oci.setAgencyID(agencyID());

		Time ot;
		ot.set(stringToInt(eventYear), stringToInt(eventMonth),
		    stringToInt(eventDay), stringToInt(eventHour),
		    stringToInt(eventMin), (int) floor(stringToDouble(eventSec)),
		    (int) ((stringToDouble(eventSec) - floor(stringToDouble(eventSec))) * 1.0E6));

		OriginPtr origin = Origin::Create();
		origin->setCreationInfo(oci);
		origin->setTime(ot);
		origin->setEarthModelID(eventEarthModel);
		origin->setMethodID(eventMCID);
		origin->setEvaluationMode(EvaluationMode(AUTOMATIC));
		origin->setEvaluationStatus(EvaluationStatus(CONFIRMED));
		origin->setLatitude(RealQuantity(station->latitude(), None, None, None, None));
		origin->setLongitude(RealQuantity(station->longitude(), None, None, None, None));

		double dist = .0, azi1 = .0, azi2 = .0;
		Math::Geo::delazi(station->latitude(), station->longitude(),
		    origin->latitude().value(), origin->longitude().value(),
		    &dist, &azi1, &azi2);

		std::string composedComment = originComment + " <> " + eventComment;
		CommentPtr com = new Comment();
		com->setCreationInfo(oci);
		com->setText(composedComment);
		origin->add(com.get());

		// Contingency plan...
		// No binding found, okay use first available vertical component
		if ( locCode.empty() && chaCode.empty() ) {

			SEISCOMP_LOG(_infoChannel, "No local binding found for station %s/%s, will be using first vertical sensor",
			    eventNetwork.c_str(), eventStation.c_str());

			InventoryPtr inv = Client::Inventory::Instance()->inventory();
			if ( inv ) {
				for (size_t i = 0; i < inv->networkCount(); ++i) {
					NetworkPtr network = inv->network(i);

					if ( network->code() != eventNetwork )
						continue;

					for (size_t j = 0; j < network->stationCount(); ++j) {
						StationPtr station = network->station(j);

						if ( station->code() != eventStation )
							continue;

						for (size_t l = 0;
						        l < station->sensorLocationCount(); ++l) {
							SensorLocationPtr sloc = station->sensorLocation(l);

							try {
								if ( sloc->end() <= time ) {
									SEISCOMP_WARNING("Ignored sensor location %s (out of date)", sloc->code().c_str());
									continue;
								}
							} catch ( ... ) {}

							if ( sloc->start() > time )
								continue;

							locCode = sloc->code();

							for (size_t k = 0; k < sloc->streamCount(); ++k) {
								StreamPtr streamtmp = sloc->stream(k);

								try {
									if ( streamtmp->end() <= time )
										continue;
								} catch ( ... ) {}

								if ( streamtmp->start() > time )
									continue;

								if ( streamtmp->code().substr(streamtmp->code().length() - 1, 1) != "Z" )
									continue;

								chaCode = streamtmp->code();
							}
						}
					}
				}
			}
			else
				SEISCOMP_LOG(_errorChannel, "No inventory acquired when trying to locate station %s/%s",
				    eventNetwork.c_str(), eventStation.c_str());
		}

		SEISCOMP_LOG(_infoChannel, "   Station: %s", station->code().c_str());
		SEISCOMP_LOG(_infoChannel, "   Network: %s", station->network()->code().c_str());
		SEISCOMP_LOG(_infoChannel, " Loc. code: %s", locCode.c_str());
		SEISCOMP_LOG(_infoChannel, "   Channel: %s", chaCode.c_str());

		Notifier::SetEnabled(true);
		EventParametersPtr ep = new EventParameters;
		ep->add(origin.get());

		// P Pick creation
		PickPtr pPick = Pick::Create();
		pPick->setCreationInfo(oci);
		pPick->setEvaluationStatus(EvaluationStatus(CONFIRMED));
		pPick->setTime(ot);
		pPick->setEvaluationMode(EvaluationMode(AUTOMATIC));
		pPick->setPhaseHint(Phase("P"));
		pPick->setWaveformID(WaveformStreamID(station->network()->code(), eventStation, locCode, chaCode, ""));
		ep->add(pPick.get());

		// S pick creation
		PickPtr sPick = Pick::Create();
		if ( eventSminusp != "not set" && !eventSminusp.empty() ) {

			long stime = ot.seconds() + stringToInt(eventSminusp);
			TimeSpan ts;
			ts.set(stime);
			Time st = (Time) ts;

			sPick->setCreationInfo(oci);
			sPick->setEvaluationStatus(EvaluationStatus(CONFIRMED));
			sPick->setTime(st);
			sPick->setEvaluationMode(EvaluationMode(AUTOMATIC));
			sPick->setPhaseHint(Phase("S"));
			sPick->setWaveformID(WaveformStreamID(station->network()->code(), eventStation, locCode, chaCode, ""));
			ep->add(sPick.get());
		}

		// P arrival creation
		ArrivalPtr pArrival = new Arrival();
		pArrival->setPickID(pPick->publicID());
		pArrival->setWeight(1.0);
		pArrival->setPhase(Phase("P"));
		pArrival->setCreationInfo(oci);
		pArrival->setDistance(dist);
		pArrival->setAzimuth(azi2);

		origin->add(pArrival.get());
		NotifierMessagePtr msg1 = Notifier::GetMessage(true);
		connection()->send("LOCATION", msg1.get());

		// S arrival creation
		if ( eventSminusp != "not set" && !eventSminusp.empty() ) {

			ArrivalPtr sArrival = new Arrival();
			sArrival->setPickID(sPick->publicID());
			sArrival->setWeight(1.0);
			sArrival->setPhase(Phase("S"));
			sArrival->setCreationInfo(oci);
			sArrival->setDistance(dist);
			sArrival->setAzimuth(azi2);

			origin->add(sArrival.get());
			NotifierMessagePtr msg2 = Notifier::GetMessage(true);
			connection()->send("LOCATION", msg2.get());
		}

		OriginQuality oq;
		oq.setUsedPhaseCount(2);
		oq.setAssociatedPhaseCount(2);
		oq.setAssociatedStationCount(1);
		oq.setDepthPhaseCount(2);
		oq.setUsedStationCount(1);
		origin->setQuality(oq);

		origin->update();

		SEISCOMP_LOG(_infoChannel, "Origin stored with publicID %s", origin->publicID().c_str());
		NotifierMessagePtr msg3 = Notifier::GetMessage(true);
		connection()->send("LOCATION", msg3.get());
		Notifier::SetEnabled(false);
	}
	else
		SEISCOMP_LOG(_infoChannel, "An origin has been received but import is set to FALSE");


	if ( _enableXMLArchive ) {

		std::string archivePath = _defaultPath + "/archive/";
		if ( !Util::pathExists(archivePath) ) {
			if ( !Util::createPath(archivePath) )
				SEISCOMP_LOG(_errorChannel, "WebobsToSeisComP3 failed to create path %s", archivePath.c_str());
		}
		std::string output = archivePath + eventYear + eventMonth + eventDay
		        + "-" + eventHour + eventMin + eventSec + ".xml";
		xmlSaveFormatFileEnc(output.c_str(), doc, "utf-8", 1);
	}

	xmlXPathFreeObject(object);

	_errorLog += "OK";

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebobsToSeisComP3::handleError(int* csock) {

	int bytecount;
	if ( (bytecount = ::send(*csock, _errorLog.c_str(), _errorLog.size(), 0)) != -1 )
		SEISCOMP_LOG(_infoChannel, "Replied to client a %d bytes message", (int ) _errorLog.size());
	else
		SEISCOMP_LOG(_errorChannel, "Error occurred when replying to client [code %d]", errno);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

