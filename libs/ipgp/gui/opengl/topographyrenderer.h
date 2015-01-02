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

#ifndef __IPGP_OPENGL_DATAMODEL_TOPOGRAPHYRENDERER_H__
#define __IPGP_OPENGL_DATAMODEL_TOPOGRAPHYRENDERER_H__

#include <ipgp/gui/opengl/renderer.h>
#include <ipgp/gui/opengl/vertex.h>
#include <ipgp/gui/opengl/topographyrenderersettings.h>

#include <QObject>
#include <QVector>


QT_FORWARD_DECLARE_CLASS(QActionGroup);


namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(Triangle);
DEFINE_IPGP_SMARTPOINTER(Hypocenter);
DEFINE_IPGP_SMARTPOINTER(Station);
DEFINE_IPGP_SMARTPOINTER(Arrival);
DEFINE_IPGP_SMARTPOINTER(CrossSection);
DEFINE_IPGP_SMARTPOINTER(Triangle);
DEFINE_IPGP_SMARTPOINTER(TopographyFile);
DEFINE_IPGP_SMARTPOINTER(TopographyRenderer);

/**
 * @class   TopographyRenderer
 * @package IPGP::Gui::OpenGL
 * @brief   OpenGL topography renderer
 *
 * This class provides the implementation of topographic data rendering and
 * offers two main supported formats: XYZ (points cloud) and STL (STereoLithography).
 */
class SC_IPGP_GUI_API TopographyRenderer : public Renderer {

	Q_OBJECT

	Q_ENUMS(ViewerMode)
	Q_ENUMS(RenderingType)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum ViewerMode {
			NONE = 0,
			STL_FILE = 1,
			XYZ_FILE = 2
		};

		enum RenderingType {
			POINTCLOUD = 0,
			MESH = 1,
			FILLEDMESH = 2,
			FILLED = 3
		};

		typedef QVector<Triangle*> TriangleList;
		typedef QList<Hypocenter*> HypocenterList;
		typedef QList<Station*> StationList;
		typedef QList<Arrival*> ArrivalList;
		typedef QList<CrossSection*> CrossSectionList;
		typedef QHash<QString, TopographyRendererSettings> SettingsList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit TopographyRenderer(QWidget* = NULL, Qt::WFlags = 0);
		~TopographyRenderer();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void mousePressEvent(QMouseEvent*);

		//! Initializes the texture buffer
		void initTexture();
		//! Over-paints the GL area with whatever...
		void displayInfos(QPainter*);

		void makeGraticule();

		void drawWithLight();
		void drawWithoutLight();

		void drawHypocenters();
		void drawStations();
		void drawArrivals();
		void drawCrossSections();
		void drawGraticule();
		void drawLabels();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		static TopographyRenderer* getInstance() {
			return _instance;
		}
		TopographyFile* file() {
			return _file;
		}

		void append3dText(const Vertex&, const QString&, const QFont&);

		/**
		 * @brief Converts a geographic+elevation point into a Vertex.
		 * @param lon the longitude of the point
		 * @param lat the latitude of the point
		 * @param ele the elevation of the point
		 * @return the Vertex corresponding to the point
		 */
		Vertex getVertex(const GLfloat& lon, const GLfloat& lat,
		                 const GLfloat& ele);

		/**
		 * @brief Converts a Vertex into a geographic+elevation point.
		 * @param v the Vertex to convert
		 * @param aboveZero specifies if the point is above sea level
		 * @return the Point stored inside a non referenced Vertex
		 */
		Vertex getPointFromVertex(const Vertex&,
		                          const bool& aboveZero = false);

		//! Updates objects without recalculating their position
		void updateHypocenters();
		void updateStations();
		void updateArrivals();
		void updateCrossSections();
		void updateEverything() {
			updateHypocenters();
			updateStations();
			updateArrivals();
			updateCrossSections();
		}

		void reassessObjectsPosition();

		void loadSTL(const QString&);
		void loadXYZ(const QString&);

		void setRenderingType(const RenderingType&);
		RenderingType& renderingType() {
			return _rendering;
		}

		void setActiveSettings(const TopographyRendererSettings&);
		TopographyRendererSettings& activeSettings() {
			return _activeSettings;
		}
		void setAvailableSettings(const SettingsList&);
		const SettingsList& availableSettings() const {
			return _availableSettings;
		}

		bool addHypocenter(Hypocenter*);
		Hypocenter* getHypocenter(const QString&);
		void clearHypocenters();

		/**
		 * @brief Adds an arrival object on the map
		 * @param the arrival's pointer
		 * @param should we add it to the canvas directly(true) or in local
		 *        arrival list (false)
		 */
		bool addArrival(Arrival*);
		Arrival* getArrival(const QString&, const QString&);
		void clearArrivals();

		/**
		 * @brief Adds a station object on the map
		 * @param the station's pointer
		 * @param should we add it to the canvas directly(true) or in local
		 *        arrival list (false)
		 */
		bool addStation(Station*);
		Station* getStation(const QString&, const QString&);
		void clearStations();


		bool addCrossSection(CrossSection*);
		CrossSection* getCrossSection(const QString&);
		void clearCrossSections();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void processData();
		void updateTexturedData();
		bool appendVertex(const Vertex&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void setGraticuleVisible(const bool&);
		void setTextureVisible(const bool&);
		void settingsChanged();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void currentSettingsChanged();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		static TopographyRenderer* _instance;
		TopographyFile* _file;
		TriangleList _triangles;
		VertexList _vertices;

		HypocenterList _hypocenters;
		ArrivalList _arrivals;
		StationList _stations;
		CrossSectionList _crossSections;

		GLuint _pointsCloud;
		GLuint _mesh;
		GLuint _filled;
		GLuint _filledMesh;
		GLuint _graticule;
		GLuint _texture;
		GLuint _oHypocenters;
		GLuint _oStations;
		GLuint _oArrivals;
		GLuint _oCrossSections;

		QVector<QVector2D> _texCoords;

		GLfloat _minLongitude;
		GLfloat _maxLongitude;
		GLfloat _minLatitude;
		GLfloat _maxLatitude;
		GLfloat _minElevation;
		GLfloat _maxElevation;

		GLfloat _vObjCoeff;

		SettingsList _availableSettings;
		TopographyRendererSettings _activeSettings;
		ViewerMode _viewer;
		RenderingType _rendering;

		QMenu _contextMenu;
		QActionGroup* _fileGroup;

		QString _filename;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
