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

#ifndef __IPGP_GUI_MAP_CANVAS_H__
#define __IPGP_GUI_MAP_CANVAS_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/layer.h>

#include <seiscomp3/math/coord.h>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QMouseEvent);
QT_FORWARD_DECLARE_CLASS(QPainter);
QT_FORWARD_DECLARE_CLASS(QPointF);
QT_FORWARD_DECLARE_CLASS(QRect);
QT_FORWARD_DECLARE_CLASS(QEvent);

namespace IPGP {
namespace Gui {
namespace Map {

DEFINE_IPGP_SMARTPOINTER(Canvas);

/**
 * @class   Canvas
 * @package IPGP::Gui::Map
 * @brief   Canvas macro class.
 *
 * A canvas basically serves as an interface between the map widget and the
 * objects, items the user wants to paint. Since elements are painted within
 * layers, canvas provides methods to add, remove, raise, lower, re-grade
 * layers. This class provides elementary methods and signals.
 */
class SC_IPGP_GUI_API Canvas : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Canvas();
		~Canvas();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		virtual void draw(QPainter&, const QPointF&, const QRect&, const int&)=0;
		virtual void mouseMoveEvent(QMouseEvent*);

		void addLayer(Layer*);
		void removeLayer(Layer*);
		void raiseLayer(Layer*);
		void lowerLayer(Layer*);

		void regradeLayers();
		void deleteLayers();

		const QList<Layer*>& layers() const;

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void mouseAtPosition(QEvent*);
		void clickOnCanvas(QMouseEvent*);
		void elementClicked(const QString&);
		void updateRequested();
		void repaintRequested();

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QList<Layer*> _layers;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
