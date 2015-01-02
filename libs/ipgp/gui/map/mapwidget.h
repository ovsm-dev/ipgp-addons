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

#ifndef __IPGP_GUI_MAPWIDGET_H__
#define __IPGP_GUI_MAPWIDGET_H__

#include <QtGui>
#include <ipgp/gui/api.h>
#include <ipgp/gui/client/misc.h>
#include <ipgp/gui/client/scheme.h>
#include <ipgp/gui/map/config.h>
#include <ipgp/gui/map/canvases/backgroundcanvas.h>
#include <ipgp/gui/map/canvases/foregroundcanvas.h>
#include <ipgp/gui/map/decorator.h>
#include <seiscomp3/math/coord.h>
#include <vector>
#include <string>




namespace IPGP {
namespace Gui {

/**
 * @class   DarkRubberBand
 * @package IPGP::Gui::Map
 * @brief   Custom rubber band
 */
class SC_IPGP_GUI_API DarkRubberBand : public QRubberBand {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		DarkRubberBand(Shape shape, QWidget* parent = NULL) :
				QRubberBand(shape, parent) {}

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void paintEvent(QPaintEvent*);
};

/**
 * @class   MapWidget
 * @package IPGP::Gui::Map
 * @brief   QWidget providing a geographic rotating world's map.
 *
 * This custom widget supports basic movements and paintings; painted objects
 * are registered into canvases harboring layers. Default implementation ships
 * with two canvases:
 *   * a "backgroundCanvas" in which tiles and other background stuff should
 *     be drawn,
 *   * a "foregroundCanvas" in which objects and other foreground stuff should
 *     be drawn.
 * Painted objects are interactive: signals are emitted whenever an event
 * occurs (geometry clicked...), tooltips can be displayed as well.
 * The default projection of the map is the mercator projection (no distortion
 * is performed on the displayed tiles).
 */
class SC_IPGP_GUI_API MapWidget : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(MapOptionalWidget)
	Q_ENUMS(MapTooltipMethod)

	Q_PROPERTY(MapTooltipMethod _mtm READ tooltipMethod WRITE setTooltipMethod)
	Q_PROPERTY(int _zoomLevel READ zoomLevel WRITE setZoomLevel)
	Q_PROPERTY(QPointF _centralCoordinates READ centralCoordinates)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum MapOptionalWidget {
			DownLeft,
			DownRight
		};
		typedef QMap<QWidget*, MapOptionalWidget> OptionalWidgetList;

		enum MapTooltipMethod {
			mtm_onQEvent,
			mtm_onQMouseEvent
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		/**
		 * @brief Default constructor!
		 * @param names the map names vector
		 * @param paths the map paths vector
		 * @param parent the QWidget's parent
		 * @param lon the map default longitude
		 * @param lat the map default latitude
		 * @param tilePattern the default tile's pattern
		 * @note  If one wants to use OpenStreetMap tiles in a standard
		 *        way (/tilepath/zoom/column/row.[png/jpg]), it is only
		 *        required to specify the proper pattern %1/%2/%3.[png/jpg].
		 */
		explicit MapWidget(const std::vector<std::string>& names,
		                   const std::vector<std::string>& paths,
		                   QWidget* parent = NULL,
		                   const qreal& lon = .0,
		                   const qreal& lat = .0,
		                   const std::string& = Map::DEFAULT_TILE_PATTERN,
		                   Qt::WFlags = 0);

		virtual ~MapWidget();

	public:
		// ------------------------------------------------------------------
		//  Operators
		// ------------------------------------------------------------------
		MapWidget& operator=(const MapWidget&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const int& zoomLevel() const {
			return _zoomLevel;
		}
		const QPointF& centralCoordinates();

		const bool coord2screen(const qreal&, const qreal&, qreal&, qreal&);
		const bool tile2screen(const qreal&, const qreal&, qreal&, qreal&);
		const bool screen2coord(const qreal&, const qreal&, qreal&, qreal&);
		const bool screen2tile(const qreal&, const qreal&, qreal&, qreal&);

		const QPointF coordToScreen(const QPointF&);
		const QPointF screenToCoord(const QPointF&);

		void setDecorator(Map::Decorator* decorator = NULL);
		void insertDecorator(Map::Decorator*);
		void appendDecorator(Map::Decorator*);

		void setDefaultGeoPosition(const QPointF&);
		void setTileLocation(const qreal& x, const qreal& y);

		//! Canvas in which tiles, plates, other background stuff are painted
		Map::BackgroundCanvas& backgroundCanvas() {
			return _backgroundCanvas;
		}
		const Map::BackgroundCanvas& backgroundCanvas() const {
			return _backgroundCanvas;
		}

		//! Canvas in which geometries, other foreground stuff are painted
		Map::ForegroundCanvas& foregroundCanvas() {
			return _foregroundCanvas;
		}
		const Map::ForegroundCanvas& foregroundCanvas() const {
			return _foregroundCanvas;
		}

		//! Widget's scheme
		Scheme& scheme() {
			return _scheme;
		}
		const Scheme& scheme() const {
			return _scheme;
		}
		void setScheme(const Scheme& scheme) {
			_scheme = scheme;
		}

		/**
		 * @brief Adds optional widget
		 * @param wid QWidget to add
		 * @param mow The corner in which the widget should be added
		 * @param autohide Defines if the widget should be hidden until the
		 *        mouse gets over it.
		 * @note  This implementation is currently piling up objects; lining up
		 *        objects shall be implemented later on...
		 */
		void addOptionalWidget(QWidget*, const MapOptionalWidget& mow = DownLeft,
		                       const bool& autohide = true);


		const MapTooltipMethod& tooltipMethod() const {
			return _mtm;
		}

		/**
		 * @brief Defines the method to use for tracking objects and displaying
		 *        associated tooltips.
		 * @param method The MapTooltipMethod choice.
		 * @note  The default behavior is 'onQMouseEvent' which is very fast,
		 *        but each mouse movement will consume some CPU time and
		 *        resources. 'onQEvent' method is by comparison ultra slow
		 *        due to the genuine lag of the widget's QEvent processor,
		 *        which will echo on the tooltip's display delay. :/
		 *        In the end, the user must go with what's best deal given its
		 *        CPU and application environment.
		 */
		void setTooltipMethod(const MapTooltipMethod& method) {
			_mtm = method;
		}

		/**
		 * @brief Sets the tile pattern to use when fetching tiles stored from
		 *        the hard drive.
		 *        %1 stands for the tile's zoom level, %2 the tile's column
		 *        and %3 the tile's line. e.g. : default OpenStreetMap pattern
		 *        looks like this %1/%2/%3.
		 * @param pattern The tile pattern to use.
		 * @note  The algorithm allow the use of zoom, column and line variables
		 *        more than once which is convenient for custom paths.
		 */
		void setTilePattern(const char*);

		void setPlatesBoundariesFile(const QString&);

		const bool& isRubberBandEnabled() const {
			return _rubberEnabled;
		}

		/**
		 * @brief Makes a clone of this widget.
		 * @note  Children of this object (layers, geometries, etc) are NOT cloned,
		 *        the user is responsible for creating them properly.
		 */
		QObject* clone();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool event(QEvent*);
		void enterEvent(QEvent*);
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		virtual void keyPressEvent(QKeyEvent*);
		void leaveEvent(QEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);
		void log(const Client::LogMessage&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void init();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setZoomLevel(const int&);
		void centerOn(const QPointF&);
		void centerOn(const qreal&, const qreal&);
		void centerOnTile(qreal&, qreal&);

		/**
		 * @brief Opens dialog box and takes a snapshot of the widget as PNG file.
		 */
		void savePicture();

		/**
		 * @brief Takes a snapshot of the widget as PNG file.
		 * @param path The full storage path.
		 */
		void takeSnapshot(const QString&);

		void doubleClickSetPosition(const QPointF&);

		void setPlatesBoundariesEnabled(const bool&);

		void setRubberBandEnabled(const bool& = true);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void showAbout();
		void zoomInRequested();
		void zoomOutRequested();
		void updateMapsPath(QString);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void zoomLevelChanged(int);
		void mouseAtPosition(QEvent*);
		void clickOnCanvas(QMouseEvent*);
		void elementClicked(const QString&);

		void logMessage(const int&, const QString&, const QString&);

		void regionSelected(const QPointF& topLeft, const QPointF& bottomRight);

		//! Signal emitted whenever this object instance is destroyed
		void nullifyQObject(QObject*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		int _zoomLevel;
		int _x, _y;

		qreal _defaultLatitude;
		qreal _defaultLongitude;
		qreal _tileX;
		qreal _tileY;
		qreal _magnification;

		bool _isDragging;
		bool _showScale;
		bool _lockUnlock;
		bool _wantVirtualZoom;

		Scheme _scheme;

		QPointF _centralCoordinates;
		QPointF _mousePosition;

		QMap<QString, QString> _mapList;

		std::vector<std::string> _mapNames;
		std::vector<std::string> _mapPaths;

		std::string _mapsPath;
		std::string _mapsName;
		std::string _tilePattern;

		QString _platesBoundariesFile;

		Map::Decorator _decorator;
		Map::BackgroundCanvas _backgroundCanvas;
		Map::ForegroundCanvas _foregroundCanvas;
		Map::MapSettings _settings;

		QMenu _contextMenu;
		QAction* _gridAction;
		QAction* _scaleAction;
		QAction* _aboutAction;
		QAction* _saveMapAction;
		QAction* _plateBoundariesAction;

		QComboBox* _comboBox;

		QPushButton* _zoomInButton;
		QPushButton* _zoomOutButton;

		bool _rubberEnabled;
		DarkRubberBand* _rubberBand;
		QPoint _rubberStartPos;

		OptionalWidgetList _optWidgets;

		MapTooltipMethod _mtm;
};

} // namespace Gui
} // namespace IPGP

#endif

