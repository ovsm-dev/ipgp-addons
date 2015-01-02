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

#ifndef __IPGP_GUI_DATAMODEL_ORIGINPLOT_H__
#define __IPGP_GUI_DATAMODEL_ORIGINPLOT_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/datamodel/squarrezoomplot.h>
#include <QObject>


namespace Seiscomp {
namespace DataModel {

class DatabaseQuery;
class Origin;

}
}


namespace IPGP {
namespace Gui {

/**
 * @class   OriginPlot
 * @package IPGP::Gui::DataModel
 * @brief   Origin residuals plots
 */
class SC_IPGP_GUI_API OriginPlot : public SquarreZoomPlot {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(Type)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Type {
			ResidualDistance = 0,
			ResidualAzimuth = 1,
			ResidualTakeOff = 2,
		};


	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit OriginPlot(QWidget* parent = NULL,
		                    const Type& type = ResidualDistance,
		                    Qt::WFlags = 0);
		~OriginPlot();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setType(const Type&);
		const Type& type() const {
			return _type;
		}
		void setOrigin(Seiscomp::DataModel::DatabaseQuery*,
		               Seiscomp::DataModel::Origin*);
		void clearAll();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void elementClicked(const QString&);
		void nullifyQObject(QObject*);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void plottableClicked(QCPAbstractPlottable*, QMouseEvent*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Type _type;

		struct OriginArrival {
				std::string publicID;
				std::string phaseCode;
				std::string networkCode;
				std::string stationCode;
				double azimuth;
				double takeOff;
				double distance;
				double residuals;
				bool hasAzimuth;
				bool hasTakeOff;
				bool hasDistance;
				bool hasResiduals;
				bool isAuto;
		};
		typedef QList<OriginArrival> ArrivalList;
};

} // namespace Gui
} // namespace IPGP
#endif
