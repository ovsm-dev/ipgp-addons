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

#ifndef __IPGP_OPENGL_DATAMODEL_RENDERER_H__
#define __IPGP_OPENGL_DATAMODEL_RENDERER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/client/misc.h>
#include <ipgp/gui/client/scheme.h>
#ifdef __APPLE__
#include "/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h"
#elif _WIN32 || _WIN64
#include <gl/gl.h>
#else
#include <GL/gl.h>
#endif
#include <QtOpenGL/QGLWidget>
#include <QVector>
#include <QVector3D>
#include <QTimer>
#include <QMenu>


namespace IPGP {
namespace Gui {
namespace OpenGL {


/**
 * @class   RendererTheme
 * @package IPGP::Gui
 * @brief   Renderer theme manager.
 */
class SC_IPGP_GUI_API RendererTheme {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		RendererTheme();
		~RendererTheme();

	public:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		void reset();

		void setTopLeftColor(const QColor&);
		void setTopRightColor(const QColor&);
		void setBottomLeftColor(const QColor&);
		void setBottomRightColor(const QColor&);
		void setInformationBoxColor(const QColor&);
		void setInstructionBoxColor(const QColor&);

		const QColor& topLeftColor() const;
		const QColor& topRightColor() const;
		const QColor& bottomLeftColor() const;
		const QColor& bottomRightColor() const;
		const QColor& informationBoxColor() const;
		const QColor& instructionBoxColor() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QColor _topLeft;
		QColor _topRight;
		QColor _bottomLeft;
		QColor _bottomRight;
		QColor _infoBoxColor;
		QColor _instBoxColor;
};

/**
 * @class   RendererSettings
 * @package IPGP::Gui
 * @brief   Renderer general settings manager
 */
class SC_IPGP_GUI_API RendererSettings {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum RenderingMethod {
			RM_GL_VERTEX,
			RM_VERTEXARRAY,
			RM_VERTEXARRAY_INDICES,
			RM_VERTEBUFFEROBJECT_INDICES
		};

		typedef QVector<float> VColor;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		RendererSettings();
		~RendererSettings();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void reset();

		void setFOV(const float&);
		void setZoomStep(const float&);
		void setMouseSensibility(const float&);
		void setRotationDelay(int);
		void setCameraDelay(int);
		void setAxisVisible(const bool&);
		void setRendererInfoVisible(const bool&);
		void setGridVisible(const bool&);
		void setLightingActivated(const bool&);

		void setAmbientLightEnabled(const bool&);
		void setDiffuseLightEnabled(const bool&);
		void setSpecularLightEnabled(const bool&);
		void setPositionLightEnabled(const bool&);

		void setGradientVisible(const bool&);
		void setBoundingBoxVisible(const bool&);
		void setSmoothingActivated(const bool&);

		void setRenderingMethod(const RenderingMethod&);

		void setAmbientLight(VColor);
		void setSpecularLight(VColor);
		void setDiffuseLight(VColor);
		void setPositionLight(VColor);

		const float& fov() const;
		const float& zoomStep() const;
		const float& mouseSensibility() const;
		const int& rotationDelay() const;
		const int& cameraDelay() const;
		const bool& axisVisible() const;
		const bool& rendererInfoVisible() const;
		const bool& gridVisible() const;
		const bool& lightingActivated() const;

		const bool& ambientLightEnabled() const;
		const bool& diffuseLightEnabled() const;
		const bool& specularLightEnabled() const;
		const bool& positionLightEnabled() const;

		const bool& gradientVisible() const;
		const bool& boundingBoxVisible() const;
		const bool& smoothingActivated() const;
		const RenderingMethod& renderingMethod() const;

		const RendererTheme& theme() const;
		RendererTheme& theme();

		const VColor& ambientLight() const;
		const VColor& specularLight() const;
		const VColor& diffuseLight() const;
		const VColor& positionLight() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		float _fov;					//! View visible ratio transformation
		float _zoomStep;			//! rhoOrbit of the camera step
		float _mouseSensibility;	//! Mouse wheel rate

		//! Auto-replotting rate for when the scene is moving: try and emulate
		//! a 60fps to match eye speed or 24fps for film ivtc (movie friendly)
		int _rotationDelay;

		//! Fast-auto repeat simulation rate: split 1sec to match eye speed
		//! -> 1000/60~= 16.6ms
		int _cameraDelay;

		bool _axis;
		bool _info;			//! Renderer information
		bool _grid;			//! Scene grid
		bool _lighting;		//! Scene lighting
		bool _ambientLightEnabled;
		bool _diffuseLightEnabled;
		bool _specularLightEnabled;
		bool _positionLightEnabled;
		bool _gradient; 	//! Scene background gradient
		bool _boundingBox;	//! Scene bounding box
		bool _smoothing;

		RenderingMethod _renderingMethod;
		RendererTheme _theme;

		VColor _ambientLight;
		VColor _specularLight;
		VColor _diffuseLight;
		VColor _positionLight;
};

DEFINE_IPGP_SMARTPOINTER(Camera);
DEFINE_IPGP_SMARTPOINTER(Renderer);

/**
 * @class   Renderer
 * @package IPGP::Gui
 * @brief   OpenGL base renderer
 *
 * This class offers a base renderer for building an OpenGL widget. It supports
 * and handles mouse movements, disposes of a camera interface (floating view).
 * This class can't be instantiate and launched as is, it is mandatory that the
 * user re-implements it so that the virtual draw() method gets properly defined.
 */
class SC_IPGP_GUI_API Renderer : public QGLWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Renderer(QWidget* = NULL, Qt::WFlags = 0);
		~Renderer();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void initializeGL();
		void paintEvent(QPaintEvent*);
		void resizeGL(int width, int height);

		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);


		void createGradient();

		/***
		 * @brief Grabs the widget's input, awaits an escape sequence before
		 *        releasing it. This means the floating camera mode is ON.
		 *        (That sequence should be Escape key or Right click.)
		 */
		void grabInput();

		/**
		 * @brief Releases the widget's input after an escape sequence has been
		 *        received. This means the floating camera mode is now OFF.
		 */
		void releaseInput();

		/**
		 * @brief  Movement counter
		 * @return Number of true elements contained in _movingTo[]
		 */
		int movingCount();

		/**
		 * @brief Stops all active movements from camera
		 */
		void stopMoving();

		void drawAxis();
		void drawBackgroundGradient();
		void drawGrid();

		void renderAxis();
		void renderGrid();
		void renderBoundingBox();

		void setupViewport(int width, int height);

		void displayInstructions(QPainter*);

		// ------------------------------------------------------------------
		//  Protected virtual interface
		// ------------------------------------------------------------------
		//! The user MUST re-implement those methods
		virtual void drawWithLight()=0;
		virtual void drawWithoutLight()=0;

		virtual void displayInfos(QPainter*);

		void log(const Client::LogMessage&, const QString&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		//! Floating camera view mode updater
		void sceneTimerEvent();

		void setAxisVisible(const bool&);
		void setRendererInfoVisible(const bool&);
		void setGridVisible(const bool&);
		void setBoundingBoxVisible(const bool&);
		void setLightingActivated(const bool&);

		void showHelp(const bool&);

		void setSmoothingActivated(const bool&);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		//! Emitted whenever an update/drawing is happening (>0 is running, <0 means finished)
		void loadingPercentage(const int&, const QString&, const QString&);
		void statusMessage(const QString&);
		void logMessage(const int&, const QString&, const QString&);

		//! Signal emitted whenever this object instance is destroyed
		void nullifyQObject(QObject*);

	public:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void makeGrid(GLfloat z, GLfloat size, GLfloat radio);

		/**
		 * @brief Compute the object bounding box and pre-store the vertices
		 *        in GPU buffer...
		 * @param xmin the X minimum value
		 * @param xmax the X maximum value
		 * @param ymin the Y minimum value
		 * @param ymax the Y maximum value
		 * @param zmin the Z minimum value
		 * @param zmax the Z maximum value
		 */
		void setBoundingBox(const GLfloat&, const GLfloat&, const GLfloat&,
		                    const GLfloat&, const GLfloat&, const GLfloat&);

		void setGeneralSettings(const RendererSettings& s) {
			_settings = s;
		}
		RendererSettings& generalSettings() {
			return _settings;
		}
		Camera* camera() {
			return _camera;
		}

		//! Widget's scheme
		Scheme& scheme() {
			return _scheme;
		}
		void setScheme(const Scheme& scheme) {
			_scheme = scheme;
		}

		const bool& mouseGrabbed() const {
			return _mouseGrabbed;
		}

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QTimer* _sceneTimer;
		QTimer* _rotationTimer;
		Camera* _camera;

		QMenu _contextMenu;
		QAction* _infoAction;
		QAction* _gridAction;
		QAction* _bboxAction;
		QAction* _lightAction;
		QAction* _helpAction;

		RendererSettings _settings;
		Scheme _scheme;

		QRadialGradient _gradient;

		//! Key state (ZQSD)
		static const int _movingDirections = 4;
		bool _movingTo[_movingDirections];
		bool _mouseGrabbed;
		bool _movedWhileGrabbed;
		bool _cameraMode;

		QPoint _grabPos;
		QPoint _lastPos;

		GLuint _axisBuffer;
		GLuint _gridBuffer;	//! The grid buffer
		GLuint _bBoxBuffer;	//! The boundary box buffer

		GLfloat _minX;
		GLfloat _maxX;
		GLfloat _minY;
		GLfloat _maxY;
		GLfloat _minZ;
		GLfloat _maxZ;

		QString _instructions;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
