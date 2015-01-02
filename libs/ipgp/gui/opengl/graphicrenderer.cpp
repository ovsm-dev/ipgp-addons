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




#define SEISCOMP_COMPONENT IPGP_GL_GRAPHICRENDERER

#include <ipgp/gui/opengl/graphicrenderer.h>
#include <ipgp/gui/opengl/gl.h>
#include <seiscomp3/logging/log.h>
#include <QtOpenGL>

namespace IPGP {
namespace Gui {
namespace OpenGL {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GraphicRenderer::GraphicRenderer(QWidget* parent, Qt::WFlags f) :
		Renderer(parent, f), _verticesBuffer(0), _rm(PointCloud) {

	// TODO Auto-generated constructor stub
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GraphicRenderer::~GraphicRenderer() {
	makeCurrent();
	glDeleteLists(_verticesBuffer, 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GraphicRenderer::drawWithLight() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GraphicRenderer::drawWithoutLight() {
	glCallList(_verticesBuffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GraphicRenderer::addVertices(const VertexList& vertices) {

	qDeleteAll(_vertices);
	qCopy(vertices.begin(), vertices.end(), _vertices.begin());

	SEISCOMP_DEBUG("Vertices in buffer: %d", _vertices.size());

	makeCurrent();

	if ( glIsList(_verticesBuffer) )
		glDeleteLists(_verticesBuffer, 1);
	_verticesBuffer = glGenLists(1);

	if ( _verticesBuffer == 0 ) {
		SEISCOMP_DEBUG("Couldn't instantiate OpenGL buffer for vertex rendering");
		return;
	}

	glNewList(_verticesBuffer, GL_COMPILE);

	glLineWidth(1.);
	glBegin(GL_POINTS);
	for (int i = 0; i < _vertices.size() - 1; ++i) {
		qglColor(_vertices.at(i)->color());
		GL::normal(*_vertices.at(i));
		GL::vertex(*_vertices.at(i));
	}
	glEnd();
	glEndList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GraphicRenderer::addVertex(const Vertex&) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GraphicRenderer::addVertex(Vertex*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace OpenGL
} // namespace Gui
} // namespace IPGP
