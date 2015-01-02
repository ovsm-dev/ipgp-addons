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

#define SEISCOMP_COMPONENT IPGP_RECORDMANAGER

#include <ipgp/gui/datamodel/recordmanager.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/math/fft.h>
#include <assert.h>

using namespace Seiscomp;
using namespace Seiscomp::Core;

namespace IPGP {
namespace Gui {

const char* CRs[] = { "FullSecond", "HalfASecond", "QuarterOfSecond" };
const double CR[] = { 1., .5, .25 };
const std::vector<double> VCR(CR, CR + sizeof(CR) / sizeof(CR[0]));
const double getComputingResolutionValue(const ComputingResolution& cr) {
	assert(cr >= 0 && cr < MaxOfComputingResolution);
	return CR[cr];
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordManager::RecordManager(QObject* parent) :
		QObject(parent), _cr(HalfASecond), _fsamp(100.) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordManager::~RecordManager() {
	clear();
	SEISCOMP_DEBUG("RecordManager is terminated");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordManager::feed(RecordSequence* rs) {
	for (size_t i = 0; i < rs->size(); ++i)
		feed(rs->at(i).get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordManager::feed(const Record* rec) {

	if ( !rec ) return;

	double stime = rec->startTime();
	_fsamp = rec->samplingFrequency();
	const double delta = 1. / _fsamp;

	ArrayPtr tmp_ar;
	const DoubleArray* data = DoubleArray::ConstCast(rec->data());
	if ( !data ) {
		tmp_ar = rec->data()->copy(Array::DOUBLE);
		data = DoubleArray::ConstCast(tmp_ar);
		if ( !data ) return;
	}

	for (int i = 0; i < data->size(); ++i) {
		_rawData.insert(stime, data->get(i));
		stime += TimeSpan(delta);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordManager::prepareFFT() {

	QList<double> keys = _rawData.keys();
	const double start = keys.front();
	const double end = keys.back();

	/*
	 for (int i = static_cast<int>(start); i < static_cast<int>(end); ++i) {

	 DoubleMap m;

	 for (DoubleMap::iterator j = _rawData.begin();
	 j != _rawData.end(); ++j) {
	 if ( static_cast<int>(j.key()) == i )
	 m.insert(j.key(), j.value());
	 }

	 _rdMap.insert(i, m);
	 }
	 */
	double cr = getComputingResolutionValue(_cr);
//	for (double i = trunc(start); i < trunc(end); i += cr) {
	for (double i = start; i < end; i += cr) {
		DoubleMap m;
		for (DoubleMap::iterator j = _rawData.begin();
		        j != _rawData.end(); ++j) {
			if ( (j.key() >= i) && (j.key() < i + cr) )
			    m.insert(j.key(), j.value());
		}
		_rdMap.insert(i, m);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordManager::computeFFT() {

	for (RDMap::iterator it = _rdMap.begin(); it != _rdMap.end(); ++it) {

		DoubleArray* fftdata = new DoubleArray(it.value().size());

		int idx = 0;
		for (DoubleMap::iterator jt = it.value().begin();
		        jt != it.value().end(); ++jt, ++idx)
			fftdata->append(idx, jt.value());

		std::vector<Seiscomp::Math::Complex> fftRes;
		Seiscomp::Math::fft(fftRes, fftdata->size(), fftdata->typedData());

		delete fftdata;

		if ( fftRes.size() == 0 ) continue;

		Entity e;
		e.time = it.key();

		for (size_t z = 0; z < fftRes.size(); ++z) {

			double amp = sqrt(fftRes[z].real() * fftRes[z].real() + fftRes[z].imag() * fftRes[z].imag());
			double freq = static_cast<double>(z) * _fsamp / static_cast<double>(fftRes.size());

			e.amps.add(amp);
			e.freqs.add(freq);
		}
		_entities.push_back(e);

		emit newEntity(e);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double RecordManager::getComputingResolution() {
	return getComputingResolutionValue(_cr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
