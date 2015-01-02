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

#ifndef __IPGP_GUI_EVENTTYPEVARIANCEWIDGET_H__
#define __IPGP_GUI_EVENTTYPEVARIANCEWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/plottingwidget.h>
#include <QMap>
#include <QPair>
#include <QList>

namespace Ui {
class EventTypeVarianceWidget;
}


namespace IPGP {
namespace Gui {

class SquarreZoomPlot;

class SC_IPGP_GUI_API EventTypeVarianceWidget : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit EventTypeVarianceWidget(Seiscomp::DataModel::DatabaseQuery*,
		                                 QWidget* parent = NULL,
		                                 Qt::WFlags = 0);
		~EventTypeVarianceWidget();

	private:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QMap<Seiscomp::Core::Time, qreal> TimedOccurrence;
		typedef QMap<QString, TimedOccurrence> OccurrenceMap;
		typedef QList<QColor> DefaultColors;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		SquarreZoomPlot* plot() const {
			return _plot;
		}

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void plotPiledBarsGraph();
		void plotPiledCurvesGraph();
		void plotCurvesGraph();

		const bool checkType(const QString&);
		void prepareData(TimedOccurrence&);

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

		void addEventType();
		void removeEventType();

		void showHideNormalizeBox(int);
		void updateSeismicTypes();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::EventTypeVarianceWidget* _ui;
		SquarreZoomPlot* _plot;
		bool _showLegend;
		QStringList _types;

		OccurrenceMap _data;
		DefaultColors _brushes;
		DefaultColors _pens;
};

} // namespace Gui
} // namespace IPGP

#endif
