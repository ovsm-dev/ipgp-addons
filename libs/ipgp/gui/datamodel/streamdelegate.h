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

#ifndef __IPGP_GUI_DATAMODEL_STREAMDELEGATE_H__
#define __IPGP_GUI_DATAMODEL_STREAMDELEGATE_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/core/datamodel/recordcache.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/io/recordstream.h>
#include <QObject>
#include <QMutex>
#include <QMetaType>
#include <QVector>
#include <QStringList>

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(StreamDelegate);

/**
 * @class   StreamDelegate
 * @package IPGP::Gui::DataModel
 * @note    Once instantiated, the user shall use the internal mutex to deal
 *          with thread safety before object destruction.
 * @brief   Records fetcher
 *
 * This class provides an delegate for retrieving records. Its main engine
 * being a Seiscomp::IO::RecordStream instance, it therefore understands the
 * same files, arclink and seedlink protocols used by the latter.
 * This delegate can be used in two modes:
 *  + a CachedRecords mode in which records are stored untouched inside a cache,
 *  + a RealTimeRecords mode in which records are emitted on the fly for
 *    immediate use (e.g. real time graphs) and stream filters can be applied.
 */
class SC_IPGP_GUI_API StreamDelegate : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Mode {
			CachedRecords = 0x01,
			RealTimeRecords = 0x02
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit StreamDelegate(Seiscomp::IO::RecordStream*,
		                        const Seiscomp::Record::Hint&,
		                        Core::RecordCache* cache,
		                        const Mode& mode = CachedRecords);
		~StreamDelegate();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const size_t& receivedRecords() const {
			return _receivedRecords;
		}
		const bool& isRunning() const {
			return _isRunning;
		}
		QMutex& lock() {
			return _recordLock;
		}
		const QMutex& lock() const {
			return _recordLock;
		}

		/**
		 * @brief Processed stations list setter.
		 * @param list of the stations that have to be fetched
		 * @note  This goes in pair with stationStreamsReady() signal...
		 */
		void setStations(const QStringList& list) {
			_stationsList = list;
		}

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool storeRecord(Seiscomp::Record*);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void readRecords();
		void stopReading();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt Signals
		// ------------------------------------------------------------------
		void newRecord(Seiscomp::Record*);
		void acquisitionTerminated();

		/**
		 * @brief Stations records being fetched in order, station by station,
		 *        the user has the possibility to dispose of a entire station
		 *        records pool has soon as it is available.
		 *        Therefore, this signal is fired whenever pool of records
		 *        for a particular station is complete.
		 * @param QString the station NET.NAME
		 */
		void stationStreamsReady(const QString&);

		/**
		 * @brief Signal fired whenever an acquisition exception occurs.
		 * @param QString OperationInterrupted message
		 */
		void reportOperationException(const QString&);

		/**
		 * @brief Signal fired whenever an record stream exception occurs.
		 * @param QString RecordStreamException message
		 */
		void reportRecordStreamException(const QString&);

		/**
		 * @brief Signal fired whenever an acquisition exception occurs.
		 * @param QString std::exception message
		 */
		void reportAcquisitionException(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::IO::RecordStream* _recordStream;
		Seiscomp::Record::Hint _recordHint;
		Core::RecordCache* _cache;
		Mode _mode;
		size_t _receivedRecords;
		bool _stop;
		bool _isRunning;
		QMutex _recordLock;
		QStringList _stationsList;
};

} // namespace Gui
} // namespace IPGP

#endif
