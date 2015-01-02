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

#ifndef __IPGP_GUI_MAP_BACKGROUNDCANVAS_H__
#define __IPGP_GUI_MAP_BACKGROUNDCANVAS_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/map/canvas.h>
#include <ipgp/gui/map/layers/tilelayer.h>
#include <ipgp/gui/map/layers/platelayer.h>

namespace IPGP {
namespace Gui {
namespace Map {

DEFINE_IPGP_SMARTPOINTER(BackgroundCanvas);

/**
 * @class   BackgroundCanvas
 * @package IPGP::Gui::Map
 * @brief   Background canvas implementation
 *
 * A BackgroundCanvas is the shelter of objects that have to appear in the
 * background of the widget. Tiles, plates [etc] should be added inside this
 * canvas. By default, this class ships with TileLayer and PlateLayers in
 * which the user may choose to drawn respectively tiles and plates boundaries.
 *
 * @note  This canvas doesn't talk ;)
 */
class SC_IPGP_GUI_API BackgroundCanvas : public Canvas {

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		BackgroundCanvas();
		~BackgroundCanvas();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void updateSettings();

		void draw(QPainter&, const QPointF&, const QRect&, const int&);

		void setMapSettings(const MapSettings&);
		const MapSettings& mapSettings() const;

		PlateLayer& plateLayer() {
			return _plateBoundaryLayer;
		}

		void setDirty(const bool&);
		const bool& isDirty() const {
			return _isDirty;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		TileLayer _tileLayer;
		PlateLayer _plateBoundaryLayer;
		MapSettings _settings;
		bool _isDirty;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
