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

#define SEISCOMP_COMPONENT IPGP_GL_RENDERERBASE

#include <ipgp/gui/opengl/renderer.h>
#include <ipgp/gui/opengl/camera.h>
#include <ipgp/gui/opengl/gl.h>

#include <seiscomp3/logging/log.h>

#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include <iomanip>




namespace IPGP {
namespace Gui {
namespace OpenGL {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RendererTheme::RendererTheme() {
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RendererTheme::~RendererTheme() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::reset() {

	_topLeft = _topRight = Qt::black;
//	_bottomLeft = _bottomRight = QColor("#8080FF");
//	_topLeft = _topRight = QColor("#214778");
	_bottomLeft = _bottomRight = QColor("#2360AB");
//	_bottomLeft = _bottomRight = QColor("#6188A9");

	_infoBoxColor = QColor(128, 16, 16, 200);
	_instBoxColor = QColor("#630F11");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::setTopLeftColor(const QColor& c) {
	_topLeft = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::setTopRightColor(const QColor& c) {
	_topRight = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::setBottomLeftColor(const QColor& c) {
	_bottomLeft = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::setBottomRightColor(const QColor& c) {
	_bottomRight = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::setInformationBoxColor(const QColor& c) {
	_infoBoxColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererTheme::setInstructionBoxColor(const QColor& c) {
	_instBoxColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& RendererTheme::topLeftColor() const {
	return _topLeft;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& RendererTheme::topRightColor() const {
	return _topRight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& RendererTheme::bottomLeftColor() const {
	return _bottomLeft;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& RendererTheme::bottomRightColor() const {
	return _bottomRight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& RendererTheme::informationBoxColor() const {
	return _infoBoxColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& RendererTheme::instructionBoxColor() const {
	return _instBoxColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RendererSettings::RendererSettings() {
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RendererSettings::~RendererSettings() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::reset() {

	_fov = 60.;
	_zoomStep = .2;
	_mouseSensibility = .1;
	_rotationDelay = 1000;
	_cameraDelay = 13;
	_axis = false;
	_info = false;
	_grid = false;
	_lighting = true;
	_gradient = true;
	_boundingBox = false;
	_smoothing = false;
	_renderingMethod = RM_GL_VERTEX;

	setAmbientLight(VColor() << .5 << 1. << 1. << 0.);
	setDiffuseLight(VColor() << 1. << 1. << 1. << 1.);
	setSpecularLight(VColor() << .0 << .8 << .8 << 1.);
	setPositionLight(VColor() << .7 << 1. << 1. << 1.);

	_ambientLightEnabled = false;
	_diffuseLightEnabled = true;
	_specularLightEnabled = false;
	_positionLightEnabled = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setFOV(const float& v) {
	_fov = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setZoomStep(const float& v) {
	_zoomStep = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setMouseSensibility(const float& v) {
	_mouseSensibility = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setRotationDelay(int v) {
	if ( v < 10. ) v = 10.;
	if ( v > 3000. ) v = 3000.;
	_rotationDelay = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setCameraDelay(int v) {
	if ( v < 10. ) v = 10.;
	if ( v > 1000. ) v = 1000.;
	_cameraDelay = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setAxisVisible(const bool& v) {
	_axis = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setRendererInfoVisible(const bool& v) {
	_info = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setGridVisible(const bool& v) {
	_grid = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setLightingActivated(const bool& v) {
	_lighting = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setAmbientLightEnabled(const bool& v) {
	_ambientLightEnabled = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setDiffuseLightEnabled(const bool& v) {
	_diffuseLightEnabled = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setSpecularLightEnabled(const bool& v) {
	_specularLightEnabled = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setPositionLightEnabled(const bool& v) {
	_positionLightEnabled = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setGradientVisible(const bool& v) {
	_gradient = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setBoundingBoxVisible(const bool& v) {
	_boundingBox = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setSmoothingActivated(const bool& v) {
	_smoothing = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setRenderingMethod(const RenderingMethod& v) {
	_renderingMethod = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setAmbientLight(VColor v) {
	_ambientLight = v;
//	if ( sizeof(v) == 4 ) std::copy(_ambientLight, _ambientLight + 4, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setSpecularLight(VColor v) {
	_specularLight = v;
//	if ( sizeof(v) == 4 ) std::copy(_specularLight, _specularLight + 4, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setDiffuseLight(VColor v) {
	_diffuseLight = v;
//	if ( sizeof(v) == 4 ) std::copy(_diffuseLight, _diffuseLight + 4, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RendererSettings::setPositionLight(VColor v) {
	_positionLight = v;
//	if ( sizeof(v) == 4 ) std::copy(_positionLight, _positionLight + 4, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const float& RendererSettings::fov() const {
	return _fov;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const float& RendererSettings::zoomStep() const {
	return _zoomStep;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const float& RendererSettings::mouseSensibility() const {
	return _mouseSensibility;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int& RendererSettings::rotationDelay() const {
	return _rotationDelay;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int& RendererSettings::cameraDelay() const {
	return _cameraDelay;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::axisVisible() const {
	return _axis;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::rendererInfoVisible() const {
	return _info;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::gridVisible() const {
	return _grid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::lightingActivated() const {
	return _lighting;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::ambientLightEnabled() const {
	return _ambientLightEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::diffuseLightEnabled() const {
	return _diffuseLightEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::specularLightEnabled() const {
	return _specularLightEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::positionLightEnabled() const {
	return _positionLightEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::gradientVisible() const {
	return _gradient;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::boundingBoxVisible() const {
	return _boundingBox;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool& RendererSettings::smoothingActivated() const {
	return _smoothing;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RendererSettings::RenderingMethod& RendererSettings::renderingMethod() const {
	return _renderingMethod;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RendererTheme& RendererSettings::theme() const {
	return _theme;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RendererTheme& RendererSettings::theme() {
	return _theme;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RendererSettings::VColor& RendererSettings::ambientLight() const {
	return _ambientLight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RendererSettings::VColor& RendererSettings::specularLight() const {
	return _specularLight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RendererSettings::VColor& RendererSettings::diffuseLight() const {
	return _diffuseLight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RendererSettings::VColor& RendererSettings::positionLight() const {
	return _positionLight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Renderer::Renderer(QWidget* parent, Qt::WFlags f) :
		QGLWidget(QGLFormat(QGL::SampleBuffers), parent, 0, f),
		_camera(new Camera) {

	setAutoFillBackground(false);
	setMinimumSize(200, 200);
	setWindowTitle(tr("IPGP::Gui::Renderer"));

	_mouseGrabbed = false;
	_movedWhileGrabbed = false;
	_cameraMode = false;

	_axisBuffer = _bBoxBuffer = 0;

	_minX = _minY = _minZ = _maxX = _maxY = _maxZ = .0;

	createGradient();

	stopMoving();

	_sceneTimer = new QTimer(this);
	connect(_sceneTimer, SIGNAL(timeout()), this, SLOT(sceneTimerEvent()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Renderer::~Renderer() {

	makeCurrent();
	glDeleteLists(_axisBuffer, 1);
	glDeleteLists(_gridBuffer, 1);
	glDeleteLists(_bBoxBuffer, 1);

	delete _camera;
	_camera = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::initializeGL() {
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::paintEvent(QPaintEvent* event) {

	makeCurrent();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if ( _settings.gradientVisible() )
	    drawBackgroundGradient();

	glTranslatef(.0, .0, .0);

	//! Apply the camera rotation and position
	_camera->applyRot();
	_camera->applyPos();

	if ( _settings.axisVisible() )
	    renderAxis();
	if ( _settings.gridVisible() )
	    renderGrid();
	if ( _settings.boundingBoxVisible() )
	    renderBoundingBox();

	drawWithoutLight();

	//! Add some lighting
	if ( _settings.lightingActivated() ) {
		glEnable(GL_DEPTH_TEST);
//		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHT0);
//		static float diffuseColor[] = { 1.0, 1.0, 1.0, 1.0 };
//		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseColor);

		if ( _settings.ambientLightEnabled() ) {
			GLfloat l[4];
			size_t pos = 0;
			for (int i = 0; i < 4; ++i, ++pos)
				l[pos] = _settings.ambientLight().at(i);
			glLightfv(GL_LIGHT0, GL_AMBIENT, l);
		}
		if ( _settings.diffuseLightEnabled() ) {
			GLfloat l[4];
			size_t pos = 0;
			for (int i = 0; i < 4; ++i, ++pos)
				l[pos] = _settings.diffuseLight().at(i);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, l);
		}
		if ( _settings.specularLightEnabled() ) {
			GLfloat l[4];
			size_t pos = 0;
			for (int i = 0; i < 4; ++i, ++pos)
				l[pos] = _settings.specularLight().at(i);
			glLightfv(GL_LIGHT0, GL_SPECULAR, l);
		}
		if ( _settings.positionLightEnabled() ) {
			GLfloat l[4];
			size_t pos = 0;
			for (int i = 0; i < 4; ++i, ++pos)
				l[pos] = _settings.positionLight().at(i);
			glLightfv(GL_LIGHT0, GL_POSITION, l);
		}
	}

	//! Draw stuff that require light first...
	//! Drawing those objects brings out a transparency effect, and therefore
	//! objects like axis, grids, etc. will be visible thru them.
	drawWithLight();

	if ( _settings.lightingActivated() ) {

		glDisable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
//		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
	}


	//! Draw stuff that do not require light nor shading...
//	drawWithoutLight();


	//! Overpaint the canvas with some information...
	QPainter painter(this);
	if ( !_instructions.isEmpty() )
	    displayInstructions(&painter);
	if ( _settings.rendererInfoVisible() )
	    displayInfos(&painter);

	painter.end();

	swapBuffers();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::resizeGL(int width, int height) {

	setupViewport(width, height);

	if ( !height ) height = 1;

	GLfloat ratio = (GLfloat) width / height;
	_camera->perspective(45., ratio);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::mousePressEvent(QMouseEvent* event) {

	if ( event->buttons() & Qt::RightButton ) {
		if ( event->modifiers() == Qt::ControlModifier )
		    (_mouseGrabbed) ? releaseInput() : grabInput();
	}

	if ( event->buttons() & Qt::MidButton ) {
		_camera->reset();

		if ( !_mouseGrabbed )
		    update();
	}

	_lastPos = event->globalPos();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::mouseMoveEvent(QMouseEvent* event) {

	if ( event->globalPos() == _grabPos ) return;

	int dx = event->globalX() - _lastPos.x();
	int dy = event->globalY() - _lastPos.y();

	if ( _mouseGrabbed ) {

		// Rotates camera
		dx = event->globalX() - _grabPos.x();
		dy = event->globalY() - _grabPos.y();

		_camera->turn(-dy * _settings.mouseSensibility(), .0, -dx * _settings.mouseSensibility());

		// Return the cursor to the _grabPosition so it
		// never reaches the edge of the screen
		QCursor::setPos(_grabPos);

		// glWidget will be updated by the timer
	}
	else if ( event->buttons() & Qt::LeftButton ) {

		_instructions = tr("Orbital mode: Move around the object by dragging it.");

		// Orbits camera
		dx = event->globalX() - _lastPos.x();
		dy = event->globalY() - _lastPos.y();

		if ( _movedWhileGrabbed ) {
			_camera->sphericalFromPosition();
			_movedWhileGrabbed = false;
		}

		_camera->orbit(-.37 * dy, -.37 * dx);
		_camera->lookAt(.0, .0, .0);

		_lastPos = event->globalPos();

		update();
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::mouseReleaseEvent(QMouseEvent* event) {
	if ( !_mouseGrabbed && event->modifiers() != Qt::ControlModifier
	        && (event->button() & Qt::RightButton) )
	    _contextMenu.exec(mapToGlobal(event->pos()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::wheelEvent(QWheelEvent* event) {

	if ( !_mouseGrabbed ) {
		_camera->lookAt(.0, .0, .0);
		if ( event->delta() > 0 )
			_camera->setRhoOrbit(_camera->getRhoOrbit() - _settings.zoomStep());
		else
			_camera->setRhoOrbit(_camera->getRhoOrbit() + _settings.zoomStep());
		_camera->commitSpherical();
		update();
	}
	else {
		switch ( event->modifiers() ) {
			case Qt::ShiftModifier:
				_camera->modifyAspectRatio(event->delta());
			break;
			case Qt::ControlModifier:
				_camera->modifyFieldOfView(event->delta());
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::keyPressEvent(QKeyEvent* event) {

	// Ignore autoRepeat key events
//	if ( event->isAutoRepeat() ) return;

	//! @note Those keys do not work on OSX
	//! @todo Solve this!!
	// Set a new moving direction
	switch ( event->key() ) {
		case Qt::Key_Z:
			case Qt::Key_Up:
			_movingTo[0] = true;
		break;
		case Qt::Key_D:
			case Qt::Key_Right:
			_movingTo[1] = true;
		break;
		case Qt::Key_S:
			case Qt::Key_Down:
			_movingTo[2] = true;
		break;
		case Qt::Key_Q:
			case Qt::Key_Left:
			_movingTo[3] = true;
		break;
		case Qt::Key_Shift:
			_camera->setRun(true);
		break;
		case Qt::Key_Return:
			_camera->lookAt(0, 0, 0);
		break;
		case Qt::Key_Escape:
			releaseInput();
		break;
		case Qt::Key_Plus: {
//			if ( _mouseGrabbed )
			_camera->setZPos(_camera->getZPos() + _settings.mouseSensibility() / 50);
		}
		break;
		case Qt::Key_Minus: {
//			if ( _mouseGrabbed )
			_camera->setZPos(_camera->getZPos() - _settings.mouseSensibility() / 50);

		}
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::keyReleaseEvent(QKeyEvent* event) {

	// Ignore autoRepeat key events
	if ( event->isAutoRepeat() ) return;

	//! @note Those keys do not work on OSX
	//! @todo Solve this!!
	// Set a new moving direction
	switch ( event->key() ) {
		case Qt::Key_Z:
			case Qt::Key_Up:
			_movingTo[0] = false;
		break;
		case Qt::Key_D:
			case Qt::Key_Right:
			_movingTo[1] = false;
		break;
		case Qt::Key_S:
			case Qt::Key_Down:
			_movingTo[2] = false;
		break;
		case Qt::Key_Q:
			case Qt::Key_Left:
			_movingTo[3] = false;
		break;
		case Qt::Key_Shift:
			_camera->setRun(false);
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::createGradient() {

	_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	_gradient.setCenter(0.45, 0.50);
	_gradient.setFocalPoint(0.40, 0.45);
	_gradient.setColorAt(0.0, QColor(105, 146, 182));
	_gradient.setColorAt(0.4, QColor(81, 113, 150));
	_gradient.setColorAt(0.8, QColor(16, 56, 121));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::grabInput() {

	if ( _mouseGrabbed ) return;

	_cameraMode = true;

	// Stores grab position
	_grabPos = _lastPos = QCursor::pos();
	// Grabs keyboard and mouse hiding the cursor
	grabMouse(Qt::BlankCursor);
	grabKeyboard();
	// Calls mouseMoveEvent even the cursor is not over the widget
	setMouseTracking(true);

	_mouseGrabbed = true;

	_sceneTimer->start(_settings.cameraDelay());

	_instructions = tr("Exploration mode: Use Z, S, Q and D keys to go "
		"forward, backward or move sideways.");
	emit statusMessage(tr("Exploration mode: Use Z, S, Q and D keys to go "
		"forward, backward or move sideways."));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::releaseInput() {

	if ( !_mouseGrabbed ) return;

	_cameraMode = false;

	QCursor::setPos(_grabPos);
	releaseMouse();
	releaseKeyboard();
	setMouseTracking(false);

	_sceneTimer->stop();
	stopMoving();

	_mouseGrabbed = false;
	_instructions = QString();
	emit statusMessage("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Renderer::movingCount() {

	int keysPressed = 0;

	for (int i = 0; i < _movingDirections; ++i)
		if ( _movingTo[i] )
		    keysPressed++;

	return keysPressed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::stopMoving() {
	for (int i = 0; i < _movingDirections; ++i)
		_movingTo[i] = false;
	_camera->setRun(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setupViewport(int width, int height) {
//	int side = qMin(width, height);
//	glViewport((width - side) / 2, (height - side) / 2, side, side);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef QT_OPENGL_ES
	glOrthof(-.5, .5, -.5, .5, 4., 15.);
#else
	glOrtho(-.5, .5, -.5, .5, 4., 15.);
#endif
	glMatrixMode(GL_MODELVIEW);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::sceneTimerEvent() {

	int moveCount = movingCount();

	if ( moveCount == 1 ) {
		// 0 1 2 3
		if ( _movingTo[0] )
		    _camera->move(0);
		if ( _movingTo[1] )
		    _camera->move(90);
		if ( _movingTo[2] )
		    _camera->move(180);
		if ( _movingTo[3] )
		    _camera->move(270);
	}
	else {
		// Diagonals
		if ( _movingTo[0] && _movingTo[1] )
		    _camera->move(45);
		if ( _movingTo[1] && _movingTo[2] )
		    _camera->move(135);
		if ( _movingTo[2] && _movingTo[3] )
		    _camera->move(225);
		if ( _movingTo[3] && _movingTo[0] )
		    _camera->move(315);
	}

	// The camera changed position
	if ( moveCount )
	    _movedWhileGrabbed = true;

	// Only updateGL when needed
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setAxisVisible(const bool& visible) {
	_settings.setAxisVisible(visible);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setRendererInfoVisible(const bool& visible) {
	_settings.setRendererInfoVisible(visible);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setGridVisible(const bool& visible) {
	_settings.setGridVisible(visible);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setBoundingBoxVisible(const bool& visible) {
	_settings.setBoundingBoxVisible(visible);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setLightingActivated(const bool& visible) {
	_settings.setLightingActivated(visible);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::showHelp(const bool& v) {
	Q_UNUSED(v);
	QMessageBox::information(this, tr("Help"),
	    QString("IPGP OpenGL Renderer 1.0"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setSmoothingActivated(const bool& v) {
	_settings.setSmoothingActivated(v);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::log(const Client::LogMessage& lm, const QString& msg) {
	emit logMessage(static_cast<int>(lm), __func__, msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::makeGrid(GLfloat z, GLfloat size, GLfloat radio) {

	makeCurrent();

	if ( glIsList(_gridBuffer) )
	    glDeleteLists(_gridBuffer, 1);
	_gridBuffer = glGenLists(1);

	if ( _gridBuffer == 0 ) return;


//	GLfloat x, y;

	glNewList(_gridBuffer, GL_COMPILE);
	glBegin(GL_LINES);




	/*
	 for (x = -radio; x < radio + size; x += size) {
	 GL::vertex(x, -radio, z);
	 GL::vertex(x, radio, z);
	 }

	 for (y = -radio; y < radio + size; y += size) {
	 GL::vertex(-radio, y, z);
	 GL::vertex(radio, y, z);
	 }

	 // Axes
	 // x
	 GL::color(.5, 0, 0);
	 GL::vertex(0, 0, 0);
	 GL::color(1, .4, .4);
	 GL::vertex(radio / 2, 0, 0);
	 // y
	 GL::color(0, .5, 0);
	 GL::vertex(0, 0, 0);
	 GL::color(.4, 1, .4);
	 GL::vertex(0, radio / 2, 0);
	 // z
	 GL::color(0, 0, .5);
	 GL::vertex(0, 0, 0);
	 GL::color(.4, .4, 1);
	 GL::vertex(0, 0, radio / 2);

	 // Label X
	 GL::color(1, 0, 0);
	 GL::vertex(radio / 2 + 0.1 + -.04, -.04, 0);
	 GL::vertex(radio / 2 + 0.1 + .04, .04, 0);
	 GL::vertex(radio / 2 + 0.1 + .04, -.04, 0);
	 GL::vertex(radio / 2 + 0.1 + -.04, .04, 0);

	 // Label Y
	 GL::color(0, 1, 0);
	 GL::vertex(-.04, radio / 2 + 0.1 + .04, 0);
	 GL::vertex(0, radio / 2 + 0.1 + 0, 0);
	 GL::vertex(.04, radio / 2 + 0.1 + .04, 0);
	 GL::vertex(-.04, radio / 2 + 0.1 + -.04, 0);
	 // Label Z
	 GL::color(0, 0, 1);
	 GL::vertex(0, -.04, radio / 2 + 0.1 + 0.04);
	 GL::vertex(0, .04, radio / 2 + 0.1 + 0.04);
	 GL::vertex(0, .04, radio / 2 + 0.1 + -0.04);
	 GL::vertex(0, -.04, radio / 2 + 0.1 + -0.04);
	 GL::vertex(0, -.04, radio / 2 + 0.1 + -0.04);
	 GL::vertex(0, .04, radio / 2 + 0.1 + 0.04);
	 */

	glEnd();
	glEndList();


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::setBoundingBox(const GLfloat& xmin, const GLfloat& xmax,
                              const GLfloat& ymin, const GLfloat& ymax,
                              const GLfloat& zmin, const GLfloat& zmax) {
	_minX = xmin;
	_maxX = xmax;
	_minY = ymin;
	_maxY = ymax;
	_minZ = zmin;
	_maxZ = zmax;

	makeCurrent();

	if ( glIsList(_bBoxBuffer) )
	    glDeleteLists(_bBoxBuffer, 1);
	_bBoxBuffer = glGenLists(1);

	if ( _bBoxBuffer == 0 ) return;


	glNewList(_bBoxBuffer, GL_COMPILE);

//	glEnable(GL_MULTISAMPLE);

	glLineWidth(2.);
	qglColor(QColor("#87CEEB"));

	// Render in dash line
//	glLineStipple(4, 0xAAAA);
//	glEnable(GL_LINE_STIPPLE);

	//! Upper left line
	glBegin(GL_LINES);
	GL::vertex(_minX, _minY, _maxZ);
	GL::vertex(_minX, _maxY, _maxZ);
	glEnd();

	//! Upper top line
	glBegin(GL_LINES);
	GL::vertex(_minX, _maxY, _maxZ);
	GL::vertex(_maxX, _maxY, _maxZ);
	glEnd();

	//! Upper right line
	glBegin(GL_LINES);
	GL::vertex(_maxX, _maxY, _maxZ);
	GL::vertex(_maxX, _minY, _maxZ);
	glEnd();

	//! Upper bottom line
	glBegin(GL_LINES);
	GL::vertex(_maxX, _minY, _maxZ);
	GL::vertex(_minX, _minY, _maxZ);
	glEnd();


	//! Down left line
	glBegin(GL_LINE_STRIP);
	GL::vertex(_minX, _minY, _minZ);
	GL::vertex(_minX, _maxY, _minZ);
	glEnd();

	//! Down top line
	glBegin(GL_LINE_STRIP);
	GL::vertex(_minX, _maxY, _minZ);
	GL::vertex(_maxX, _maxY, _minZ);
	glEnd();

	//! Down right line
	glBegin(GL_LINE_STRIP);
	GL::vertex(_maxX, _maxY, _minZ);
	GL::vertex(_maxX, _minY, _minZ);
	glEnd();

	//! Down bottom line
	glBegin(GL_LINE_STRIP);
	GL::vertex(_maxX, _minY, _minZ);
	GL::vertex(_minX, _minY, _minZ);
	glEnd();


	glBegin(GL_LINE_STRIP);
	GL::vertex(_minX, _minY, _maxZ);
	GL::vertex(_minX, _minY, _minZ);
	glEnd();

	glBegin(GL_LINE_STRIP);
	GL::vertex(_minX, _maxY, _maxZ);
	GL::vertex(_minX, _maxY, _minZ);
	glEnd();

	glBegin(GL_LINE_STRIP);
	GL::vertex(_maxX, _minY, _maxZ);
	GL::vertex(_maxX, _minY, _minZ);
	glEnd();

	glBegin(GL_LINE_STRIP);
	GL::vertex(_maxX, _maxY, _maxZ);
	GL::vertex(_maxX, _maxY, _minZ);
	glEnd();

//	glDisable(GL_LINE_STIPPLE);

	glEndList();
//	glDisable(GL_MULTISAMPLE);

	drawAxis();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::renderAxis() {
	glCallList(_axisBuffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::renderGrid() {
	glCallList(_gridBuffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::renderBoundingBox() {
	glCallList(_bBoxBuffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::drawAxis() {

	makeCurrent();

	if ( glIsList(_axisBuffer) )
	    glDeleteLists(_axisBuffer, 1);
	_axisBuffer = glGenLists(1);

	if ( _axisBuffer == 0 ) return;

	glNewList(_axisBuffer, GL_COMPILE);

	glLineWidth(2.);

	//! min X to 0
	glBegin(GL_LINES);
	qglColor(Qt::red);
	GL::vertex(_minX, .0, .0);
	qglColor(Qt::white);
	GL::vertex(.0, .0, .0);
	glEnd();

	//! 0 to max X
	glBegin(GL_LINES);
	qglColor(Qt::white);
	GL::vertex(.0, .0, .0);
	qglColor(Qt::red);
	GL::vertex(_maxX, .0, .0);
	glEnd();

	//! min Y to 0
	glBegin(GL_LINES);
	qglColor(Qt::green);
	GL::vertex(.0, _minY, .0);
	qglColor(Qt::white);
	GL::vertex(.0, .0, .0);
	glEnd();

	//! 0 to max Y
	glBegin(GL_LINES);
	qglColor(Qt::white);
	GL::vertex(.0, .0, .0);
	qglColor(Qt::green);
	GL::vertex(.0, _maxY, .0);
	glEnd();

	//! min Z to 0
	glBegin(GL_LINES);
	qglColor(Qt::blue);
	GL::vertex(.0, .0, _minZ);
	qglColor(Qt::white);
	GL::vertex(.0, .0, .0);
	glEnd();

	//! 0 to max Z
	glBegin(GL_LINES);
	qglColor(Qt::white);
	GL::vertex(.0, .0, .0);
	qglColor(Qt::blue);
	GL::vertex(.0, .0, _maxZ);
	glEnd();


	GLfloat height = .06;
	GLfloat radius = .02;

//	GL::cone(.0, .0, _minZ, height, radius, GL::DOWN);
	GL::cone(.0, .0, _maxZ, height, radius, GL::UP);

//	GL::cone(_minX, .0, 0., height, radius, GL::LEFT);
	GL::cone(_maxX, .0, 0., height, radius, GL::RIGHT);

//	GL::cone(.0, _minY, 0., height, radius, GL::TOFRONT);
	GL::cone(.0, _maxY, 0., height, radius, GL::TOBACK);

	glEndList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::drawBackgroundGradient() {

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP);
	qglColor(_settings.theme().topLeftColor());
	glVertex2f(-1, 1);
	qglColor(_settings.theme().bottomLeftColor());
	glVertex2f(-1, -1);
	qglColor(_settings.theme().topRightColor());
	glVertex2f(1, 1);
	qglColor(_settings.theme().bottomRightColor());
	glVertex2f(1, -1);
	glEnd();
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::drawGrid() {


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::displayInstructions(QPainter* painter) {

	/*
	 QFontMetrics metrics = QFontMetrics(font());
	 QRect rect = QRect(0, 0, width(), metrics.height() * 2);
	 QRect rect2 = QRect(2, 2, width() - 2, metrics.height() * 2 - 3);

	 painter->save();
	 painter->setRenderHint(QPainter::HighQualityAntialiasing);
	 painter->setBrush(QColor("#630F11"));
	 painter->setPen(Qt::white);
	 painter->drawRoundRect(rect, 1, 1);
	 painter->setBrush(QColor("#5B0C0E"));
	 painter->drawRoundRect(rect2, 1, 1);
	 painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap, _instructions);
	 painter->restore();
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Renderer::displayInfos(QPainter* painter) {

	painter->save();
	painter->setPen(QColor(255, 239, 239));
	painter->setBrush(_settings.theme().informationBoxColor());

	static const int margin = 5;

	QTextDocument doc;
	doc.setDefaultStyleSheet("* { color: #FFEFEF }");

	QString tr = QString("<table>"
		"<tr><td align=\"right\"><b>FPS</b>:</td><td>%1</td>"
		"<td align=\"right\"><b>%7</b>:</td><td>%4<td></tr>"
		"<tr><td align=\"right\"><b>AR</b>:</td><td>%2</td>"
		"<td align=\"right\"><b>%8</b>:</td><td>%5<td></tr>"
		"<tr><td align=\"right\"><b>FOV</b>:</td><td>%3</td>"
		"<td align=\"right\"><b>%9</b>:</td><td>%6<td></tr>"
		"</table>")
	        .arg(2)
	        .arg(QString::number(_camera->aspectRatio(), 'f', 2))
	        .arg(QString::number(_camera->fov(), 'f', 2))
	        .arg(QString::number(_camera->getAlphaOrbit(), 'f', 2))
	        .arg(QString::number(_camera->getBetaOrbit(), 'f', 2))
	        .arg(QString::number(_camera->getRhoOrbit(), 'f', 2))
	        .arg(QChar(0xb1, 0x03)/*alpha*/)
	        .arg(QChar(0xb2, 0x03)/*beta*/)
	        .arg(QChar(0xc1, 0x03)/*rho*/);
	doc.setHtml(tr);
	doc.setTextWidth(doc.size().width() + margin);

	QRect rect1 = QRect(QPoint(margin, height() - doc.size().height() - margin),
	    doc.size().toSize());

	painter->drawRect(rect1);
	painter->save();
	painter->translate(rect1.topLeft());
	doc.drawContents(painter);
	painter->restore();

	QString tr2 = QString("<table>"
		"<tr><td align=\"right\"><b>X</b>:</td><td>%1</td>"
		"<td align=\"right\"><b>R(X)</b>:</td><td>%2<td></tr>"
		"<tr><td align=\"right\"><b>Y</b>:</td><td>%3</td>"
		"<td align=\"right\"><b>R(Y)</b>:</td><td>%4<td></tr>"
		"<tr><td align=\"right\"><b>Z</b>:</td><td>%5</td>"
		"<td align=\"right\"><b>R(Z)</b>:</td><td>%6<td></tr>"
		"</table>")
	        .arg(QString::number(_camera->getXPos(), 'f', 2))
	        .arg(QString::number(_camera->getXRot(), 'f', 2))
	        .arg(QString::number(_camera->getYPos(), 'f', 2))
	        .arg(QString::number(_camera->getYRot(), 'f', 2))
	        .arg(QString::number(_camera->getZPos(), 'f', 2))
	        .arg(QString::number(_camera->getZRot(), 'f', 2));
	doc.setHtml(tr2);
	doc.setTextWidth(doc.size().width() + margin);

	QRect rect2 = QRect(QPoint(margin + rect1.topRight().x(),
	    height() - doc.size().height() - margin), doc.size().toSize());

	painter->drawRect(rect2);
	painter->save();
	painter->translate(rect2.topLeft());
	doc.drawContents(painter);
	painter->restore();

	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace OpenGL
} // namespace Gui
} // namespace IPGP
