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

#ifndef __IPGP_GUI_DATAMODEL_PARTICLEMOTIONWIDGET_H__
#define __IPGP_GUI_DATAMODEL_PARTICLEMOTIONWIDGET_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/datamodel/misc.h>
#include <ipgp/core/datamodel/recordcache.h>
#include <ipgp/core/datamodel/flags.h>
#include <ipgp/gui/client/misc.h>
#include <ipgp/gui/client/scheme.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/utils/timer.h>
#include <QMainWindow>

class QCustomPlot;
class QCPColorMap;
class QCPGraph;
class QProgressIndicator;


namespace Ui {
class ParticleMotionWidget;
class ParticleMotionSettings;
}

namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
}
}

QT_FORWARD_DECLARE_CLASS(QListWidgetItem);
QT_FORWARD_DECLARE_CLASS(QTableWidgetItem);
QT_FORWARD_DECLARE_CLASS(QComboBox);
QT_FORWARD_DECLARE_CLASS(QSpinBox);


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(StreamDelegate);
DEFINE_IPGP_SMARTPOINTER(ParticleMotionWidget);
DEFINE_IPGP_SMARTPOINTER(ParticleMotionGLWidget);
DEFINE_IPGP_SMARTPOINTER(LogDialog);
DEFINE_IPGP_SMARTPOINTER(OriginRecordViewer);
DEFINE_IPGP_SMARTPOINTER(FrequencyViewerPlot);
DEFINE_IPGP_SMARTPOINTER(ColorMapViewerPlot);


class SC_IPGP_GUI_API KeyboardFilter : public QObject {
	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		KeyboardFilter(ParticleMotionWidget* pm, QObject* parent = NULL);

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool eventFilter(QObject* dist, QEvent* event);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		ParticleMotionWidget* _pm;
};




/**
 * @class   ParticleMotionWidget
 * @package IPGP::Gui::Widgets
 * @brief   Particle motion view widget
 *
 * This class provides a widget in which data from an event's stations can
 * be analyzed and graphics of components movement produced.
 */
class SC_IPGP_GUI_API ParticleMotionWidget : public QMainWindow {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_PROPERTY(Scheme _scheme READ scheme WRITE setScheme)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct Settings {

				enum DistanceUnit {
					DU_KM = 0,
					DU_DEGREES = 1,
				};

				Settings();
				~Settings() {}

				size_t messagesDurationMS;
				size_t acquisitionTimeout; //! Default timeout in sec
				DistanceUnit du;
				bool loadAfterInit;
				bool paintWaveRegionBox;
				bool paintGapsInStreams;
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ParticleMotionWidget(Seiscomp::DataModel::DatabaseQuery*,
		                              QWidget* parent = NULL,
		                              Qt::WFlags f = 0);
		~ParticleMotionWidget();

		friend class KeyboardFilter;

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void closeEvent(QCloseEvent*);
		virtual void keyPressEvent(QKeyEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		QCustomPlot* pznsPlot() const {
			return _pznsPlot;
		}
		QCustomPlot* pewnsPlot() const {
			return _pewnsPlot;
		}

		Seiscomp::IO::RecordStream* recordStream() const {
			return _recordStream.get();
		}

		void setStreamUrl(const std::string& url) {
			_recordStreamUrl = url;
		}
		const std::string& streamUrl() const {
			return _recordStreamUrl;
		}

		void setScheme(const Scheme& scheme) {
			_scheme = scheme;
		}
		const Scheme& scheme() const {
			return _scheme;
		}

		/**
		 * @brief Sets the time period around origin time to fetch data
		 * @param sec The default period in seconds (@default is 360.)
		 */
		void setTimeWindowSecs(const qreal& sec = 360.) {
			_timeWindowSec = sec;
		}
		const qreal& timeWindowSecs() const {
			return _timeWindowSec;
		}

		//! Sets the origin to be analyzed in the widget
		void setOrigin(const std::string& publicID);

		//! Initiate the streams available filters. The one defined with a "@"
		//! in its key will be set as default filter
		void initFilters(const QMap<QString, QString>&);

		bool openStream();
		void closeStream();

		void stopFetching();

		bool addStation(const std::string& networkCode,
		                const std::string& stationCode);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		bool addStream(const std::string& networkCode,
		               const std::string& stationCode,
		               const std::string& locationCode,
		               const std::string& channelCode);

		void setStartTime(const Seiscomp::Core::Time&);
		void setEndTime(const Seiscomp::Core::Time&);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow&);

		void handleDelegateState();

		void log(Client::LogMessage, const QString&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void saveCollectedData();

		void stationSelected(QTableWidgetItem*);
		void componentChannelChanged(int);
		void filterChanged(const QString&);
		void useSMinusPTriggered(bool);

		//! Handles data acquisition timeout.
		void checkAcquisitionTimeout();

		/**
		 * @brief Refreshes the data and re-plots graphics
		 * @param bool indicates if a check on the station has to be performed
		 *        in order to make the difference between a new click (station
		 *        click) and a refresh request from user
		 */
		void prepareData(const bool& stationCheck = true);

		/**
		 * @brief Plots the graphics with data ranging from the selected
		 *        time region of the station's stream(s).
		 */
		void plotData();

		/**
		 * @brief Checks the data stream, see if any gap(s) is/are present
		 * @param code The station name (noted as "networkCode.stationCode")
		 * @param isFirstCheck Indicate if this is the first time the
		 *        station's stream get checked.
		 * @note  When checking the station's streams, if it is the first
		 *        check ever, the gap count will be ignored (counting gaps
		 *        while the delegate is still running produces wrong results)
		 *        and the stream will be tagged "Pre-checked", otherwise, the
		 *        gaps number will be displayed if higher than zero.
		 */
		void checkStreamsGaps(const QString&, const bool& isFirstCheck = false);

		/**
		 * @brief Adds an "Missing Data" tag to specified station
		 * @param code The station to be tagged (noted as net.sta)
		 */
		void handleMissingData(const QString&);

		//! Performs final checks upon data retrieved from delegate.
		void delegateIsOver();

		void acquisitionExceptionReported(const QString&);
		void operationExceptionReported(const QString&);

		void stationStreamsReady(const QString&);

		//! Waiting widget actions
		void showWaitingWidget();
		void hideWaitingWidget();

		//! Picking utilities
		void addStartPick();
		void addEndPick();

		/**
		 * @brief Slot to be called whenever a new pick has been added by the
		 *        picker so that an instance of it could be registered and
		 *        elements of the interface properly handled.
		 * @param time The pick time in GMT
		 * @param name The pick name
		 */
		void addNewPick(const qreal&, const QString&);

		void settingsValidated();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void stopDelegate();
		void refreshStreams();
		void requestGapsCheck(const QString&);

		//! Status signals
		void working();
		void idling();

		//! Signal emitted whenever this object instance is destroyed
		void nullifyQObject(QObject*);

		void stopPickingMode(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::ParticleMotionWidget* _ui;
		QComboBox* _filterBox;
		OriginRecordViewer* _orv;
		QCustomPlot* _pznsPlot;
		QCustomPlot* _pewnsPlot;

		FrequencyViewerPlot* _freqZ;
		FrequencyViewerPlot* _freqNS;
		FrequencyViewerPlot* _freqEW;
		ColorMapViewerPlot* _zSpectro;

		QProgressIndicator* _pi;
		ParticleMotionGLWidget* _pmgl;
		Core::RecordCache _cache;
		Core::FlagSet<Client::State> _state;
		Core::FlagSet<Client::State> _delegateState;
		QMap<QString, QString> _filters;

		LogDialog* _logDialog;

		Seiscomp::DataModel::DatabaseQuery* _query;
		Seiscomp::Math::Filtering::InPlaceFilter<double>* _filter;
		Seiscomp::DataModel::OriginPtr _origin;
		Seiscomp::DataModel::PickPtr _pPick;
		Seiscomp::DataModel::PickPtr _sPick;

		ParticleMotionWidget::Settings _settings;
		Scheme _scheme;

		StationStreamStatusList _fetchedStationsStatus;

		StreamDelegate* _delegate;

		ComponentData _zData;
		ComponentData _nsData;
		ComponentData _ewData;

		std::string _recordStreamUrl;
		Seiscomp::Record::Hint _recordInputHint;
		Seiscomp::IO::RecordStreamPtr _recordStream;

		ActiveStreamList _activeStreams;
		QString _activeGainUnit;

		QColor _waveTraceColor;

		Seiscomp::Core::TimeWindow _tw;
		qreal _timeWindowSec;
		QString _currentPickPicking;
		QString _currentStationPicking;

		StreamsGapsList _streamsGaps;

		Seiscomp::Util::StopWatch* _stopWatch;
		QTimer* _acquisitionTimer;

		struct WaveRegion {
				WaveRegion();
				void reset();
				bool hasStartTimeSet;
				bool hasEndTimeSet;
				qreal startTime;
				qreal endTime;
		};
		WaveRegion _wr;

		struct RecordItem {
				RecordItem();
				RecordItem(const qreal& t, const qreal& z, const qreal& ns,
				           const qreal& ew);
				qreal time;
				qreal zValue;
				qreal nsValue;
				qreal ewValue;
		};
		QVector<RecordItem> _recordItems;

		struct FFTProperty {
				FFTProperty() {
					fsamp = .0;
					sampleCount = 0;
				}
				double fsamp;
				QString streamID;
				int sampleCount;
		};
		FFTProperty _zChannel;
		FFTProperty _nsChannel;
		FFTProperty _ewChannel;

		QAction* _autoReplot;
		QAction* _startWavePick;
		QAction* _endWavePick;
		QAction* _autoRescaleKeyAxes;
		QAction* _autoRescaleValueAxes;
		QSpinBox* _unpickedStationsDistance;
		QAction* _addUnpickedStations;
		QAction* _hideUnpickedStations;
		QAction* _applyButton;

		Ui::ParticleMotionSettings* _configDialogUi;
		QDialog* _configDialog;

		QMap<QString, OriginRecordViewer*> _stationMap;
};

} // namespace Gui
} // namespace IPGP

#endif
