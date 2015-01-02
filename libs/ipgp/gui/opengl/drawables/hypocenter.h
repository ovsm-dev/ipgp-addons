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

#ifndef __IPGP_OPENGL_HYPOCENTER_H__
#define __IPGP_OPENGL_HYPOCENTER_H__

#include <ipgp/gui/opengl/vertex.h>
#include <ipgp/gui/opengl/topographyrenderersettings.h>
#include <QColor>


namespace IPGP {
namespace Gui {
namespace OpenGL {


class SC_IPGP_GUI_API Hypocenter {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Hypocenter();
		~Hypocenter();
		bool operator==(const Hypocenter&) const;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setName(const QString&);
		const QString& name() const;

		void setGeoPosition(const float& lat, const float& lon,
		                    const float& depth);

		void setDepth(const float&);
		const float& depth() const;

		void setLatitude(const float&);
		const float& latitude() const;

		void setLongitude(const float&);
		const float& longitude() const;

		void setMagnitude(const float&);
		const float& magnitude() const;

		void setGeoPositionUncertainties(const float& lat,
		                                 const float& lon,
		                                 const float& depth);

		void setDepthUncertainy(const float&);
		const float& depthUncertainy() const;

		void setLatitudeUncertainy(const float&);
		const float& latitudeUncertainy() const;

		void setLongitudeUncertainy(const float&);
		const float& longitudeUncertainy() const;


		void setColor(const QColor&);
		const QColor& color() const;

		void setUncertainyColor(const QColor&);
		const QColor& uncertaintyColor() const;

		void setVertex(const Vertex&);
		Vertex& vertex();

		void setRendererSettings(const TopographyRendererSettings&);
		TopographyRendererSettings& rendererSettings();

		/**
		 * @brief Updates hypocenter's spatial coordinates accordingly to
		 *        the current TopographyRendererSettings profile instance.
		 */
		void updateVertex();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		float _depth;
		float _latitude;
		float _longitude;
		float _magnitude;
		float _depthUncertainty;
		float _latitudeUncertainty;
		float _longitudeUncertainty;

		QColor _color;
		QColor _uncertaintyColor;

		QString _name;

		Vertex _vertex;
		TopographyRendererSettings _settings;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
