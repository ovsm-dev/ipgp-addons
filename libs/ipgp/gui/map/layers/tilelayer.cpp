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

#include <ipgp/gui/map/layers/tilelayer.h>
#include <ipgp/gui/map/tile.h>
#include <ipgp/gui/map/config.h>
#include <QPixmap>
#include <QRect>
#include <QPainter>
#include <QPointF>
#include <QString>
#include <QDebug>


namespace IPGP {
namespace Gui {
namespace Map {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TileLayer::TileLayer(const QString& name, const QString& desc,
                     const bool& visible) :
		Layer(Layer::Layer_Tile, name, desc, visible, false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TileLayer::~TileLayer() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TileLayer::draw(QPainter& painter, const QPointF& startTile,
                     const QRect& viewport, const int& zoom) {

	if ( !isVisible() )
		return;

	painter.save();

	const int tilesHoriz = viewport.width() / TILE_SIZE + 2;
	const int tilesVert = viewport.height() / TILE_SIZE + 2;

	for (int i = 0; i < tilesHoriz; ++i) {
		for (int j = 0; j < tilesVert; ++j) {

			const int x = TILE_SIZE * (i - startTile.x() + static_cast<int>(startTile.x()));
			const int y = TILE_SIZE * (j - startTile.y() + static_cast<int>(startTile.y()));

			Tile tile((static_cast<int>(startTile.x()) + i) % pow2[zoom],
			    (static_cast<int>(startTile.y()) + j) % pow2[zoom],
			    zoom, mapSettings().tilePath, mapSettings().tilePattern);

			QPixmap tempImage;
			if ( !tempImage.load(tile.path()) && mapSettings().paintDefaultBackground ) {

				tempImage = QPixmap(TILE_SIZE, TILE_SIZE);
				QPainter dummyPainter;
				dummyPainter.begin(&tempImage);
				dummyPainter.fillRect(0, 0, tempImage.width(), tempImage.height(),
				    mapSettings().defaultBackground);
				dummyPainter.setFont(QFont("Arial", 64));
				dummyPainter.setOpacity(.2);
				dummyPainter.setPen(Qt::black);
				dummyPainter.drawText(tempImage.rect(), Qt::AlignCenter, "?");
				dummyPainter.setFont(QFont("Arial", 8));
				dummyPainter.setPen(Qt::black);
				dummyPainter.drawText(tempImage.rect(), Qt::AlignCenter | Qt::AlignVCenter,
				    QString("\n\n\tTile: %1 is missing...").arg(tile.suffix()));
				dummyPainter.drawRect(0, 0, tempImage.width(), tempImage.height());
				dummyPainter.end();

			}

			painter.drawPixmap(x, y, TILE_SIZE, TILE_SIZE, tempImage);

			if ( mapSettings().showTileset ) {
				painter.setPen(Qt::black);
				painter.drawRect(x, y, TILE_SIZE, TILE_SIZE);
				painter.drawText(x, y, TILE_SIZE, TILE_SIZE, Qt::AlignHCenter | Qt::AlignVCenter,
				    QString("Tile z%1 c%2 r%3").arg(tile.z()).arg(tile.x()).arg(tile.y()));
			}
		}
	}
	painter.restore();

	//! Make the whole thing a little bit darker
	if ( isDirty() ) {

		painter.save();
		painter.fillRect(viewport, QColor(0, 0, 0, 150));
		painter.restore();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Map
} // namespace Gui
} // namespace IPGP
