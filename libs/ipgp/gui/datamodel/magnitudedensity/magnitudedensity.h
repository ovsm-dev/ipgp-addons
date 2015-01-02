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

#ifndef __IPGP_GUI_MAGNITUDEDENSITY_H__
#define __IPGP_GUI_MAGNITUDEDENSITY_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/plottingwidget.h>

class QCustomPlot;
class QCPAbstractPlottable;
class QCPPlotTitle;

namespace Ui {
class MagnitudeDensity;
}


namespace IPGP {
namespace Gui {

class SquarreZoomPlot;

class SC_IPGP_GUI_API MagnitudeDensity : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MagnitudeDensity(Seiscomp::DataModel::DatabaseQuery*,
		                          QWidget* parent = NULL,
		                          Qt::WFlags = 0);
		~MagnitudeDensity();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		SquarreZoomPlot* plot() const {
			return _plot;
		}

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void replot();
		void print(const ToolBox::ExportConfig&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void plottableClicked(QCPAbstractPlottable*, QMouseEvent*);
		void showFormulaBox();
		void showCustomSizeBox();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::MagnitudeDensity* _ui;
		SquarreZoomPlot* _plot;

		struct EventItem {
				Seiscomp::Core::Time time;
				double latitude;
				double longitude;
				double magnitude;
				double magnitudeSize;
				double depth;
				std::string publicID;
				bool isAuto;
				bool hasDepth;
				bool hasMagnitude;
		};
		typedef QList<EventItem> ItemList;
};

} // namespace Gui
} // namespace IPGP

#endif
