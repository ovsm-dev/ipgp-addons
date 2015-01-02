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

#ifndef __IPGP_GUI_MAP_LEGENDDECORATOR_H__
#define __IPGP_GUI_MAP_LEGENDDECORATOR_H__

#include <QObject>
#include <ipgp/gui/api.h>
#include <ipgp/gui/map/decorators/mapdecorator.h>
#include <ipgp/gui/map/config.h>

namespace IPGP {
namespace Gui {

class MapWidget;

namespace Map {

class SC_IPGP_GUI_API LegendDecorator : public QObject, public MapDecorator {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit LegendDecorator(MapWidget*, const bool& visible = true,
		                         const bool& antialiasing = false);
		~LegendDecorator();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void paintEvent(QPainter&);
		const bool& isVisible() const;
		const bool& isAntialiased() const;
		const bool& isAutoAdjustEnabled() const;
		const QSize& size() const;
		const ElementPosition& position() const;
		const QPen& pen() const;
		const QString& header() const;
		const QFont& font() const;

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setVisible(const bool&);
		void setAntialiased(const bool&);
		void setAutoAdjust(const bool&);
		void setFont(const QFont&);
		void setHeader(const QString&);
		void setPen(const QPen&);
		void setPosition(const ElementPosition&);
		void setSize(const QSize&);
		void removeElements() {
			_elements.clear();
		}
		void removeElement(const QString& key) {
			_elements.remove(key);
		}
		void addElement(const QString&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		bool _antialiased;
		bool _visible;
		bool _autoAdjust;
		QPen _pen;
		QFont _font;
		QSize _size;
		ElementPosition _position;
		QMap<QString, QString> _elements;
		QString _header;
};

} // namespace Map
} // namespace Gui
} // namespace IPGP

#endif
