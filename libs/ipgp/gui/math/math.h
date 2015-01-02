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


#ifndef __IPGP_GUI_MATH_H__
#define __IPGP_GUI_MATH_H__

#include <ipgp/gui/api.h>
#include <QPointF>
#include <QVector>
#include <QString>


namespace IPGP {
namespace Gui {
namespace Math {

/**
 * @brief Iterates thru vector<double> and returns minimum value
 * @param vector the vector to iterate
 * @return minimum double value
 */
const double SC_IPGP_GUI_API getMin(const QVector<double>& vector);

/**
 * @brief Iterates thru vector<double> and returns maximum value
 * @param vector the vector to iterate
 * @return maximum double value
 */
const double SC_IPGP_GUI_API getMax(const QVector<double>& vector);


/**
 * @brief Intersection point coordinates calculator in which the line's
 *        start point is the circle center.
 * @param circleCenter the circle's center coordinates / line start point
 * @param circleRadius the radius of the circle
 * @param circleThikness the thikness of the circle
 * @param point the line end point
 * @return QPointF coordinates of the intersection point
 */
const QPointF SC_IPGP_GUI_API
intersectPoint(const QPointF& circleCenter, const qreal& circleRadius,
               const qreal& circleThikness, const QPointF& point);


/**
 * @brief Evaluates if a line and a circle have at least one intersection point
 * @param x1 line start point x coordinate
 * @param y1 line start point y coordinate
 * @param x2 line end point x coordinate
 * @param y2 line end point y coordinate
 * @param cx circle center x coordinate
 * @param cy circle center y coordinate
 * @param cr circle's radius
 * @return true if intersection, false otherwise
 */
const bool SC_IPGP_GUI_API
circleLineIntersect(const qreal& x1, const qreal& y1,
                    const qreal& x2, const qreal& y2,
                    const qreal& cx, const qreal& cy,
                    const qreal& cr);


/**
 *
 */
const bool SC_IPGP_GUI_API pointLiesOnLine(const QPointF& point,
                                           const QPointF& lineStart,
                                           const QPointF& lineEnd,
                                           QString* equation);
}
}
}

#endif

