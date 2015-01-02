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

#ifndef __IPGP_GUI_MAP_LAYER_H__
#define __IPGP_GUI_MAP_LAYER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/map/config.h>
#include <QObject>
#include <QString>
#include <QRect>
#include <QPainter>
#include <QPointF>


QT_FORWARD_DECLARE_CLASS(QEvent);
QT_FORWARD_DECLARE_CLASS(QMouseEvent);

namespace IPGP {
namespace Gui {
namespace Map {


DEFINE_IPGP_SMARTPOINTER(Layer);

/**
 * @class   Layer
 * @package IPGP::Gui::Map
 * @brief   Layer macro class.
 *
 * This class provides an elementary layer interface. Custom layers should
 * expend this class to inherit global types, methods and signals.
 */
class SC_IPGP_GUI_API Layer : public QObject {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Type {
			Layer_Geometry,
			Layer_Drawable,
			Layer_Tile,
			Layer_Legend,
			Layer_Plate,
			Layer_None
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Layer(const Layer::Type& type = Layer_None,
		               const QString& name = QString(),
		               const QString& desc = QString(),
		               const bool& visible = true,
		               const bool& antialiasing = false);
		virtual ~Layer();

	public:
		// ------------------------------------------------------------------
		//  Operators
		// ------------------------------------------------------------------
		bool operator==(const Layer&) const;
		bool operator!=(const Layer&) const;
		bool operator!() const;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		virtual void draw(QPainter&, const QPointF&, const QRect&, const int&);

		const Layer::Type& type() const;

		void setName(const QString&);
		const QString& name() const;

		void setDescription(const QString&);
		const QString& description() const;

		void setVisible(const bool&);
		const bool& isVisible() const;

		void setAntialiasingEnabled(const bool&);
		const bool& isAntialiasingEnabled() const;

		void setMapSettings(const MapSettings&);
		const MapSettings& mapSettings() const;

		void setDirty(const bool&);
		const bool& isDirty() const;

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void mouseAtPosition(QEvent*);
		void mouseClicked(QMouseEvent*);
		void elementClicked(const QString&);
		void highlightElement(const QString&);
		void restoreElementsSizes();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString _name;
		QString _description;
		bool _visible;
		bool _antialiasing;
		bool _dirty;
		Layer::Type _type;
		mutable MapSettings _mapSettings;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
