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

#define SEISCOMP_COMPONENT IPGP_TOPOFILE

#include <ipgp/gui/opengl/topographyfile.h>
#include <ipgp/gui/opengl/vertex.h>
#include <ipgp/gui/opengl/triangle.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/logging/log.h>
#include <stdio.h>
#include <fstream>

using namespace Seiscomp;


namespace {

template<typename T> void clearList(QList<T*>& l) {
	qDeleteAll(l);
	l.clear();
}

template<typename T> void clearVector(QVector<T*>& v) {
	qDeleteAll(v);
	v.clear();
}

}


namespace IPGP {
namespace Gui {
namespace OpenGL {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyFile::Data::Data() {
	minLatitude = maxLatitude = minLongitude = maxLongitude = minElevation
	        = maxElevation = -1.;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyFile::Data::clear() {
	::clearVector(triangles);
	::clearVector(vertices);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyFile::TopographyFile(const Type& t) :
		_type(t) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TopographyFile::~TopographyFile() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyFile::setType(const Type& t) {
	_type = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const TopographyFile::Type& TopographyFile::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& TopographyFile::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const TopographyFile::Data& TopographyFile::data() const {
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TopographyFile::reload() {
	_data.clear();
	read(_file);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
XYZFile::XYZFile() :
		TopographyFile(XYZ) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
XYZFile::~XYZFile() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool XYZFile::read(const QString& file) {

	_file = file;
	std::ifstream in(file.toStdString().c_str());
	if ( !in.good() ) return false;

	_data.clear();

	Util::StopWatch sw;
	sw.restart();

	int count = 0;
	std::string s0;
	float n0, n1, n2;

	while ( !in.eof() ) {
		in >> n0 >> n1 >> n2;

		if ( count == 0 ) {
			_data.minLongitude = _data.maxLongitude = n0;
			_data.minLatitude = _data.maxLatitude = n1;
			_data.minElevation = _data.maxElevation = n2;
		}

		if ( _data.minLatitude > n0 )
		    _data.minLatitude = n0;
		if ( _data.minLongitude > n1 )
		    _data.minLongitude = n1;
		if ( _data.minElevation > n2 )
		    _data.minElevation = n2;

		if ( _data.maxLatitude < n0 )
		    _data.maxLatitude = n0;
		if ( _data.maxLongitude < n1 )
		    _data.maxLongitude = n1;
		if ( _data.maxElevation < n2 )
		    _data.maxElevation = n2;

		_data.vertices << new Vertex(n0, n1, n2);
		++count;
	}
	in.close();

	QStringList l = file.split('/');
	SEISCOMP_DEBUG("Loaded XYZ file ../%s in %s", l.last().toStdString().c_str(),
	    Seiscomp::Core::Time(sw.elapsed()).toString("%T.%f").c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
STLFile::STLFile() :
		TopographyFile(STL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
STLFile::~STLFile() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool STLFile::isBinarySTL(const QString& file) {

	bool binary = false;
	FILE* fp = fopen(file.toStdString().c_str(), "r");
	/* Find size of file */
	fseek(fp, 0, SEEK_END);
//	int file_size = ftell(fp);
	int facenum;
	/* Check for binary or ASCII file */
	fseek(fp, STL_LABEL_SIZE, SEEK_SET);
	fread(&facenum, sizeof(int), 1, fp);
//	int expected_file_size = STL_LABEL_SIZE + 4 + (sizeof(short) + sizeof(STLFacet)) * facenum;
//	if ( file_size == expected_file_size )
//		binary = true;
	unsigned char tmpbuf[128];
	fread(tmpbuf, sizeof(tmpbuf), 1, fp);
	for (unsigned int i = 0; i < sizeof(tmpbuf); i++) {
		if ( tmpbuf[i] > 127 ) {
			binary = true;
			break;
		}
	}

	// Now we know if the stl file is ascii or binary.
	fclose(fp);

	return binary;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool STLFile::isBinarySTLColored(const QString& file, bool& magicsMode) {

	FILE* fp = fopen(file.toStdString().c_str(), "rb");
	char buf[STL_LABEL_SIZE + 1];
	fread(buf, sizeof(char), STL_LABEL_SIZE, fp);
	std::string strInput(buf);
	size_t cInd = strInput.rfind("COLOR=");
	size_t mInd = strInput.rfind("MATERIAL=");
	(cInd != std::string::npos && mInd != std::string::npos) ? magicsMode = true : magicsMode = false;
	int facenum;
	fread(&facenum, sizeof(int), 1, fp);

	for (int i = 0; i < std::min(facenum, 1000); ++i) {

		unsigned short attr;
		Vertex n;
		Vertex v[3];
		fread(&n, sizeof(Vertex), 1, fp);
		fread(&v, sizeof(Vertex), 3, fp);
		fread(&attr, sizeof(unsigned short), 1, fp);

		//! Check if attr is negative (for an unsigned short? what?)
		if ( attr != 0 )
		    if ( attr != static_cast<unsigned short>(0xffffffff) )
		        return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool STLFile::read(const QString& file) {

	_file = file;
	_data.clear();

	Util::StopWatch sw;
	sw.restart();

	if ( !isBinarySTL(file) ) {

		std::ifstream in(file.toStdString().c_str());
		if ( !in.good() ) return false;

		char title[80];
		std::string s0, s1;
		float n0, n1, n2, f0, f1, f2, f3, f4, f5, f6, f7, f8;
		in.read(title, 80);
		while ( !in.eof() ) {
			in >> s0;                                // facet || endsolid
			if ( s0 == "facet" ) {
				in >> s1 >> n0 >> n1 >> n2;            // normal x y z
				in >> s0 >> s1;            // outer loop
				in >> s0 >> f0 >> f1 >> f2;            // vertex x y z
				in >> s0 >> f3 >> f4 >> f5;            // vertex x y z
				in >> s0 >> f6 >> f7 >> f8;            // vertex x y z
				in >> s0;            // endloop
				in >> s0;            // endfacet
				_data.triangles << new Triangle(Vertex(n0, n1, n2), Vertex(f0, f1, f2),
				    Vertex(f3, f4, f5), Vertex(f6, f7, f8));
			}
			else if ( s0 == "endsolid" ) break;
		}
		in.close();
	}
	else {

		//! TODO Re-write this part in pure Qt/c++
		FILE* f = fopen(file.toStdString().c_str(), "rb");
		if ( !f )
		    return false;

		bool magicsMode;
		bool stlIsColored = isBinarySTLColored(file, magicsMode);

		char title[80];
		int nFaces;
		fread(title, 80, 1, f);
		fread((void*) &nFaces, 4, 1, f);
		float v[12]; // normal=3, vertices=3*3 = 12
		// Every Face is 50 Bytes: Normal(3*float), Vertices(9*float), 2 Bytes Spacer

		bool isInit = false;
		for (int i = 0; i < nFaces; ++i) {

			//! Read the 4 vertices, first being the triangle normal
			//! TODO Investigate why this method crashes
//			Vertex vertex[4];
//			fread((void*)&vertex, sizeof(Vertex), 4, f);

			for (size_t j = 0; j < 12; ++j)
				fread((void*) &v[j], sizeof(float), 1, f);

			unsigned short attr;
			fread(&attr, sizeof(unsigned short), 1, f);

			if ( !isInit ) {
				_data.minLongitude = _data.maxLongitude = v[3];
				_data.minLatitude = _data.maxLatitude = v[4];
				_data.minElevation = _data.maxElevation = v[5];
				isInit = true;
			}

			Core::Math::Numbers<float> lats, lons, eles;
			lats.add(v[4]);
			lats.add(v[7]);
			lats.add(v[10]);
			lons.add(v[2]);
			lons.add(v[6]);
			lons.add(v[9]);
			eles.add(v[5]);
			eles.add(v[8]);
			eles.add(v[11]);

			Core::Math::checkIfItIsSmaller<float>(&_data.minLatitude, lats.min());
			Core::Math::checkIfItIsSmaller<float>(&_data.minLongitude, lons.min());
			Core::Math::checkIfItIsSmaller<float>(&_data.minElevation, eles.min());

			Core::Math::checkIfItIsBigger<float>(&_data.maxLatitude, lats.max());
			Core::Math::checkIfItIsBigger<float>(&_data.maxLongitude, lons.max());
			Core::Math::checkIfItIsBigger<float>(&_data.maxElevation, eles.max());

			//			fread((void*) &uint16, sizeof(unsigned short), 1, f); // spacer between successive faces

			QColor c;
			(stlIsColored) ? c = getColor(attr) : c = Qt::gray;

			_data.triangles << new Triangle(Vertex(v[0], v[1], v[2], c),
			    Vertex(v[3], v[4], v[5], c), Vertex(v[6], v[7], v[8], c),
			    Vertex(v[9], v[10], v[11], c));
		}
		fclose(f);
	}

	QStringList l = file.split('/');

	SEISCOMP_DEBUG("Loaded STL file ../%s in %s", l.last().toStdString().c_str(),
	    Seiscomp::Core::Time(sw.elapsed()).toString("%T.%f").c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor STLFile::getColor(const unsigned short& val) {

	unsigned short r = val % 32 * 8;
	unsigned short g = ((val / 32) % 32) * 8;
	unsigned short b = ((val / 1024) % 32) * 8;

	return QColor(r, g, b);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace OpenGL
} // namespace Gui
} // namespace IPGP
