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

#ifndef __IPGP_GUI_WIDGETS_ORIGINRECORDVIEWER_H__
#define __IPGP_GUI_WIDGETS_ORIGINRECORDVIEWER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/datamodel/recordviewer.h>
#include <ipgp/gui/datamodel/misc.h>
#include <ipgp/core/datamodel/recordcache.h>
#include <ipgp/gui/datamodel/originrecordviewer/originrecordpicker.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/origin.h>
#include <QWidget>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <seiscomp3/math/filter.h>


namespace Seiscomp {
namespace DataModel {
class DatabaseQuery;
}
}

namespace IPGP {
namespace Gui {


/**
 * @class   OriginRecordViewer
 * @package IPGP::Gui::Widgets
 * @brief   Interactive origin stream widget
 *
 * This class provides a widget in which origin's stations streams are plotted.
 * Data get fetched by internal delegate if not found in provided cache, it is
 * possible to display a Fast-Fourier-Transform of the origin's streams.
 */
class SC_IPGP_GUI_API OriginRecordViewer : public RecordViewer {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_PROPERTY( qreal _timeWindow READ timeWindowSecs WRITE setTimeWindowSecs)
	Q_ENUMS(StreamView)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum StreamView {
			Raw,
			Frequency,
			Complete = Raw + Frequency
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit OriginRecordViewer(Seiscomp::DataModel::DatabaseQuery*,
		                            Core::RecordCache* cache,
		                            const StreamView& sv = Raw,
		                            QWidget* parent = NULL,
		                            Qt::WFlags = 0);
		~OriginRecordViewer();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		//! Clears the layout and setup entities
		void prepareEntities(const StreamView&);
		//!
		void preparePickerComponents();

		/**
		 * @brief Sets the widget's origin from which information will be pulled
		 *        out: picks, arrivals, etc.
		 * @param publicID the ID of the origin
		 */
		void setOrigin(const std::string& publicID);

		/**
		 * @brief Sets the station of which streams should be displayed
		 * @param network The station's network
		 * @param name The station's name
		 * @param channel The station's channel
		 */
		void setStation(const std::string& network, const std::string& name,
		                const std::string& channel);

		/**
		 * @brief Displays the streams associated with the specified sensor
		 * @param sensor 2 first characters of the sensor's channel code
		 */
		void showStationStream(const QString&);

		/**
		 * @brief Sets the filter to apply on streams.
		 * @param filter The Seiscomp::Math::Filtering::InPlaceFilter<double> object
		 * @note  OriginRecordViewer will take ownership of the pointer and
		 *        therefore it will be managed internally (deleted whenever it
		 *        is required)
		 */
		void setStreamFilter(Seiscomp::Math::Filtering::InPlaceFilter<double>*);

		//! Resets the viewer to its native three components state Z, 1, 2 sensors
		void resetStreams();

		//! General reset function: reset streams and then the picker
		void reset();

		//! Makes a copy of the picker's markers for the current station instance
		void saveCurrentPicks();

		/**
		 * @brief  Checks and see if an instance of a pick is already registered
		 *         for the current station
		 * @param  pickID The ID of the pick
		 * @return true if the pick exists, false otherwise
		 */
		const bool hasPick(const QString&);

		/**
		 * @brief  Pick (marker) fetcher
		 * @param  pickID The ID of the pick
		 * @return The desired pick constant or an un-initialized pick if the
		 *         the latter hasn't been found.
		 */
		const OriginRecordPicker::Marker pick(const QString&);


		Seiscomp::IO::RecordStreamPtr recordStream() const {
			return _recordStream;
		}

		void setStreamUrl(const std::string& url) {
			_recordStreamUrl = url;
		}
		const std::string& streamUrl() const {
			return _recordStreamUrl;
		}

		//! Picker instance
		OriginRecordPicker* picker() {
			return _picker;
		}

		/**
		 * @brief Sets the time period around origin time to fetch data
		 * @param sec The value in seconds (@default is 360.)
		 */
		void setTimeWindowSecs(const qreal& sec = 360.) {
			_timeWindow = sec;
		}
		const qreal& timeWindowSecs() const {
			return _timeWindow;
		}

		/**
		 * @brief Enables the widget to fetch data if station stream is
		 *        missing from the cache.
		 * @param fetch Enables/disable record fetching automation (@default is true)
		 */
		void setAutoFetchRecords(const bool& fetch = true) {
			_autoFetchRecords = fetch;
		}

		/**
		 * @brief Sets the color to use when drawing streams
		 * @param color The color to use
		 * @note  The user has to call replot() whenever...
		 */
		void setWaveTraceColor(const QColor& color) {
			_waveTraceColor = color;
		}

		const QColor& waveTraceColor() {
			return _waveTraceColor;
		}

		const bool& autoRescaleStreamsKeyAxis() const {
			return _autoRescaleStreamsKeyAxis;
		}

		const bool& autoRescaleStreamsValueAxis() const {
			return _autoRescaleStreamsValueAxis;
		}

		/**
		 * @brief Sets the TimeWindow in which the widget will try and fetch
		 *        records, inspect them, find gaps if any...
		 * @param TimeWindow the time window
		 */
		void setTimeWindowBuffer(const Seiscomp::Core::TimeWindow& tw) {
			_tw = tw;
		}

		const Seiscomp::Core::TimeWindow& timeWindowBuffer() const {
			return _tw;
		}

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		bool openStream();
		void closeStream();

		void fetchWaveForms(const QStringList&);
		void stopFetching();

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
		void drawRecordTraces();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		//! Signal emitted whenever the delegate is running and a stop request
		//! has been triggered...
		void stopDelegate();

		//! Status signals fired whenever the state of the widget evolves
		void working();
		void idling();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::DataModel::DatabaseQuery* _query;
		Seiscomp::Math::Filtering::InPlaceFilter<double>* _filter;
		Core::RecordCache* _cache;
		Seiscomp::DataModel::OriginPtr _origin;
		Seiscomp::Core::TimeWindow _tw;

		StreamView _sv;

		QString _networkCode;
		QString _stationCode;
		QString _activeGainUnit;

		ComponentData _zData;
		ComponentData _nsData;
		ComponentData _ewData;

		qreal _timeWindow;
		ActiveStreamList _activeStreams;
		ActiveStream _visibleStream;

		std::string _recordStreamUrl;
		Seiscomp::Record::Hint _recordInputHint;
		Seiscomp::IO::RecordStreamPtr _recordStream;

		OriginRecordPicker* _picker;

		bool _autoFetchRecords;
		bool _autoRescaleStreamsKeyAxis;
		bool _autoRescaleStreamsValueAxis;
		bool _resetRequired;

		QColor _waveTraceColor;

		QAction* _usedStream;
		QList<QAction*> _notUsedStreams;

		QMap<QString, QList<OriginRecordPicker::Marker> > _picks;

		friend class OriginRecordPicker;
};

} // namespace Gui
} // namespace IPGP

#endif
