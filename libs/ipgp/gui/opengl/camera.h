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

#ifndef __IPGP_GUI_OPENGL_CAMERA_H__
#define __IPGP_GUI_OPENGL_CAMERA_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>

#ifdef __APPLE__
#include "/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h"
#elif _WIN32 || _WIN64
#pragma comment(lib, "opengl32")
#include <gl/gl.h>
#else
#include <GL/gl.h>
#endif


namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(Camera);
/**
 * @class   Camera
 * @package IPGP::Gui
 * @brief   OpenGL camera implementation
 *
 * Applied internally to a QGLWidget, this class manages an emulate
 * camera-related information, and, handles OpenGL transformations.
 */
class SC_IPGP_GUI_API Camera {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Camera(const GLfloat& x, const GLfloat& y, const GLfloat& z,
		                const GLfloat& newNear, const GLfloat& newFar);
		Camera();
		~Camera();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		//! Sets angles relative to current values (setRot assigns new values)
		void turn(GLfloat dx, GLfloat dy, GLfloat dz);
		//! Sets position relative to current values (setPos assigns new values)
		//! it simulates a Half-Life kind of camera movement
		void move(const GLfloat& angle);
		//! Sets spherical angles relative to current values (setOrbit assigns new values)
		void orbit(GLfloat dAlpha, GLfloat dBeta);

		//! Calls gluPerspective and optionally sets near and far
		void perspective(GLfloat yFov = 45., GLfloat ratio = 4. / 3., GLfloat newNear = -1., GLfloat newFar = -1.);

		//! Calls glTranslatef using -{x,y,z}Pos
		void applyPos();
		//! Calls glRotatef using -{x,y,z}Rot
		void applyRot();


		//! Mathematical implementation of gluLookAt (doesn't make OpenGL calls)
		void lookAt(GLfloat x = 0, GLfloat y = 0, GLfloat z = 0);
		//! Adjust spherical coords from the current cartesian coordinates
		void sphericalFromPosition();
		//! Sets the cartesian coordinates given the current spherical coordinates
		void commitSpherical();

		//! Resets parameters to standard initialization values
		void reset();

		//! Returns the distance value between the current viewing position
		//! and the central position of the view
		const GLfloat distFromOrigin();

		//! Setters
		void setPos(const GLfloat& x, const GLfloat& y, const GLfloat& z);
		void setXPos(const GLfloat&);
		void setYPos(const GLfloat&);
		void setZPos(const GLfloat&);

		void modifyAspectRatio(const int& delta);
		void modifyFieldOfView(const int& delta);

		void setRot(const GLfloat& newXRot, const GLfloat& newYRot, const GLfloat& newZRot);
		void setXRot(const GLfloat&);
		void setYRot(const GLfloat&);
		void setZRot(const GLfloat&);

		void setOrbit(const GLfloat& newAlpha, const GLfloat& newBeta, const GLfloat& newRho);

		// Use commitSpherical() to commit changes to {x,y,z}Pos
		void setAlphaOrbit(GLfloat alpha);
		void setBetaOrbit(GLfloat beta);
		void setRhoOrbit(GLfloat rho);

		void setLimits(GLfloat newX, GLfloat newY, GLfloat newZ, GLfloat newRho);
		void setStep(GLfloat newSpeed);

		void setRun(bool newRun);

		//! Getters
		GLfloat getXPos() const;
		GLfloat getYPos() const;
		GLfloat getZPos() const;

		const GLfloat& aspectRatio() const;
		const GLfloat& fov() const;

		GLfloat getXRot() const;
		GLfloat getYRot() const;
		GLfloat getZRot() const;

		GLfloat getXMax() const;
		GLfloat getYMax() const;
		GLfloat getZMax() const;
		GLfloat getRhoMax() const;

		GLfloat getAlphaOrbit() const;
		GLfloat getBetaOrbit() const;
		GLfloat getRhoOrbit() const;

		GLfloat getNear() const;
		GLfloat getFar() const;
		GLfloat getStep() const;

		bool getRun() const;

		//! Sets val between min and max
		//! @todo Make those templates
		static GLfloat limit(GLfloat val, const GLfloat& min, const GLfloat& max);
		static int limit(int val, const int& min, const int& max);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		//! Real coords
		//! Camera Azimuth
		GLfloat xPos;
		//! Camera elevation above the horizon
		GLfloat yPos;
		//! Almost never used, but supported
		GLfloat zPos;

		//! Rotation
		GLfloat xRot, yRot, zRot;

		//! Position limits: [-max, max]
		GLfloat xMax, yMax, zMax, rhoMax;

		//! Minimum and Maximum viewing distance
		GLfloat nearEnd;
		GLfloat farEnd;

		//! Moving speed
		GLfloat step;
		bool run;
		GLfloat runDelta;

		GLfloat _aspectRatio;
		GLfloat _aspectRatioStep;

		GLfloat _fov;
		GLfloat _fovStep;

		/**
		 * Camera orbitation around origin, these coords are applied to
		 * {x,y,z}Rot with commitSpherical()
		 * Basic idea: http://deslab.mit.edu/DesignLab/courses/13.016/graphics/viewing.html
		 * http://en.wikipedia.org/wiki/Spherical_coordinates
		 * theta -> alphaOrbit
		 * phi -> betaOrbit
		 * rho -> rhoOrbit
		 */
		GLfloat alphaOrbit, betaOrbit, rhoOrbit;
};


} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
