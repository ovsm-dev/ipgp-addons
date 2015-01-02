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

#ifndef __IPGP_GUI_MAGNITUDEVARIANCEWIDGET_H__
#define __IPGP_GUI_MAGNITUDEVARIANCEWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/plottingwidget.h>


namespace Ui {
class MagnitudeVarianceWidget;
}


namespace IPGP {
namespace Gui {

class SquarreZoomPlot;

/**
 * @class   MagnitudeVarianceWidget
 * @package IPGP::Gui::Widgets
 * @brief   A magnitude variation plotter
 */
class SC_IPGP_GUI_API MagnitudeVarianceWidget : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MagnitudeVarianceWidget(Seiscomp::DataModel::DatabaseQuery*,
		                                 QWidget* parent = NULL,
		                                 Qt::WFlags = 0);
		~MagnitudeVarianceWidget();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		SquarreZoomPlot* plot() const {
			return _plot;
		}
		void setMagnitudeTypes(const Core::MagnitudeTypes&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void plotPiledBarsGraph();
		void plotPiledCurvesGraph();
		void plotCurvesGraph();

		const bool checkMagnitude(const std::string&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void replot();
		void print(const ToolBox::ExportConfig& ec);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void showLegend(bool);
		void showHideNormalizeBox(int);

		void addMagnitude();
		void removeMagnitude();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::MagnitudeVarianceWidget* _ui;
		SquarreZoomPlot* _plot;
		bool _showLegend;
};

} // namespace Gui
} // namespace IPGP

#endif
