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



#ifndef __IPGP_GUI_MISC_MISC_H__
#define __IPGP_GUI_MISC_MISC_H__

#include <ipgp/gui/api.h>
#include <QtCore>
#include <QColor>
#include <QMap>
#include <QPainter>
#include <QPixmap>



namespace IPGP {
namespace Gui {
namespace Misc {


/**
 * @brief Depth coloration getter according to local OVSM/IPGP GMT configuration
 * @param depthKm hypocenter depth
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API getDepthColoration(const double& depthKm);


/**
 * @brief MSK coloration getter according to local OVSM/IPGP Shakemap configuration
 * @param value MSK value
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API getMSKColor(const double&);


/**
 * @brief MSK coloration getter according to local OVSM/IPGP Shakemap configuration
 * @param value MSK value
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API getPGAColor(const double&);


/**
 * @brief MSK coloration getter according to local OVSM/IPGP Shakemap configuration
 * @param value MSK value
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API getMSKColoration(const int& value);

/**
 * @brief Default painting color for residuals. A gradient may be generated :
 *        - positive value is represented by a red gradient,
 *        - negative value is represented by a blue gradient.
 * @param the residual value in sec
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API getResidualsColoration(const qreal&);

/**
 * @brief  Evaluates the color of magnitude varying from black to dark red.
 * @param  magvalue The value of magnitude
 * @param  magmax   The value of maximum magnitude estimation scale
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API
getMagnitudeColoration(const double& magvalue,
                       const double magmax = 10.);

/**
 * @brief Light Yellow 1 color getter!
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API lightYellow1();


/**
 * @brief Light Yellow 2 color getter!
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API lightYellow2();


/**
 * @brief Light Green color getter!
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API lightGreen();


/**
 * @brief Light Red color getter!
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API lightRed();


/**
 * @brief Light Orange color getter!
 * @return QColor constant
 */
const QColor SC_IPGP_GUI_API lightOrange();



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * @brief QMap search engine
 * @param key the key string to locate
 * @param map the map to iterate
 * @return NULL or the object pointer
 */
template<typename T> T SC_IPGP_GUI_API
getObjectPointer(const std::string& key, const QMap<std::string, T>& map) {

	typename QMap<std::string, T>::iterator it = map.begin();
	while ( it != map.end() ) {
		if ( it.key() == key )
			return it.value();
	}
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



/**
 * @brief Draws a shadowed pixmap
 * @param p the painter pointer
 * @param pix the pixmap
 * @param dx the pixmap x position
 * @param dy the pixmap y position
 * @param onlyShadow should the shadow be drawn without the pixmap
 */
void SC_IPGP_GUI_API
drawShadowedPixmap(QPainter* p, const QPixmap& pix, int dx, int dy,
                   bool onlyShadow = false);



const QColor defaultTraceColor = QColor(128, 128, 128, 255);
const QColor defaultTraceBackgroundColor = Qt::white;
const QColor waitingForStreamDataColor = QColor(251, 251, 122, 255);
const QColor noStreamDataColor = QColor(255, 127, 127, 255);
const QColor waveRegionColor = QColor(230, 230, 250, 70);
const QPen waveRegionPen = QPen(QColor(84, 84, 234));
const QColor gapColor = QColor(255, 255, 191);
const QColor gridColor = QColor(192, 192, 255);

}
}
}

#endif
