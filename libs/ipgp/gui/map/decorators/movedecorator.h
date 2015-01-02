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



#ifndef __IPGP_GUI_MAP_MOVEDECORATOR_H__
#define __IPGP_GUI_MAP_MOVEDECORATOR_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/map/decorators/mapdecorator.h>
#include <QObject>
#include <QPointF>



namespace IPGP {
namespace Gui {

class MapWidget;

namespace Map {
/**
 * @class MovdeDecorator!
 * @brief Provides grab/move/click actions to the map.
 */
class SC_IPGP_GUI_API MoveDecorator : public QObject, public MapDecorator {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MoveDecorator(MapWidget*);

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const bool& enabled() const;

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setEnabled(const bool&);
		void setKineticEnabled(const bool&);
		//		void changeLatitudePosition(const qreal&);
		void changeLongitudePosition(const qreal&);
		void dragMap(const qreal&, const qreal&);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void changePositionTo(const QPointF&);

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		int _oldX, _oldY;
		bool _dragging;
		bool _enabled;
		bool _kineticEnabled;
		double _refX, _refY;
		double _offsetX, _offsetY;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
