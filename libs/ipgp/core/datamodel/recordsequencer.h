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

#ifndef __IPGP_CORE_DATAMODEL_RECORDSEQUENCER_H__
#define __IPGP_CORE_DATAMODEL_RECORDSEQUENCER_H__


#include <ipgp/gui/api.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/core/recordsequence.h>
#include <vector>
#include <iterator>
#include <map>


namespace IPGP {
namespace Core {

class SC_IPGP_GUI_API RecordSequencer {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct Data {
				Data(const double& t, const double& v) :
						time(t), value(v) {}
				double time;
				double value;
		};
		typedef std::vector<Data> DataVector;
		typedef DataVector::iterator iterator;
		typedef DataVector::const_iterator const_iterator;
		typedef std::map<double, DataVector> DataMap;
		typedef std::pair<double, DataVector> DataPair;
		typedef std::map<double, double> DoubleMap;
		typedef std::pair<double, double> DoublePair;

		struct Entity {
				double time;
				Core::Math::Numbers<double> amps;
				Core::Math::Numbers<double> freqs;
		};
		typedef std::vector<Entity> EntityVector;

	public:
		explicit RecordSequencer();
		~RecordSequencer();

	public:
		void feed(Seiscomp::RecordSequence*);
		void feed(const Seiscomp::Record*);
		void prepareFFT();
		void computeFFT();
		const EntityVector& entities() const {
			return _entities;
		}
		void clear() {
			_data.clear();
			_dataMap.clear();
			_entities.clear();
		}

	private:
		DataVector _data;
		DataMap _dataMap;
		EntityVector _entities;
		double _fsamp;
};

} // namespace Core
} // namespace IPGP

#endif
