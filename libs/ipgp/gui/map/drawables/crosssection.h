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



#ifndef __IPGP_GUI_MAP_CROSSSECTION_H__
#define __IPGP_GUI_MAP_CROSSSECTION_H__

#include <ipgp/gui/defs.h>
#include <ipgp/gui/api.h>
#include <ipgp/gui/map/geometry.h>


namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class CrossSection
 * @brief A cross section is the 2D representation of a trench on the map.
 *        Points A and B stands for trench start and end, and are bordered
 *        within a significant width.
 */
class SC_IPGP_GUI_API CrossSection : public Geometry {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		CrossSection();
		~CrossSection() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		void setTrench(const qint32&);
		const qint32& trench() const;

		void setDepthMin(const qint32&);
		const qint32& depthMin() const;

		void setDepthMax(const qint32&);
		const qint32& depthMax() const;

		void setStartPoint(const QPoint&);
		const QPoint& startPoint() const;

		void setEndPoint(const QPoint&);
		const QPoint& endPoint() const;

		void setStartPointF(const QPointF&);
		const QPointF& startPointF() const;

		void setEndPointF(const QPointF&);
		const QPointF& endPointF() const;

		void setDistance(const qreal&);
		const qreal& distance() const;

		void setTextA(const QString&);
		const QString& textA() const;

		void setTextB(const QString&);
		const QString& textB() const;

		void setShadowEnabled(const bool&);
		const bool& shadowEnabled() const;


	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		qint32 _trenchkm;
		qint32 _depthMinkm;
		qint32 _depthMaxkm;
		QPointF _startPointF;
		QPointF _endPointF;
		QPoint _startPoint;
		QPoint _endPoint;
		qreal _distancekm;
		QPen _startEndPen;
		QPen _trenchPen;
		QString _textA;
		QString _textB;
		bool _shadow;
};


} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif

