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

#include <ipgp/gui/opengl/drawables/arrival.h>
#include <ipgp/gui/opengl/drawables/hypocenter.h>
#include <ipgp/gui/opengl/drawables/station.h>

namespace IPGP {
namespace Gui {
namespace OpenGL {

Arrival::Arrival() :
		_station(NULL), _hypocenter(NULL), _residuals(.0), _autoSelectColor(true) {}

Arrival::~Arrival() {}

void Arrival::setName(const QString& n) {
	_name = n;
}
const QString& Arrival::name() const {
	return _name;
}

void Arrival::setStation(Station* s) {
	_station = s;
}
Station* Arrival::station() {
	return _station;
}

void Arrival::setHypocenter(Hypocenter* h) {
	_hypocenter = h;
}
Hypocenter* Arrival::hypocenter() {
	return _hypocenter;
}

void Arrival::setResiduals(const float& v) {
	_residuals = v;
}
const float& Arrival::residuals() const {
	return _residuals;
}

void Arrival::setPhaseCode(const QString& c) {
	_phaseCode = c;
}
const QString& Arrival::phaseCode() const {
	return _phaseCode;
}


} // namespace OpenGL
} // namespace Gui
} // namespace IPGP
