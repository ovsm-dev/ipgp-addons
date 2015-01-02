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



#include <ipgp/gui/opengl/gl.h>
#include <ipgp/gui/opengl/vertex.h>

#include <QDebug>
#include <QString>
#include <QColor>
#include <math.h>


/*  Poor man's approximation of PI */
#define PI 3.1415926535898
/*  Macro for sin & cos in degrees */
#define Cos(th) cos(PI/180*(th))
#define Sin(th) sin(PI/180*(th))
/*  D degrees of rotation */
#define DEF_D 5


namespace IPGP {
namespace Gui {
namespace OpenGL {
namespace GL {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void normal(const GLfloat& x, const GLfloat& y, const GLfloat& z) {
	glNormal3f(x, z, -y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void normal(const Vertex& vec) {
	glNormal3f(vec.x(), vec.z(), -vec.y());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void vertex(const GLfloat& x, const GLfloat& y, const GLfloat& z) {
	glVertex3f(x, z, -y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void vertex(const Vertex& vec) {
	glVertex3f(vec.x(), vec.z(), -vec.y());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void vertex(const GLfloat& th2, const GLfloat& ph2) {
	GLfloat x = Sin(th2) * Cos(ph2);
	GLfloat y = Cos(th2) * Cos(ph2);
	GLfloat z = Sin(ph2);
	vertex(x, y, z);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void translate(const GLfloat& x, const GLfloat& y, const GLfloat& z) {
	glTranslatef(x, z, -y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void rotate(const GLfloat& x, const GLfloat& y, const GLfloat& z) {
	glRotatef(x, 1, 0, 0);
	glRotatef(z, 0, 1, 0);
	glRotatef(y, 0, 0, 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void scale(const GLfloat& x, const GLfloat& y, const GLfloat& z) {
	glScalef(x, z, y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void color(const GLfloat& r, const GLfloat& g, const GLfloat& b,
           const GLfloat& a) {
	glColor4f(r, g, b, a);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void color(const Vertex& vec, const GLfloat& a) {
	glColor4f(vec.x(), vec.y(), vec.z(), a);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GLenum checkError(const QString& file, const int& line) {

	GLenum glErr = glGetError();
//	while ( glErr != GL_NO_ERROR ) {
//		const GLubyte* sError = gluErrorString(glErr);
//
//		qDebug() << "** GL Error" << endl <<
//		         "  Number: " << glErr << endl <<
//		         "  Message:" << sError << endl <<
//		         "  File:   " << file << endl <<
//		         "  Line:   " << line;
//
//		glErr = glGetError();
//	}

	return glErr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void useVBO(bool enable) {

	if ( enable ) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_INDEX_ARRAY);
	}
	else {
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_INDEX_ARRAY);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void cone(const GLfloat& x, const GLfloat& y, const GLfloat& z,
          const GLfloat& height, const GLfloat& radius, const Direction& d,
          const QColor& color) {

	glColor3f(color.redF(), color.greenF(), color.blueF());

	// draw the upper part of the cone
	glBegin(GL_TRIANGLE_FAN);

	if ( d == UP ) {
		normal(x, y, z + height);
		vertex(x, y, z + height);
	}
	else if ( d == DOWN ) {
		normal(x, y, z - height);
		vertex(x, y, z - height);
	}
	else if ( d == LEFT ) {
		normal(x - height, y, z);
		vertex(x - height, y, z);
	}
	else if ( d == RIGHT ) {
		normal(x + height, y, z);
		vertex(x + height, y, z);
	}
	else if ( d == TOFRONT ) {
		normal(x, y - height, z);
		vertex(x, y - height, z);
	}
	else if ( d == TOBACK ) {
		normal(x, y + height, z);
		vertex(x, y + height, z);
	}

	for (int angle = 0; angle < 360; ++angle) {
		if ( d == UP || d == DOWN ) {
			normal(sin(angle) * radius, cos(angle) * radius, z);
			vertex(sin(angle) * radius, cos(angle) * radius, z);
		}
		else if ( d == LEFT || d == RIGHT ) {
			normal(x, cos(angle) * radius, sin(angle) * radius);
			vertex(x, cos(angle) * radius, sin(angle) * radius);
		}
		else if ( d == TOFRONT || d == TOBACK ) {
			normal(sin(angle) * radius, y, cos(angle) * radius);
			vertex(sin(angle) * radius, y, cos(angle) * radius);
		}
	}
	glEnd();

	// draw the base of the cone
//	glBegin(GL_TRIANGLE_FAN);
//	vertex(x, y, z);
//	for (int angle = 0; angle < 360; ++angle) {
	// normal is just pointing down
//	    glNormal3f(0, -1, 0);
//		vertex(sin(angle) * radius, cos(angle) * radius, z);
//	}
//	glEnd();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void drawSphere(GLfloat r, const GLfloat& x, const GLfloat& y, const GLfloat& z,
                const QColor& color) {

	int lats = 20;
	int longs = 20;
	if ( r == .0 )
		r = .001;

	glPushMatrix();
	translate(x, y, z);
	glColor3f(color.redF(), color.greenF(), color.blueF());

	for (int i = 0; i <= lats; ++i) {
		double lat0 = M_PI * (-.5 + (double) (i - 1) / lats);
		double z0 = sin(lat0);
		double zr0 = cos(lat0);

		double lat1 = M_PI * (-.5 + (double) i / lats);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= longs; ++j) {
			double lng = 2 * M_PI * (double) (j - 1) / longs;
			double xv = cos(lng);
			double yv = sin(lng);

			normal(xv * zr0, yv * zr0, z0);
			vertex(xv * zr0 * r, yv * zr0 * r, z0 * r);
			normal(xv * zr1, yv * zr1, z1);
			vertex(xv * zr1 * r, yv * zr1 * r, z1 * r);
		}
		glEnd();
	}
	glPopMatrix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void circle(const GLfloat& cx, const GLfloat& cy, const GLfloat& r,
            const int& num_segments) {

//	glBegin(GL_LINE_LOOP);
//	for (int ii = 0; ii < num_segments; ii++) {
//
//		GLfloat theta = 2.0f * 3.1415926f * GLfloat(ii) / GLfloat(num_segments); //get the current angle
//
//		GLfloat x = r * cosf(theta); //calculate the x component
//		GLfloat y = r * sinf(theta); //calculate the y component
//
//		glVertex2f(x + cx, y + cy); //output vertex
//	}
//	glEnd();

	GLfloat theta = 2 * 3.1415926 / GLfloat(num_segments);
	GLfloat c = cosf(theta); //precalculate the sine and cosine
	GLfloat s = sinf(theta);
	GLfloat t;

	GLfloat x = r; //we start at angle = 0
	GLfloat y = 0;

	glBegin(GL_LINE_LOOP);
	for (int ii = 0; ii < num_segments; ++ii) {
		glVertex2f(x + cx, y + cy); //output vertex

		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
	glEnd();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void tetrahedron(const GLfloat& x, const GLfloat& y, const GLfloat& z,
                 const GLfloat& size, const QColor& c) {

	/*
	 float half = size / 2.;
	 glBegin(GL_TRIANGLE_STRIP);
	 glColor3f(1, 1, 1);
	 glVertex3f(x + half, half, z - half);
	 glColor3f(1, 0, 0);
	 glVertex3f(-x + half, half, z - half);
	 glColor3f(0, 1, 0);
	 glVertex3f(x - half, half, z - half);
	 glColor3f(0, 0, 1);
	 glVertex3f(x - half, y - half, z / .2);
	 glColor3f(1, 1, 1);
	 glVertex3f(x - half, half, z - half);
	 glColor3f(1, 0, 0);
	 glVertex3f(-x + half, y - half, z - half);
	 glEnd();
	 */


	static const float tetrahedron[][3] = {
	                                        { 0.0f, 1.0f, 0.0f }, { 0.943f, -0.333f, 0.0f }, { -0.471f, -0.333f, 0.8165f
	                                        }, { -0.471f, -0.333f, -0.8165f } };
	static const float materialAmbientColor[][3] = {
	                                                 { 1.0f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	                                                 { 0.0f, 0.0f, 0.0f } };
	static const float materialSpecularColor[][3] = {
	                                                  { 1.0f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	                                                  { 0.0f, 0.0f, 0.0f } };

	static const int point[] = { 1, 2, 0, 3, 1 }; // triangle strip

	glPushMatrix();

//    glRotatef(tetrahedronAngle, 0.0f, 1.0f, 0.0f);

	for (int i = 0; i < 3 /* sides */; i++) {
		glBegin(GL_TRIANGLE_STRIP);

		glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientColor[i]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecularColor[i]);

		glVertex3fv(tetrahedron[point[i]]);
		glVertex3fv(tetrahedron[point[i + 1]]);
		glVertex3fv(tetrahedron[point[i + 2]]);

		glEnd();
	}

	glPopMatrix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double remap(const double& x, const double& oMin,
             const double& oMax, const double& nMin,
             const double& nMax) {

	// range check
	if ( oMin == oMax )
		return .0; //! Should exit properly

	if ( nMin == nMax )
		return .0; //! Should exit properly

	double oldMin, oldMax;
	// check reversed input range
	bool reverseInput = false;

	(oMin < oMax) ? oldMin = oMin : oldMin = oMax;
	(oMin < oMax) ? oldMax = oMax : oldMax = oMin;

	if ( oldMin != oMin )
		reverseInput = true;

	// check reversed output range
	bool reverseOutput = false;
	double newMin, newMax;
	(nMin < nMax) ? newMin = nMin : newMin = nMax;
	(nMin < nMax) ? newMax = nMax : newMax = nMin;

	if ( newMin != nMin )
		reverseOutput = true;

	double portion = (x - oldMin) * (newMax - newMin) / (oldMax - oldMin);
	if ( reverseInput )
		portion = (oldMax - x) * (newMax - newMin) / (oldMax - oldMin);

	double result = portion + newMin;
	if ( reverseOutput )
		result = newMax - portion;

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace GL
} // namespace OpenGL
} // namespace Gui
} // namespace IPGP
