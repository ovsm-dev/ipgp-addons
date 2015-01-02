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

#ifndef __IPGP_GUI_MAPDESCRIPTOR_HPP__
#define __IPGP_GUI_MAPDESCRIPTOR_HPP__

#include <ipgp/gui/api.h>
#include <string>
#include <vector>

class SC_IPGP_GUI_API MapDescriptor {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef std::vector<std::string> StringVector;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		MapDescriptor() :
				_tilePattern("%1/osm_%1_%2_%3"), _defaultLatitude(.0),
				_defaultLongitude(.0) {}
		~MapDescriptor() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		inline const StringVector& paths() const {
			return _paths;
		}
		inline const StringVector& names() const {
			return _names;
		}
		inline const std::string platesFilepath() const {
			return _platesFilepath;
		}
		inline const std::string& tilePattern() const {
			return _tilePattern;
		}
		inline const double& defaultLatitude() const {
			return _defaultLatitude;
		}
		inline const double& defaultLongitude() const {
			return _defaultLongitude;
		}
		void setPaths(const StringVector& vector) {
			_paths = vector;
		}
		void setNames(const StringVector& vector) {
			_names = vector;
		}
		void setPlatesFilepath(const std::string& filepath) {
			_platesFilepath = filepath;
		}
		void setTilePattern(const std::string& pattern) {
			_tilePattern = pattern;
		}
		void setDefaultLatitude(const double& value) {
			_defaultLatitude = value;
		}
		void setDefaultLongitude(const double& value) {
			_defaultLongitude = value;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::vector<std::string> _paths;
		std::vector<std::string> _names;
		std::string _tilePattern;
		std::string _platesFilepath;
		double _defaultLatitude;
		double _defaultLongitude;
};


#endif
