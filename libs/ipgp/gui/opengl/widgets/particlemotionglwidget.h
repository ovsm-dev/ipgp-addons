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

#ifndef __IPGP_OPENGLGUI_PARTICLEMOTIONWIDGET_H___
#define __IPGP_OPENGLGUI_PARTICLEMOTIONWIDGET_H___


#include <ipgp/gui/opengl/canvas.h>
#include <QVector3D>
#include <QVector>
#include <QColor>

namespace IPGP {
namespace Gui {

class SC_IPGP_GUI_API Particle {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Particle(const qreal& value, const qreal& time,
		         const QColor& color = Qt::black) :
				_value(value), _time(time), _color(color) {}
		~Particle() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const qreal& value() const {
			return _value;
		}
		const qreal& time() const {
			return _time;
		}
		const QColor& color() const {
			return _color;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		qreal _value;
		qreal _time;
		QColor _color;
};


class SC_IPGP_GUI_API ParticleMotionGLWidget : public Canvas {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QPair<QVector3D, QColor> ParticleData;
		typedef QVector<ParticleData> DataSet;

		enum DrawingMethod {
			dm_UseAutoGradient,
			dm_UseCustomGradient,
			dm_None
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ParticleMotionGLWidget(QWidget* parent = 0);
		~ParticleMotionGLWidget();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Feeds data into graphic.
		 * @param x the X data vector.
		 * @param y the Y data vector.
		 * @param z the Z data vector.
		 * @param paintWithGradient specifies if a gradient should be used
		 *        when plotting the data, or just use the default monochromatic
		 *        red color.
		 */
		void feed(const QVector<qreal>&, const QVector<qreal>&,
		          const QVector<qreal>&, const bool& autoGradient);

		void feed(const QVector<Particle>&, const QVector<Particle>&,
		          const QVector<Particle>&);

		/**
		 * @brief Delete everything from the graphic.
		 * @param refresh specifies whether or not a replot should be performed.
		 */
		void clear(const bool& refresh = true);

		//! Redraws the scene without altering the current view settings
		void redraw();

//		const bool& paintUsingGradient() const {
//			return _paintUsingGradient;
//		}
//		void setPaintUsingGradient(const bool& value) {
//			_paintUsingGradient = value;
//		}

		void setDrawingMethod(const DrawingMethod& dm) {
			_dm = dm;
		}
		const DrawingMethod& drawingMethod() const {
			return _dm;
		}

	private:
		const qreal getMinParticleValue(const QVector<Particle>&) const;
		const qreal getMaxParticleValue(const QVector<Particle>&) const;

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void draw();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		DataSet _data;
		//		bool _paintUsingGradient;
		DrawingMethod _dm;
};

} // namespace Gui
} // namespace IPGP

#endif
