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



#ifndef __IPGP_GUI_MAP_EPICENTER_H__
#define __IPGP_GUI_MAP_EPICENTER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>
#include <QSizeF>

#define EpicenterDefaultBrush Qt::transparent
#define EpicenterDefaultPenWidth 3.7
#define EpicenterDefaultSize QSizeF(16., 16.)

class QPointF;
class QPen;
class QBrush;

namespace IPGP {
namespace Gui {
namespace Map {

class SC_IPGP_GUI_API Epicenter : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Epicenter(const QPointF& posUncertainty = QPointF(.0, .0),
		                   const qreal& depth = .0, const qreal& mag = -1.,
		                   const qreal& uncertaintyOpacity = .5,
		                   const QPen& uncertaintyPen = QPen(),
		                   const QBrush& uncertaintyBrush = QBrush()) :
				_positionUncertainty(posUncertainty), _depth(depth), _mag(mag),
				_uncertaintyOpacity(uncertaintyOpacity), _uncertaintyPen(uncertaintyPen),
				_uncertaintyBrush(uncertaintyBrush) {
			setType(Geometry::d_Epicenter);
		}

		virtual ~Epicenter() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		void setPositionUncertainty(const QPointF&);
		const QPointF& positionUncertainty() const;

		void setMagnitude(const qreal&);
		const qreal& magnitude() const;

		void setDepth(const qreal&);
		const qreal& depth() const;

		QBrush& uncertaintyBrush();
		void setUncertaintyBrush(const QBrush&);

		QPen& uncertaintyPen();
		void setUncertaintyPen(const QPen&);

		const qreal& uncertaintyOpacity() const;
		void setUncertaintyOpacity(const qreal&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QPointF _positionUncertainty;
		qreal _depth;
		qreal _mag;
		qreal _uncertaintyOpacity;
		QPen _uncertaintyPen;
		QBrush _uncertaintyBrush;
};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif

