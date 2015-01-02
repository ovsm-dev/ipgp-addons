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

#include <ipgp/gui/opengl/drawables/crosssection.h>
#include <ipgp/gui/opengl/topographyrenderer.h>
#include <ipgp/gui/opengl/gl.h>
#include <seiscomp3/math/geo.h>
#include <QDebug>


#define _settings TopographyRenderer::getInstance()->activeSettings()


namespace IPGP {
namespace Gui {
namespace OpenGL {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection::CrossSection() :
		_latitudeA(.0), _latitudeB(.0), _longitudeA(.0), _longitudeB(.0),
		_depthMin(.0), _depthMax(.0), _width(.0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CrossSection::~CrossSection() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setName(const QString& n) {
	_name = n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& CrossSection::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setPointA(const float& lon, const float& lat) {

	_latitudeA = lat;
	_longitudeA = lon;

	_pointA.setX(GL::remap(_longitudeA, _settings.longitude().min, _settings.longitude().max,
	_settings.longitude().convRangeMin, _settings.longitude().convRangeMax));
	_pointA.setY(GL::remap(_latitudeA, _settings.latitude().min, _settings.latitude().max,
	_settings.latitude().convRangeMin, _settings.latitude().convRangeMax));
	_pointA.setZ(.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setPointB(const float& lon, const float& lat) {

	_latitudeB = lat;
	_longitudeB = lon;

	_pointB.setX(GL::remap(_longitudeB, _settings.longitude().min, _settings.longitude().max,
	_settings.longitude().convRangeMin, _settings.longitude().convRangeMax));
	_pointB.setY(GL::remap(_latitudeB, _settings.latitude().min, _settings.latitude().max,
	_settings.latitude().convRangeMin, _settings.latitude().convRangeMax));
	_pointB.setZ(.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::setDepth(const float& min, const float& max) {
	_depthMin = min;
	_depthMax = max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrossSection::computeCube() {

	float widthDeg = GL::remap(Seiscomp::Math::Geo::km2deg(_width), _settings.latitude().min,
	_settings.latitude().max, _settings.latitude().convRangeMin, _settings.latitude().convRangeMax);
	float yTopA = GL::remap(_latitudeA + widthDeg/2., _settings.latitude().min,
	_settings.latitude().max, _settings.latitude().convRangeMin, _settings.latitude().convRangeMax);
	float yBottomA = GL::remap(_latitudeA - widthDeg/2., _settings.latitude().min,
	_settings.latitude().max, _settings.latitude().convRangeMin, _settings.latitude().convRangeMax);
	float yTopB = GL::remap(_latitudeB + widthDeg/2., _settings.latitude().min,
	_settings.latitude().max, _settings.latitude().convRangeMin, _settings.latitude().convRangeMax);
	float yBottomB = GL::remap(_latitudeB - widthDeg/2., _settings.latitude().min,
	_settings.latitude().max, _settings.latitude().convRangeMin, _settings.latitude().convRangeMax);

	float depthMin = GL::remap(Seiscomp::Math::Geo::km2deg(_depthMin),
	_settings.elevation().min, _settings.elevation().max,
	(_depthMin <.0) ?_settings.elevation().convRangeMinAboveZero:_settings.elevation().convRangeMinBelowZero,
	(_depthMin <.0) ? _settings.elevation().convRangeMaxAboveZero:_settings.elevation().convRangeMaxBelowZero);

	float depthMax = GL::remap(Seiscomp::Math::Geo::km2deg(_depthMax),
	_settings.elevation().min, _settings.elevation().max,
	(_depthMax <.0) ?_settings.elevation().convRangeMinAboveZero:_settings.elevation().convRangeMinBelowZero,
	(_depthMax <.0) ? _settings.elevation().convRangeMaxAboveZero:_settings.elevation().convRangeMaxBelowZero);

	_upperTopLeft.setX(_pointA.x());
	_upperTopLeft.setY(yTopA);
	_upperTopLeft.setZ(depthMin);
	_upperTopRight.setX(_pointB.x());
	_upperTopRight.setY(yTopB);
	_upperTopRight.setZ(depthMin);

	_upperBottomLeft.setX(_pointA.x());
	_upperBottomLeft.setY(yBottomA);
	_upperBottomLeft.setZ(depthMin);
	_upperBottomRight.setX(_pointB.x());
	_upperBottomRight.setY(yBottomB);
	_upperBottomRight.setZ(depthMin);

	_lowerTopLeft = _upperTopLeft;
	_lowerTopRight = _upperTopRight;
	_lowerBottomLeft = _upperBottomLeft;
	_lowerBottomRight = _upperBottomRight;

	_lowerTopLeft.setZ(depthMax);
	_lowerTopRight.setZ(depthMax);
	_lowerBottomRight.setZ(depthMax);
	_lowerBottomLeft.setZ(depthMax);

	qDebug("Computed cube");
	qDebug() << "upperTopLeft: " << _upperTopLeft.x() << " - " << _upperTopLeft.y() << " -" << _upperTopLeft.z();
	qDebug() << "upperTopRight: " << _upperTopRight.x() << " - " << _upperTopRight.y() << " -" << _upperTopRight.z();
	qDebug() << "upperBottomLeft: " << _upperBottomLeft.x() << " - " << _upperBottomLeft.y() << " -" << _upperBottomLeft.z();
	qDebug() << "upperBottomRight: " << _upperBottomRight.x() << " - " << _upperBottomRight.y() << " -" << _upperBottomRight.z();

	qDebug() << "lowerTopLeft: " << _lowerTopLeft.x() << " - " << _lowerTopLeft.y() << " -" << _lowerTopLeft.z();
	qDebug() << "lowerTopRight: " << _lowerTopRight.x() << " - " << _lowerTopRight.y() << " -" << _lowerTopRight.z();
	qDebug() << "lowerBottomLeft: " << _lowerBottomLeft.x() << " - " << _lowerBottomLeft.y() << " -" << _lowerBottomLeft.z();
	qDebug() << "lowerBottomRight: " << _lowerBottomRight.x() << " - " << _lowerBottomRight.y() << " -" << _lowerBottomRight.z();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
 // namespace OpenGL
}// namespace Gui
} // namespace IPGP
