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

#ifndef __IPGP_CORE_SMTPCLIENT_H__
#define __IPGP_CORE_SMTPCLIENT_H__

#include <ipgp/core/api.h>

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>


using boost::asio::ip::tcp;
using namespace boost::archive::iterators;


namespace IPGP {
namespace Core {
namespace Network {


/**
 * @class   SMTPClient
 * @package IPGP::Core::Network
 * @brief   Mail sender
 *
 * This class provides an SMTP client and supports user's authentification.
 * How to make it work?
 * @code
 * 		//! note: username and passwd should be empty if the server doesn't
 * 		//! check for them...
 *
 * 		SMTPClient mailer(hostname, servername, serverport, username, passwd);
 *
 * 		//! Open the socket connection
 * 		mailer.authenticate();
 *
 * 		if ( mailer.hasError() ) {
 * 			//! Something went wrong and the connection has failed...
 * 			//! Check hostname, serverport, username, passwd
 * 			exit 1;
 * 		}
 *
 * 		//! Send the mail
 * 		mailer.send(from, to, subject, content);
 *
 * 		if ( mailer.hasError() ) {
 * 			//! Something went wrong...
 * 			cerr << mailer.errorMessage() << endl;
 * 		}
 *
 * 		//! Close the connection
 * 		mailer.quit();
 * @endcode
 */
class SC_IPGP_CORE_API SMTPClient {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum AuthentificationStatus {
			AUTH_NONE,
			AUTHENTICATING,
			AUTHENTICATED,
			AUTHENTICATIONFAILED
		};

		typedef base64_from_binary<transform_width<const char*, 6, 8> > base64_text;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit SMTPClient(const std::string& serveraddress,
		                    const std::string& servername,
		                    const size_t& port,
		                    const std::string& username = "",
		                    const std::string& userpassword = "");
		~SMTPClient() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void authenticate();
		const bool& hasError() const {
			return _hasError;
		}
		const std::string& errorMessage() const {
			return _errorMessage;
		}
		const bool send(const std::string& from, const std::string& to,
		                const std::string& subject, const std::string& message);
		void quit();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const std::string encodeBase64(const std::string&);

		void handleResolve(const boost::system::error_code& err,
		                   tcp::resolver::iterator endpoint_iterator);

		void writeLine(const std::string&);

		void handleConnect(const boost::system::error_code& err,
		                   tcp::resolver::iterator endpoint_iterator);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::string _serverAddress;
		std::string _serverName;
		std::string _userName;
		std::string _userPassword;
		size_t _serverPort;
		boost::asio::io_service _ioService;
		tcp::resolver _resolver;
		tcp::socket _socket;
		boost::asio::streambuf _request;
		boost::asio::streambuf _response;
		bool _hasError;
		std::string _errorMessage;
		std::ostringstream _responseData;
		std::ostringstream _responseHeader;
		AuthentificationStatus _as;
};


} // namespace Network
} // namespace Core
} // namespace IPGP

#endif
