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
 ************************************************************************/

#ifndef __IPGP_OPENGL_DATAMODEL_TRIANGLE_H___
#define __IPGP_OPENGL_DATAMODEL_TRIANGLE_H___

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/opengl/vertex.h>
#include <QColor>

namespace IPGP {
namespace Gui {
namespace OpenGL {


DEFINE_IPGP_SMARTPOINTER(Triangle);
class SC_IPGP_GUI_API Triangle {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Triangle();
		explicit Triangle(const Vertex& n, const Vertex& a, const Vertex& b, const Vertex& c);
		~Triangle();

	public:
		// ------------------------------------------------------------------
		//  Operators
		// ------------------------------------------------------------------
		Triangle& operator=(const Triangle&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setNormal(const Vertex& n);
		void setVertexA(const Vertex& v);
		void setVertexB(const Vertex& v);
		void setVertexC(const Vertex& v);
		const Vertex& normal() const;
		const Vertex& vertexA() const;
		const Vertex& vertexB() const;
		const Vertex& vertexC() const;

		//! Don't use this method for plotting
		QVector<Vertex> vertexes();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Vertex _n; //! Normal
		Vertex _a; //! Vertex A
		Vertex _b; //! Vertex B
		Vertex _c; //! Vertex C
};

} // namesapce OpenGL
} // namespace Gui
} // namespace IPGP

#endif
