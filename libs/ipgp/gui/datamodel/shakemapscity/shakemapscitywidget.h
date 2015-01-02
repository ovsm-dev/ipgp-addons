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

#ifndef __IPGP_GUI_DATAMODEL_SHAKEMAPSCITYWIDGET_H__
#define __IPGP_GUI_DATAMODEL_SHAKEMAPSCITYWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QWidget>
#include <QObject>
#include <QStringList>
#include <QModelIndex>
#include <QTimer>
#include <ipgp/core/misc/misc.h>
#include <ipgp/core/misc/types.h>
#include <seiscomp3/processing/shakemapprocessor.h>
#include <seiscomp3/processing/pga2mskprocessor.h>


class QCustomPlot;
class QCPAbstractPlottable;
class CustomItem;

QT_FORWARD_DECLARE_CLASS(QAction);
QT_FORWARD_DECLARE_CLASS(QCheckBox);
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);


namespace Ui {
class ShakemapsCityWidget;
}

namespace Seiscomp {
class Regions;

namespace DataModel {
class Origin;
class Magnitude;
}
}



namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(OriginWidget);

/**
 * @class   ShakemapsCityWidget
 * @package IPGP::Gui::Widgets
 * @brief   Event city shakemaps estimation widget
 *
 * This class provides an interactive widget in which estimations upon an
 * event impact on regions, cities can be done.
 */
class SC_IPGP_GUI_API ShakemapsCityWidget : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(ShakemapsType)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum ShakemapsType {
			ST_MSKONLY = 0x1,
			ST_MSKPGA = 0x2
		};

		typedef QList<IPGP::Core::pgaPlot> PGAPlotList;
		typedef QList<Seiscomp::Processing::ShakemapProcessorPtr> ShakeProcList;
		typedef QList<Seiscomp::Processing::PGA2MSKProcessorPtr> PGAProcList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ShakemapsCityWidget(Seiscomp::DataModel::Origin*,
		                             Seiscomp::DataModel::Magnitude*,
		                             QWidget* parent = NULL,
		                             Qt::WFlags = 0);
		~ShakemapsCityWidget();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void init(const std::string& cityFile, const int& islandID);
		void setLayoutMargin(int left, int top, int right, int bottom);
		void setShakemapsProcessors(const ShakeProcList&);
		void setPGA2MSKProcessors(const PGAProcList&);

		const qreal& nearestCityDistDEG() const {
			return _nearestCityDistDEG;
		}
		const qreal& nearestCityDistKM() const {
			return _nearestCityDistKM;
		}
		const QString& nearestCityName() const {
			return _nearestCityName;
		}
		OriginWidget* mapWidget() const {
			return _map;
		}
		const QList<IPGP::Core::ShakemapsCity>& cityList() const {
			return _cityList;
		}

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void updateView();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void plottableClicked(QCPAbstractPlottable*, QMouseEvent*);
		void moveOnCity(const QModelIndex&);
		void updateViewModel(const ShakemapsType&);
		void indicateCityInList(const QString&);
		void erraseCityIndicators();
		void tableViewHeaderMenu(const QPoint&);
		void showHideTableViewHeaderItems();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::ShakemapsCityWidget* _ui;
		QStandardItemModel* _standardItemModel;
		QCustomPlot* _plot;
		QCheckBox** _townRadioButtons;

		Seiscomp::DataModel::OriginPtr _origin;
		Seiscomp::DataModel::MagnitudePtr _magnitude;

		QStringList _islandList;
		QList<IPGP::Core::ShakemapsCity> _cityList;
		ShakeProcList _shakeProcessors;
		PGAProcList _p2mProcessors;
		PGAPlotList _pgaPlotList;
		PGAPlotList _shakemapsPlotList;

		QString _nearestCityName;
		qreal _nearestCityDistKM;
		qreal _nearestCityDistDEG;

		OriginWidget* _map;
		QTimer _timer;

		ShakemapsType _shakeMode;

		QAction* _pgaMinAction;
		QAction* _pgaMaxAction;
		QAction* _mskMinAction;
		QAction* _mskMaxAction;
		QAction* _epidistAction;
		QAction* _azimuthAction;
};

} // namespace Gui
} // namespace IPGP

#endif
