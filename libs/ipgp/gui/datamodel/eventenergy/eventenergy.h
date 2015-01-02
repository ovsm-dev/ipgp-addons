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




#ifndef __IPGP_GUI_DATAMODEL_EVENTENERGY_H__
#define __IPGP_GUI_DATAMODEL_EVENTENERGY_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/datamodel/plottingwidget.h>



class QCPAbstractPlottable;

namespace Ui {
class EventEnergy;
}


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(SquarreZoomPlot);

class SC_IPGP_GUI_API EventEnergy : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit EventEnergy(Seiscomp::DataModel::DatabaseQuery*,
		                     QWidget* parent = NULL, Qt::WFlags = 0);
		~EventEnergy();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setMagnitudeTypes(const Core::MagnitudeTypes&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const bool checkMagnitude(const std::string&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public virtual Qt interface
		// ------------------------------------------------------------------
		void replot();
		void print(const ToolBox::ExportConfig&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private virtual Qt interface
		// ------------------------------------------------------------------
		void addMagnitude();
		void removeMagnitude();

		void plottableClicked(QCPAbstractPlottable*, QMouseEvent*);
		void showHideLegend();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::EventEnergy* _ui;
		SquarreZoomPlot* _plot;

		struct EventMagnitude {
				Seiscomp::Core::Time time;
				double magnitude;
				double magnitudeEnergy;
				double magnitudeSize;
				double depth;
				std::string publicID;
				bool isAuto;
				bool hasDepth;
		};
		typedef QList<EventMagnitude> MagnitudeList;
};

} // namespace Gui
} // namespace IPGP
#endif
