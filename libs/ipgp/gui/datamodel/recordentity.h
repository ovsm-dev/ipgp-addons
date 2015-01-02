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

#ifndef __IPGP_GUI_DATAMODEL_RECORDENTITY_H__
#define __IPGP_GUI_DATAMODEL_RECORDENTITY_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <exception>
#include <string>
#include <QString>
#include <QStack>
#include <QMap>
#include <QPair>
#include <QPen>
#include <seiscomp3/math/filter.h>


class QCustomPlot;
class QCPAxis;
class QCPAxisRect;
class QCPItemText;
class QCPItemRect;
class QCPItemLine;
class QCPGraph;
class QCPAbstractItem;
class QCPColorMap;

namespace Seiscomp {
class RecordSequence;

namespace DataModel {
class Pick;
class Origin;
}
}

namespace IPGP {
namespace Gui {

/**
 * @class   RecordEntityException
 * @package IPGP::Gui::DataModel
 * @brief   Macro for RecordEntity class exceptions
 */
class SC_IPGP_GUI_API RecordEntityException : public std::exception {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit RecordEntityException(const char* msg) :
				_msg(msg) {}
		explicit RecordEntityException(const std::string& msg) :
				_msg(msg) {}
		explicit RecordEntityException(const QString& msg) :
				_msg(msg.toStdString()) {}
		~RecordEntityException() throw () {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const char* what() const throw () {
			return this->_msg.c_str();
		}
		const std::string& whatString() const throw () {
			return this->_msg;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::string _msg;
};

/**
 * @class    RecordEntity
 * @@package IPGP::Gui::DataModel
 * @brief    RecordEntity class!
 *
 * This class provides a simple RecordEntity container in which RecordSequence
 * objects are plotted.
 */
class SC_IPGP_GUI_API RecordEntity  {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		//! Record entities are stored vertically from top to bottom, the user as
		//! to set the proper RecordEntity position in order to picks and other
		//! QCPItems to get the rightful position when drawn.
		enum Position {
			FirstFromTop,
			Middle,
			Last,
			Unknown
		};

		enum Type {
			Numeric,
			Frequency
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit RecordEntity(QCustomPlot*, const QString&,
		                      const Type& t = Numeric);
		~RecordEntity();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		//! Destroys the RecordEntity by deleting all its objects
		void destroy() throw (RecordEntityException);

		/**
		 * @brief Resets this record entity to its default state: traces, gaps
		 *        are destroyed and background set to WaitingForStreams status
		 * @param newName The new name of this entity
		 * @note  The user has to update manually this entity.
		 */
		void reset(const QString&) throw (RecordEntityException);

		/**
		 * @brief Rescale items main function
		 * @param bool replot Asks for replot after items have been rescaled
		 */
		void rescaleItems(const bool& = false);
		void rescaleItems(const double& min, const double& max,
		                  const bool& replot = false);
		void rescale(const bool& replot = true);

		//! Getters
		const QString& name() const {
			return _name;
		}
		QCustomPlot* plot() const {
			return _comp;
		}
		QCPAxisRect* rect() const {
			return _rect;
		}
		QCPAxis* bottomAxis() const {
			return _bottomAxis;
		}
		QCPAxis* topAxis() const {
			return _topAxis;
		}
		QCPAxis * leftAxis() const {
			return _leftAxis;
		}
		QCPAxis* rightAxis() const {
			return _rightAxis;
		}
		QCPGraph* trace() const {
			return _trace;
		}
		QCPItemText* label() const {
			return _label;
		}
		const bool& isValid() const {
			return _isValid;
		}
		const bool& isSelected() const {
			return _isSelected;
		}
		const Position& position() const {
			return _cp;
		}
		const Type& type() const {
			return _type;
		}
		const float& visibleAmpMin() const {
			return _visibleAmpMin;
		}
		const float& visibleAmpMax() const {
			return _visibleAmpMax;
		}

		//! Setters
		void setName(const QString& n) throw (RecordEntityException);
		void setPlot(QCustomPlot* p) throw (RecordEntityException);
		void setRectAxis(QCPAxisRect* r) throw (RecordEntityException);
		void setValid(const bool& v) {
			_isValid = v;
		}
		void setSelected(const bool& v) {
			_isSelected = v;
		}
		void setPosition(const Position& p) {
			_cp = p;
		}
		void setType(const Type& t) {
			_type = t;
		}
		void setVisibleAmpMin(const float& v) {
			_visibleAmpMin = v;
		}
		void setVisibleAmpMax(const float& v) {
			_visibleAmpMax = v;
		}

		void setSequence(Seiscomp::RecordSequence*, const bool& update = false) throw (RecordEntityException);
		void appendSequence(Seiscomp::RecordSequence*, const bool& update = false,
		                    const qreal& buffer = 1200.) throw (RecordEntityException);

		void setSequenceFilter(Seiscomp::Math::Filtering::InPlaceFilter<double>* f) {
			_filter = f;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		void drawTrace(Seiscomp::RecordSequence*);
		void rescaleGaps();
		void rescaleGaps(const double& min, const double& max);

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString _name;
		QCustomPlot* _comp;
		QCPAxisRect* _rect;
		QCPAxis* _bottomAxis;
		QCPAxis* _topAxis;
		QCPAxis* _leftAxis;
		QCPAxis* _rightAxis;
		QCPGraph* _trace;
		QCPColorMap* _colorMap;
		QCPItemText* _label;
		Seiscomp::Math::Filtering::InPlaceFilter<double>* _filter;
		QStack<QCPItemRect*> _gaps;
		float _visibleAmpMin;
		float _visibleAmpMax;
		bool _isValid;
		bool _isSelected;
		Position _cp;
		Type _type;
};

} // namespace Gui
} // namespace IPGP

#endif
