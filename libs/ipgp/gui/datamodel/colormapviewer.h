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




#ifndef __IPGP_GUI_DATAMODEL_COLORMAPVIEWER_H___
#define __IPGP_GUI_DATAMODEL_COLORMAPVIEWER_H___

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <ipgp/core/math/math.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/math/fft.h>
#include <QLabel>
#include <QComboBox>



namespace IPGP {
namespace Gui {


enum SC_IPGP_GUI_API ColorMapType {
	Spectrogram, Scalogram
};


class SC_IPGP_GUI_API ColorMapViewerPlot : public QCustomPlot {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(DrawingMethod)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Ruler {
			None,
			Vertical,
			Horizontal
		};

		struct RecordData {
				double time;		//! first record time
				double fsamp;		//! frequency sample
				Seiscomp::DoubleArray stack;
		};

		struct Stream {
				Stream();
				QString id;
				QString unit;
				QStack<RecordData> data;
				QPen tracePen;
		};

		struct DataPoint {
				double startTime;
				double endTime;
				Core::Math::Numbers<double> amplitudes;
				Core::Math::Numbers<double> frequencies;
		};

		typedef QHash<QString, Stream> StreamList;
		typedef std::vector<Seiscomp::Math::Complex> ComplexVector;
		typedef QVector<DataPoint> DataPointVector;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ColorMapViewerPlot(const ColorMapType& = Spectrogram,
		                            QWidget* = NULL, Qt::WFlags = 0);
		~ColorMapViewerPlot();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void mousePressEvent(QMouseEvent*);
		void resizeEvent(QResizeEvent*);
		void changeRulerState();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void reset();
		void addStream(Stream&);
		void setType(const ColorMapType&, const bool& update = true);
		void setFrequencyProfile(const bool& rescaleAuto = true,
		                         const qreal& freqRangeMin = .1,
		                         const qreal& freqRangeMax = 25.,
		                         const bool& update = true);

		void redraw();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void setType(const int&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const ColorMapType& type() const {
			return _t;
		}
		void setRuler(const Ruler& r) {
			_r = r;
		}
		const Ruler& ruler() const {
			return _r;
		}
		const bool& rescaleFrequencyRange() const {
			return _rescaleFreqRange;
		}
		const qreal& frequencyRangeMin() const {
			return _fmin;
		}
		const qreal& frequencyRangeMax() const {
			return _fmax;
		}

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void markerIndication(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		ColorMapType _t;
		Ruler _r;
		QCPItemLine* _marker;
		QCPColorMap* _colorMap;
		QComboBox* _typeBox;
		QLabel* _infoLabel;
		QCPAxisRect* _topAxisRect;
		QCPAxisRect* _bottomAxisRect;
		QCPGraph* _trace;
		QCPAxis* _timeScale;
		QString _type;
		StreamList _streams;
		double _fmin;
		double _fmax;
		bool _rescaleFreqRange;
};



} // namespace Gui
} // namespace IPGP

#endif
