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

#ifndef __IPGP_OPENGLGUI_CANVAS_H___
#define __IPGP_OPENGLGUI_CANVAS_H___


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>

#ifdef __APPLE__
#include "/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h"
#elif _WIN32 || _WIN64
#include <gl/gl.h>
#else
#include <GL/gl.h>
#endif

#include <QtOpenGL/QGLWidget>
#include <ipgp/core/datamodel/flags.h>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QTime>

QT_FORWARD_DECLARE_CLASS(QPushButton);


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(Camera1);


class SC_IPGP_GUI_API Camera1 : public QObject {

	Q_OBJECT

	public:
		enum RefreshMethod {
			OnActionPerformed,
			Timed
		};

		enum MouseDragItem {
			LeftButton,
			WheelButton,
			RightButton,
			None
		};

		enum RotationAxis {
			XAxis = 0x1,
			YAxis = 0x2,
			ZAxis = 0x4,
			ra_None = 0x8
		};

	public:
		Camera1(QObject* parent = NULL);
		virtual ~Camera1();

	private:
		void setScaling(const qreal&);

	public:
		void keyPressEvent(QKeyEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void mouseReleaseEvent(QMouseEvent* event);
		void mouseMoveEvent(QMouseEvent* event);
		void wheelEvent(QWheelEvent* event);

		void block();

		void setRefreshMethod(const RefreshMethod& rm) {
			_rm = rm;
		}

		void setRefreshFrequency(const qint32& freq) {
			_timer.setInterval(freq);
		}

		void setFrameSize(const QSize& size) {
			_frameSize = size;
		}

		void setZoomSensitivity(const qreal& value) {
			_zoomSensitivity = value;
		}

	private Q_SLOTS:
		void refresh();

	public Q_SLOTS:
		void zoomOut();
		void zoomIn();

	Q_SIGNALS:
		void updateRequested();
		void showDragCursor();
		void restoreDefaulCursor();

	private:
		RefreshMethod _rm;
		MouseDragItem _mdi;
		RotationAxis _ra;
		qreal _scaling;
		qreal _zoomSensitivity;

		qreal _xAngle;
		qreal _yAngle;
		qreal _zAngle;
		QPoint _lastPosition;
		QSize _frameSize;
		//		bool _dragging;
		QTimer _timer;
};




DEFINE_IPGP_SMARTPOINTER(Canvas);

/**
 * @class   Canvas
 * @package IPGP::OpenGLGUI::DataModel
 * @brief   Custom OpenGL widget
 *
 * This class provides an OpenGL widget pre-configured and supporting mouse
 * interactions through drawn data.
 * This widget is as pure as it gets, draw() and drawFrontLayer() must be
 * re-implemented by the user!
 */
class SC_IPGP_GUI_API Canvas : public QGLWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum RotationMode {
			RM_XAxis = 0x01,
			RM_YAxis = 0x02,
			RM_ZAxis = 0x04,
			RM_Default = RM_YAxis
		};

		enum CanvasDescription {
			CD_ShowXYGrid = 0x0001,
			CD_ShowYZGrid = 0x0002,
			CD_ShowZXGrid = 0x0004,
			CD_LockWheelScaling = 0x0008, //! locks the scaling from .02f to 1.f
			CD_ShowXAxis = 0x0010,
			CD_ShowYAxis = 0x0020,
			CD_ShowZAxis = 0x0040,
			CD_AutoRotateScene = 0x0080,
			CD_ShowRendererInfo = 0x0100,
			CD_Default = CD_ShowXAxis | CD_ShowYAxis | CD_ShowZAxis
			        | CD_ShowRendererInfo
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Canvas(QWidget* parent = 0,
		                const RotationMode& rm = RM_Default,
		                const CanvasDescription& cd = CD_Default,
		                Qt::WFlags f = 0);
		virtual ~Canvas();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		virtual void initializeGL();
		virtual void resizeGL(int, int);
		virtual void paintGL();

		/**
		 * @brief Draws objects like grids, axis and others that have to be
		 *        part of the background scene.
		 */
		virtual void draw();

		/**
		 * @brief Draws objects like renderer info, comments, etc that have
		 *        to be drawn in front, genuinely text objects.
		 */
		virtual void drawFrontLayer();


		virtual void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void mouseReleaseEvent(QMouseEvent*);

		void autorotateTimerState();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setRotationMode(const Canvas::RotationMode&);
		const Core::FlagSet<RotationMode>& rotationMode() const {
			return _rm;
		}
		const Core::FlagSet<CanvasDescription>& canvasDescription() const {
			return _cd;
		}
		const QString& xAxisName() const {
			return _xAxisName;
		}
		const QString& yAxisName() const {
			return _yAxisName;
		}
		const QString& zAxisName() const {
			return _zAxisName;
		}
//		const GLfloat& scalingValue() const {
//			return _scaling;
//		}

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setRendererInfoVisible(bool);
		void setXYGridVisible(bool);
		void setYZGridVisible(bool);
		void setZXGridVisible(bool);
		void setXRange(int min, int max);
		void setYRange(int min, int max);
		void setZRange(int min, int max);
		void setAutoRotationEnabled(bool state);
		//		void setAutoRotationTimerInterval(int msec);
		void setModified(bool state = true);
		//		int autoRotationTimerInterval();

		void setXAxisName(const QString& name) {
			_xAxisName = name;
		}
		void setYAxisName(const QString& name) {
			_yAxisName = name;
		}
		void setZAxisName(const QString& name) {
			_zAxisName = name;
		}

//		void setScalingValue(const GLfloat& v) {
//			_scaling = v;
//		}

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void rotate();
		void contextMenuRequest(QPoint);
		void contextMenuAction(QAction*);

		void zoomIn();
		void zoomOut();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
//		void autoRotationStateChanged(bool);

	protected:
		// ------------------------------------------------------------------
		//  Protected members
		// ------------------------------------------------------------------
		QPushButton* _zoomInButton;
		QPushButton* _zoomOutButton;

//		bool _dragging;
		int _range_x_min;
		int _range_x_max;
		int _range_y_min;
		int _range_y_max;
		int _range_z_min;
		int _range_z_max;

//		QTimer* _rotationTimer;
//		GLfloat _xRotation;
//		GLfloat _yRotation;
//		GLfloat _zRotation;
//		GLfloat _scaling;
		GLuint func_list;
		bool modified;
		//		QPoint _lastPosition;

		Core::FlagSet<RotationMode> _rm;
		Core::FlagSet<CanvasDescription> _cd;

		QMenu* _contextMenu;

		Camera1 _camera;

		QString _xAxisName;
		QString _yAxisName;
		QString _zAxisName;

		// Renderer information
		QTime _lastTime;
		int _frameCount;
		int _lastCount;
};

} // namespace Gui
} // namespace IPGP


#endif
