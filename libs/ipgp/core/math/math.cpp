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


#include <ipgp/core/math/math.h>
#include <iomanip>
#include <stdio.h>



namespace IPGP {
namespace Core {
namespace Math {


#define TRUE_NULL 9999999

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void fft(CArray& x) {

	const size_t N = x.size();

	if ( N <= 1 )
		return;

	// divide
	CArray even = x[std::slice(0, N / 2, 2)];
	CArray odd = x[std::slice(1, N / 2, 2)];

	// conquer
	fft(even);
	fft(odd);

	// combine
	for (size_t k = 0; k < N / 2; ++k) {
		Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
		x[k] = even[k] + t;
		x[k + N / 2] = even[k] - t;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ifft(CArray& x) {

	// conjugate the complex numbers
	x = x.apply(std::conj);

	// forward fft
	fft(x);

	// conjugate the complex numbers again
	x = x.apply(std::conj);

	// scale the numbers
	x /= x.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int dayOfYear(int y, int m, int d) {

	int jd = (1461 * (y + 4800 + (m - 14) / 12)) / 4 + (367 * (m - 2 - 12
	    * ((m - 14) / 12))) / 12 - (3 * ((y + 4900
	    + (m - 14) / 12) / 100)) / 4 + d - 32075;
	int a = (jd + 31738) % 146097 % 36524 % 1461;
	int b = a / 1460;
	int dayOfYear = (a - b) % 365 + b + 1;

	return dayOfYear;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double roundDoubleUp(const double& number) {
	return ((number >= 0.5) ? ceil(number) : floor(number));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double roundDouble(const double& number, const int& precision) {

	double ret;
	std::stringstream s;
	s << std::setprecision(precision) << std::setiosflags(std::ios_base::fixed) << number;
	s >> ret;

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double changeDoubleDecimals(const double& num, size_t n) {

	std::stringstream str1;
	str1.setf(std::ios::fixed, std::ios::floatfield);
	str1 << num;
	n += str1.str().find('.') + 1;

	/*
	 * Double won't show zeros anyway...
	 * TODO: find another method
	 std::string decimals = str1.str().substr(0, n);
	 while (decimals.size() != n) {
	 decimals.append("0");
	 }
	 std::string str = decimals;
	 */

	std::string str = str1.str().substr(0, n);
	std::stringstream str2;
	str2 << str;

	double num2;
	str2 >> num2;

	return num2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double roundUp(const double& n) {
	return ((n >= 0.5) ? ceil(n) : floor(n));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
boost::mt19937 gen;
const int randomInt(const int min, const int max) {

	boost::uniform_int<> dist(min, max);
	boost::variate_generator<boost::mt19937&, boost::uniform_int<> > randomInt(gen, dist);

	return randomInt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const int smallestNumber(const size_t count, ...) {

	va_list ap;
	int min;

	va_start(ap, count);
	for (size_t i = 0; i < count; ++i) {

		if ( i == 0 )
			min = va_arg(ap, int);

		if ( va_arg(ap, int) < min )
			min = va_arg(ap, int);
	}
	va_end(ap);

	return min;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const double smallestNumber(const size_t count, ...) {

	va_list ap;
	double min;

	va_start(ap, count);
	for (size_t i = 0; i < count; ++i) {

		if ( i == 0 )
			min = va_arg(ap, double);

		if ( va_arg(ap, double) < min )
			min = va_arg(ap, double);
	}
	va_end(ap);

	return min;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const int smallestNumber(const std::vector<int>& v) {

	int n = -1;
	for (size_t i = 0; i < v.size(); ++i) {
		if ( i == 0 )
			n = v[i];
		if ( v[i] < n )
			n = v[i];
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const double smallestNumber(const std::vector<double>& v) {

	double n = -1.;
	for (size_t i = 0; i < v.size(); ++i) {
		if ( i == 0 )
			n = v[i];
		if ( v[i] < n )
			n = v[i];
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const int biggestNumber(const size_t count, ...) {

	if ( count <= 0 )
		return -1;

	va_list ap;
	int max = -1;

	va_start(ap, count);
	for (size_t i = 0; i < count; ++i) {

		if ( i == 0 )
			max = va_arg(ap, int);

		if ( va_arg(ap, int) > max )
			max = va_arg(ap, int);
	}
	va_end(ap);

	return max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const double biggestNumber(const size_t count, ...) {

	if ( count <= 0 )
		return -1.;

	va_list ap;
	double max = -1.;

	va_start(ap, count);
	for (size_t i = 0; i < count; ++i) {

		if ( i == 0 )
			max = va_arg(ap, double);

		if ( va_arg(ap, double) > max )
			max = va_arg(ap, double);
	}
	va_end(ap);

	return max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const int biggestNumber(const std::vector<int>& v) {

	int n = -1;
	for (size_t i = 0; i < v.size(); ++i) {
		if ( i == 0 )
			n = v[i];
		if ( v[i] > n )
			n = v[i];
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const double biggestNumber(const std::vector<double>& v) {

	double n = -1.;
	for (size_t i = 0; i < v.size(); ++i) {
		if ( i == 0 )
			n = v[i];
		if ( v[i] > n )
			n = v[i];
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> void checkIfItIsSmaller(int* number, const int& value) {
	if ( *number > value )
		*number = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> void checkIfItIsSmaller(float* number, const float& value) {
	if ( *number > value )
		*number = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> void checkIfItIsSmaller(double* number, const double& value) {
	if ( *number > value )
		*number = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> void checkIfItIsBigger(int* number, const int& value) {
	if ( *number < value )
		*number = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> void checkIfItIsBigger(float* number, const float& value) {
	if ( *number < value )
		*number = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> void checkIfItIsBigger(double* number, const double& value) {
	if ( *number < value )
		*number = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
bool numberIsInRange<int>(const int& rangeMin, const int& rangeMax,
                          const int& number) {

	bool retCode = false;
	if ( (rangeMin < number) && (number < rangeMax) )
		retCode = true;

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
bool numberIsInRange<float>(const float& rangeMin, const float& rangeMax,
                            const float& number) {

	bool retCode = false;
	if ( (rangeMin < number) && (number < rangeMax) )
		retCode = true;

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
bool numberIsInRange<double>(const double& rangeMin, const double& rangeMax,
                             const double& number) {

	bool retCode = false;
	if ( (rangeMin < number) && (number < rangeMax) )
		retCode = true;

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
bool numberIsInVector(const std::vector<int>& vector, const int& value) {

	bool retcode = false;
	for (size_t i = 0; i < vector.size(); ++i)
		if ( vector[i] == value )
			retcode = true;

	return retcode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
bool numberIsInVector(const std::vector<double>& vector, const double& value) {

	bool retcode = false;
	for (size_t i = 0; i < vector.size(); ++i)
		if ( vector[i] == value )
			retcode = true;

	return retcode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
bool numberIsInVector(const std::vector<float>& vector, const float& value) {

	bool retcode = false;
	for (size_t i = 0; i < vector.size(); ++i)
		if ( vector[i] == value )
			retcode = true;

	return retcode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> const size_t
percentageOfSomething(const size_t& size, const size_t& iterator) throw (MathException) {

	// Check zero division first
	if ( size == 0 )
		throw MathException("Division by zero not possible");

	return (100 * iterator) / size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> const int
percentageOfSomething(const int& size, const int& iterator) throw (MathException) {

	// Check zero division first
	if ( size == 0 )
		throw MathException("Division by zero not possible");

	return (100 * iterator) / size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<> const double
percentageOfSomething(const double& size, const double& iterator) throw (MathException) {

	// Check zero division first
	if ( size == .0 )
		throw MathException("Division by zero not possible");

	return (100. * iterator) / size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const int remap(const int& value, const int& oMin, const int& oMax,
                const int& nMin, const int& nMax) throw (MathException) {

	// range check
	if ( oMin == oMax )
		throw MathException("Original min and max are the same");

	if ( nMin == nMax )
		throw MathException("New min and max are the same");

	int oldMin, oldMax;
	// check reversed input range
	bool reverseInput = false;

	(oMin < oMax) ? oldMin = oMin : oldMin = oMax;
	(oMin < oMax) ? oldMax = oMax : oldMax = oMin;

	if ( oldMin != oMin )
		reverseInput = true;

	// check reversed output range
	bool reverseOutput = false;
	int newMin, newMax;
	(nMin < nMax) ? newMin = nMin : newMin = nMax;
	(nMin < nMax) ? newMax = nMax : newMax = nMin;

	if ( newMin != nMin )
		reverseOutput = true;

	int portion = (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
	if ( reverseInput )
		portion = (oldMax - value) * (newMax - newMin) / (oldMax - oldMin);

	int result = portion + newMin;
	if ( reverseOutput )
		result = newMax - portion;

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
const double remap(const double& value, const double& oMin, const double& oMax,
                   const double& nMin, const double& nMax) throw (MathException) {

	// range check
	if ( oMin == oMax )
		throw MathException("Original min and max are the same");

	if ( nMin == nMax )
		throw MathException("New min and max are the same");

	double oldMin, oldMax;
	// check reversed input range
	bool reverseInput = false;

	(oMin < oMax) ? oldMin = oMin : oldMin = oMax;
	(oMin < oMax) ? oldMax = oMax : oldMax = oMin;

	if ( oldMin != oMin )
		reverseInput = true;

	// check reversed output range
	bool reverseOutput = false;
	double newMin, newMax;
	(nMin < nMax) ? newMin = nMin : newMin = nMax;
	(nMin < nMax) ? newMax = nMax : newMax = nMin;

	if ( newMin != nMin )
		reverseOutput = true;

	double portion = (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
	if ( reverseInput )
		portion = (oldMax - value) * (newMax - newMin) / (oldMax - oldMin);

	double result = portion + newMin;
	if ( reverseOutput )
		result = newMax - portion;

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LinearRegression
leastMeanSquareRegression(const std::vector<double>& xs,
                          const std::vector<double>& ys) throw (MathException) {

	if ( xs.size() != ys.size() || xs.size() == 0 )
		throw MathException("Empty data set for linear regression calculation");

	LinearRegression reg;

	//calculate various sums
	for (size_t i = 0; i < xs.size(); ++i) {
		//sum of x
		reg.SUMx = reg.SUMx + xs.at(i);
		//sum of y
		reg.SUMy = reg.SUMy + ys.at(i);
		//sum of squared x*y
		reg.SUMxy = reg.SUMxy + xs.at(i) * ys.at(i);
		//sum of squared x
		reg.SUMxx = reg.SUMxx + xs.at(i) * xs.at(i);
	}

	//calculate the means of x and y
	reg.AVGy = reg.SUMy / xs.size();
	reg.AVGx = reg.SUMx / xs.size();

	//slope or a1
	reg.slope = (xs.size() * reg.SUMxy - reg.SUMx * reg.SUMy) / (xs.size() * reg.SUMxx - reg.SUMx * reg.SUMx);

	//y itercept or a0
	reg.y_intercept = reg.AVGy - reg.slope * reg.AVGx;

#ifdef DEBUG
	printf("x mean(AVGx) = %0.5E\n", reg.AVGx);
	printf("y mean(AVGy) = %0.5E\n", reg.AVGy);
	printf("\n");
	printf("The linear equation that best fits the given data is:\n");
	printf("       y = %2.8lfx + %2.8f\n", reg.slope, reg.y_intercept);
	printf("------------------------------------------------------------\n");
	printf("   Original (x,y)   (y_i - y_avg)^2     (y_i - a_o - a_1*x_i)^2\n");
	printf("------------------------------------------------------------\n");
#endif

	//calculate squared residues, their sum etc.
	for (size_t i = 0; i < xs.size(); ++i) {

		//current (y_i - a0 - a1 * x_i)^2
		reg.Yres = pow((ys.at(i) - reg.y_intercept - (reg.slope * xs.at(i))), 2);

		//sum of (y_i - a0 - a1 * x_i)^2
		reg.SUM_Yres += reg.Yres;

		//current residue squared (y_i - AVGy)^2
		reg.res = pow(ys.at(i) - reg.AVGy, 2);

		//sum of squared residues
		reg.SUMres += reg.res;

#ifdef DEBUG
		printf("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
				xs.at(i), ys.at(i), reg.res, reg.Yres);
#endif
	}

	//calculate r^2 coefficient of determination
	reg.Rsqr = (reg.SUMres - reg.SUM_Yres) / reg.SUMres;


	reg.stdDev = sqrt(reg.SUMres / (xs.size() - 1));
	reg.stdErr = sqrt(reg.SUM_Yres / (xs.size() - 2));
	reg.detCoef = (reg.SUMres - reg.SUM_Yres) / reg.SUMres;
	reg.corrCoef = sqrt(reg.Rsqr);

#ifdef DEBUG
	printf("--------------------------------------------------\n");
	printf("Sum of (y_i - y_avg)^2 = %0.5E\t\n", reg.SUMres);
	printf("Sum of (y_i - a_o - a_1*x_i)^2 = %0.5E\t\n", reg.SUM_Yres);
	printf("Standard deviation(St) = %0.5E\n", reg.stdDev);
	printf("Standard error of the estimate(Sr) = %0.5E\t\n", reg.stdErr);
	printf("Coefficient of determination(r^2) = %0.5E\t\n", reg.detCoef);
	printf("Correlation coefficient(r) = %0.5E\t\n", reg.corrCoef);
#endif

	return reg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Math
} // namespace Core
} // namespace IPGP
