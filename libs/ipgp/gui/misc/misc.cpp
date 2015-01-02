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


#include <ipgp/gui/misc/misc.h>
#include <ipgp/core/math/math.h>
#include <QDebug>
#include <QImage>
#include <math.h>



using namespace IPGP::Core::Math;



namespace {

typedef QPair<qreal, QColor> ColorPair;
typedef QList<ColorPair> ColorList;

QColor interpolate(const QColor& start, const QColor& end, const qreal& ratio) {
	int red = (int) (ratio * start.red() + (1 - ratio) * end.red());
	int green = (int) (ratio * start.green() + (1 - ratio) * end.green());
	int blue = (int) (ratio * start.blue() + (1 - ratio) * end.blue());
	return QColor(red, green, blue);
}

QColor interpolate(const QPointF& point, const QPointF& startPoint,
                   const QPointF& endPoint, const QColor& start,
                   const QColor& end) {
	double segmentLength = sqrt((endPoint.x() - startPoint.x()) * (endPoint.x() - startPoint.x())
	        + (endPoint.y() - startPoint.y()) * (endPoint.y() - startPoint.y()));
	double pdist = sqrt((point.x() - startPoint.x()) * (point.x() - startPoint.x())
	        + (point.y() - startPoint.y()) * (point.y() - startPoint.y()));
	return interpolate(start, end, pdist / segmentLength);
}

QColor colorInterpolation(const QColor& a, const QColor& b, const qreal& ratio) {
	return QColor((b.red() - a.red()) * ratio + a.red(),
	    (b.green() - a.green()) * ratio + a.green(),
	    (b.blue() - a.blue()) * ratio + a.blue());
}

QColor getColor(const double& value, ColorList& list, QColor color = Qt::black) {

	for (int i = 0; i < list.size(); ++i) {

		if ( value <= list.at(i).first ) {
			color = list.at(i).second;
			break;
		}

		if ( i != (list.size() - 1) ) {
			if ( value > list.at(i).first && value < list.at(i + 1).first ) {
				double ratio = .0;
				try {
					ratio = remap<double>(value, list.at(i).first, list.at(i + 1).first, .0, 1.);
				} catch ( ... ) {}
				color = interpolate(list.at(i).second, list.at(i + 1).second, ratio);
				break;
			}
			else
				continue;
		}
	}

	return color;
}

}



namespace IPGP {
namespace Gui {
namespace Misc {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor getDepthColoration(const double& depthKm) {

	::ColorList map;
	map << ::ColorPair(-5., Qt::darkRed);
	map << ::ColorPair(.0, Qt::red);
	map << ::ColorPair(10., QColor(255, 42, 0));
	map << ::ColorPair(20., QColor(255, 93, 0));
	map << ::ColorPair(30., QColor(255, 144, 0));
	map << ::ColorPair(40., QColor(255, 195, 0));
	map << ::ColorPair(50., QColor(255, 246, 0));
	map << ::ColorPair(60., QColor(255, 255, 0));
	map << ::ColorPair(70., QColor(238, 255, 3));
	map << ::ColorPair(80., QColor(139, 255, 21));
	map << ::ColorPair(90., QColor(63, 250, 54));
	map << ::ColorPair(100., QColor(8, 41, 102));
	map << ::ColorPair(110., QColor(0, 160, 183));
	map << ::ColorPair(120., QColor(0, 72, 250));
	map << ::ColorPair(130., QColor(0, 24, 220));
	map << ::ColorPair(200., Qt::magenta);
	map << ::ColorPair(600., Qt::gray);

	return ::getColor(depthKm, map);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor getMSKColor(const double& value) {

	QColor q = Qt::white;

	if ( value <= .0 )
	    return q;

//	if ( value > .0 && value < 1.1 )
//		q = QColor::fromRgb(255, 255, 255, 255);
//

	if ( value > 1.1 && value <= 1.5 )
	    q = QColor::fromRgb(248, 248, 252, 255);

	if ( value > 1.5 && value <= 2. )
	    q = QColor::fromRgb(233, 233, 252, 255);

	if ( value > 2. && value <= 2.5 )
	    q = QColor::fromRgb(230, 232, 255, 255);

	if ( value > 2.5 && value <= 3. )
	    q = QColor::fromRgb(210, 223, 255, 255);

	if ( value > 3. && value <= 3.5 )
	    q = QColor::fromRgb(204, 226, 255, 255);

	if ( value > 3.5 && value <= 4. )
	    q = QColor::fromRgb(187, 234, 255, 255);

	if ( value > 4. && value <= 4.5 )
	    q = QColor::fromRgb(178, 243, 255, 255);

	if ( value > 4.5 && value <= 5. )
	    q = QColor::fromRgb(171, 255, 248, 255);

	if ( value > 5. && value <= 5.5 )
	    q = QColor::fromRgb(193, 255, 234, 255);

	if ( value > 5.5 && value <= 6. )
	    q = QColor::fromRgb(195, 255, 202, 255);

	if ( value > 6. && value <= 6.5 )
	    q = QColor::fromRgb(213, 255, 176, 255);

	if ( value > 6.5 && value <= 7. )
	    q = QColor::fromRgb(238, 255, 135, 255);

	if ( value > 7. && value <= 7.5 )
	    q = QColor::fromRgb(255, 250, 110, 255);

	if ( value > 7.5 && value <= 8. )
	    q = QColor::fromRgb(255, 221, 134, 255);

	if ( value > 8. && value <= 8.5 )
	    q = QColor::fromRgb(255, 184, 87, 255);

	if ( value > 8.5 && value <= 9. )
	    q = QColor::fromRgb(255, 132, 73, 255);

	if ( value > 9. && value <= 9.5 )
	    q = QColor::fromRgb(255, 93, 63, 255);

	if ( value > 9.5 && value <= 10. )
	    q = QColor::fromRgb(241, 89, 89, 255);

	if ( value > 10. && value <= 10.5 )
	    q = QColor::fromRgb(213, 82, 82, 255);

	if ( value > 10.5 )
	    q = QColor::fromRgb(151, 35, 35, 255);

	return q;


	/*
	 ::ColorList map;
	 map << ::ColorPair(.0, Qt::white);
	 map << ::ColorPair(2., QColor(210, 223, 255));
	 map << ::ColorPair(3., QColor(187, 234, 255));
	 map << ::ColorPair(4., QColor(171, 255, 148));
	 map << ::ColorPair(5., QColor(196, 255, 200));
	 map << ::ColorPair(6., QColor(238, 255, 135));
	 map << ::ColorPair(7., QColor(255, 220, 134));
	 map << ::ColorPair(8., QColor(255, 132, 73));
	 map << ::ColorPair(9., QColor(239, 49, 49));
	 map << ::ColorPair(10., QColor(170, 80, 80));
	 map << ::ColorPair(15., Qt::darkRed);

	 return ::getColor(value, map, Qt::white);
	 */

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor getPGAColor(const double& value) {

	QColor q = Qt::white;

	if ( value <= 0 )
	    return q;

//	if ( value > .0 && value < 1.5 )
//		q = QColor::fromRgb(234, 234, 252, 255);

	if ( value >= 1.5 && value <= 3.2 )
	    q = QColor::fromRgb(210, 223, 255, 255);

	if ( value > 3.2 && value <= 6.8 )
	    q = QColor::fromRgb(198, 237, 255, 255);

	if ( value > 6.8 && value <= 15. )
	    q = QColor::fromRgb(171, 255, 248, 255);

	if ( value > 15. && value <= 32. )
	    q = QColor::fromRgb(195, 255, 202, 255);

	if ( value > 32. && value <= 68. )
	    q = QColor::fromRgb(238, 255, 135, 255);

	if ( value > 68. && value <= 150. )
	    q = QColor::fromRgb(255, 221, 134, 255);

	if ( value > 150. && value <= 320. )
	    q = QColor::fromRgb(255, 132, 73, 255);

	if ( value > 320. && value <= 680. )
	    q = QColor::fromRgb(243, 87, 87, 255);

	if ( value > 680. )
	    q = QColor::fromRgb(152, 29, 29, 255);

	return q;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor getMSKColoration(const int& value) {

	QColor q = Qt::white;
	switch ( value ) {
		case 1:
			q = QColor::fromRgb(255, 255, 255);
		break;
		case 2:
			q = QColor::fromRgb(210, 223, 255);
		break;
		case 3:
			q = QColor::fromRgb(187, 234, 255);
		break;
		case 4:
			q = QColor::fromRgb(171, 255, 248);
		break;
		case 5:
			q = QColor::fromRgb(196, 255, 200);
		break;
		case 6:
			q = QColor::fromRgb(238, 255, 135);
		break;
		case 7:
			q = QColor::fromRgb(255, 220, 134);
		break;
		case 8:
			q = QColor::fromRgb(255, 132, 73);
		break;
		case 9:
			q = QColor::fromRgb(239, 49, 49);
		break;
		case 10:
			q = QColor::fromRgb(170, 80, 80);
		break;
		default:
			q = QColor::fromRgb(255, 255, 255);
		break;
	}
	return q;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor getResidualsColoration(const qreal& value) {

	QColor q = Qt::black;

	int color = (int) (255 / fabs(value)) % 255;

	if ( color > 255 )
		color = 255;
	else if ( color < 0 )
	    color = 0;

	if ( value > .0 ) {
		// Red gradient
		q = QColor::fromRgb(255, color, color);
	}
	else {
		// Blue gradient
		q = QColor::fromRgb(color, color, 255);
	}

	return q;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor getMagnitudeColoration(const double& magvalue,
                                    const double magmax) {

	QColor q = Qt::black;

	int color = sin(magvalue * 0.1) * 100 + magmax * magmax;

	if ( color > 255 )
		color = 255;
	else if ( color < 0 )
	    color = 0;

	q = QColor::fromRgb(color, 0, 0, 255);

	return q;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor lightYellow1() {
	return QColor(255, 222, 0, 70);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor lightYellow2() {
	return QColor(255, 174, 0, 70);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor lightGreen() {
	return QColor(150, 222, 0, 70);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor lightRed() {
	return QColor(255, 0, 91, 70);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor lightOrange() {
	return QColor(255, 104, 0, 70);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int fsize = 4;
static double filter[9][9] = { { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                               { 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void drawShadowedPixmap(QPainter* p, const QPixmap& pix, int dx, int dy,
                        bool onlyShadow) {

	QImage msk = pix.toImage();
	QImage shadow(msk.width() + 2 * fsize, msk.height() + 2 * fsize, QImage::Format_ARGB32);

	double divisor = 0.0;
	for (int i = 0; i < 2 * fsize + 1; i++)
		for (int j = 0; j < 2 * fsize + 1; j++)
			divisor += filter[i][j];

	for (int y = 0; y < shadow.height(); y++)
		for (int x = 0; x < shadow.width(); x++) {

			int l = 0;

			for (int sy = -fsize; sy <= fsize; sy++)
				for (int sx = -fsize; sx <= fsize; sx++) {

					int tx = x + sx - fsize;
					int ty = y + sy - fsize;

					if ( tx < 0 || ty < 0 || tx >= msk.width() || ty >= msk.height() )
						l += (int) (filter[sx + fsize][sy + fsize] * 255);
					else
						l += (int) (filter[sx + fsize][sy + fsize] * (255 - qAlpha(msk.pixel(tx, ty))));
				}

			l = (int) (l / divisor);
			l = 255 - l;
			l = qBound(0, l, 225);

			shadow.setPixel(x, y, qRgba(0, 0, 0, l));
		}

	p->drawImage(dx, dy, shadow);

	if ( !onlyShadow )
	    p->drawPixmap(dx, dy, pix);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Misc
} // namespace Gui
} // namespace IPGP
