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

#ifndef __IPGP_OPENGL_TOPOGRAPHYRENDERERSETTINGS_H__
#define __IPGP_OPENGL_TOPOGRAPHYRENDERERSETTINGS_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#ifdef __APPLE__
#include "/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h"
#elif _WIN32 || _WIN64
#include <gl/gl.h>
#else
#include <GL/gl.h>
#endif


namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(TopographyRendererSettings);
class SC_IPGP_GUI_API TopographyRendererSettings {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		//! The user must take care of making the proper conversion before
		//! loading any kind of data... Those structures are meant only for
		//! referencing any two dimensional coordinates with elevation info
		//! into a three dimensional scene.

		struct GeoPosition {
				bool operator==(const GeoPosition&);
				GLfloat min;			//! Minimum value (true WGS84 degrees)
				GLfloat max;			//! Maximum value (true WGS84 degrees)
				GLfloat convRangeMin;	//! Minimum value (OpenGL converted)
				GLfloat convRangeMax;	//! Maximum value (OpenGL converted)
		};

		struct Elevation {
				bool operator==(const Elevation&);
				GLfloat min;	//! Minimum value (true WGS84 degrees/meters)
				GLfloat max;	//! Maximum value (true WGS84 degrees/meters)
				GLfloat convRangeMinBelowZero;//! Minimum value (OpenGL converted)
				GLfloat convRangeMaxBelowZero;//! Maximum value (OpenGL converted)
				GLfloat convRangeMinAboveZero;//! Minimum value (OpenGL converted)
				GLfloat convRangeMaxAboveZero;//! Maximum value (OpenGL converted)
		};

		struct Graticule {
				enum Position {
					AtMaxEle,
					AtSeaLevel,
					AtMinEle
				};
				Graticule() :
						enabled(true), step(1.), decimatedStep(.01), position(AtSeaLevel) {}
				bool enabled;
				GLfloat step; //! in degrees for lat/lon and km for ele
				GLfloat decimatedStep; //! under the degree step
				Position position;
		};

		enum FileType {
			NONE = 0,
			STL_FILE = 1,
			XYZ_FILE = 2
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		TopographyRendererSettings();
		~TopographyRendererSettings();
		bool operator==(const TopographyRendererSettings&);
		bool operator!=(const TopographyRendererSettings&);

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		void setStationsVisible(const bool&);
		const bool& stationsVisible() const;

		void setGraticuleVisible(const bool&);
		const bool& graticuleVisible() const;
		void setTextureVisible(const bool&);
		const bool& textureVisible() const;

		void setLatitude(const GeoPosition&);
		void setLongitude(const GeoPosition&);
		void setElevation(const Elevation&);

		void setLatitudeGraticule(const Graticule&);
		void setLongitudeGraticule(const Graticule&);
		void setElevationGraticule(const Graticule&);

		void setName(const QString&);
		void setFilepath(const QString&);
		void setFileType(const FileType&);

		GeoPosition& latitude();
		const GeoPosition& latitude() const {
			return _lat;
		}
		GeoPosition& longitude();
		const GeoPosition& longitude() const {
			return _lon;
		}
		Elevation& elevation();
		const Elevation& elevation() const {
			return _ele;
		}

		Graticule& latitudeGraticule();
		Graticule& longitudeGraticule();
		Graticule& elevationGraticule();

		QString& name();
		QString filepath();
		FileType& fileType();

		const QString& name() const {
			return _name;
		}
		const QString& filepath() const {
			return _filepath;
		}
		const FileType& fileType() const {
			return _filetype;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		bool _stationsVisible;
		bool _graticuleVisible;
		bool _textureVisible;
		GeoPosition _lat;
		GeoPosition _lon;
		Elevation _ele;
		Graticule _latGraticule;
		Graticule _lonGraticule;
		Graticule _eleGraticule;
		QString _name;
		QString _filepath;
		FileType _filetype;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
