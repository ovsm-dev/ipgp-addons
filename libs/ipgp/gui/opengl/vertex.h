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

#ifndef __IPGP_OPENGL_DATAMODEL_VERTEX__
#define __IPGP_OPENGL_DATAMODEL_VERTEX__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>

#include <QVector>
#include <QColor>


namespace IPGP {
namespace Gui {
namespace OpenGL {


/**
 * @class	Vertex
 * @package	IPGP::Gui
 * @brief	3D vertex of float scalar
 * @todo	Make this class a template for various types (int, double, ...)
 */
DEFINE_IPGP_SMARTPOINTER(Vertex);
class SC_IPGP_GUI_API Vertex {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Vertex();
		Vertex(const float& x, const float& y, const float& z, const QColor& = Qt::white);
		Vertex(const Vertex&);
		Vertex(const float v[3]);
		Vertex(const QVector<float>&);
		~Vertex();

	public:
		// ------------------------------------------------------------------
		//  Operators
		// ------------------------------------------------------------------
		Vertex& operator=(const Vertex&);
		float& operator[](int);
		float operator[](int) const;
		Vertex& operator+=(const Vertex&);
		Vertex& operator-=(const Vertex&);
		Vertex& operator*=(float);
		Vertex& operator/=(float);
		Vertex operator+(const Vertex&) const;
		Vertex operator-(const Vertex&) const;
		Vertex operator*(float) const;
		Vertex operator/(float) const;
		Vertex operator-() const;
		bool operator==(const Vertex&) const;
		bool operator!=(const Vertex&) const;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const float& x() const;
		const float& y() const;
		const float& z() const;
		const QColor& color() const;

		float lenght() const;
		Vertex normalize();

		void setX(const float&);
		void setY(const float&);
		void setZ(const float&);
		void setColor(const QColor&);
		void set(const float& nx, const float& ny, const float& nz);
		void set(const float& val);
		void print(void);

		QVector<float> toQVector();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		float _x;
		float _y;
		float _z;
		QColor _color;
};

typedef QVector<Vertex*> VertexList;

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
