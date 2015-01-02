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




#ifndef __IPGP_GUI_DATAMODEL_WADATIDIAGRAM_H__
#define __IPGP_GUI_DATAMODEL_WADATIDIAGRAM_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/datamodel/plottingwidget.h>



class QCPAbstractPlottable;

namespace Ui {
class WadatiDiagram;
}


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(SquarreZoomPlot);

class SC_IPGP_GUI_API WadatiDiagram : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct EventStation {
				Seiscomp::Core::Time time;
				double ttp;
				double tts;
				double magnitude;
				double magnitudeSize;
				double depth;
				std::string networkCode;
				std::string stationCode;
				std::string originID;
				bool isAuto;
				bool hasDepth;
				bool hasMag;
		};
		typedef QList<EventStation> StationList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit WadatiDiagram(Seiscomp::DataModel::DatabaseQuery*,
		                       QWidget* parent = NULL, Qt::WFlags = 0);
		~WadatiDiagram();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setEvents(Core::OriginList*, const bool& copy = false);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		const bool checkStation(const QString&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public virtual Qt interface
		// ------------------------------------------------------------------
		void replot();
		void print(const ToolBox::ExportConfig&);
		void updateStationList();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private virtual Qt interface
		// ------------------------------------------------------------------
		void rowDoubleClicked(const QModelIndex&);
		void plottableClicked(QCPAbstractPlottable*, QMouseEvent*);
		void showHideLegend();

		void showFormulaBox();
		void showCustomSizeBox();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::WadatiDiagram* _ui;
		SquarreZoomPlot* _plot;
		QList<Seiscomp::DataModel::PickPtr> _pickList;
		QMap<QString, QColor> _colorMap;
		QString _selectedStation;
};

} // namespace Gui
} // namespace IPGP
#endif
