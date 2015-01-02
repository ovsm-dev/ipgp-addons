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

#define SEISCOMP_COMPONENT IPGP_GL_TOPORENDERER

#include <ipgp/gui/opengl/topographyrenderer.h>
#include <ipgp/gui/opengl/topographyfile.h>
#include <ipgp/gui/opengl/camera.h>
#include <ipgp/gui/opengl/triangle.h>
#include <ipgp/gui/opengl/gl.h>
#include <ipgp/gui/opengl/drawables/arrival.h>
#include <ipgp/gui/opengl/drawables/hypocenter.h>
#include <ipgp/gui/opengl/drawables/station.h>
#include <ipgp/gui/opengl/drawables/crosssection.h>
#include <ipgp/core/math/math.h>
#include <ipgp/core/file/file.h>
#include <ipgp/gui/misc/misc.h>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/timer.h>

#include <QtGui>
#include <QtOpenGL>
#include <QMatrix4x4>
#include <qmath.h>




using namespace Seiscomp;
using namespace Seiscomp::Core;


namespace {

QString getString(const unsigned char* pBufCrypt) {
	QString ret = "";
	while ( *pBufCrypt ) {
		ret.append(QChar(*pBufCrypt));
		++pBufCrypt;
	}
	return ret;
}

template<typename T> void clearList(QList<T*>& l) {
//	if ( l.size() == 0 ) return;
	qDeleteAll(l);
	l.clear();
}

template<typename T> void clearVector(QVector<T*>& v) {
//	if ( v.size() == 0 ) return;
	qDeleteAll(v);
	v.clear();
}



}

namespace IPGP {
namespace Gui {
namespace OpenGL {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! Singleton
TopographyRenderer* TopographyRenderer::_instance = NULL;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyRenderer::TopographyRenderer(QWidget* parent, Qt::WFlags f) :
		Renderer(parent, f), _file(NULL), _fileGroup(NULL) {

	// Mutliple instances can be run
	_instance = this;

	setObjectName("TopographyRenderer");

	_minLongitude = _maxLongitude = _minLatitude = _maxLatitude = _minElevation
	        = _maxElevation = .0;
	_viewer = NONE;

	_pointsCloud = _mesh = _filled = _filledMesh = _graticule = _oHypocenters
	        = _oArrivals = _oStations = _oCrossSections = 0;

	_rendering = POINTCLOUD;

	initTexture();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyRenderer::~TopographyRenderer() {

	makeCurrent();

	glDeleteLists(_pointsCloud, 1);
	glDeleteLists(_mesh, 1);
	glDeleteLists(_filledMesh, 1);
	glDeleteLists(_filled, 1);
	glDeleteLists(_graticule, 1);
	glDeleteLists(_texture, 1);
	glDeleteLists(_oArrivals, 1);
	glDeleteLists(_oCrossSections, 1);
	glDeleteLists(_oHypocenters, 1);
	glDeleteLists(_oStations, 1);

	::clearVector(_triangles);
	::clearVector(_vertices);
	::clearList(_stations);
	::clearList(_arrivals);
	::clearList(_crossSections);
	::clearList(_hypocenters);

	if ( _file )
	    delete _file;
	_file = NULL;

	if ( _instance == this )
	    _instance = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::mousePressEvent(QMouseEvent* event) {

	Renderer::mousePressEvent(event);

	if ( !mouseGrabbed() )
	    if ( event->buttons() & Qt::RightButton )
	        _contextMenu.exec(mapToGlobal(event->pos()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::initTexture() {

	makeCurrent();

	glEnable(GL_TEXTURE_2D);

//	_texture = bindTexture(QPixmap(QString(":images/terrain.jpg")));
	_texture = bindTexture(QPixmap(QString(":images/land_tex.bmp")));
	glBindTexture(GL_TEXTURE_2D, _texture);

	//! Set nearest filtering mode for texture minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//! Set bilinear filtering mode for texture magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//! Wrap texture coordinates by repeating
	//! f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 204, 204, 0,
//	GL_BGR_EXT, GL_UNSIGNED_BYTE, QString(":images/terrain.jpg").toStdString().c_str());

	glDisable(GL_TEXTURE_2D);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::displayInfos(QPainter* painter) {

	painter->save();
	painter->setPen(Qt::white);
	painter->drawText(QPoint(10, 15), QString("%1 - %2 - %3")
	        .arg(getString(glGetString(GL_VENDOR)))
	        .arg(getString(glGetString(GL_RENDERER)))
	        .arg(getString(glGetString(GL_VERSION))));
	painter->restore();

	Renderer::displayInfos(painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::makeGraticule() {

	makeCurrent();

	if ( glIsList(_graticule) )
	    glDeleteLists(_graticule, 1);
	_graticule = glGenLists(1);

	if ( _graticule == 0 ) {
		SEISCOMP_ERROR("Renderer couldn't create the OpenGL list for the map graticule");
		return;
	}

	GLfloat zele;
	switch ( _activeSettings.longitudeGraticule().position ) {
		case TopographyRendererSettings::Graticule::AtSeaLevel:
			zele = .0;
		break;
		case TopographyRendererSettings::Graticule::AtMaxEle:
			zele = _activeSettings.elevation().convRangeMaxAboveZero;
		break;
		case TopographyRendererSettings::Graticule::AtMinEle:
			zele = _activeSettings.elevation().convRangeMinBelowZero;
		break;
	}

	glNewList(_graticule, GL_COMPILE);


	qglColor(Qt::white);

	glLineStipple(1, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);


	if ( _activeSettings.longitudeGraticule().enabled ) {

		GLfloat minLon = GL::remap(_minX, _activeSettings.longitude().convRangeMin,
		    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
		    _activeSettings.longitude().max);
		GLfloat maxLon = GL::remap(_maxX, _activeSettings.longitude().convRangeMin,
		    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
		    _activeSettings.longitude().max);
		if ( abs(maxLon - minLon) > 2 ) {
			for (int i = (int) minLon + 1; i < (int) maxLon;
			        i += (int) _activeSettings.longitudeGraticule().step) {
				GLfloat v = GL::remap(i, _activeSettings.longitude().min,
				    _activeSettings.longitude().max, _activeSettings.longitude().convRangeMin,
				    _activeSettings.longitude().convRangeMax);
				glBegin(GL_LINES);
				GL::vertex(v, _minY, zele);
				GL::vertex(v, _maxY, zele);
				glEnd();
			}
		}
		else {
			for (GLfloat i = minLon + _activeSettings.longitudeGraticule().decimatedStep;
			        i < maxLon;
			        i += _activeSettings.longitudeGraticule().decimatedStep) {
				GLfloat v = GL::remap(i, _activeSettings.longitude().min,
				    _activeSettings.longitude().max, _activeSettings.longitude().convRangeMin,
				    _activeSettings.longitude().convRangeMax);
				glBegin(GL_LINES);
				GL::vertex(v, _minY, zele);
				GL::vertex(v, _maxY, zele);
				glEnd();
			}
		}
	}

	switch ( _activeSettings.latitudeGraticule().position ) {
		case TopographyRendererSettings::Graticule::AtSeaLevel:
			zele = .0;
		break;
		case TopographyRendererSettings::Graticule::AtMaxEle:
			zele = _activeSettings.elevation().convRangeMaxAboveZero;
		break;
		case TopographyRendererSettings::Graticule::AtMinEle:
			zele = _activeSettings.elevation().convRangeMinBelowZero;
		break;
	}

	if ( _activeSettings.latitudeGraticule().enabled ) {

		GLfloat minLat = GL::remap(_minY, _activeSettings.latitude().convRangeMin,
		    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
		    _activeSettings.latitude().max);
		GLfloat maxLat = GL::remap(_maxY, _activeSettings.latitude().convRangeMin,
		    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
		    _activeSettings.latitude().max);
		if ( abs(maxLat - minLat) > 2 ) {
			for (int i = (int) minLat + 1; i < (int) maxLat;
			        i += (int) _activeSettings.latitudeGraticule().step) {
				GLfloat v = GL::remap(i, _activeSettings.latitude().min,
				    _activeSettings.latitude().max, _activeSettings.latitude().convRangeMin,
				    _activeSettings.latitude().convRangeMax);
				glBegin(GL_LINES);
				GL::vertex(_minX, v, zele);
				GL::vertex(_maxX, v, zele);
				glEnd();
			}
		}
		else {
			for (GLfloat i = minLat + _activeSettings.latitudeGraticule().decimatedStep;
			        i < maxLat;
			        i += _activeSettings.latitudeGraticule().decimatedStep) {
				GLfloat v = GL::remap(i, _activeSettings.latitude().min,
				    _activeSettings.latitude().max, _activeSettings.latitude().convRangeMin,
				    _activeSettings.latitude().convRangeMax);
				glBegin(GL_LINES);
				GL::vertex(_minX, v, zele);
				GL::vertex(_maxX, v, zele);
				glEnd();
			}
		}
	}

	zele = .0;
	if ( _activeSettings.elevationGraticule().enabled ) {
		GLfloat minEleB = GL::remap(_minZ, _activeSettings.elevation().convRangeMinBelowZero,
		    _activeSettings.elevation().convRangeMaxBelowZero, _activeSettings.elevation().min,
		    _activeSettings.elevation().max);
		GLfloat maxEleA = GL::remap(_maxZ, _activeSettings.elevation().convRangeMinAboveZero,
		    _activeSettings.elevation().convRangeMaxAboveZero, _activeSettings.elevation().min,
		    _activeSettings.elevation().max);

		if ( abs(minEleB) > _activeSettings.elevationGraticule().step ) {
			for (GLfloat i = minEleB; i < zele;
			        i += _activeSettings.elevationGraticule().step) {
				GLfloat v = GL::remap(i, _activeSettings.elevation().min,
				    _activeSettings.elevation().max, _activeSettings.elevation().convRangeMinBelowZero,
				    _activeSettings.elevation().convRangeMaxBelowZero);
				glBegin(GL_LINES);
				GL::vertex(_minX, _minY, v);
				GL::vertex(_maxX, _minY, v);
				glEnd();
			}
		}
		else if ( abs(maxEleA) > _activeSettings.elevationGraticule().step ) {
			for (GLfloat i = zele; i < maxEleA;
			        i += _activeSettings.elevationGraticule().decimatedStep) {
				GLfloat v = GL::remap(i, _activeSettings.elevation().min,
				    _activeSettings.elevation().max, _activeSettings.elevation().convRangeMinAboveZero,
				    _activeSettings.elevation().convRangeMaxAboveZero);
				glBegin(GL_LINES);
				GL::vertex(_minX, _minY, v);
				GL::vertex(_maxX, _minY, v);
				glEnd();
			}
		}
	}

	glDisable(GL_LINE_STIPPLE);
	glEndList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawWithLight() {

	//! Draw drawables first
	drawHypocenters();
	drawStations();

	//! Over-paint them with the context
	switch ( _rendering ) {
		case POINTCLOUD:
			// Do we really need light on points??
//			glCallList(_pointsCloud);
		break;
		case MESH:
			glCallList(_mesh);
		break;
		case FILLED:
			glCallList(_filled);
		break;
		case FILLEDMESH:
			glCallList(_filledMesh);
		break;
		default:
			break;
	}


//	GL::tetrahedron(.5, .5, .5, .02);

//	GL::drawSphere(.2, .5, .5, .5);
//	GL::circle(-.5, -.5, .2, 20);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawWithoutLight() {

	if ( _activeSettings.graticuleVisible() )
	    drawGraticule();

	if ( _rendering == POINTCLOUD )
	    glCallList(_pointsCloud);

	drawArrivals();
	drawCrossSections();

	drawLabels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawHypocenters() {
	glCallList(_oHypocenters);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawStations() {
	glCallList(_oStations);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawArrivals() {
	glCallList(_oArrivals);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawCrossSections() {
	glCallList(_oCrossSections);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawGraticule() {
	glCallList(_graticule);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::drawLabels() {

	QFont font(this->font());
	font.setPointSize(11);


	GLfloat zele;
	switch ( _activeSettings.longitudeGraticule().position ) {
		case TopographyRendererSettings::Graticule::AtSeaLevel:
			zele = .0;
		break;
		case TopographyRendererSettings::Graticule::AtMaxEle:
			zele = _activeSettings.elevation().convRangeMaxAboveZero;
		break;
		case TopographyRendererSettings::Graticule::AtMinEle:
			zele = _activeSettings.elevation().convRangeMinBelowZero;
		break;
	}

	GLfloat lblSpace = .01;

	qglColor(Qt::white);

	//! Longitudes (X)
	if ( _activeSettings.graticuleVisible() && _activeSettings.longitudeGraticule().enabled ) {
		GLfloat minLon = GL::remap(_minX, _activeSettings.longitude().convRangeMin,
		    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
		    _activeSettings.longitude().max);
		GLfloat maxLon = GL::remap(_maxX, _activeSettings.longitude().convRangeMin,
		    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
		    _activeSettings.longitude().max);
		if ( fabs(maxLon - minLon) > 2 ) {
			for (int i = (int) minLon + 1; i < (int) maxLon; i += 1) {
				GLfloat v = GL::remap(i, _activeSettings.longitude().min,
				    _activeSettings.longitude().max, _activeSettings.longitude().convRangeMin,
				    _activeSettings.longitude().convRangeMax);
				append3dText(Vertex(v + lblSpace, _maxY, zele), QString(
				    (i < .0) ? " %1 W" : " %1 E").arg(i), font);
			}
		}
		else {
			font.setPointSize(9);
			for (GLfloat i = minLon + _activeSettings.longitudeGraticule().decimatedStep;
			        i < maxLon;
			        i += _activeSettings.longitudeGraticule().decimatedStep) {
				GLfloat v = GL::remap(i, _activeSettings.longitude().min,
				    _activeSettings.longitude().max, _activeSettings.longitude().convRangeMin,
				    _activeSettings.longitude().convRangeMax);
				append3dText(Vertex(v + lblSpace, _maxY, zele), QString(
				    (i < .0) ? " %1 W" : " %1 E").arg(QString::number(i, 'f', 2)), font);
			}
		}
	}

	switch ( _activeSettings.latitudeGraticule().position ) {
		case TopographyRendererSettings::Graticule::AtSeaLevel:
			zele = .0;
		break;
		case TopographyRendererSettings::Graticule::AtMaxEle:
			zele = _activeSettings.elevation().convRangeMaxAboveZero;
		break;
		case TopographyRendererSettings::Graticule::AtMinEle:
			zele = _activeSettings.elevation().convRangeMinBelowZero;
		break;
	}

	//! Latitude (Y)
	if ( _activeSettings.graticuleVisible() && _activeSettings.latitudeGraticule().enabled ) {
		GLfloat minLat = GL::remap(_minY, _activeSettings.latitude().convRangeMin,
		    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
		    _activeSettings.latitude().max);
		GLfloat maxLat = GL::remap(_maxY, _activeSettings.latitude().convRangeMin,
		    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
		    _activeSettings.latitude().max);
		if ( fabs(maxLat - minLat) > 2 ) {
			for (int i = (int) minLat + 1; i < (int) maxLat; i += 1) {
				GLfloat v = GL::remap(i, _activeSettings.latitude().min,
				    _activeSettings.latitude().max, _activeSettings.latitude().convRangeMin,
				    _activeSettings.latitude().convRangeMax);
				append3dText(Vertex(_maxX + lblSpace, v, zele), QString(
				    (i < .0) ? " %1 S" : " %1 N").arg(i), font);
			}
		}
		else {
			font.setPointSize(9);
			for (GLfloat i = minLat + _activeSettings.latitudeGraticule().decimatedStep;
			        i < maxLat;
			        i += _activeSettings.latitudeGraticule().decimatedStep) {
				GLfloat v = GL::remap(i, _activeSettings.latitude().min,
				    _activeSettings.latitude().max, _activeSettings.latitude().convRangeMin,
				    _activeSettings.latitude().convRangeMax);
				append3dText(Vertex(_maxX + lblSpace, v, zele), QString(
				    (i < .0) ? " %1 S" : " %1 N").arg(QString::number(i, 'f', 2)), font);
			}
		}
	}

	zele = .0;
	//! Elevation (Z)
	if ( _activeSettings.graticuleVisible() && _activeSettings.elevationGraticule().enabled ) {

		font.setPointSize(9);

		GLfloat maxEleA = GL::remap(_maxZ, _activeSettings.elevation().convRangeMinAboveZero,
		    _activeSettings.elevation().convRangeMaxAboveZero, _activeSettings.elevation().min,
		    _activeSettings.elevation().max);

		GLfloat minEleB = GL::remap(_minZ, _activeSettings.elevation().convRangeMinBelowZero,
		    _activeSettings.elevation().convRangeMaxBelowZero, _activeSettings.elevation().min,
		    _activeSettings.elevation().max);

		if ( fabs(minEleB) > _activeSettings.elevationGraticule().step ) {
			for (GLfloat i = minEleB; i < zele;
			        i += _activeSettings.elevationGraticule().step) {
				GLfloat v = GL::remap(i, _activeSettings.elevation().min,
				    _activeSettings.elevation().max, _activeSettings.elevation().convRangeMinBelowZero,
				    _activeSettings.elevation().convRangeMaxBelowZero);
				append3dText(Vertex(_maxX + lblSpace, _minY, v),
				    (fabs(i) < 1000.) ? QString("%1 m").arg((int) i) :
				            QString("%1 km").arg((int) (i * pow(10, -3))), font);
			}
		}
		else if ( fabs(maxEleA) > _activeSettings.elevationGraticule().decimatedStep ) {
			for (GLfloat i = zele; i < maxEleA;
			        i += _activeSettings.elevationGraticule().decimatedStep) {
				GLfloat v = GL::remap(i, _activeSettings.elevation().min,
				    _activeSettings.elevation().max, _activeSettings.elevation().convRangeMinAboveZero,
				    _activeSettings.elevation().convRangeMaxAboveZero);
				append3dText(Vertex(_maxX + lblSpace, _minY, v), QString("%1 m").arg((int) i), font);
			}
		}

	}

	font.setBold(true);
	font.setPointSize(9);
	QListIterator<Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Station* s = itS.next();
		append3dText(s->vertex() + Vertex(.0, .0, .01), s->name(), font);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::append3dText(const Vertex& v, const QString& str,
                                      const QFont& f) {
	renderText(v.x(), v.z(), -v.y(), str, f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Vertex TopographyRenderer::getVertex(const GLfloat& lon, const GLfloat& lat,
                                     const GLfloat& ele) {

	double x = GL::remap(lon, _activeSettings.longitude().min, _activeSettings.longitude().max,
	    _activeSettings.longitude().convRangeMin, _activeSettings.longitude().convRangeMax);
	double y = GL::remap(lat, _activeSettings.latitude().min, _activeSettings.latitude().max,
	    _activeSettings.latitude().convRangeMin, _activeSettings.latitude().convRangeMax);
	double z;
	(ele < .0) ?
	        z = GL::remap(ele, _activeSettings.elevation().min, _activeSettings.elevation().max,
	            _activeSettings.elevation().convRangeMinBelowZero, _activeSettings.elevation().convRangeMaxBelowZero) :
	        z = GL::remap(ele, _activeSettings.elevation().min, _activeSettings.elevation().max,
	            _activeSettings.elevation().convRangeMinAboveZero, _activeSettings.elevation().convRangeMaxAboveZero);

	return Vertex(x, y, z);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Vertex TopographyRenderer::getPointFromVertex(const Vertex& v,
                                              const bool& aboveZero) {

	double lon = GL::remap(v.x(), _activeSettings.longitude().convRangeMin,
	    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
	    _activeSettings.longitude().max);
	double lat = GL::remap(v.y(), _activeSettings.latitude().convRangeMin,
	    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
	    _activeSettings.latitude().max);
	double ele;
	(aboveZero) ?
	        ele = GL::remap(v.z(), _activeSettings.elevation().convRangeMinAboveZero,
	            _activeSettings.elevation().convRangeMaxAboveZero, _activeSettings.elevation().min,
	            _activeSettings.elevation().max) :
	        ele = GL::remap(v.z(), _activeSettings.elevation().convRangeMinBelowZero,
	            _activeSettings.elevation().convRangeMaxBelowZero, _activeSettings.elevation().min,
	            _activeSettings.elevation().max);

	return Vertex(lon, lat, ele);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::updateHypocenters() {

	makeCurrent();

	if ( glIsList(_oHypocenters) )
	    glDeleteLists(_oHypocenters, 1);
	_oHypocenters = glGenLists(1);

	if ( _oHypocenters == 0 ) return;

	glNewList(_oHypocenters, GL_COMPILE);



	QListIterator<Hypocenter*> itH(_hypocenters);
	while ( itH.hasNext() ) {
		Hypocenter* h = itH.next();
		GL::drawSphere(((4.9 * (h->magnitude() - 1.2)) / 2.) * _vObjCoeff,
		    h->vertex().x(), h->vertex().y(), h->vertex().z(), h->color());
	}
	glEndList();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::updateStations() {

	makeCurrent();

	if ( glIsList(_oStations) )
	    glDeleteLists(_oStations, 1);
	_oStations = glGenLists(1);

	if ( _oStations == 0 ) return;

	glNewList(_oStations, GL_COMPILE);
	QListIterator<Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Station* s = itS.next();
		GL::drawSphere(_vObjCoeff, s->vertex().x(), s->vertex().y(), s->vertex().z(), s->color());
	}
	glEndList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::updateArrivals() {

	makeCurrent();

	if ( glIsList(_oArrivals) )
	    glDeleteLists(_oArrivals, 1);
	_oArrivals = glGenLists(1);

	if ( _oArrivals == 0 ) return;

	glNewList(_oArrivals, GL_COMPILE);

	QListIterator<Arrival*> itA(_arrivals);
	while ( itA.hasNext() ) {
		Arrival* a = itA.next();

		glBegin(GL_LINES);
		qglColor(Qt::white);
		GL::normal(a->station()->vertex());
		GL::vertex(a->station()->vertex());
		GL::normal(a->hypocenter()->vertex());
		GL::vertex(a->hypocenter()->vertex());

		glEnd();
	}
	glEndList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::updateCrossSections() {

	makeCurrent();

	if ( glIsList(_oCrossSections) )
	    glDeleteLists(_oCrossSections, 1);
	_oCrossSections = glGenLists(1);

	if ( _oCrossSections == 0 ) return;

	glNewList(_oCrossSections, GL_COMPILE);

	glLineWidth(1.5);

	QListIterator<CrossSection*> it(_crossSections);
	while ( it.hasNext() ) {
		CrossSection* cs = it.next();

		qglColor(cs->cubeColor());


		// upper rect
		glBegin(GL_LINES);
		GL::vertex(cs->upperTopLeft());
		GL::vertex(cs->upperTopRight());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->upperTopRight());
		GL::vertex(cs->upperBottomRight());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->upperBottomRight());
		GL::vertex(cs->upperBottomLeft());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->upperBottomRight());
		GL::vertex(cs->upperTopLeft());
		glEnd();


		// lower rect
		glBegin(GL_LINES);
		GL::vertex(cs->lowerTopLeft());
		GL::vertex(cs->lowerTopRight());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->lowerTopRight());
		GL::vertex(cs->lowerBottomRight());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->lowerBottomRight());
		GL::vertex(cs->lowerBottomLeft());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->lowerBottomRight());
		GL::vertex(cs->lowerTopLeft());
		glEnd();




		glBegin(GL_LINES);
		GL::vertex(cs->upperTopLeft());
		GL::vertex(cs->lowerTopLeft());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->upperTopRight());
		GL::vertex(cs->lowerTopRight());
		glEnd();


		glBegin(GL_LINES);
		GL::vertex(cs->upperBottomLeft());
		GL::vertex(cs->lowerBottomLeft());
		glEnd();

		glBegin(GL_LINES);
		GL::vertex(cs->upperBottomRight());
		GL::vertex(cs->lowerBottomRight());
		glEnd();

	}

	glEndList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::reassessObjectsPosition() {

	QListIterator<Hypocenter*> itH(_hypocenters);
	while ( itH.hasNext() ) {
		Hypocenter* h = itH.next();
		h->setRendererSettings(activeSettings());
		h->updateVertex();
	}

	QListIterator<Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Station* s = itS.next();
		s->setRendererSettings(activeSettings());
		s->updateVertex();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::loadSTL(const QString& file) {

	if ( _file )
	    delete _file;
	_file = NULL;

	_filename = file;

	_file = new STLFile;
	if ( _file->read(file) ) {

		::clearVector(_triangles);
		::clearVector(_vertices);

		_triangles = _file->data().triangles;
		_vertices = _file->data().vertices;
		_minLatitude = _file->data().minLatitude;
		_maxLatitude = _file->data().maxLatitude;
		_minLongitude = _file->data().minLongitude;
		_maxLongitude = _file->data().maxLongitude;
		_minElevation = _file->data().minElevation;
		_maxElevation = _file->data().maxElevation;

		processData();
	}
	else
		SEISCOMP_ERROR("Couldn't parse file %s as STL file", file.toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::loadXYZ(const QString& file) {

	if ( _file )
	    delete _file;
	_file = NULL;

	_filename = file;

	_file = new XYZFile;
	if ( _file->read(file) ) {

		::clearVector(_triangles);
		::clearVector(_vertices);

		_triangles = _file->data().triangles;
		_vertices = _file->data().vertices;
		_minLatitude = _file->data().minLatitude;
		_maxLatitude = _file->data().maxLatitude;
		_minLongitude = _file->data().minLongitude;
		_maxLongitude = _file->data().maxLongitude;
		_minElevation = _file->data().minElevation;
		_maxElevation = _file->data().maxElevation;

		processData();
	}
	else
		SEISCOMP_ERROR("Couldn't parse file %s as XYZ file", file.toStdString().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::setRenderingType(const RenderingType& t) {
	_rendering = t;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::
setActiveSettings(const TopographyRendererSettings& set) {

	_activeSettings = set;

	GLfloat minLon = GL::remap(_minX, _activeSettings.longitude().convRangeMin,
	    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
	    _activeSettings.longitude().max);
	GLfloat maxLon = GL::remap(_maxX, _activeSettings.longitude().convRangeMin,
	    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
	    _activeSettings.longitude().max);

	//! set the visual object drawing coefficient
	(fabs(maxLon - minLon) > 2) ? _vObjCoeff = .001 : _vObjCoeff = .005;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::setAvailableSettings(const SettingsList& l) {

	_availableSettings = l;

	_contextMenu.clear();
	QMenu* menu = _contextMenu.addMenu("Topography");

	if ( _fileGroup )
	    delete _fileGroup;
	_fileGroup = new QActionGroup(this);

	for (SettingsList::const_iterator it = _availableSettings.constBegin();
	        it != _availableSettings.constEnd(); ++it) {
		QAction* a = menu->addAction(it.key());
		a->setCheckable(true);
		if ( it == _availableSettings.constBegin() )
		    a->setChecked(true);
		connect(a, SIGNAL(triggered()), this, SLOT(settingsChanged()));
		_fileGroup->addAction(a);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::processData() {

	//! Pre-store vertices in video memory
	makeCurrent();

	if ( glIsList(_mesh) )
	    glDeleteLists(_mesh, 1);
	_mesh = glGenLists(1);

	if ( glIsList(_filled) )
	    glDeleteLists(_filled, 1);
	_filled = glGenLists(1);

	if ( glIsList(_filledMesh) )
	    glDeleteLists(_filledMesh, 1);
	_filledMesh = glGenLists(1);

	if ( glIsList(_pointsCloud) )
	    glDeleteLists(_pointsCloud, 1);
	_pointsCloud = glGenLists(1);

	if ( _mesh == 0 && _filledMesh == 0 && _filled == 0 && _pointsCloud == 0 ) {
		SEISCOMP_ERROR("Failed to generate rendering buffers");
		return;
	}

	Util::StopWatch sw;
	sw.restart();


	//! STL file
	if ( _triangles.size() > 0 ) {

		::clearVector(_vertices);

		//! Mesh
		glNewList(_mesh, GL_COMPILE);
		glLineWidth(1.);
		qglColor(Qt::white);
		for (TriangleList::const_iterator it = _triangles.constBegin();
		        it != _triangles.constEnd(); ++it) {
			glBegin(GL_LINES);
			GL::normal((*it)->normal());
			GL::vertex((*it)->vertexA());
			GL::vertex((*it)->vertexB());
			GL::vertex((*it)->vertexC());
			glEnd();
		}
		glEndList();


		//! Filled polygons + mesh
		glNewList(_filledMesh, GL_COMPILE);

		if ( _activeSettings.textureVisible() ) {
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glLineWidth(1.);
		qglColor(Qt::white);
		for (TriangleList::const_iterator it = _triangles.constBegin();
		        it != _triangles.constEnd(); ++it) {
			glBegin(GL_TRIANGLE_STRIP);
			//					qglColor((*it)->vertexA().color());
			GL::normal((*it)->normal());
			if ( _activeSettings.textureVisible() )
			    glTexCoord2f(0, 0);
			GL::vertex((*it)->vertexA());
			if ( _activeSettings.textureVisible() )
			    glTexCoord2f(1, 0);
			GL::vertex((*it)->vertexB());
			if ( _activeSettings.textureVisible() )
			    glTexCoord2f(0, 1);
			GL::vertex((*it)->vertexC());
			glEnd();
		}
		if ( _activeSettings.textureVisible() )
		    glDisable(GL_TEXTURE_2D);

		qglColor(Qt::black);
		for (TriangleList::const_iterator it = _triangles.constBegin();
		        it != _triangles.constEnd(); ++it) {
			glBegin(GL_LINES);
			GL::normal((*it)->normal());
			GL::vertex((*it)->vertexA());
			GL::vertex((*it)->vertexB());
			GL::vertex((*it)->vertexC());
			glEnd();
		}
		glEndList();

		//! Filled polygons
		glNewList(_filled, GL_COMPILE);

		if ( _activeSettings.textureVisible() ) {
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glLineWidth(1.);
		qglColor(Qt::white);
		for (TriangleList::const_iterator it = _triangles.constBegin();
		        it != _triangles.constEnd(); ++it) {
			glBegin(GL_TRIANGLE_STRIP);
			GL::normal((*it)->normal());
			if ( _activeSettings.textureVisible() )
			    glTexCoord2f(0, 0);
			GL::vertex((*it)->vertexA());
			if ( _activeSettings.textureVisible() )
			    glTexCoord2f(1, 0);
			GL::vertex((*it)->vertexB());
			if ( _activeSettings.textureVisible() )
			    glTexCoord2f(0, 1);
			GL::vertex((*it)->vertexC());
			glEnd();
		}

		if ( _activeSettings.textureVisible() )
		    glDisable(GL_TEXTURE_2D);

		glEndList();


		//! Points cloud
		glNewList(_pointsCloud, GL_COMPILE);
		glLineWidth(1.);
		qglColor(Qt::white);
		glBegin(GL_POINTS);
		for (TriangleList::const_iterator it = _triangles.constBegin();
		        it != _triangles.constEnd(); ++it) {
			GL::normal((*it)->normal());
			GL::vertex((*it)->vertexA());
			GL::vertex((*it)->vertexB());
			GL::vertex((*it)->vertexC());
		}
		glEnd();
		glEndList();
	}

	//! XYZ file
	else if ( _vertices.size() > 0 ) {

		::clearVector(_triangles);
		//! Points cloud
		glNewList(_pointsCloud, GL_COMPILE);
		glLineWidth(1.);
		qglColor(Qt::white);
		glBegin(GL_POINTS);
		for (VertexList::const_iterator it = _vertices.constBegin();
		        it != _vertices.constEnd(); ++it) {
			GL::normal((*it)->x(), (*it)->y(), (*it)->z());
			GL::vertex((*it)->x(), (*it)->y(), (*it)->z());
		}
		glEnd();
		glEndList();
	}

	setBoundingBox(_minLongitude, _maxLongitude, _minLatitude, _maxLatitude, _minElevation, _maxElevation);
	makeGraticule();

	GLfloat minLon = GL::remap(_minX, _activeSettings.longitude().convRangeMin,
	    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
	    _activeSettings.longitude().max);
	GLfloat maxLon = GL::remap(_maxX, _activeSettings.longitude().convRangeMin,
	    _activeSettings.longitude().convRangeMax, _activeSettings.longitude().min,
	    _activeSettings.longitude().max);
	GLfloat minLat = GL::remap(_minY, _activeSettings.latitude().convRangeMin,
	    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
	    _activeSettings.latitude().max);
	GLfloat maxLat = GL::remap(_maxY, _activeSettings.latitude().convRangeMin,
	    _activeSettings.latitude().convRangeMax, _activeSettings.latitude().min,
	    _activeSettings.latitude().max);
	GLfloat maxEleA = GL::remap(_maxZ, _activeSettings.elevation().convRangeMinAboveZero,
	    _activeSettings.elevation().convRangeMaxAboveZero, _activeSettings.elevation().min,
	    _activeSettings.elevation().max);
	GLfloat minEleB = GL::remap(_minZ, _activeSettings.elevation().convRangeMinBelowZero,
	    _activeSettings.elevation().convRangeMaxBelowZero, _activeSettings.elevation().min,
	    _activeSettings.elevation().max);

	SEISCOMP_DEBUG("Generated and populated buffers in %s", Time(sw.elapsed()).toString("%T.%f").c_str());
	SEISCOMP_DEBUG("Longitudes: [%f;%f] [%f;%f]", minLon, maxLon, _minLongitude, _maxLongitude);
	SEISCOMP_DEBUG("Latitudes : [%f;%f] [%f;%f]", minLat, maxLat, _minLatitude, _maxLatitude);
	SEISCOMP_DEBUG("Elevation : [%f;%f] [%f;%f]", minEleB, maxEleA, _minElevation, _maxElevation);
	SEISCOMP_DEBUG("Vertices  : %d", _vertices.size());
	SEISCOMP_DEBUG("Triangles : %d", _triangles.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::updateTexturedData() {

	//! Pre-store vertices in video memory
	makeCurrent();

	if ( glIsList(_filled) )
	    glDeleteLists(_filled, 1);
	_filled = glGenLists(1);

	if ( glIsList(_filledMesh) )
	    glDeleteLists(_filledMesh, 1);
	_filledMesh = glGenLists(1);

	if ( _filledMesh == 0 && _filled == 0 ) {
		SEISCOMP_ERROR("Failed to generate rendering buffers");
		return;
	}

	//! STL file
	if ( _triangles.size() == 0 ) {
		SEISCOMP_ERROR("There is no data to fill the polygons buffers");
		return;
	}

	Util::StopWatch sw;
	sw.restart();

	//! Filled polygons + mesh
	glNewList(_filledMesh, GL_COMPILE);

	if ( _activeSettings.textureVisible() ) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	glLineWidth(1.);
	qglColor(Qt::white);
	for (TriangleList::const_iterator it = _triangles.constBegin();
	        it != _triangles.constEnd(); ++it) {
		glBegin(GL_TRIANGLE_STRIP);
		//					qglColor((*it)->vertexA().color());
		GL::normal((*it)->normal());
		if ( _activeSettings.textureVisible() )
		    glTexCoord2f(0, 0);
		GL::vertex((*it)->vertexA());
		if ( _activeSettings.textureVisible() )
		    glTexCoord2f(1, 0);
		GL::vertex((*it)->vertexB());
		if ( _activeSettings.textureVisible() )
		    glTexCoord2f(0, 1);
		GL::vertex((*it)->vertexC());
		glEnd();
	}
	if ( _activeSettings.textureVisible() )
	    glDisable(GL_TEXTURE_2D);

	qglColor(Qt::black);
	for (TriangleList::const_iterator it = _triangles.constBegin();
	        it != _triangles.constEnd(); ++it) {
		glBegin(GL_LINES);
		GL::normal((*it)->normal());
		GL::vertex((*it)->vertexA());
		GL::vertex((*it)->vertexB());
		GL::vertex((*it)->vertexC());
		glEnd();
	}
	glEndList();


	//! Filled polygons
	glNewList(_filled, GL_COMPILE);

	if ( _activeSettings.textureVisible() ) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	glLineWidth(1.);
	qglColor(Qt::white);
	for (TriangleList::const_iterator it = _triangles.constBegin();
	        it != _triangles.constEnd(); ++it) {
		glBegin(GL_TRIANGLE_STRIP);
		//					qglColor((*it)->vertexA().color());
		GL::normal((*it)->normal());
		if ( _activeSettings.textureVisible() )
		    glTexCoord2f(0, 0);
		GL::vertex((*it)->vertexA());
		if ( _activeSettings.textureVisible() )
		    glTexCoord2f(1, 0);
		GL::vertex((*it)->vertexB());
		if ( _activeSettings.textureVisible() )
		    glTexCoord2f(0, 1);
		GL::vertex((*it)->vertexC());
		glEnd();
	}

	if ( _activeSettings.textureVisible() )
	    glDisable(GL_TEXTURE_2D);

	glEndList();

	SEISCOMP_DEBUG("Re-generated and populated polygons buffers in %s",
	    Time(sw.elapsed()).toString("%T.%f").c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyRenderer::appendVertex(const Vertex& v) {

	for (VertexList::const_iterator it = _vertices.constBegin();
	        it != _vertices.constEnd(); ++it)
		if ( (*it)->x() == v.x() && (*it)->y() == v.y() && (*it)->z() == v.z() )
		    return false;

	_vertices << new Vertex(v.x(), v.y(), v.z());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::setGraticuleVisible(const bool& v) {
	_activeSettings.setGraticuleVisible(v);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::setTextureVisible(const bool& visible) {

	if ( _activeSettings.textureVisible() == visible )
	    return;

	_activeSettings.setTextureVisible(visible);
	if ( _rendering == FILLED || _rendering == FILLEDMESH ) {
		updateTexturedData();
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::settingsChanged() {

	QList<QAction*> l = _fileGroup->actions();
	for (int i = 0; i < l.size(); ++i) {
		const TopographyRendererSettings set = _availableSettings.value(l.at(i)->text());
		if ( l.at(i)->isChecked() && set.name() != _activeSettings.name() ) {
			setActiveSettings(set);
			if ( set.fileType() == TopographyRendererSettings::XYZ_FILE )
			    loadXYZ(set.filepath());
			if ( set.fileType() == TopographyRendererSettings::STL_FILE )
			    loadSTL(set.filepath());
			break;
		}
	}

	//! Keep whatever widget parenting us that settings have been changed
	//! so that it could redraw stuff consequently... we do not initiate
	//! objects updates on our own! let bosses be bosses?!
	emit currentSettingsChanged();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyRenderer::addHypocenter(Hypocenter* h) {

	if ( !h ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr hypocenters to map"));
		return false;
	}

	if ( !_hypocenters.contains(h) )
	    _hypocenters.append(h);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Hypocenter* TopographyRenderer::getHypocenter(const QString& name) {

	QListIterator<Hypocenter*> itH(_hypocenters);
	while ( itH.hasNext() ) {
		Hypocenter* h = itH.next();
		if ( h->name() == name )
		    return h;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::clearHypocenters() {
	::clearList(_hypocenters);
	updateHypocenters();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyRenderer::addArrival(Arrival* arrival) {

	if ( !arrival ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr arrival to map"));
		return false;
	}

	if ( !_arrivals.contains(arrival) )
	    _arrivals.append(arrival);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Arrival* TopographyRenderer::getArrival(const QString& station,
                                        const QString& phaseCode) {

	QListIterator<Arrival*> itA(_arrivals);
	while ( itA.hasNext() ) {
		Arrival* a = itA.next();
		if ( a->station()->name() == station && a->phaseCode() == phaseCode )
		    return a;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::clearArrivals() {
	::clearList(_arrivals);
	updateArrivals();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyRenderer::addStation(Station* station) {

	if ( !station ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr station to map"));
		return false;
	}

	if ( !_stations.contains(station) )
	    _stations.append(station);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station* TopographyRenderer::getStation(const QString& network,
                                        const QString& code) {

	QListIterator<Station*> itS(_stations);
	while ( itS.hasNext() ) {
		Station* s = itS.next();
		if ( s->name() == code && s->network() == network )
		    return s;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::clearStations() {
	::clearList(_stations);
	updateStations();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TopographyRenderer::addCrossSection(CrossSection* cs) {

	if ( !cs ) {
		log(Client::LM_ERROR, QString("Couldn't append NULL ptr cross section's to map"));
		return false;
	}

	if ( !_crossSections.contains(cs) )
	    _crossSections.append(cs);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection* TopographyRenderer::getCrossSection(const QString& name) {

	QListIterator<CrossSection*> it(_crossSections);
	while ( it.hasNext() ) {
		CrossSection* cs = it.next();
		if ( cs->name() == name )
		    return cs;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyRenderer::clearCrossSections() {
	::clearList(_crossSections);
	updateCrossSections();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace OpenGL
} // namespace Gui
} // namespace IPGP
