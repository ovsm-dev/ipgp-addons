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

#ifndef __IPGP_GUI_MAP_DECORATOR_H__
#define __IPGP_GUI_MAP_DECORATOR_H__


#include <ipgp/gui/api.h>
#include <qglobal.h>
#include <qobjectdefs.h>

QT_FORWARD_DECLARE_CLASS(QMouseEvent);
QT_FORWARD_DECLARE_CLASS(QEvent);
QT_FORWARD_DECLARE_CLASS(QKeyEvent);
QT_FORWARD_DECLARE_CLASS(QPaintEvent);
QT_FORWARD_DECLARE_CLASS(QPainter);


namespace IPGP {
namespace Gui {
namespace Map {

/**
 * @class   Decorator
 * @package IPGP::Gui::Map
 * @brief   Virtual decorator macro class.
 */
class SC_IPGP_GUI_API Decorator {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Decorator();
		virtual ~Decorator();

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		//! The user is responsible for properly re-implementing these
		//! interfaces and should beware of child layering mechanism :
		//! decorators are added in cascade and therefore each decorator
		//! should transmit back the object instance after utilization.
		virtual void mouseMoveEvent(QMouseEvent*);
		virtual void mousePressEvent(QMouseEvent*);
		virtual void mouseReleaseEvent(QMouseEvent*);
		virtual void mouseDoubleClickEvent(QMouseEvent*);
		virtual void paintEvent(QPainter&);
		virtual void keyPressEvent(QKeyEvent*);
		virtual void keyReleaseEvent(QKeyEvent*);
		virtual void leaveEvent(QEvent*);

		void setDecorator(Decorator* decorator = NULL);
		void insertDecorator(Decorator*);
		void appendDecorator(Decorator*);

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Decorator* _child;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
