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

#ifndef __IPGP_OPENGL_TOPOGRAPHYMAP_ARRIVAL_H__
#define __IPGP_OPENGL_TOPOGRAPHYMAP_ARRIVAL_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>

namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(Hypocenter);
DEFINE_IPGP_SMARTPOINTER(Station);

class SC_IPGP_GUI_API Arrival {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Arrival();
		~Arrival();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setName(const QString&);
		const QString& name() const;

		void setStation(Station*);
		Station* station();

		void setHypocenter(Hypocenter*);
		Hypocenter* hypocenter();

		void setResiduals(const float&);
		const float& residuals() const;

		void setPhaseCode(const QString&);
		const QString& phaseCode() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Station* _station;
		Hypocenter* _hypocenter;
		QString _name;
		QString _phaseCode;
		float _residuals;
		bool _autoSelectColor;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
