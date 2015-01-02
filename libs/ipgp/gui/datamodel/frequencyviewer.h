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




#ifndef __IPGP_GUI_DATAMODEL_FREQUENCYVIEWER_H___
#define __IPGP_GUI_DATAMODEL_FREQUENCYVIEWER_H___

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/math/fft.h>
#include <QLabel>
#include <QComboBox>




namespace IPGP {
namespace Gui {

enum SC_IPGP_GUI_API EPlotType {
	FreqAmpCounts, FreqAmpdB, FreqPhase
};



class SC_IPGP_GUI_API FrequencyViewerPlot : public QCustomPlot {

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

		struct Stream {
				Stream();
				QString id;
				QString unit;
				qreal fsamp;
				Seiscomp::DoubleArray data;
				std::vector<Seiscomp::Math::Complex> fftRes;
				QPen tracePen;
		};

		typedef QHash<QString, Stream> StreamList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit FrequencyViewerPlot(const EPlotType& = FreqAmpCounts,
		                             QWidget* = NULL, Qt::WFlags = 0);
		~FrequencyViewerPlot();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void resizeEvent(QResizeEvent*);
		void changeRulerState();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void reset();
		void addStream(Stream&);
		void setType(const EPlotType&, const bool& update = true);
		void setFrequencyProfile(const bool& rescaleAuto = true,
		                         const qreal& freqRangeMin = .1,
		                         const qreal& freqRangeMax = 25.,
		                         const bool& update = true);
		void setAmplitudeProfile(const bool& rescaleAuto = false,
		                         const qreal& ampRangeMin = .0,
		                         const qreal& ampRangeMax = 1.,
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
		const EPlotType& type() const {
			return _t;
		}
		void setRuler(const Ruler& r) {
			_r = r;
		}
		const Ruler& ruler() const {
			return _r;
		}
		const bool& rescaleFrequencyRange() const {
			return _rescaleFrequencyRange;
		}
		const bool& rescaleAmplitudeRange() const {
			return _rescaleAmplitudeRange;
		}
		const qreal& freqRangeMin() const {
			return _freqRangeMin;
		}
		const qreal& freqRangeMax() const {
			return _freqRangeMax;
		}
		const qreal& ampRangeMin() const {
			return _ampRangeMin;
		}
		const qreal& ampRangeMax() const {
			return _ampRangeMax;
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
		EPlotType _t;
		Ruler _r;
		QCPItemLine* _marker;
		QComboBox* _typeBox;
		QLabel* _infoLabel;
		QString _type;
		StreamList _streams;
		bool _rescaleFrequencyRange;
		bool _rescaleAmplitudeRange;
		qreal _freqRangeMin;
		qreal _freqRangeMax;
		qreal _ampRangeMin;
		qreal _ampRangeMax;
};



} // namespace Gui
} // namespace IPGP

#endif
