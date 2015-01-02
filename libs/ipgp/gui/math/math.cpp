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


#include <ipgp/gui/math/math.h>
#include <math.h>



namespace IPGP {
namespace Gui {
namespace Math {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double getMin(const QVector<double>& vector) {

	double min;
	for (int i = 0; i < vector.size(); ++i) {
		if ( i == 0 )
			min = vector.at(i);
		if ( min > vector.at(i) )
			min = vector.at(i);
	}
	return min;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double getMax(const QVector<double>& vector) {

	double max;
	for (int i = 0; i < vector.size(); ++i) {
		if ( i == 0 )
			max = vector.at(i);
		if ( max < vector.at(i) )
			max = vector.at(i);
	}
	return max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF intersectPoint(const QPointF& circleCenter, const qreal& circleRadius,
                             const qreal& circleThikness, const QPointF& point) {

	qreal theta = atan2(point.y() - circleCenter.y(), point.x() - circleCenter.x());

	QPointF intersect;
	intersect.setX(circleCenter.x() + (circleRadius + circleThikness) * cos(theta));
	intersect.setY(circleCenter.y() + (circleRadius + circleThikness) * sin(theta));

	return intersect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool circleLineIntersect(const qreal& x1, const qreal& y1,
                               const qreal& x2, const qreal& y2,
                               const qreal& cx, const qreal& cy,
                               const qreal& cr) {

	qreal dx = x2 - x1;
	qreal dy = y2 - y1;
	qreal a = dx * dx + dy * dy;
	qreal b = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
	qreal c = cx * cx + cy * cy;
	c += x1 * x1 + y1 * y1;
	c -= 2 * (cx * x1 + cy * y1);
	c -= cr * cr;
	qreal bb4ac = b * b - 4 * a * c;

	if ( bb4ac < 0 )
		return false;
	else
		return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool pointLiesOnLine(const QPointF& point, const QPointF& lineStart,
                           const QPointF& lineEnd, QString* equation) {

	bool retCode;

	double slope, intercept;
	double left, top, right, bottom; // Bounding Box For Line Segment

	QPointF segment;
	segment.setX(lineEnd.x() - lineStart.x());
	segment.setY(lineEnd.y() - lineStart.y());

	slope = segment.y() / segment.x();
	intercept = lineEnd.y() - slope * lineStart.x();

	if ( lineStart.x() < lineEnd.x() ) {
		left = lineStart.x();
		right = lineEnd.x();
	}
	else {
		left = lineEnd.x();
		right = lineStart.x();
	}
	if ( lineStart.y() < lineEnd.y() ) {
		top = lineStart.y();
		bottom = lineEnd.y();
	}
	else {
		top = lineStart.y();
		bottom = lineEnd.y();
	}

	*equation = QString("%1x %2%3").arg(slope).arg(((intercept < 0) ? " " : "+ ")).arg(intercept);

	if ( slope * point.x() + intercept > (point.y() - 0.01) &&
	        slope * point.x() + intercept < (point.y() + 0.01) )
	                {
		if ( point.x() >= left && point.x() <= right &&
				point.y() >= top && point.y() <= bottom )
		                {
			retCode = true;
		}
		else
			retCode = false;
	}
	else
		retCode = false;

	return retCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}
