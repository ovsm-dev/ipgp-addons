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


#ifndef __IPGP_GUI_DATAMODEL_RECORDMANAGER_H__
#define __IPGP_GUI_DATAMODEL_RECORDMANAGER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QObject>
#include <ipgp/core/math/math.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/core/recordsequence.h>
#include <QVector>
#include <QMap>
#include <QMetaType>


namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(RecordManager);

struct SC_IPGP_GUI_API Entity {
		double time;
		Core::Math::Numbers<double> amps;
		Core::Math::Numbers<double> freqs;
};

enum SC_IPGP_GUI_API ComputingResolution {
	FullSecond,
	HalfASecond,
	QuarterOfSecond,
	MaxOfComputingResolution
};

class SC_IPGP_GUI_API RecordManager : public QObject {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QMap<double, double> DoubleMap;
		typedef QMap<double, DoubleMap> RDMap;
		typedef QVector<Entity> EntityVector;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		RecordManager(QObject* = NULL);
		~RecordManager();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void feed(Seiscomp::RecordSequence*);
		void feed(const Seiscomp::Record*);
		void prepareFFT();
		void computeFFT();
		const double getComputingResolution();
		void setComputingResolution(const ComputingResolution& cr) {
			_cr = cr;
		}
		const ComputingResolution& computingResolution() const {
			return _cr;
		}
		const EntityVector& entities() const {
			return _entities;
		}
		void clear() {
			_rawData.clear();
			_rdMap.clear();
			_entities.clear();
		}

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void newEntity(Entity);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		DoubleMap _rawData;
		RDMap _rdMap;
		EntityVector _entities;
		ComputingResolution _cr;
		double _fsamp;
};

}
}

Q_DECLARE_METATYPE(IPGP::Gui::Entity)
#endif
