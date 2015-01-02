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




#ifndef __IPGP_GUI_OPENGL_GRAPHICRENDERER_H__
#define __IPGP_GUI_OPENGL_GRAPHICRENDERER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/opengl/renderer.h>
#include <ipgp/gui/opengl/vertex.h>
#include <QObject>

namespace IPGP {
namespace Gui {
namespace OpenGL {

/**
 * @class   GraphicRenderer
 * @package IPGP::Gui::OpenGL
 * @brief   OpenGL simple graphic renderer
 *
 * This class provides the implementation of simple vertices rendering
 */
class SC_IPGP_GUI_API GraphicRenderer : public Renderer {

	Q_OBJECT

	public:
		enum RenderingType {
			PointCloud,
			Curve
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		GraphicRenderer(QWidget* = NULL, Qt::WFlags = 0);
		~GraphicRenderer();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void drawWithLight();
		void drawWithoutLight();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void addVertices(const VertexList&);
		void addVertex(const Vertex&);
		void addVertex(Vertex*);

		void setRenderingType(const RenderingType& rm) {
			_rm = rm;
		}
		const RenderingType& renderingType() const {
			return _rm;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		VertexList _vertices;
		GLuint _verticesBuffer;
		RenderingType _rm;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
