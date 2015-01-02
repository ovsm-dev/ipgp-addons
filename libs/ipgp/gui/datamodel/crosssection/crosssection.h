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

#ifndef __IPGP_GUI_CrossSection_H__
#define __IPGP_GUI_CrossSection_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/plottingwidget.h>
#include <QStringList>

class QCPAbstractPlottable;

namespace Ui {
class CrossSection;
}


namespace IPGP {
namespace Gui {

class SquarreZoomPlot;

/**
 * @class   CrossSection
 * @package IPGP::Gui::Widgets
 * @brief   A cross-section plotter
 *
 * This class implements
 */
class SC_IPGP_GUI_API CrossSection : public PlottingWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct Profile {
				Profile();
				QString name;
				qreal depthMin;
				qreal depthMax;
				qreal widthKM;
				qreal latitudeA;
				qreal longitudeA;
				qreal latitudeB;
				qreal longitudeB;
		};
		typedef QMap<int, Profile> ProfileMap;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit CrossSection(Seiscomp::DataModel::DatabaseQuery*,
		                      QWidget* parent = NULL,
		                      Qt::WFlags = 0);
		~CrossSection();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool loadSettings();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void appendProfile(const Profile&);
		void removeProfile(const int&);
		void removeProfile(const QString&);

		SquarreZoomPlot* verticalProjectionPlot() const {
			return _verticalProjection;
		}
		SquarreZoomPlot* horizontallProjectionPlot() const {
			return _horizontalProjection;
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
		void handleCrossSectionState();
		void plottableClicked(QCPAbstractPlottable*, QMouseEvent*);
		void updateCurrentProfile(const int&);
		void reverseRangeClicked(const bool&);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void disableCrossSectionPainting();
		void enableCrossSection(const QPointF& start, const QPointF& end,
		                        const qreal& depthMin, const qreal& depthMax,
		                        const qreal& trench);

		//! Emitted after plotting, simple list of event inside cross section
		void objectsInSection(const QStringList&);

		void plottingStarted();
		void plottingFinished();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::CrossSection* _ui;
		SquarreZoomPlot* _verticalProjection;
		SquarreZoomPlot* _horizontalProjection;

		ProfileMap _profiles;

		struct EventMagnitude {
				std::string publicID;
				Seiscomp::Core::Time time;
				double latitude;
				double longitude;
				double magnitude;
				double magnitudeSize;
				double depth;
				bool isAuto;
				//				bool hasPosition;
				bool hasDepth;
				bool hasMagnitude;
		};
		typedef QList<EventMagnitude> MagnitudeList;
};

} // namespace Gui
} // namespace IPGP

#endif
