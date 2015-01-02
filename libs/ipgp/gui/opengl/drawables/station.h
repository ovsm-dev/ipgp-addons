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

#ifndef __IPGP_OPENGL_TOPOGRAPHYMAP_STATION_H__
#define __IPGP_OPENGL_TOPOGRAPHYMAP_STATION_H__

#include <ipgp/gui/opengl/vertex.h>
#include <ipgp/gui/opengl/topographyrenderersettings.h>
#include <QColor>


namespace IPGP {
namespace Gui {
namespace OpenGL {

class SC_IPGP_GUI_API Station {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Station();
		~Station();
		bool operator==(const Station&) const;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setName(const QString&);
		const QString& name() const;

		void setNetwork(const QString&);
		const QString& network() const;

		void setGeoPosition(const float& lat, const float& lon, const float& ele);

		void setElevation(const float&);
		const float& elevation() const;

		void setLatitude(const float&);
		const float& latitude() const;

		void setLongitude(const float&);
		const float& longitude() const;

		void setColor(const QColor&);
		const QColor& color() const;

		void setVertex(const Vertex&);
		Vertex& vertex();

		void setRendererSettings(const TopographyRendererSettings&);
		TopographyRendererSettings& rendererSettings();

		/**
		 * @brief Updates station's spatial coordinates accordingly to
		 *        the current TopographyRendererSettings profile instance.
		 */
		void updateVertex();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		float _elevation;
		float _latitude;
		float _longitude;

		QColor _color;

		QString _name;
		QString _network;

		Vertex _vertex;
		TopographyRendererSettings _settings;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
