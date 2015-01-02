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

#ifndef __IPGP_GUI_STREAMWIDGET_H__
#define __IPGP_GUI_STREAMWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/datamodel/recordviewer.h>
#include <ipgp/gui/datamodel/recordmanager.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <ipgp/core/datamodel/flags.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/math/filter.h>


#include <QFrame>
#include <QTimer>
#include <QStringList>
#include <QMap>


namespace Ui {
class StreamWidget;
}

namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
class Inventory;
}
}

QT_FORWARD_DECLARE_CLASS(QListWidgetItem);
QT_FORWARD_DECLARE_CLASS(QThread);
QT_FORWARD_DECLARE_CLASS(QComboBox);

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(StreamDelegate);
DEFINE_IPGP_SMARTPOINTER(StreamWidget);
DEFINE_IPGP_SMARTPOINTER(SpectrogramPlot);

/**
 * @class	SpetrogramPlot
 * @package	IPGP::Gui::Widgets
 * @brief	Spectrogram color map
 */
class SC_IPGP_GUI_API SpectrogramPlot : public QCustomPlot {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QMap<double, QCPColorMap*> CCMap;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit SpectrogramPlot(QWidget* parent = NULL, Qt::WFlags f = 0);
		~SpectrogramPlot();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setData(const QString&, Seiscomp::RecordSequence*);
		void setTimeWindow(const double& start, const double& end);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void processEntity(Entity);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		CCMap _cmap;
		RecordManager* _rm;
		QString _streamID;
		QCPColorGradient _gradient;
};


/**
 * @class   StreamWidget
 * @package IPGP::Gui::Widgets
 * @brief   Inventory stream widget
 *
 * This class offers a real time plotting widget for stations streams.
 * On initialization, the inventory will be fetched by using information
 * contained in the query instance... which means that if there's no valid
 * query instance, no stations will be added to the list and consequently,
 * no streams will be available for display.
 * The default time window of sample is by default 10minutes with a stream
 * scrolling period of 2 seconds.
 */
class SC_IPGP_GUI_API StreamWidget : public QFrame {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_PROPERTY(Settings _settings READ settings WRITE setSettings)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QMap<QString, QString> StringMap;
		typedef QMap<QString, Seiscomp::RecordSequence*> SequenceMap;

		struct Settings {

				enum VisibleSensors {
					Z,
					NorthSouth,
					EastWest,
					All = Z | NorthSouth | EastWest
				};

				Settings() :
						buffer(60.), refreshRate(2000),
						visibleSensors(Core::FlagSet<VisibleSensors>(All)) {}
				~Settings() {}

				double buffer;   //! In seconds
				int refreshRate; //! In milliseconds
				Core::FlagSet<VisibleSensors> visibleSensors;
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit StreamWidget(Seiscomp::DataModel::DatabaseQuery*,
		                      QWidget* parent = NULL,
		                      Qt::WFlags = 0);
		~StreamWidget();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setupInventory();

		void setStreamUrl(const std::string& url) {
			_recordStreamUrl = url;
		}
		const std::string& streamUrl() const {
			return _recordStreamUrl;
		}

		void initFilters(const QMap<QString, QString>&);

		void setSettings(const Settings& set) {
			_settings = set;
		}
		const Settings& settings() const {
			return _settings;
		}

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		bool openStream();
		void closeStream();
		void startReading();
		void stopReading();

		void resetSequences();

		bool addStation(const std::string& networkCode,
		                const std::string& stationCode);
		bool addStream(const std::string& networkCode,
		               const std::string& stationCode,
		               const std::string& locationCode,
		               const std::string& channelCode);

		void setStartTime(const Seiscomp::Core::Time&);
		void setEndTime(const Seiscomp::Core::Time&);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void stationSelected(QListWidgetItem*);
		void storeNewRecord(Seiscomp::Record*);
		void filterChanged(const QString&);
		void updateStreams();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void stopDelegate();
		void nullifyQObject(QObject*);

		//! Widget's states
		void working();
		void idling();
		void loadingPercentage(const int&, const QString&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::DataModel::DatabaseQuery* _query;
		Seiscomp::DataModel::Inventory* _inventory;
		Ui::StreamWidget* _ui;
		QThread* _delegateThread;
		StreamDelegatePtr _delegate;
		Seiscomp::Math::Filtering::InPlaceFilter<double>* _filter;
		QComboBox* _filterBox;
		RecordViewer* _viewer;
		SpectrogramPlot* _spectro;
		QTimer _timer;
		QString _selectedStation;
		StringMap _sensors;
		std::string _recordStreamUrl;
		Seiscomp::Record::Hint _recordInputHint;
		Seiscomp::IO::RecordStreamPtr _recordStream;
		StringMap _filters;
		Settings _settings;
		SequenceMap _sequences;
};

} // namespace Gui
} // namespace IPGP

#endif
