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

#define SEISCOMP_COMPONENT SMTPCLIENT

#include <ipgp/core/network/smtpclient.h>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <iostream>
#include <istream>
#include <ostream>
#include <seiscomp3/logging/log.h>



namespace IPGP {
namespace Core {
namespace Network {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SMTPClient::SMTPClient(const std::string& serveraddress,
                       const std::string& servername, const size_t& port,
                       const std::string& user, const std::string& password) :
		_serverAddress(serveraddress), _serverName(servername),
		_userName(user), _userPassword(password),
		_serverPort(port), _resolver(_ioService), _socket(_ioService),
		_hasError(false), _as(AUTH_NONE) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SMTPClient::authenticate() {

	_as = AUTHENTICATING;
	tcp::resolver::query qry(_serverAddress, boost::lexical_cast<std::string,
	        size_t>(_serverPort));
	_resolver.async_resolve(qry, boost::bind(&SMTPClient::handleResolve,
	    this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
	_ioService.run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SMTPClient::quit() {
	writeLine("QUIT");
	_ioService.stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool SMTPClient::send(const std::string& from, const std::string& to,
                            const std::string& subject, const std::string& message) {

	if ( _as != AUTHENTICATED || _hasError ) {
		SEISCOMP_ERROR("Failed to send email: %s", _errorMessage.c_str());
		return false;
	}

	SEISCOMP_INFO("Preparing to send email to %s", to.c_str());

	writeLine("MAIL FROM: \"" + _serverName + "\"<" + from + ">");
	writeLine("RCPT TO: " + to);
	writeLine("DATA");
	writeLine("From: <" + from + ">");
	writeLine("To: " + to);
	writeLine("Subject: " + subject);
	writeLine("");
	writeLine(message);
	writeLine("");
	writeLine(".");

	return true; //TODO: smtp server response must be checked.
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string SMTPClient::encodeBase64(const std::string& data) {

	std::stringstream os;
	size_t sz = data.size();
	std::copy(base64_text(data.c_str()), base64_text(data.c_str() + sz),
	    ostream_iterator<char>(os));
//	std::copy(base64_text(data.c_str()), base64_text(data.c_str() + sz), ostream_iterator(os));

	return os.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SMTPClient::handleResolve(const boost::system::error_code& err,
                               tcp::resolver::iterator endpoint_iterator) {

	if ( !err ) {
		SEISCOMP_INFO("Email server host resolution done");
		tcp::endpoint endpoint = *endpoint_iterator;
		_socket.async_connect(endpoint, boost::bind(&SMTPClient::handleConnect,
		    this, boost::asio::placeholders::error, ++endpoint_iterator));
	}
	else {
		SEISCOMP_ERROR("Failed to resolve email server host address");
		_hasError = true;
		_errorMessage = err.message();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SMTPClient::writeLine(const std::string& data) {

	std::ostream req_strm(&_request);
	req_strm << data << "\r\n";
	boost::asio::write(_socket, _request);
	req_strm.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SMTPClient::handleConnect(const boost::system::error_code& err,
                               tcp::resolver::iterator endpoint_iterator) {

	if ( !err ) {
		SEISCOMP_INFO("Connected to email host server");
		_hasError = false;
		std::ostream req_strm(&_request);
		writeLine("EHLO " + _serverName);
		writeLine("AUTH LOGIN");
		writeLine(encodeBase64(_userName));
		writeLine(encodeBase64(_userPassword));
		_as = AUTHENTICATED;
	}
	else {
		SEISCOMP_ERROR("Failed to connect to email server host");
		_as = AUTHENTICATIONFAILED;
		_hasError = true;
		_errorMessage = err.message();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Network
} // namespace Core
} // namespace IPGP
