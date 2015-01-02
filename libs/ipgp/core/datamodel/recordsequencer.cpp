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

#define SEISCOMP_COMPONENT IPGP_RECORDSEQUENCER

#include <ipgp/core/datamodel/recordsequencer.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/math/fft.h>

using namespace Seiscomp;
using namespace Seiscomp::Core;


namespace IPGP {
namespace Core {

RecordSequencer::RecordSequencer() :
		_fsamp(100.) {}

RecordSequencer::~RecordSequencer() {
	clear();
}

void RecordSequencer::feed(RecordSequence* rs) {
	for (size_t i = 0; i < rs->size(); ++i)
		feed(rs->at(i).get());
}

void RecordSequencer::feed(const Record* rec) {

	if ( !rec ) return;

	double stime = rec->startTime();
	_fsamp = rec->samplingFrequency();
	double delta = 1. / _fsamp;

	ArrayPtr tmp_ar;
	const DoubleArray* data = DoubleArray::ConstCast(rec->data());
	if ( !data ) {
		tmp_ar = rec->data()->copy(Array::DOUBLE);
		data = DoubleArray::ConstCast(tmp_ar);
		if ( !data ) return;
	}

	for (int i = 0; i < data->size(); ++i) {
		_data.push_back(Data(stime, data->get(i)));
		stime += TimeSpan(delta);
	}

	SEISCOMP_INFO("Fed buffer with %zu ", _data.size());
}


void RecordSequencer::prepareFFT() {

	DoubleMap map;
	double start, end;
	for (size_t i = 0; i < _data.size(); ++i) {

		if ( i == 0 )
		    start = end = _data.at(i).time;

		if ( start > _data.at(i).time )
		    start = _data.at(i).time;
		if ( end < _data.at(i).time )
		    end = _data.at(i).time;

		map[_data.at(i).time] = _data.at(i).value;
	}

	if ( map.size() == 0 ) return;

	for (int i = start; i < end; i += 1.) {

		DataVector v;

		for (DoubleMap::iterator j = map.begin();
		        j != map.end(); ++j) {
			if ( fabs(j->first) == i )
			    v.push_back(Data(j->first, j->second));
		}

		//! Add plain second of data
		_dataMap[i] = v;
	}
}



void RecordSequencer::computeFFT() {

	if ( _dataMap.size() == 0 ) {
		SEISCOMP_WARNING("%s - no data to compute", __func__);
		return;
	}

	for (DataMap::iterator it = _dataMap.begin(); it != _dataMap.end(); ++it) {

		DoubleArray* fftdata = new DoubleArray(it->second.size());
		for (size_t i = 0; i < it->second.size(); ++i)
			fftdata->append(i, it->second.at(i).value);

		std::vector<Seiscomp::Math::Complex> fftRes;
		Seiscomp::Math::fft(fftRes, fftdata->size(), fftdata->typedData());

		delete fftdata;

		if ( fftRes.size() == 0 ) continue;

		Entity e;
		e.time = it->first;

		for (size_t z = 0; z < fftRes.size(); ++z) {

			double amp = sqrt(fftRes[z].real() * fftRes[z].real() + fftRes[z].imag() * fftRes[z].imag());
			double freq = static_cast<double>(z) * _fsamp / static_cast<double>(fftRes.size());

			e.amps.add(amp);
			e.freqs.add(freq);
		}

		_entities.push_back(e);
	}
}

} // namespace Core
} // namespace IPGP
