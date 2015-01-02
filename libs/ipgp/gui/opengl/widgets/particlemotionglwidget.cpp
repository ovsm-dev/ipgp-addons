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

#include <ipgp/gui/opengl/widgets/particlemotionglwidget.h>
#include <QtOpenGL>
#include <QDebug>
#include <ipgp/gui/math/math.h>


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionGLWidget::ParticleMotionGLWidget(QWidget* parent) :
		Canvas(parent), _dm(dm_None) {

	setXAxisName("EW");
	setYAxisName("NS");
	setZAxisName("Z");
	setXRange(0, 10);
	setYRange(0, 10);
	setZRange(0, 10);
//	setScalingValue(.09f);
	setRendererInfoVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParticleMotionGLWidget::~ParticleMotionGLWidget() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionGLWidget::feed(const QVector<qreal>& xdata,
                                  const QVector<qreal>& ydata,
                                  const QVector<qreal>& zdata,
                                  const bool& autoGradient) {

	if ( xdata.size() != ydata.size() && ydata.size() != zdata.size() )
		return;

	_data.clear();

	if ( autoGradient )
		_dm = dm_UseAutoGradient;

	_range_x_min = Math::getMin(xdata);
	_range_x_max = Math::getMax(xdata);
	_range_y_min = Math::getMin(ydata);
	_range_y_max = Math::getMax(ydata);
	_range_z_min = Math::getMin(zdata);
	_range_z_max = Math::getMax(zdata);

	for (int i = 0; i < xdata.size(); ++i)
		_data.append(ParticleData(QVector3D(xdata.at(i), ydata.at(i),
		    zdata.at(i)), Qt::red));

//	_scaling = .0002;

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionGLWidget::feed(const QVector<Particle>& xdata,
                                  const QVector<Particle>& ydata,
                                  const QVector<Particle>& zdata) {

	if ( xdata.size() != ydata.size() && ydata.size() != zdata.size() )
		return;

	_data.clear();
	_dm = dm_UseCustomGradient;

	_range_x_min = getMinParticleValue(xdata);
	_range_x_max = getMaxParticleValue(xdata);
	_range_y_min = getMinParticleValue(ydata);
	_range_y_max = getMaxParticleValue(ydata);
	_range_z_min = getMinParticleValue(zdata);
	_range_z_max = getMaxParticleValue(zdata);

	for (int i = 0; i < xdata.size(); ++i)
		_data.append(ParticleData(QVector3D(xdata.at(i).value(), ydata.at(i).value(),
		    zdata.at(i).value()), xdata.at(i).color()));

//	_scaling = .0002;

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionGLWidget::clear(const bool& refresh) {

	_data.clear();

	if ( refresh )
		updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionGLWidget::draw() {

	Canvas::draw();

	glBegin(GL_LINES);
	for (int i = 0; i < _data.size() - 1; ++i) {

		switch ( _dm ) {
			case dm_UseAutoGradient:
				qglColor(QColor(sin(i * 0.3) * 100 + 100, sin(i * 0.6 + 0.7) * 100 + 100,
				    sin(i * 0.4 + 0.6) * 100 + 100));
			break;
			case dm_UseCustomGradient:
				qglColor(_data.at(i).second);
			break;
			default:
				qglColor(QColor(Qt::red));
			break;
		}
		glVertex3d(_data.at(i).first.x(), _data.at(i).first.y(), _data.at(i).first.z());
		glVertex3d(_data.at(i + 1).first.x(), _data.at(i + 1).first.y(), _data.at(i + 1).first.z());
	}
	glEnd();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ParticleMotionGLWidget::redraw() {

	Canvas::draw();

	/*
	 glBegin(GL_LINES);
	 for (int i = 0; i < _data.size() - 1; ++i) {

	 if ( _paintUsingGradient )
	 qglColor(QColor(sin(i * 0.3) * 100 + 100, sin(i * 0.6 + 0.7) * 100 + 100,
	 sin(i * 0.4 + 0.6) * 100 + 100));
	 else
	 qglColor(QColor(Qt::red));

	 glVertex3d(_data.at(i).x(), _data.at(i).y(), _data.at(i).z());
	 glVertex3d(_data.at(i + 1).x(), _data.at(i + 1).y(), _data.at(i + 1).z());
	 }
	 glEnd();
	 */
	this->draw();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal ParticleMotionGLWidget::
getMinParticleValue(const QVector<Particle>& vector) const {

	qreal value;
	for (int i = 0; i < vector.size(); ++i) {
		if ( i == 0 )
			value = vector.at(i).value();

		if ( vector.at(i).value() < value )
			value = vector.at(i).value();
	}

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const qreal ParticleMotionGLWidget::
getMaxParticleValue(const QVector<Particle>& vector) const {

	qreal value;
	for (int i = 0; i < vector.size(); ++i) {
		if ( i == 0 )
			value = vector.at(i).value();

		if ( vector.at(i).value() > value )
			value = vector.at(i).value();
	}

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
