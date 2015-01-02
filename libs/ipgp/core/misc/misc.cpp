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

#include <ipgp/core/misc/misc.h>
#include <seiscomp3/core/strings.h>
#include <stdlib.h>
#include <iostream>
#ifdef linux
#include <wait.h>
#endif

using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

namespace IPGP {
namespace Core {
namespace Misc {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char* getOperatingSystemName(const std::string& cmd) {

	int buff_size = 32;
	char* buff = new char[buff_size];

	char* ret = NULL;
	std::string str = "";

	int fd[2];
	int old_fd[3];
	if ( pipe(fd) != 0 ) {
		std::cerr << "Couldn't execute pipe() to listen to command results" << std::endl;
		return NULL;
	}

	old_fd[0] = dup(STDIN_FILENO);
	old_fd[1] = dup(STDOUT_FILENO);
	old_fd[2] = dup(STDERR_FILENO);

	int pid = fork();
	switch ( pid ) {
		case 0:
			close(fd[0]);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			dup2(fd[1], STDOUT_FILENO);
			dup2(fd[1], STDERR_FILENO);
			if ( system(cmd.c_str()) != 0 ) {
				std::cerr << "Couldn't execute system() to launch command" << std::endl;
				return NULL;
			}
			//execlp((const char*)cmd, cmd,0);
			close(fd[1]);
			exit(0);
		break;
		case -1:
			std::cerr << "Couldn't execute fork() to fork process" << std::endl;
			exit(1);
		default:
			close(fd[1]);
			dup2(fd[0], STDIN_FILENO);

			int rc = 1;
			while ( rc > 0 ) {
				rc = read(fd[0], buff, buff_size);
				str.append(buff, rc);
				//memset(buff, 0, buff_size);
			}

			ret = new char[strlen((char*) str.c_str())];

			strcpy(ret, (char*) str.c_str());

			waitpid(pid, NULL, 0);
			close(fd[0]);
	}

	dup2(STDIN_FILENO, old_fd[0]);
	dup2(STDOUT_FILENO, old_fd[1]);
	dup2(STDERR_FILENO, old_fd[2]);

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void randomID(char* s, const int length) {

	for (int i = 0; i < length; ++i) {
		int randomChar = rand() % (26 + 26 + 10);
		if ( randomChar < 26 )
			s[i] = 'a' + randomChar;
		else if ( randomChar < 26 + 26 )
			s[i] = 'A' + randomChar - 26;
		else
			s[i] = '0' + randomChar - 26 - 26;
	}

	s[length] = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival* getArrival(const std::string& pickID, const std::vector<ArrivalPtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->pickID().compare(pickID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Pick* getPick(const std::string& pickID, const std::vector<PickPtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->publicID().compare(pickID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Pick* getPick(const std::string& station, const std::string& network,
              const std::string& code, const std::vector<PickPtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( (vector[i].get()->waveformID().networkCode().compare(network) == 0)
		        && (vector[i].get()->waveformID().stationCode().compare(station) == 0)
		        && (vector[i].get()->phaseHint().code().compare(code) == 0) )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Amplitude* getAmplitude(const std::string& pickID,
                        const std::vector<AmplitudePtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->pickID().compare(pickID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* getOrigin(const std::string& originID,
                  const std::vector<OriginPtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->publicID().compare(originID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* getEvent(const std::string& eventID,
                const std::vector<EventPtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->publicID().compare(eventID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Magnitude* getMagnitude(const std::string& magnitudeID,
                        const std::vector<MagnitudePtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->publicID().compare(magnitudeID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationMagnitude* getStationMagnitude(const std::string& amplitudeID,
                                      const std::vector<StationMagnitudePtr>& vector) {

	for (size_t i = 0; i < vector.size(); ++i) {
		if ( vector[i].get()->amplitudeID().compare(amplitudeID) == 0 )
			return vector[i].get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string getH71PickPolarity(std::vector<PickPtr> picks,
                                     const std::string& name,
                                     const std::string& phase) {

	std::string polarity = " ";
	for (unsigned int i = 0; i < picks.size(); ++i) {

		Pick *p = picks.at(i).get();
		if ( (p->phaseHint().code().compare(phase) == 0)
		        && (p->waveformID().stationCode().compare(name) == 0) ) {
			try {
				polarity = p->polarity().toString();
				if ( polarity.compare("negative") == 0 )
					polarity = "D";
				else if ( polarity.compare("positive") == 0 )
					polarity = "U";
				else
					polarity = " ";
			}
			catch ( ... ) {
				}
		}
	}

	return polarity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::DataModel::MagnitudePtr
getMagnitudeFromCache(const std::string& publicID, DatabaseQuery* query) {

	MagnitudePtr mag = Magnitude::Find(publicID);

	if ( !mag ) {
		PublicObjectPtr obj = query->getObject(Magnitude::TypeInfo(), publicID);
		mag = Magnitude::Cast(obj);
	}

	return mag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}

