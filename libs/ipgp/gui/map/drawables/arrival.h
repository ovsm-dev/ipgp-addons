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



#ifndef __IPGP_GUI_MAP_ARRIVAL_H__
#define __IPGP_GUI_MAP_ARRIVAL_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>
#include <ipgp/gui/map/drawables/epicenter.h>
#include <ipgp/gui/map/drawables/station.h>

class QString;

namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class Arrival
 * @brief An arrival is basically the representation of a link between an
 *        Epicenter and a Station. P and S phases are supported and used
 *        for station color setting. Gradient of blue and red will be used
 *        to dissociate whether the phase's residuals are positives or negatives.
 * @note  Only P phase residuals are used in the coloration process.
 */
class SC_IPGP_GUI_API Arrival : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Arrival(const QString& phaseCode = QString(),
		                 const qreal& residuals = .0,
		                 const qreal& opacity = .3) :
				_epicenter(0), _station(0), _phaseCode(phaseCode),
				_residuals(residuals), _autoSelectColor(true) {
			setType(Geometry::d_Arrival);
			setOpacity(opacity);
		}

		virtual ~Arrival() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		Epicenter* epicenter();
		void setEpicenter(Epicenter*);

		const QString& phaseCode() const;
		void setPhaseCode(const QString&);

		const qreal& residuals() const;
		void setResiduals(const qreal&);

		bool isAutoSelectColor() const;
		void setAutoSelectColor(const bool&);

		Station* station();
		void setStation(Station*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Epicenter* _epicenter;
		Station* _station;
		QString _phaseCode;
		qreal _residuals;

		bool _autoSelectColor;
};


}
}
}

#endif

