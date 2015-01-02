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



#ifndef __IPGP_GUI_MAP_STATION_H__
#define __IPGP_GUI_MAP_STATION_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>
#include <QMap>

#define StationCircleDefaultSize QSizeF(6.5, 6.5)
#define StationTriangleDefaultSize QSizeF(13., 13.)


namespace IPGP {
namespace Gui {
namespace Map {

class SC_IPGP_GUI_API Station : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum StationShape {
			ss_rectangle,
			ss_ellipse,
			ss_triangle
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Station(const QString& network = QString(),
		                 const QString& locationCode = QString()) :
				_network(network), _locationCode(locationCode),
				_autoSelectColor(false), _shape(ss_ellipse) {
			setType(Geometry::d_Station);
			setBrush(Qt::gray);
		}

		virtual ~Station() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		const bool& isAutoSelectColor() const;
		void setAutoSelectColor(const bool&);

		const QString& locationCode() const;
		void setLocationCode(const QString&);

		const QString& network() const;
		void setNetwork(const QString&);

		const Station::StationShape& shape() const;
		void setShape(const Station::StationShape&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString _network;
		QString _locationCode;
		QMap<QString, QString> _channelCodes;
		bool _autoSelectColor;
		Station::StationShape _shape;
};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif

