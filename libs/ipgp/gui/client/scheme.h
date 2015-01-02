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

#ifndef __IPGP_GUI_SCHEME_H__
#define __IPGP_GUI_SCHEME_H__

#include <ipgp/gui/api.h>
#include <QString>
#include <QMap>
#include <vector>
#include <memory>



namespace IPGP {
namespace Gui {

/**
 * @class   ParameterBase
 * @package IPGP::Gui::Client
 * @brief   Virtual generic parameter class
 * @note    No methods implementation inside this class, everything shall be done
 *          within classes that extend it.
 */
class SC_IPGP_GUI_API ParameterBase {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		virtual ~ParameterBase() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		template<class T> const T& get() const;
		template<class T, class U> void setValue(const U& rhs);
};

/**
 * @class   Parameter
 * @package IPGP::Gui::Client
 * @brief   Parameter class interface.
 *
 * The user must take care of proper casting encapsulation, otherwise it will
 * throw a std::bad_cast exception.
 *
 * Here is an simple example:
 * @code
 * 		std::vector<std::string> v;
 * 		Parameter<std::string> p1("Hello");
 * 		v.push_back(&p1);
 * 		std::cout << v[0]->get<std::string>(); //read the string
 * 		v[0]->set<std::string>("BANANA"); //set the string to something else
 * 		v[0]->get<int>(); //throws a std::bad_cast exception
 * @endcode
 *
 * @note   The next step in implementation would be to use typed enums for each
 *         variable so that each one of them could be instantiated using a simple
 *         typed alias of its name.
 */
template<typename T>
class SC_IPGP_GUI_API Parameter : public ParameterBase {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Parameter(const T& rhs) :
				_value(rhs) {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const T& get() const {
			return _value;
		}
		void setValue(const T& rhs) {
			_value = rhs;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		T _value;
};


template<class T> const T& ParameterBase::get() const {
	return dynamic_cast<const Parameter<T>&>(*this).get();
}

template<class T, class U> void ParameterBase::setValue(const U& rhs) {
	return dynamic_cast<Parameter<T>&>(*this).setValue(rhs);
}


/**
 * @class   Scheme
 * @package IPGP::Gui::Client
 * @brief   GUI scheme!
 *
 * This class inquires GUI Application about various scheme informations.
 *
 * @todo    Implements parameters in a more generic way using ParameterBase
 *          class and derivatives.
 */
class SC_IPGP_GUI_API Scheme {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Scheme();
		~Scheme() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const size_t& generalPrecision() const;
		const size_t& depthPrecision() const;
		const size_t& distancePrecision() const;
		const size_t& locationPrecision() const;
		const size_t& rmsPrecision() const;
		const size_t& azimuthPrecision() const;
		const size_t& pickTimePrecision() const;
		const size_t& pgaPrecision() const;
		const bool& distanceInKM() const;
		const QString& styleSheet() const;
		const QString& fontFamily() const;
		const double& fontSize() const;

		void setGeneralPrecision(const size_t&);
		void setDepthPrecision(const size_t&);
		void setDistancePrecision(const size_t&);
		void setLocationPrecision(const size_t&);
		void setRmsPrecision(const size_t&);
		void setAzimuthPrecision(const size_t&);
		void setPickTimePrecision(const size_t&);
		void setPgaPrecision(const size_t&);
		void setUseDistanceInKM(const bool&);
		void setStyleSheet(const QString&);
		void setFontFamilty(const QString&);
		void setFontSize(const double&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		size_t _generalPrecision;
		size_t _depthPrecision;
		size_t _distancePrecision;
		size_t _locationPrecision;
		size_t _rmsPrecision;
		size_t _azimuthPrecision;
		size_t _pickTimePrecision;
		size_t _pgaPrecision;

		QString _styleSheet;
		QString _fontFamily;

		double _fontSize;

		bool _distanceInKM;
};



/**
 * @class   GlobalSettings
 * @package IPGP::Gui::Client
 * @brief   Global settings!
 *
 * This class inquires GUI Application about various global parameters.
 */
class SC_IPGP_GUI_API GlobalSettings {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QMap<QString, QString> StringMap;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit GlobalSettings();
		~GlobalSettings() {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const size_t& loadEventDB() const;
		const size_t& streamAcquisitionTimeout() const;
		const QString& streamURI() const;
		const QString& databaseURI() const;
		const StringMap& streamFilters() const;

		void setLoadEventDB(const size_t&);
		void setStreamAcquisitionTimeout(const size_t&);
		void setStreamURI(const QString&);
		void setDatabaseURI(const QString&);
		void setStreamFilters(const StringMap&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		size_t _eventDB;
		size_t _streamTimeout;
		QString _streamURI;
		QString _databaseURI;
		StringMap _streamFilters;
};


/**
 * @brief  Station tooltip formatter.
 * @note   Transmitted values have to be properly formatted first, this method
 *         is only in charge of the layout.
 * @param  stationCode The station's code
 * @param  stationDescription The station's description
 * @param  latitude The station's latitude
 * @param  longitude The station's longitude
 * @param  distance The station's distance
 * @param  elevation The station's elevation
 * @param  azimuth The station's azimuth
 * @param  type The station's type
 * @return An HTML representation of the station's tooltip.
 */
const QString SC_IPGP_GUI_API stationTooltip(const QString& stationCode,
                                             const QString& stationDescription,
                                             const QString& latitude,
                                             const QString& longitude,
                                             const QString& distance,
                                             const QString& elevation,
                                             const QString& azimuth,
                                             const QString& type);

/**
 * @brief  Epicenter tooltip formatter.
 * @note   Transmitted values have to be properly formatted first, this method
 *         is only in charge of the layout.
 * @param  publicID The origin's/event's publicID
 * @param  datetime The origin's/event's date and time
 * @param  latitude The origin's/event's latitude
 * @param  latitudeUncertainty The origin's/event's latitude uncertainty
 * @param  longitude The origin's/event's longitude
 * @param  longitudeUncertainty The origin's/event's longitude uncertainty
 * @param  depth The origin's/event's depth
 * @param  depthUncertainty The origin's/event's depth uncertainty
 * @param  rms The origin's/event's Root Mean Square
 * @param  azimuth The origin's/event's azimuth
 * @param  magnitude The origin's/event's magnitude
 * @param  magnitudeType The origin's/event's magnitude type (denomination, e.g. ML)
 * @param  distance The origin's/event's distance
 * @param  region The origin's/event's region name
 * @param  author The origin's/event's author
 * @return An HTML representation of the epicenter's tooltip.
 */
const QString SC_IPGP_GUI_API epicenterTooltip(const QString& publicID,
                                               const QString& datetime,
                                               const QString& latitude,
                                               const QString& latitudeUncertainty,
                                               const QString& longitude,
                                               const QString& longitudeUncertainty,
                                               const QString& depth,
                                               const QString& depthUncertainty,
                                               const QString& rms,
                                               const QString& azimuth,
                                               const QString& magnitude,
                                               const QString& magnitudeType,
                                               const QString& distance,
                                               const QString& region,
                                               const QString& author);

} // namespace Gui
} // namespace IPGP

#endif
