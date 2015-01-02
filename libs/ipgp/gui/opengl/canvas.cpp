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

#include <ipgp/gui/opengl/canvas.h>
#include <QtOpenGL>

#include <QDebug>
#include <QObject>
#include <QtGui>
#include "math.h"



namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Camera1::Camera1(QObject* parent) :
		QObject(parent) {

	_xAngle = 40.0;
	_yAngle = -40.0;
	_zAngle = 0.0;
	_scaling = 0.28;
	_zoomSensitivity = 1.125;
	_rm = OnActionPerformed;
	_mdi = None;
	_ra = XAxis;
	_timer.setInterval(1000);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(refresh()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Camera1::~Camera1() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::keyPressEvent(QKeyEvent* event) {

//			if ( event->key() == Qt::Key_R ) {

//					if ( _cd.isSet(CD_AutoRotateScene) )
//						_cd.remove(CD_AutoRotateScene);
//					else
//						_cd.set(CD_AutoRotateScene);

//					autorotateTimerState();
//				}

	if ( event->key() == Qt::Key_Up && !event->modifiers() ) {
		_xAngle += 180. * .005;
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	if ( event->key() == Qt::Key_Down && !event->modifiers() ) {
		_xAngle -= 180. * .005;
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	if ( event->key() == Qt::Key_Left ) {
		_zAngle += 180. * .005;
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	if ( event->key() == Qt::Key_Right ) {
		_zAngle -= 180. * .005;
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	if ( event->key() == Qt::Key_Up
	        && event->modifiers() == Qt::ShiftModifier ) {
		_yAngle += 180. * .005;
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	if ( event->key() == Qt::Key_Down
	        && event->modifiers() == Qt::ShiftModifier ) {
		_yAngle -= 180. * .005;
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	if ( event->key() == Qt::Key_Plus )
		zoomIn();

	if ( event->key() == Qt::Key_Minus )
		zoomOut();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::mousePressEvent(QMouseEvent* event) {

	_mdi = None;

	if ( event->button() == Qt::LeftButton )
		_mdi = LeftButton;

	if ( event->button() == Qt::RightButton )
		_mdi = RightButton;

	if ( event->button() == Qt::MiddleButton )
		_mdi = WheelButton;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::mouseReleaseEvent(QMouseEvent* event) {

	Q_UNUSED(event);

	if ( _mdi != None ) {
		_mdi = None;
		emit restoreDefaulCursor();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::mouseMoveEvent(QMouseEvent* event) {

	qreal dx = static_cast<qreal>(event->x() - _lastPosition.x()) / _frameSize.width();
	qreal dy = static_cast<qreal>(event->y() - _lastPosition.y()) / _frameSize.height();

	if ( _mdi != None ) {

		switch ( _mdi ) {
			case LeftButton:
				_xAngle += 180. * dy;
				_yAngle += 180. * dx;
			break;
			case RightButton:
				_xAngle += 180. * dy;
				_zAngle += 180. * dx;
			break;
			case WheelButton:
				//! Handle this in the block method!
				_xAngle += 180. * dx;
//				_zAngle += 180. * dx;
//				_yAngle += 180. * dy;
			break;
			default:
				break;
		}

		emit showDragCursor();
		if ( _rm == OnActionPerformed )
			emit updateRequested();
	}

	_lastPosition = event->pos();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::wheelEvent(QWheelEvent* event) {

	setScaling((static_cast<qreal>(-event->delta()) / 8.) / 15.);

	if ( _rm == OnActionPerformed )
		emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::setScaling(const qreal& steps) {
	_scaling /= pow(_zoomSensitivity, steps);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::zoomOut() {

	setScaling((120. / 8.) / 15.);

	if ( _rm == OnActionPerformed )
		emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::zoomIn() {

	setScaling((-120. / 8.) / 15);

	if ( _rm == OnActionPerformed )
		emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::block() {

	glTranslatef(0.0, 0.0, -10.0);
	glScalef(_scaling, _scaling, _scaling);

	if ( _mdi != WheelButton ) {
		glRotatef(_xAngle, 1., .0, .0);
		glRotatef(_yAngle, .0, 1., .0);
		glRotatef(_zAngle, .0, .0, 1.);
	}
	else {
		glRotatef(_xAngle, 1., .0, .0);
		glRotatef(_yAngle, .0, .0, .0);
		glRotatef(_zAngle, .0, .0, 1.);
	}

	glRotatef(90.0, 1.0, 0.0, 0.0);
	glRotatef(180.0, 0.0, 0.0, 1.0);
	glScalef(-1, -1, -1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Camera1::refresh() {
	emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



const int const_rotation = 3;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Canvas::Canvas(QWidget* parent, const RotationMode& rm,
               const CanvasDescription& cd, Qt::WFlags f) :
		QGLWidget(parent), _rm(Core::FlagSet<RotationMode>(rm)),
		_cd(Core::FlagSet<CanvasDescription>(cd)) {

	setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer));
	setWindowFlags(f);
	setMouseTracking(true);

	connect(&_camera, SIGNAL(updateRequested()), this, SLOT(updateGL()));

//	_range_x_min = -100;
//	_range_x_max = 100;
//	_range_y_min = -100;
//	_range_y_max = 100;
//	_range_z_min = -10;
//	_range_z_max = 10;

	_range_x_min = -100;
	_range_x_max = 100;
	_range_y_min = -100;
	_range_y_max = 100;
	_range_z_min = -100;
	_range_z_max = 100;

	_xAxisName = "x";
	_yAxisName = "y";
	_zAxisName = "z";

	modified = true;

	setAttribute(Qt::WA_OpaquePaintEvent, false);

//	_rotationTimer = new QTimer(this);
//	connect(_rotationTimer, SIGNAL(timeout()), this, SLOT(rotate()));
//	setAutoRotationTimerInterval(10 * const_rotation);

	setFocusPolicy(Qt::WheelFocus);


	// Timer settings
	_frameCount = 0;
	_lastCount = 0;
	_lastTime = QTime::currentTime();


	// Zoom-in button icon
	QIcon ziIcon;
	ziIcon.addFile(QString(":images/szin.png"), QSize(28, 28), QIcon::Normal, QIcon::Off);
	_zoomInButton = new QPushButton(this);
	_zoomInButton->setObjectName("zoomInButton");
	_zoomInButton->setIcon(ziIcon);
	_zoomInButton->setToolTip("Zoom in!");
	_zoomInButton->setGeometry(5, 5, 30, 30);
	_zoomInButton->hide();
	connect(_zoomInButton, SIGNAL(pressed()), this, SLOT(zoomIn()));

	// Zoom-out button icon
	QIcon zoIcon;
	zoIcon.addFile(QString(":images/szout.png"), QSize(28, 28), QIcon::Normal, QIcon::Off);
	_zoomOutButton = new QPushButton(this);
	_zoomOutButton->setObjectName("zoomOutButton");
	_zoomOutButton->setGeometry(5, 30 + 7, 30, 30);
	_zoomOutButton->setIcon(zoIcon);
	_zoomOutButton->setToolTip("Zoom out!");
	_zoomOutButton->hide();
	connect(_zoomOutButton, SIGNAL(pressed()), this, SLOT(zoomOut()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Canvas::~Canvas() {

//	if ( _rotationTimer ) {
//		_rotationTimer->stop();
//		disconnect(_rotationTimer);
//
//		delete _rotationTimer;
//		_rotationTimer = NULL;
//	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setRotationMode(const Canvas::RotationMode& rm) {

	_rm.remove(RM_XAxis);
	_rm.remove(RM_YAxis);
	_rm.remove(RM_ZAxis);

	// TODO: allow user to set more than one rotation mode
	_rm.set(rm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setRendererInfoVisible(bool val) {

//	if ( _cd.isSet(CD_ShowRendererInfo) && val )
//		return;
//	else if ( _cd.isSet(CD_ShowRendererInfo) && !val )
//		_cd.remove(CD_ShowRendererInfo);
//	else if ( !_cd.isSet(CD_ShowRendererInfo) && val )
//		_cd.set(CD_ShowRendererInfo);

	val ? _cd.set(CD_ShowRendererInfo) : _cd.remove(CD_ShowRendererInfo);

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setXYGridVisible(bool val) {

	if ( _cd.isSet(CD_ShowXYGrid) && val )
		return;
	else if ( _cd.isSet(CD_ShowXYGrid) && !val )
		_cd.remove(CD_ShowXYGrid);
	else if ( !_cd.isSet(CD_ShowXYGrid) && val )
		_cd.set(CD_ShowXYGrid);

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setYZGridVisible(bool val) {

	if ( _cd.isSet(CD_ShowYZGrid) && val )
		return;
	else if ( _cd.isSet(CD_ShowYZGrid) && !val )
		_cd.remove(CD_ShowYZGrid);
	else if ( !_cd.isSet(CD_ShowYZGrid) && val )
		_cd.set(CD_ShowYZGrid);

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setZXGridVisible(bool val) {

	if ( _cd.isSet(CD_ShowZXGrid) && val )
		return;
	else if ( _cd.isSet(CD_ShowZXGrid) && !val )
		_cd.remove(CD_ShowZXGrid);
	else if ( !_cd.isSet(CD_ShowZXGrid) && val )
		_cd.set(CD_ShowZXGrid);

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setXRange(int min, int max) {

	if ( _range_x_min == min && _range_x_max == max )
		return;

	_range_x_min = min;
	_range_x_max = max;
	modified = true;

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setYRange(int min, int max) {

	if ( _range_y_min == min && _range_y_max == max )
		return;

	_range_y_min = min;
	_range_y_max = max;
	modified = true;

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setZRange(int min, int max) {

	if ( _range_z_min == min && _range_z_max == max )
		return;

	_range_z_min = min;
	_range_z_max = max;
	modified = true;

	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setAutoRotationEnabled(bool state) {

	if ( state && _cd.isSet(CD_AutoRotateScene) )
		return;
	else if ( !state && _cd.isSet(CD_AutoRotateScene) )
		_cd.remove(CD_AutoRotateScene);
	else if ( state && !_cd.isSet(CD_AutoRotateScene) )
		_cd.set(CD_AutoRotateScene);

	autorotateTimerState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//void Canvas::setAutoRotationTimerInterval(int msec) {
//	_rotationTimer->setInterval(msec);
//}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setModified(bool state) {

	if ( modified == state )
		return;

	modified = state;
	updateGL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//int Canvas::autoRotationTimerInterval() {
//	return _rotationTimer->interval();
//}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::draw() {

//	glDisable(GL_LIGHTING); // By default lighting disabled

	glLoadName(0);

	// Light sources and directions
	const GLfloat first_pos[4] = { 0.0f, 0.0f, 50.0f, 0.0f };
	const GLfloat first_dir[4] = { 0.0f, 0.0f, -1.0f, 0.0f };
	const GLfloat second_pos[4] = { 0.0f, 0.0f, -50.0f, 0.0f };
	const GLfloat second_dir[4] = { 0.0f, 0.0f, 1.0f, 0.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, first_pos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, first_dir);
	glLightfv(GL_LIGHT1, GL_POSITION, second_pos);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, second_dir);

	double x_min = (double) _range_x_min - 0.5;
	double x_max = (double) _range_x_max + 0.5;
	double y_min = (double) _range_y_min - 0.5;
	double y_max = (double) _range_y_max + 0.5;
	double z_min = (double) _range_z_min - 0.5;
	double z_max = (double) _range_z_max + 0.5;


	QFont font(this->font());
	font.setPointSizeF(16.);


	if ( _cd.isSet(CD_ShowXAxis) ) {

		glBegin(GL_LINES);
		qglColor(Qt::white);
		glVertex3d(0.0f, 0.0f, 0.0f);
		qglColor(Qt::red);
		glVertex3d((double) _range_x_max, 0.0f, 0.0f);
		glEnd();

		qglColor(Qt::white);
		renderText((double) _range_x_max + 1.7, 0.0, 0.0, _xAxisName, font);
	}



	if ( _cd.isSet(CD_ShowYAxis) ) {

		glBegin(GL_LINES);
		qglColor(Qt::white);
		glVertex3d(0.0f, 0.0f, 0.0f);
		qglColor(Qt::blue);
		glVertex3d(0.0f, (double) _range_y_max, 0.0f);
		glEnd();

		qglColor(Qt::white);
		renderText(0.0, (double) _range_y_max + 1.7, 0.0, _yAxisName, font);
	}

	if ( _cd.isSet(CD_ShowZAxis) ) {

		glBegin(GL_LINES);
		qglColor(Qt::white);
		glVertex3d(0.0f, 0.0f, 0.0f);
		qglColor(Qt::green);
		glVertex3d(0.0f, 0.0f, (double) _range_z_max);
		glEnd();

		qglColor(Qt::white);
		renderText(0.0, 0.0, (double) _range_z_max + 1.7, _zAxisName, font);
	}



	qglColor(QColor(128, 128, 128, 128));
	if ( _cd.isSet(CD_ShowXYGrid) ) {

		glBegin(GL_LINES);
		for (int i = _range_y_min; i <= _range_y_max; i++) {
			if ( i != 0 ) {
				glVertex3d(x_min, (double) i, 0.0);
				glVertex3d(x_max, (double) i, 0.0);
			}
		}
		for (int i = _range_x_min; i <= _range_x_max; i++) {
			if ( i != 0 ) {
				glVertex3d((double) i, y_min, 0.0);
				glVertex3d((double) i, y_max, 0.0);
			}
		}
		glEnd();
	}

	if ( _cd.isSet(CD_ShowYZGrid) ) {

		glBegin(GL_LINES);
		for (int i = _range_z_min; i <= _range_z_max; ++i) {
			if ( i != 0 ) {
				glVertex3d(0.0, y_min, (double) i);
				glVertex3d(0.0, y_max, (double) i);
			}
		}
		for (int i = _range_y_min; i <= _range_y_max; ++i) {
			if ( i != 0 ) {
				glVertex3d(0.0, (double) i, z_min);
				glVertex3d(0.0, (double) i, z_max);
			}
		}
		glEnd();
	}

	if ( _cd.isSet(CD_ShowZXGrid) ) {

		glBegin(GL_LINES);
		for (int i = _range_z_min; i <= _range_z_max; ++i) {
			if ( i != 0 ) {
				glVertex3d(x_min, 0.0, (double) i);
				glVertex3d(x_max, 0.0, (double) i);
			}
		}
		for (int i = _range_x_min; i <= _range_x_max; ++i) {
			if ( i != 0 ) {
				glVertex3d((double) i, 0.0, z_min);
				glVertex3d((double) i, 0.0, z_max);
			}
		}
		glEnd();
	}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::drawFrontLayer() {

	if ( _cd.isSet(CD_ShowRendererInfo) ) {

		++_frameCount;
		QTime newTime = QTime::currentTime();
		// if 1 sec (1000 msec) ellapsed
		if ( _lastTime.msecsTo(newTime) >= 1000 ) {
			_lastCount = _frameCount;
			_frameCount = 0;
			_lastTime = QTime::currentTime();
		}

		/*
		 qglColor(Qt::white);
		 renderText(10, 20, QString::fromUtf8("%1:%2° %3:%4° %5:%6°")
		 .arg(_xAxisName).arg((int) _xRotation)
		 .arg(_yAxisName).arg((int) _yRotation)
		 .arg(_zAxisName).arg((int) _zRotation));
		 renderText(10, 35, QString("Scaling: %1%").arg(_scaling));
		 renderText(10, 50, QString("FPS: %2").arg(_lastCount));
		 */
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::initializeGL() {


//	qglClearColor(palette().color(QPalette::NoRole));
	setBackgroundRole(QPalette::Window);

	//	qglClearColor(Qt::black);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

//	func_list = glGenLists(1);

	const GLfloat black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	const GLfloat diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat specular[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 8.0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 90.0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLighti(GL_LIGHT0, GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 8.0);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90.0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLighti(GL_LIGHT1, GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::resizeGL(int width, int height) {

	_camera.setFrameSize(QSize(width, height));
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat x = GLfloat(width) / height;
	glFrustum(-x, x, -1.0, 1.0, 4.0, 15.0);
	glMatrixMode(GL_MODELVIEW);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//	_camera.look();



//	glTranslatef(0.0, 0.0, -10.0);
//	glScalef(_scaling, _scaling, _scaling);
//	glRotatef(_xRotation, 1.0, 0.0, 0.0);
//	glRotatef(_yRotation, 0.0, 1.0, 0.0);
//	glRotatef(_zRotation, 0.0, 0.0, 1.0);

//	glRotatef(90.0, 1.0, 0.0, 0.0);
//	glRotatef(180.0, 0.0, 0.0, 1.0);
//	glScalef(-1, -1, -1);

	_camera.block();

	draw();

	drawFrontLayer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::keyPressEvent(QKeyEvent* event) {

	_camera.keyPressEvent(event);
//	return;

	if ( event->key() == Qt::Key_R ) {

		if ( _cd.isSet(CD_AutoRotateScene) )
			_cd.remove(CD_AutoRotateScene);
		else
			_cd.set(CD_AutoRotateScene);

		autorotateTimerState();
	}
//
//	if ( event->key() == Qt::Key_Up && !event->modifiers() ) {
//		_xRotation += 180 * .005f;
//		updateGL();
//	}
//
//	if ( event->key() == Qt::Key_Down && !event->modifiers() ) {
//		_xRotation -= 180 * .005f;
//		updateGL();
//	}
//
//	if ( event->key() == Qt::Key_Left ) {
//		_zRotation += 180 * .005f;
//		updateGL();
//	}
//
//	if ( event->key() == Qt::Key_Right ) {
//		_zRotation -= 180 * .005f;
//		updateGL();
//	}
//
//	if ( event->key() == Qt::Key_Up
//	        && event->modifiers() == Qt::ShiftModifier ) {
//		_yRotation += 180 * .005f;
//		updateGL();
//	}
//
//	if ( event->key() == Qt::Key_Down
//	        && event->modifiers() == Qt::ShiftModifier ) {
//		_yRotation -= 180 * .005f;
//		updateGL();
//	}
//
//
//	double numDegrees = -120 / 8.;
//	double numSteps = numDegrees / 15.;
//
//	if ( event->key() == Qt::Key_Plus ) {
//		_scaling /= (GLfloat) pow(1.125, numSteps);
//		updateGL();
//	}
//
//	if ( event->key() == Qt::Key_Minus ) {
//		_scaling /= (GLfloat) pow(1.125, -numSteps);
//		updateGL();
//	}

//	QGLWidget::keyPressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::mousePressEvent(QMouseEvent* event) {

	_camera.mousePressEvent(event);
//	return;

	/*
	 if ( event->button() == Qt::RightButton
	 && event->modifiers() == Qt::ControlModifier )
	 contextMenuRequest(event->pos());

	 //	_lastPosition = event->pos();

	 if ( !_cd.isSet(CD_AutoRotateScene) )
	 return;

	 _cd.remove(CD_AutoRotateScene);

	 autorotateTimerState();
	 */

//	QGLWidget::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::contextMenuRequest(QPoint pos) {

	_contextMenu = new QMenu(this);
	_contextMenu->setAttribute(Qt::WA_DeleteOnClose);

	// Axis show/hide
	QMenu* axisMenu = _contextMenu->addMenu("Axis");
	QAction* aX = new QAction(axisMenu);
	aX->setCheckable(true);
	_cd.isSet(CD_ShowXAxis) ? aX->setChecked(true) : aX->setChecked(false);
	aX->setText(QString("X axis"));
	aX->setData(QVariant(CD_ShowXAxis));
	axisMenu->addAction(aX);

	QAction* aY = new QAction(axisMenu);
	aY->setCheckable(true);
	_cd.isSet(CD_ShowYAxis) ? aY->setChecked(true) : aY->setChecked(false);
	aY->setText(QString("Y axis"));
	aY->setData(QVariant(CD_ShowYAxis));
	axisMenu->addAction(aY);

	QAction* aZ = new QAction(axisMenu);
	aZ->setCheckable(true);
	_cd.isSet(CD_ShowZAxis) ? aZ->setChecked(true) : aZ->setChecked(false);
	aZ->setText(QString("Z axis"));
	aZ->setData(QVariant(CD_ShowZAxis));
	axisMenu->addAction(aZ);

	// Grids
	QMenu* gridMenu = _contextMenu->addMenu("Grids");
	QAction* aXY = new QAction(gridMenu);
	aXY->setCheckable(true);
	_cd.isSet(CD_ShowXYGrid) ? aXY->setChecked(true) : aXY->setChecked(false);
	aXY->setText(QString("XY grid"));
	aXY->setData(QVariant(CD_ShowXYGrid));
	gridMenu->addAction(aXY);

	QAction* aYZ = new QAction(gridMenu);
	aYZ->setCheckable(true);
	_cd.isSet(CD_ShowYZGrid) ? aYZ->setChecked(true) : aYZ->setChecked(false);
	aYZ->setText(QString("YZ grid"));
	aYZ->setData(QVariant(CD_ShowYZGrid));
	gridMenu->addAction(aYZ);

	QAction* aZX = new QAction(gridMenu);
	aZX->setCheckable(true);
	_cd.isSet(CD_ShowZXGrid) ? aZX->setChecked(true) : aZX->setChecked(false);
	aZX->setText(QString("ZX grid"));
	aZX->setData(QVariant(CD_ShowZXGrid));
	gridMenu->addAction(aZX);


	// Rotation
	QMenu* moveMenu = _contextMenu->addMenu("Movement");
	QAction* aRotate = new QAction(moveMenu);
	aRotate->setCheckable(true);
	_cd.isSet(CD_AutoRotateScene) ? aRotate->setChecked(true) : aRotate->setChecked(false);
	aRotate->setText(QString("Rotate scene"));
	aRotate->setData(QVariant(CD_AutoRotateScene));
	moveMenu->addAction(aRotate);


	// Renderer information
	QMenu* rendererMenu = _contextMenu->addMenu("Renderer");
	QAction* aInfo = new QAction(rendererMenu);
	aInfo->setCheckable(true);
	_cd.isSet(CD_ShowRendererInfo) ? aInfo->setChecked(true) : aInfo->setChecked(false);
	aInfo->setText(QString("Scene information"));
	aInfo->setData(QVariant(CD_ShowRendererInfo));
	rendererMenu->addAction(aInfo);


	connect(_contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuAction(QAction*)));
	_contextMenu->popup(mapToGlobal(pos));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::contextMenuAction(QAction* action) {

	if ( !action )
		return;

	switch ( action->data().toInt() ) {
		case CD_ShowXAxis:
			_cd.isSet(CD_ShowXAxis) ? _cd.remove(CD_ShowXAxis) : _cd.set(CD_ShowXAxis);
		break;
		case CD_ShowYAxis:
			_cd.isSet(CD_ShowYAxis) ? _cd.remove(CD_ShowYAxis) : _cd.set(CD_ShowYAxis);
		break;
		case CD_ShowZAxis:
			_cd.isSet(CD_ShowZAxis) ? _cd.remove(CD_ShowZAxis) : _cd.set(CD_ShowZAxis);
		break;
		case CD_ShowXYGrid:
			_cd.isSet(CD_ShowXYGrid) ? _cd.remove(CD_ShowXYGrid) : _cd.set(CD_ShowXYGrid);
		break;
		case CD_ShowYZGrid:
			_cd.isSet(CD_ShowYZGrid) ? _cd.remove(CD_ShowYZGrid) : _cd.set(CD_ShowYZGrid);
		break;
		case CD_ShowZXGrid:
			_cd.isSet(CD_ShowZXGrid) ? _cd.remove(CD_ShowZXGrid) : _cd.set(CD_ShowZXGrid);
		break;
		case CD_AutoRotateScene:
			_cd.isSet(CD_AutoRotateScene) ? _cd.remove(CD_AutoRotateScene) : _cd.set(CD_AutoRotateScene);
			autorotateTimerState();
		break;
		case CD_ShowRendererInfo:
			_cd.isSet(CD_ShowRendererInfo) ? _cd.remove(CD_ShowRendererInfo) : _cd.set(CD_ShowRendererInfo);
		break;
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::mouseReleaseEvent(QMouseEvent* event) {

	_camera.mouseReleaseEvent(event);
//	return;

//	if ( _dragging ) {
//		this->setCursor(Qt::ArrowCursor);
//		_dragging = false;
//	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::mouseMoveEvent(QMouseEvent* event) {

	_camera.mouseMoveEvent(event);
//	return;

	if ( (event->pos().x() < 30) && (event->pos().x() > 0)
	        && (event->pos().y() < 30 * 2) && (event->pos().y() > 0) ) {
		_zoomInButton->show();
		_zoomOutButton->show();
	}
	else {
		_zoomInButton->hide();
		_zoomOutButton->hide();
	}

//	GLfloat dx = GLfloat(event->x() - _lastPosition.x()) / width();
//	GLfloat dy = GLfloat(event->y() - _lastPosition.y()) / height();
//
//	if ( event->buttons() & Qt::LeftButton ) {
//		_xRotation += 180 * dy;
//		_yRotation += 180 * dx;
//		_dragging = true;
//		this->setCursor(Qt::ClosedHandCursor);
//		updateGL();
//	}
//	else if ( event->buttons() & Qt::RightButton ) {
//		_xRotation += 180 * dy;
//		_zRotation += 180 * dx;
//		_dragging = true;
//		this->setCursor(Qt::ClosedHandCursor);
//		updateGL();
//	}
//
//	_lastPosition = event->pos();

//	QGLWidget::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::zoomIn() {

	/*
	 double numDegrees = -120 / 8.;
	 double numSteps = numDegrees / 15.;

	 _scaling /= (GLfloat) pow(1.125, numSteps);

	 if ( _cd.isSet(CD_LockWheelScaling) ) {

	 if ( _scaling > 1.f )
	 _scaling = 1.f;

	 if ( _scaling < .02f )
	 _scaling = .02f;
	 }

	 update();
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::zoomOut() {

	/*
	 double numDegrees = 120 / 8.;
	 double numSteps = numDegrees / 15.;

	 _scaling /= (GLfloat) pow(1.125, numSteps);

	 if ( _cd.isSet(CD_LockWheelScaling) ) {

	 if ( _scaling > 1.f )
	 _scaling = 1.f;

	 if ( _scaling < .02f )
	 _scaling = .02f;
	 }

	 update();
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::wheelEvent(QWheelEvent* event) {

	_camera.wheelEvent(event);
	/*
	 double numDegrees = -event->delta() / 8.;
	 double numSteps = numDegrees / 15.;

	 _scaling /= (GLfloat) pow(1.125, numSteps);

	 if ( _cd.isSet(CD_LockWheelScaling) ) {

	 if ( _scaling > 1.f )
	 _scaling = 1.f;

	 if ( _scaling < .02f )
	 _scaling = .02f;
	 }

	 updateGL();
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::autorotateTimerState() {

//	if ( _cd.isSet(CD_AutoRotateScene) )
//		_rotationTimer->start();
//	else
//		_rotationTimer->stop();

//	emit autoRotationStateChanged (_autoRotation);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::rotate() {

	/*
	 if ( _rm.isSet(RM_XAxis) )
	 _xRotation = int(const_rotation * _xRotation + 2) % (360 * const_rotation) / (double) const_rotation;

	 if ( _rm.isSet(RM_YAxis) )
	 _yRotation = int(const_rotation * _yRotation + 2) % (360 * const_rotation) / (double) const_rotation;

	 if ( _rm.isSet(RM_ZAxis) )
	 _zRotation = int(const_rotation * _zRotation + 2) % (360 * const_rotation) / (double) const_rotation;

	 updateGL();
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
