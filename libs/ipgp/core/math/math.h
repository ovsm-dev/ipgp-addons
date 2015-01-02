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


#ifndef __IPGP_CORE_MATH_H__
#define __IPGP_CORE_MATH_H__


#include <ipgp/core/api.h>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <cstdarg>
#include <vector>
#include <iosfwd>
#include <algorithm>
#include <iostream>
#include <exception>

#include <complex>
#include <valarray>


namespace IPGP {
namespace Core {
namespace Math {


class SC_IPGP_CORE_API MathException : public std::exception {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MathException(const char* msg) :
				_msg(msg) {}
		explicit MathException(const std::string& msg) :
				_msg(msg) {}
		~MathException() throw () {}

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


// PI constant
static const double PI = 4.0 * atan(1.);
typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

/**
 * @brief Evaluates Fast Fourier Transform (FFT) of a sequence by using the
 *        Cooley-Tukey (in-place) method.
 * @param the reference to the array of data
 * @note  Here is how to make it work:
 *        @code
 *        	const Complex test[] = { 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
 *        	CArray data(test, 8);
 *        	fft(data);
 *        @endcode
 */
void SC_IPGP_CORE_API fft(CArray&);

/**
 * @brief Evaluates the Inverse Fast Fourier Transform (IFFT) of a sequence
 * @param the reference to the array of data
 * @note @see fft() for the HIW part...
 */
void SC_IPGP_CORE_API ifft(CArray&);

/**
 * @brief Calculates the day number of year from tripod YMD
 * @param y the year
 * @param m the month
 * @param d the day
 * @return the day number from YMD
 */
const int SC_IPGP_CORE_API dayOfYear(int y, int m, int d);

/**
 * @brief Rounds double to next upper value
 * @param n double to round up
 * @return rounded double
 */
const double SC_IPGP_CORE_API roundDoubleUp(const double& number);

/**
 * @brief Rounds double with precision check
 * @param number the double input
 * @param precision the number of decimals to check and round up to
 * @return rounded double
 */
const double SC_IPGP_CORE_API roundDouble(const double& number,
                                          const int& precision);

/**
 * @brief Hides decimals from double
 * @param num input number
 * @param n decimal number to show
 * @return converted value of input number
 */
const double SC_IPGP_CORE_API changeDoubleDecimals(const double& num, size_t n);


const double SC_IPGP_CORE_API roundUp(const double& n);

/**
 * @brief Generates a random int
 * @param min starting range
 * @param max ending range
 * @return random generated int
 */
const int SC_IPGP_CORE_API randomInt(const int min, const int max);

/**
 * @brief  Returns which number is smallest
 * @param  count the number of numbers to be checked
 * @return The smallest number value
 */
template<typename T>
const T SC_IPGP_CORE_API smallestNumber(const size_t count, ...);

/**
 * @brief  Returns which number is biggest
 * @param  count the number of numbers to be checked
 * @return The biggest number value
 */
template<typename T>
const T SC_IPGP_CORE_API biggestNumber(const size_t count, ...);

template<typename T>
const T SC_IPGP_CORE_API smallestNumber(const std::vector<T>&);

template<typename T>
const T SC_IPGP_CORE_API biggestNumber(const std::vector<T>&);


/**
 * @class   Numbers
 * @package IPGP::Core::Math
 * @brief   Simple entity to store and evaluate a pool of numbers.
 */
template<typename T>
SC_IPGP_CORE_API class Numbers {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Numbers() {}
		~Numbers() {
			data.clear();
		}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void add(const T& n) {
			data.push_back(n);
		}
		const T max() throw (MathException) {
			if ( data.size() == 0 )
			    throw MathException("max() >> container is empty");
			typename std::vector<T>::iterator max = std::max_element(data.begin(), data.end());
			return *max;
		}
		T max() const throw (MathException) {
			if ( data.size() == 0 )
			    throw MathException("max() >> container is empty");
			typename std::vector<T>::iterator max = std::max_element(data.begin(), data.end());
			return *max;
		}
		typename std::vector<T>::iterator maxPosition() {
			return std::max_element(data.begin(), data.end());
		}
		const T min() throw (MathException) {
			if ( data.size() == 0 )
			    throw MathException("min() >> container is empty");
			typename std::vector<T>::iterator min = std::min_element(data.begin(), data.end());
			return *min;
		}
		T min() const throw (MathException) {
			if ( data.size() == 0 )
			    throw MathException("min() >> container is empty");
			typename std::vector<T>::iterator min = std::min_element(data.begin(), data.end());
			return *min;
		}
		typename std::vector<T>::iterator minPosition() {
			return std::min_element(data.begin(), data.end());
		}
		const T mean() throw (MathException) {
			if ( data.size() == 0 )
			    throw MathException("mean() >> container is empty");
			return (min() + max()) / 2;
		}
		T at(const size_t& i) throw (MathException) {
			if ( i < 0 || i > data.size() )
			    throw MathException("at() >> position out of range");
			return data[i];
		}
		T at(const size_t& i) const throw (MathException) {
			if ( i < 0 || i > data.size() )
			    throw MathException("at() >> position out of range");
			return data[i];
		}
		size_t size() {
			return data.size();
		}
		size_t size() const {
			return data.size();
		}
		std::vector<T> vector() {
			return data;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::vector<T> data;
};


/**
 * @brief Compares one number to another, assigns the second number value to
 *        the first one if it is smaller
 * @param Type T of the number
 * @param Type T of the value
 */
template<typename T> void SC_IPGP_CORE_API checkIfItIsSmaller(T*, const T&);

/**
 * @brief Compares one number to another, assigns the second number value to
 *        the first one if it is bigger
 * @param Type T of the number
 * @param Type T of the value
 */
template<typename T> void SC_IPGP_CORE_API checkIfItIsBigger(T*, const T&);


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * @brief Iterates int, float or double vector and returns max stored value
 * @param vector the vector to search thru
 * @return the max stored value
 */
template<typename T> T SC_IPGP_CORE_API getVectorMaxValue(const std::vector<T>& v) {

	typename std::vector<T>::const_iterator it = v.begin();
	T retValue;
	if ( it == v.begin() )
	    retValue = *it;
	++it;
	while ( it != v.end() ) {
		if ( retValue < *it )
		    retValue = *it;
		++it;
	}
	return retValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * @brief Iterates int, float or double vector and returns min stored value
 * @param vector the vector to search thru
 * @return the min stored value
 */
template<typename T> T SC_IPGP_CORE_API getVectorMinValue(const std::vector<T>& v) {

	typename std::vector<T>::const_iterator it = v.begin();
	T retValue;
	if ( it == v.begin() )
	    retValue = *it;
	++it;
	while ( it != v.end() ) {
		if ( retValue > *it )
		    retValue = *it;
		++it;
	}
	return retValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



/**
 * @brief Tells if number is in a specified range
 * @param rangeMin Range minimum value
 * @param rangeMax Range maximum value
 * @param number The number to checkout
 * @return True if true, false otherwise
 */
template<class T> bool SC_IPGP_CORE_API
numberIsInRange(const T& rangeMin, const T& rangeMax, const T& number);




/**
 * @brief Tells if a number is stored in vector
 * @param vector The vector of number
 * @param value The number to checkout
 * @return True if true, false otherwise
 */
template<class T> bool SC_IPGP_CORE_API
numberIsInVector(const std::vector<T>& vector, const T& value);


/**
 * @brief  Evaluates the accomplished percentage of something...
 * @param  size the number of max iteration
 * @param  iterator the current iteration value
 * @return size_t value in percentage
 */
template<typename T> const T SC_IPGP_CORE_API
percentageOfSomething(const T& size, const T& iterator) throw (MathException);

/**
 * @brief Re-maps a number from a range into another.
 * @param value The original number value
 * @param minValue The old range minimum value
 * @param maxValue The old range maximum value
 * @param newMinValue The new range minimum value
 * @param newMaxValue The new range maximum value
 * @return The value of the number inside the new range
 */
template<typename T> const T SC_IPGP_CORE_API
remap(const T& value, const T& minValue, const T& maxValue,
      const T& newMinValue, const T& newMaxValue) throw (MathException);

/**
 * @struct LinearRegression
 * @brief  Storage entity for linear regression calculation.
 */
struct SC_IPGP_CORE_API LinearRegression {

		LinearRegression() :
				SUMx(.0), SUMy(.0), SUMxy(.0), SUMxx(.0), SUMres(.0), res(.0),
				slope(.0), y_intercept(.0), SUM_Yres(.0), AVGy(.0), AVGx(.0),
				Yres(.0), Rsqr(.0), stdDev(.0), stdErr(.0), detCoef(.0),
				corrCoef(.0) {}

		LinearRegression& operator=(const LinearRegression& lr) {

			SUMx = lr.SUMx;
			SUMy = lr.SUMy;
			SUMxy = lr.SUMxy;
			SUMxx = lr.SUMxx;
			SUMres = lr.SUMres;
			res = lr.res;
			slope = lr.slope;
			y_intercept = lr.y_intercept;
			SUMres = lr.SUMres;
			AVGy = lr.AVGy;
			AVGx = lr.AVGx;
			Yres = lr.Yres;
			Rsqr = lr.Rsqr;
			stdDev = lr.stdDev;
			stdErr = lr.stdErr;
			detCoef = lr.detCoef;
			corrCoef = lr.corrCoef;

			return *this;
		}

		bool operator==(const LinearRegression& lr) {
			return (SUMx == lr.SUMx && SUMy == lr.SUMy && SUMxy == lr.SUMxy &&
			        SUMxx == lr.SUMxx && SUMres == lr.SUMres && res == lr.res &&
			        slope == lr.slope && y_intercept == lr.y_intercept &&
			        SUMres == lr.SUMres && AVGy == lr.AVGy && AVGx == lr.AVGx &&
			        Yres == lr.Yres && Rsqr == lr.Rsqr && stdDev == lr.stdDev &&
			        stdErr == lr.stdErr && detCoef == lr.detCoef && corrCoef == lr.corrCoef);
		}

		double SUMx;        // sum of x values
		double SUMy;        // sum of y values
		double SUMxy;       // sum of x * y
		double SUMxx;       // sum of x^2
		double SUMres;      // sum of squared residue
		double res;         // residue squared
		double slope;       // slope of regression line
		double y_intercept; // y intercept of regression line
		double SUM_Yres;    // sum of squared of the discrepancies
		double AVGy;        // mean of y
		double AVGx;        // mean of x
		double Yres;        // squared of the discrepancies
		double Rsqr;        // coefficient of determination
		double stdDev;      // standard deviation
		double stdErr;      // standard error
		double detCoef;     // determination coefficient
		double corrCoef;    // correlation coefficient
};

/**
 * @brief  Computes least mean square linear regression of given data.
 * @param  xs An xvalues vector
 * @param  ys An yvalues vector
 * @return The computation result as a LinearRegression entity
 */
const LinearRegression SC_IPGP_CORE_API
leastMeanSquareRegression(const std::vector<double>& xs,
                          const std::vector<double>& ys) throw (MathException);


} // end Math
} // end Core
} // end IPGP


#endif


