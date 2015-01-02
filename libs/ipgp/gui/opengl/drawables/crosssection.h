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

#ifndef __IPGP_OPENGL_TOPOGRAPHYMAP_CROSSSECTION_H__
#define __IPGP_OPENGL_TOPOGRAPHYMAP_CROSSSECTION_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/opengl/vertex.h>
#include <ipgp/gui/opengl/topographyrenderersettings.h>
#include <QColor>

namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(CrossSection);
class SC_IPGP_GUI_API CrossSection {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		CrossSection();
		~CrossSection();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setName(const QString&);
		const QString& name() const;

		void setPointA(const float& lon, const float& lat);
		void setPointB(const float& lon, const float& lat);
		void setDepth(const float& min, const float& max);
		void setWidth(const float& v) {
			_width = v;
		}

		void computeCube();

		const float& width() const {
			return _width;
		}

		const Vertex& upperTopLeft() const {
			return _upperTopLeft;
		}
		const Vertex& upperTopRight() const {
			return _upperTopRight;
		}
		const Vertex& upperBottomLeft() const {
			return _upperBottomLeft;
		}
		const Vertex& upperBottomRight() const {
			return _upperBottomRight;
		}

		const Vertex& lowerTopLeft() const {
			return _lowerTopLeft;
		}
		const Vertex& lowerTopRight() const {
			return _lowerTopRight;
		}
		const Vertex& lowerBottomLeft() const {
			return _lowerBottomLeft;
		}
		const Vertex& lowerBottomRight() const {
			return _lowerBottomRight;
		}

		void setCubeColor(const QColor& c) {
			_cubeColor = c;
		}
		const QColor& cubeColor() const {
			return _cubeColor;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		float _latitudeA;
		float _latitudeB;
		float _longitudeA;
		float _longitudeB;
		float _depthMin;
		float _depthMax;
		float _width;
		Vertex _pointA;
		Vertex _pointB;
		Vertex _upperTopLeft;
		Vertex _upperTopRight;
		Vertex _upperBottomLeft;
		Vertex _upperBottomRight;
		Vertex _lowerTopLeft;
		Vertex _lowerTopRight;
		Vertex _lowerBottomLeft;
		Vertex _lowerBottomRight;
		QString _name;
		QColor _cubeColor;
//		TopographyRendererSettings _settings;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
