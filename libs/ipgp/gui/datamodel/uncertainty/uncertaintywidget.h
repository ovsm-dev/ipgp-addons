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

#ifndef __IPGP_GUI_UNCERTAINTYWIDGET_H__
#define __IPGP_GUI_UNCERTAINTYWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/plottingwidget.h>

class QCustomPlot;

namespace Ui {
class UncertaintyWidget;
}


namespace IPGP {
namespace Gui {

class SC_IPGP_GUI_API UncertaintyWidget : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum PlotType {
			ptMagnitude,
			ptRMS,
			ptLatitude,
			ptLongitude,
			ptDepth,
			ptPhase
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit UncertaintyWidget(Seiscomp::DataModel::DatabaseQuery*,
		                           QWidget* parent = NULL, Qt::WFlags = 0);
		~UncertaintyWidget();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		QCustomPlot* latPlot() const {
			return _latPlot;
		}
		QCustomPlot* lonPlot() const {
			return _lonPlot;
		}
		QCustomPlot* rmsPlot() const {
			return _rmsPlot;
		}
		QCustomPlot* depthPlot() const {
			return _depthPlot;
		}
		QCustomPlot* magPlot() const {
			return _magPlot;
		}
		QCustomPlot* phasePlot() const {
			return _phasePlot;
		}

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void errPlotPaint(QCustomPlot*, const int& err10, const int& err20,
		                  const int& err30, const int& err40, const int& err50,
		                  const int& err60, const int& errOther,
		                  enum PlotType type = ptLatitude);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void replot();
		void print(const ToolBox::ExportConfig&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::UncertaintyWidget* _ui;
		QCustomPlot* _latPlot;
		QCustomPlot* _lonPlot;
		QCustomPlot* _depthPlot;
		QCustomPlot* _rmsPlot;
		QCustomPlot* _magPlot;
		QCustomPlot* _phasePlot;
};

} // namespace Gui
} // namespace IPGP

#endif
