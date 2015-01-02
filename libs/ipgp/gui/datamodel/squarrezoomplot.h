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

#ifndef __IPGP_GUI_SQUARREZOOMPLOT_H__
#define __IPGP_GUI_SQUARREZOOMPLOT_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QPoint>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>

QT_FORWARD_DECLARE_CLASS( QRubberBand);
QT_FORWARD_DECLARE_CLASS(QMouseEvent);
QT_FORWARD_DECLARE_CLASS(QWidget);

namespace IPGP {
namespace Gui {


/**
 * @class   SquarreZoomPlot
 * @package IPGP::Gui::DataModel
 * @brief   Special use case of QCustomPlot!
 *
 * This class provides a new implementation of QCustomPlot widget in which the
 * user can zoom in on the plot by rectangular region selection with the left
 * mouse button, and then restore the original zoom level with the mouse mid
 * (middle) button.
 */
class SC_IPGP_GUI_API SquarreZoomPlot : public QCustomPlot {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_PROPERTY(bool _zoomMode READ zoomModeEnabled WRITE setZoomMode)

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit SquarreZoomPlot(QWidget* parent = NULL, Qt::WFlags = 0);
		virtual ~SquarreZoomPlot();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setZoomMode(bool);
		const bool& zoomModeEnabled() const;

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);

		void restoreOriginalRange();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QRubberBand* _rubberBand;
		QPoint _origin;
		QCPRange _xRange;
		QCPRange _yRange;
		bool _zoomMode;
};


} // namespace Gui
} // namespace IPGP

#endif
