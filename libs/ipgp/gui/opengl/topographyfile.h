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

#ifndef __IPGP_GUI_DATAMODEL_TOPOGRAPHYFILE_H__
#define __IPGP_GUI_DATAMODEL_TOPOGRAPHYFILE_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QVector>
#include <QString>
#include <QColor>


namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(Triangle);
DEFINE_IPGP_SMARTPOINTER(Vertex);

DEFINE_IPGP_SMARTPOINTER(TopographyFile);
/**
 * @class   TopographyFile
 * @package IPGP::Gui
 * @brief   Topography file base class implementation
 */
class SC_IPGP_GUI_API TopographyFile {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Type {
			UNKNOWN,	//! Default type
			STL,		//! STereoLithography file
			XYZ 		//! XYZ file (points cloud)
		};

		typedef QVector<Triangle*> TriangleList;
		typedef QVector<Vertex*> VertexList;
		struct Data {
				Data();
				void clear();
				TriangleList triangles;
				VertexList vertices;
				float minLatitude;
				float maxLatitude;
				float minLongitude;
				float maxLongitude;
				float minElevation;
				float maxElevation;
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		TopographyFile(const Type& = UNKNOWN);
		virtual ~TopographyFile();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		virtual bool read(const QString&)=0;
		void setType(const Type&);
		const Type& type() const;
		const QString& name() const;
		const Data& data() const;
		void reload();

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Type _type;
		Data _data;
		QString _name;
		QString _file;
};



DEFINE_IPGP_SMARTPOINTER(XYZFile);
/**
 * @class   XYZFile
 * @package IPGP::Gui
 * @brief   XYZ file reader
 */
class SC_IPGP_GUI_API XYZFile : public TopographyFile {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		XYZFile();
		~XYZFile();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool read(const QString&);
};



DEFINE_IPGP_SMARTPOINTER(STLFile);
/**
 * @class   STLFile
 * @package IPGP::Gui
 * @brief   STereoLithography file reader
 */
class SC_IPGP_GUI_API STLFile : public TopographyFile {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum {
			STL_LABEL_SIZE = 80
		};

		enum STLError {
			E_NOERROR,
			E_CANTOPEN,
			E_UNESPECTEDEOF
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		STLFile();
		~STLFile();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool read(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const bool isBinarySTL(const QString&);
		const bool isBinarySTLColored(const QString&, bool&);
		QColor getColor(const unsigned short&);
};



} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
