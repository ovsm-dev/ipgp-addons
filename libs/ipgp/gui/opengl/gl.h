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

#ifndef __IPGP_OPENGL_GL_H__
#define __IPGP_OPENGL_GL_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QColor>

#ifdef __APPLE__
#include "/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h"
#elif _WIN32 || _WIN64
#include <gl/gl.h>
#else
#include <GL/gl.h>
#endif




QT_FORWARD_DECLARE_CLASS(QString);


namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(Vertex);

namespace GL {

void normal(const GLfloat& x, const GLfloat& y, const GLfloat& z);
void normal(const Vertex&);

void vertex(const GLfloat& x, const GLfloat &y, const GLfloat& z);
void vertex(const Vertex& vec);

/**
 * @brief Draws a vertex in polar coordinates
 */
void vertex(const GLfloat& th2, const GLfloat& ph2);

void translate(const GLfloat& x, const GLfloat& y, const GLfloat& z);
void rotate(const GLfloat& x, const GLfloat& y, const GLfloat& z);
void scale(const GLfloat& x, const GLfloat& y, const GLfloat& z);

void color(const GLfloat& r, const GLfloat& g, const GLfloat& b, const GLfloat& a = 1.);
void color(const Vertex& vec, const GLfloat& a = 1.);

GLenum checkError(const QString& file, const int& line);

void useVBO(bool enable);

enum Direction {
	UP, DOWN, LEFT, RIGHT, TOFRONT, TOBACK
};
void cone(const GLfloat& x, const GLfloat& y, const GLfloat& z,
          const GLfloat& height, const GLfloat& radius, const Direction& = UP,
          const QColor& color = Qt::white);

/**
 * @brief Draws a mono-colored sphere at a certain position
 * @param radius the radius percentage of the sphere (1/radius)
 * @param x The X position of the center
 * @param y the Y position of the center
 * @param z The Z position of the center
 * @param color The color of the sphere
 * @note  The normals are calibrated standardly
 */
void drawSphere(GLfloat radius, const GLfloat& x, const GLfloat& y,
                const GLfloat& z, const QColor& color = Qt::white);



void circle(const GLfloat& cx, const GLfloat& cy, const GLfloat& r, const int& num_segments);


void tetrahedron(const GLfloat& x, const GLfloat& y, const GLfloat& z,
                 const GLfloat& size, const QColor& c = Qt::yellow);

/**
 * @brief Re-maps a value from a certain range to a new one. OpenGL ES 2.x
 *        only supports values from [-1;1], one should convert anything that
 *        overlaps this range before plotting whatever...
 * @param x The value to remap
 * @param oMin The original range minimum value
 * @param oMax The origin range maximum value
 * @param nMin The new range minimum value
 * @param nMax The new range maximum value
 * @return The converted number value within the new range
 */
double remap(const double& x, const double& oMin, const double& oMax,
             const double& nMin, const double& nMax);

} // namespace GL
} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
