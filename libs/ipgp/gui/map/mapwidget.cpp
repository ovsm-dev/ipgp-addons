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

#define SEISCOMP_COMPONENT IPGP_MAPWIDGET

#include <ipgp/gui/map/mapwidget.h>
#include <ipgp/gui/map/util.h>
#include <ipgp/gui/map/canvas.h>
#include <ipgp/gui/map/decorators/movedecorator.h>
#include <ipgp/gui/map/decorators/griddecorator.h>
#include <ipgp/gui/map/decorators/scaledecorator.h>
#include <ipgp/gui/map/drawables/plate.h>
#include <ipgp/core/string/string.h>
#include <ipgp/core/geo/geo.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Math::Geo;

using namespace IPGP::Gui::Map;
using namespace IPGP::Core::String;




namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DarkRubberBand::paintEvent(QPaintEvent* paintEvent) {

	QPen pen(Qt::white);
	pen.setWidth(2);
	QPainter painter;
	painter.begin(this);
	painter.setPen(pen);
	painter.drawRect(paintEvent->rect());
	painter.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget::MapWidget(const std::vector<std::string>& names,
                     const std::vector<std::string>& paths, QWidget* parent,
                     const qreal& lon, const qreal& lat,
                     const std::string& tilePattern, Qt::WFlags f) :
		QWidget(parent, f), _defaultLatitude(lat), _defaultLongitude(lon),
		_lockUnlock(false), _rubberEnabled(false) {

	if ( names.size() != paths.size() ) {
		SEISCOMP_ERROR("Map names number doesn't match map filepaths (%d names / %d paths)",
		    (int )names.size(), (int )paths.size());
	}
	else {

		if ( names.size() != 0 && paths.size() != 0 ) {

			_mapList.clear();
			std::string path;
			for (size_t i = 0; i < names.size(); ++i) {
				path = paths.at(i);
				if ( path.size() > 0 ) {
					if ( path.at(path.size() - 1) != '/' )
					    path += '/';
					_mapList.insert(names.at(i).c_str(), path.c_str());
				}
			}
			_mapsPath = paths[0];
			_mapsName = names[0];
		}
		else
			SEISCOMP_INFO("Initializing empty MapWidget");
	}

	_mapNames = names;
	_mapPaths = paths;

	_tilePattern = tilePattern;

	_zoomLevel = 3;
	_magnification = .0;

	_isDragging = false;
	_wantVirtualZoom = true;

	_mtm = mtm_onQMouseEvent;

	_tileX = pow2[_zoomLevel] / 2;
	_tileY = pow2[_zoomLevel] / 2;
	_centralCoordinates = QPointF(_tileX, _tileY);

	setWindowFlags(Qt::Window);
	setMouseTracking(true);
	setBackgroundRole(QPalette::Base);
	setFocusPolicy(Qt::ClickFocus);

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget::~MapWidget() {
	emit nullifyQObject(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setTilePattern(const char* pattern) {

	_tilePattern = pattern;
	_settings.tilePattern = pattern;

	_backgroundCanvas.setMapSettings(_settings);
	_backgroundCanvas.updateSettings();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setPlatesBoundariesFile(const QString& filepath) {
	_platesBoundariesFile = filepath;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::init() {

	_comboBox = new QComboBox(this);

	QMapIterator<QString, QString> it(_mapList);
	QStringList list;
	while ( it.hasNext() ) {
		it.next();
		list << it.key();
	}
	_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	_comboBox->insertItems(0, list);

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(_comboBox->sizePolicy().hasHeightForWidth());
	_comboBox->setSizePolicy(sizePolicy);
	_comboBox->setToolTip("Select map tileset.");
	// Show it first (so it can be placed properly inside the widget's viewport
	// and hide it when it's properly positioned
	_comboBox->show();
	int x = width() - _comboBox->width() - BUTTON_PADDING;
	_comboBox->setGeometry(x, 5, _comboBox->width(), _comboBox->height());
	_comboBox->hide();

	connect(_comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateMapsPath(QString)));

	// Tricking ComboBox to start on the right index regarding order
	// specified by user.
	// TODO: find why ComboBox->setInsertPolicy just won't disable its
	// ordering alphabetically when told so
	if ( _comboBox->currentText().toStdString() != _mapsName )
	    for (int i = 0; i < _comboBox->count(); ++i)
		    if ( _comboBox->itemText(i).toStdString() == _mapsName )
		        _comboBox->setCurrentIndex(i);

	// Zoom-in button icon
	QIcon ziIcon;
	ziIcon.addFile(QString(":images/szin.png"), QSize(28, 28), QIcon::Normal, QIcon::Off);
	_zoomInButton = new QPushButton(this);
	_zoomInButton->setObjectName("zoomInButton");
	_zoomInButton->setIcon(ziIcon);
	_zoomInButton->setToolTip("Zoom in!");
	_zoomInButton->setGeometry(BUTTON_PADDING, BUTTON_PADDING, ZOOM_BUTTON_SIZE, ZOOM_BUTTON_SIZE);
	_zoomInButton->hide();
	connect(_zoomInButton, SIGNAL(clicked()), this, SLOT(zoomInRequested()));

	// Zoom-out button icon
	QIcon zoIcon;
	zoIcon.addFile(QString(":images/szout.png"), QSize(28, 28), QIcon::Normal, QIcon::Off);
	_zoomOutButton = new QPushButton(this);
	_zoomOutButton->setObjectName("zoomOutButton");
	_zoomOutButton->setGeometry(BUTTON_PADDING, ZOOM_BUTTON_SIZE + BUTTON_PADDING, ZOOM_BUTTON_SIZE, ZOOM_BUTTON_SIZE);
	_zoomOutButton->setIcon(zoIcon);
	_zoomOutButton->setToolTip("Zoom out!");
	_zoomOutButton->hide();
	connect(_zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOutRequested()));

	MoveDecorator* moveDecorator = new MoveDecorator(this);
	connect(moveDecorator, SIGNAL(changePositionTo(const QPointF&)), this, SLOT(doubleClickSetPosition(const QPointF&)));
	setDecorator(moveDecorator);

	GridDecorator* gridDecorator = new GridDecorator(this);
	appendDecorator(gridDecorator);

	ScaleDecorator* scaleDecorator = new ScaleDecorator(this, false);
	appendDecorator(scaleDecorator);

	QMenu* decoMenu = _contextMenu.addMenu("Decorators");
	decoMenu->setIcon(QIcon(":images/decorator.png"));

	QMenu* layersMenu = _contextMenu.addMenu("Layers");
	layersMenu->setIcon(QIcon(":images/layers.png"));

	_gridAction = decoMenu->addAction("Grid");
	_gridAction->setCheckable(true);
	_gridAction->setChecked(true);
	connect(_gridAction, SIGNAL(triggered(bool)), gridDecorator, SLOT(setVisible(const bool&)));

	_scaleAction = decoMenu->addAction("Scale");
	_scaleAction->setCheckable(true);
	_scaleAction->setChecked(false);
	connect(_scaleAction, SIGNAL(triggered(bool)), scaleDecorator, SLOT(setVisible(const bool&)));

	_plateBoundariesAction = layersMenu->addAction("Plates");
	_plateBoundariesAction->setCheckable(true);
	_plateBoundariesAction->setChecked(true);
	connect(_plateBoundariesAction, SIGNAL(triggered(bool)), this, SLOT(setPlatesBoundariesEnabled(const bool&)));

	_saveMapAction = _contextMenu.addAction("Save picture");
	connect(_saveMapAction, SIGNAL(triggered()), this, SLOT(savePicture()));

	_aboutAction = _contextMenu.addAction("About");
	_aboutAction->setIcon(QIcon(":images/info.png"));
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));

	_settings.tilePath = _mapsPath.c_str();
	_settings.tilePattern = _tilePattern.c_str();
//	_settings.interactive = true;
//	_settings.tileset = false;
	_settings.defaultBackground = Qt::white;

	_backgroundCanvas.setMapSettings(_settings);
	_backgroundCanvas.updateSettings();

	connect(&_backgroundCanvas, SIGNAL(updateRequested()), this, SLOT(update()));
	connect(this, SIGNAL(mouseAtPosition(QEvent*)), &_backgroundCanvas, SIGNAL(mouseAtPosition(QEvent*)));
	connect(this, SIGNAL(clickOnCanvas(QMouseEvent*)), &_backgroundCanvas, SIGNAL(clickOnCanvas(QMouseEvent*)));
	connect(&_backgroundCanvas, SIGNAL(elementClicked(const QString&)), this, SIGNAL(elementClicked(const QString&)));

	connect(&_foregroundCanvas, SIGNAL(updateRequested()), this, SLOT(update()));
	connect(&_foregroundCanvas, SIGNAL(repaintRequested()), this, SLOT(repaint()));
	connect(this, SIGNAL(mouseAtPosition(QEvent*)), &_foregroundCanvas, SIGNAL(mouseAtPosition(QEvent*)));
	connect(this, SIGNAL(clickOnCanvas(QMouseEvent*)), &_foregroundCanvas, SIGNAL(clickOnCanvas(QMouseEvent*)));
	connect(&_foregroundCanvas, SIGNAL(elementClicked(const QString&)), this, SIGNAL(elementClicked(const QString&)));

	_rubberBand = new DarkRubberBand(QRubberBand::Rectangle, this);
//	QPalette pal;
//	pal.setColor(QPalette::Background, Qt::yellow);
//	pal.setBrush(QPalette::Highlight, Qt::blue);
//	_rubberBand->setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::savePicture() {

	QString outputFile = QFileDialog::getSaveFileName(this, tr("Save map as PNG"),
	    QDir::currentPath(), tr("Portable Network Graphic (*.png)"), 0,
	    QFileDialog::DontUseNativeDialog);

	if ( !outputFile.isEmpty() ) {
		if ( outputFile.right(4) != ".png" || outputFile.right(4) != ".PNG" )
		    outputFile.append(".png");
		takeSnapshot(outputFile);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::takeSnapshot(const QString& filepath) {

	QPixmap pixmap(this->size());
	this->render(&pixmap);
	pixmap.save(filepath);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::doubleClickSetPosition(const QPointF& position) {

	qreal x, y;
	screen2coord(position.x(), position.y(), x, y);
	centerOn(x, y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setPlatesBoundariesEnabled(const bool& enabled) {

	_backgroundCanvas.plateLayer().setVisible(enabled);
	_backgroundCanvas.plateLayer().clear();

	if ( !enabled ) return;

	QList<QPointF> list;
	QFile platePoints(_platesBoundariesFile);

	if ( !platePoints.open(QIODevice::ReadOnly | QIODevice::Text) )
	    return;

	QTextStream in(&platePoints);
	while ( !in.atEnd() ) {

		if ( in.readLine().left(1) == ">" || in.readLine().left(1) == "#" )
		    continue;

		std::string line = in.readLine().toStdString();

		try {
			qreal lon = stringToDouble(stripWhiteSpace(line.substr(0, 14)));
			qreal lat = stringToDouble(stripWhiteSpace(line.substr(14, line.size() - 14)));

			Plate* plate = new Plate;
			plate->addPoint(QPointF(lon, lat));
			plate->pen().setWidth(2);
			plate->pen().setColor(Qt::darkRed);
			plate->setBrush(Qt::darkRed);

			if ( !_backgroundCanvas.plateLayer().addPlate(plate) )
			    delete plate, plate = NULL;

		} catch ( ... ) {}
	}
	platePoints.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setRubberBandEnabled(const bool& enabled) {

	_rubberEnabled = enabled;
	if ( _rubberEnabled )
	    setCursor(Qt::CrossCursor);

	_backgroundCanvas.setDirty(_rubberEnabled);

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::showAbout() {
	QMessageBox::information(this, QString::fromUtf8("About this widget..."),
	    QString::fromUtf8("IPGP MapWidget Copyright (C) 2012"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget& MapWidget::operator=(const MapWidget& other) {

	_defaultLatitude = other._defaultLatitude;
	_defaultLongitude = other._defaultLongitude;
	_zoomLevel = other._zoomLevel;
	_x = other._x;
	_y = other._y;
	_tileX = other._tileX;
	_tileY = other._tileY;
	_isDragging = other._isDragging;
	_showScale = other._showScale;
	_lockUnlock = other._lockUnlock;
	_wantVirtualZoom = other._wantVirtualZoom;
	_centralCoordinates = other._centralCoordinates;
	_mapList = other._mapList;
	_mapsPath = other._mapsPath;
	_mapsName = other._mapsName;
	_tilePattern = other._tilePattern;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QObject* MapWidget::clone() {

	MapWidget* clone = new MapWidget(_mapNames, _mapPaths, this->parentWidget(),
	    _defaultLongitude, _defaultLatitude);
	*clone = *this;

	return clone;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::resizeEvent(QResizeEvent* event) {

	int x = width() - _comboBox->width() - 5;
	_comboBox->setGeometry(x, 5, _comboBox->width(), _comboBox->height());

	// Reposition all additional widgets
	int padding = _settings.defaultOptionalObjectsPadding;
	int lsumsizes = 0, rsumsizes = 0;
	int lcount = _optWidgets.size(), rcount = _optWidgets.size();

	for (OptionalWidgetList::iterator it = _optWidgets.begin();
	        it != _optWidgets.end(); ++it) {

		QWidget* wid = it.key();

		if ( it.value() == DownLeft ) {

			lsumsizes += wid->size().height();
			wid->setGeometry(padding, height() - (lcount * padding) - lsumsizes,
			    wid->size().width(), wid->size().height());

			lcount--;
		}

		if ( it.value() & DownRight ) {

			rsumsizes += wid->size().height();
			wid->setGeometry(width() - padding - wid->size().width(),
			    height() - (rcount * padding) - rsumsizes - padding,
			    wid->size().width(), wid->size().height());

			rcount--;
		}
	}

	centerOn(_defaultLatitude, _defaultLongitude);

	/* Zoom in or out while resizing the widget
	 if ( event->oldSize().width() > event->size().width() )
	 setZoomLevel(zoomLevel() - 1);
	 else
	 setZoomLevel(zoomLevel() + 1);
	 */
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::addOptionalWidget(QWidget* wid,
                                  const MapOptionalWidget& mow,
                                  const bool& autohide) {

	if ( _optWidgets.contains(wid) ) {
		//! Do not destroy the object, the user may be rightfully mistaken...
		SEISCOMP_ERROR("Can't add already added widget onto the map");
		return;
	}

	int padding = _settings.defaultOptionalObjectsPadding;
	wid->setParent(this);

	if ( mow & DownLeft ) {

		if ( _optWidgets.size() == 0 )
			wid->setGeometry(padding, height() - padding - wid->size().height(),
			    wid->size().width(), wid->size().height());
		else {

			OptionalWidgetList::const_iterator it = _optWidgets.begin();
			int sumsizes = 0;
			for (; it != _optWidgets.end(); ++it) {
				sumsizes += it.key()->size().height();
			}

			wid->setGeometry(padding, height() - padding
			        - (_optWidgets.size() * padding) - sumsizes - wid->size().height(),
			    wid->size().width(), wid->size().height());
		}
	}

	if ( mow & DownRight ) {

		if ( _optWidgets.size() == 0 )
			wid->setGeometry(width() - padding - wid->size().width(),
			    height() - padding - wid->size().height(),
			    wid->size().width(), wid->size().height());
		else {

			OptionalWidgetList::const_iterator it = _optWidgets.begin();
			int sumsizes = 0;
			for (; it != _optWidgets.end(); ++it) {
				sumsizes += it.key()->size().height();
			}

			wid->setGeometry(width() - padding - wid->size().width(),
			    height() - (_optWidgets.size() * padding) - sumsizes - padding - wid->size().height(),
			    wid->size().width(), wid->size().height());
		}
	}

	autohide ? wid->hide() : wid->show();

	_optWidgets.insert(wid, mow);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::paintEvent(QPaintEvent* event) {

	Q_UNUSED(event);

	QPainter painter(this);

	_backgroundCanvas.draw(painter, centralCoordinates(), rect(), zoomLevel());
	_decorator.paintEvent(painter);
	_foregroundCanvas.draw(painter, centralCoordinates(), rect(), zoomLevel());

	if ( _lockUnlock )
	    centerOn(_defaultLatitude, _defaultLongitude);

	if ( _rubberEnabled ) {

		static const int margin = 5;

		QFont f(this->font());
		QFontMetrics m(f);

		QString title = "Coordinates";
		QString tr = QString("    Latitude: %1\nLongitude: %2")
		        .arg(Core::Geo::getStringPosition(screenToCoord(_mousePosition).x(), Core::Geo::Longitude).c_str())
		        .arg(Core::Geo::getStringPosition(screenToCoord(_mousePosition).y(), Core::Geo::Latitude).c_str());

		painter.save();

		QTextDocument doc;
		doc.setDefaultStyleSheet("* { color: #000000 }");
		doc.setPlainText(tr);
		doc.setTextWidth(doc.size().width());

		QRect contentRect = QRect(QPoint(margin, height() - doc.size().height() - margin), doc.size().toSize());
		QRect titleRect = QRect(QPoint(margin + 1, contentRect.topLeft().y() - m.height() - margin - 1),
		    QSize(doc.size().width() - 1, m.height() + margin * 2));

		painter.setPen(QColor(255, 255, 255, 200));
		painter.fillRect(titleRect, QColor(0, 0, 0, 200));
		painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, title);
		painter.setBrush(QColor(255, 255, 255, 200));
		painter.setPen(QColor(0, 0, 0, 200));
		painter.drawRect(contentRect);
		painter.translate(contentRect.topLeft());
		doc.drawContents(&painter);

		painter.restore();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MapWidget::event(QEvent* event) {
	if ( event->type() == QEvent::ToolTip && _mtm == mtm_onQEvent )
	    emit mouseAtPosition(event);
	return QWidget::event(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::enterEvent(QEvent* event) {
	QWidget::enterEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& MapWidget::centralCoordinates() {
	_centralCoordinates = QPointF(_tileX, _tileY);
	return _centralCoordinates;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF MapWidget::coordToScreen(const QPointF& point) {

	qreal x, y;

	if ( !coord2tile(point.y(), point.x(), _zoomLevel, x, y) )
	    return QPointF();

	qreal xx, yy;
	if ( !tile2screen(x, y, xx, yy) )
	    return QPointF();

	return QPointF(xx, yy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF MapWidget::screenToCoord(const QPointF& point) {

	qreal x, y;

	if ( !screen2tile(point.x(), point.y(), x, y) )
	    return QPointF();

	qreal xx, yy;
	if ( !tile2coord(x, y, _zoomLevel, yy, xx) )
	    return QPointF();

	return QPointF(xx, yy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool MapWidget::coord2screen(const qreal& lat, const qreal& lon,
                                   qreal& screenX, qreal& screenY) {

	qreal x;
	qreal y;

	if ( !coord2tile(lat, lon, _zoomLevel, x, y) )
	    return false;

	return tile2screen(x, y, screenX, screenY);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setDefaultGeoPosition(const QPointF& position) {
	_defaultLongitude = position.x();
	_defaultLatitude = position.y();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool MapWidget::tile2screen(const qreal& x, const qreal& y,
                                  qreal& screenX, qreal& screenY) {

	if ( !validCoordinatesTile(x, y, _zoomLevel) )
	    return false;

	screenX = (x - _tileX) * TILE_SIZE;
	screenY = (y - _tileY) * TILE_SIZE;

	if ( screenX < 0.0 )
	    screenX += pow2[_zoomLevel] * TILE_SIZE;

	if ( screenY < 0.0 )
	    screenY += pow2[_zoomLevel] * TILE_SIZE;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool MapWidget::screen2coord(const qreal &screenX, const qreal &screenY,
                                   qreal &lat, qreal & lon) {

	qreal x;
	qreal y;

	if ( !screen2tile(screenX, screenY, x, y) ) {
		SEISCOMP_ERROR("%s This should NEVER happen. (%f/%f/%f)", __func__,
		    (qreal ) screenX, (qreal ) screenY, (qreal ) _zoomLevel);
		return false;
	}

	return tile2coord(x, y, _zoomLevel, lat, lon);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool MapWidget::screen2tile(const qreal &screenX, const qreal &screenY,
                                  qreal &x, qreal & y) {

	x = screenX / TILE_SIZE + _tileX;
	y = screenY / TILE_SIZE + _tileY;

	if ( x < 0.0 ) {
		x += pow2[_zoomLevel];
	}
	else if ( x > pow2[_zoomLevel] ) {
		x -= pow2[_zoomLevel];
	}

	if ( y < 0.0 ) {
		y += pow2[_zoomLevel];
	}
	else if ( y > pow2[_zoomLevel] ) {
		y -= pow2[_zoomLevel];
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mousePressEvent(QMouseEvent* event) {

	if ( _rubberEnabled ) {
		if ( !_rubberBand->isVisible() && event->button() == Qt::LeftButton ) {
			_rubberStartPos = event->pos();
			_rubberBand->setGeometry(QRect(_rubberStartPos, QSize()));
			_rubberBand->show();
		}
		return;
	}

	if ( event->button() == Qt::RightButton )
	    _contextMenu.exec(mapToGlobal(event->pos()));

	if ( event->button() == Qt::LeftButton )
	    emit clickOnCanvas(event);

	_decorator.mousePressEvent(event);
	_isDragging = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mouseReleaseEvent(QMouseEvent* event) {

	if ( _rubberBand->isVisible() ) {
		const QRect& zoomRect = _rubberBand->geometry();
		if ( zoomRect.width() != 0 && zoomRect.height() != 0 ) {
			emit regionSelected(screenToCoord(zoomRect.topLeft()), screenToCoord(zoomRect.bottomRight()));
			_rubberBand->hide();
			setRubberBandEnabled(false);
		}
	}

	_decorator.mouseReleaseEvent(event);
//	setCursor(Qt::ArrowCursor);
	_isDragging = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	_decorator.mouseDoubleClickEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mouseMoveEvent(QMouseEvent* event) {

	_mousePosition = event->posF();

	if ( _rubberBand->isVisible() ) {
		_rubberBand->setGeometry(QRect(_rubberStartPos, event->pos()).normalized());
		return;
	}

	// Zoom buttons
	QRect r = QRect(BUTTON_PADDING, BUTTON_PADDING, ZOOM_BUTTON_SIZE, ZOOM_BUTTON_SIZE * 2);
	if ( r.contains(event->pos()) ) {
		_zoomInButton->show();
		_zoomOutButton->show();
	}
	else {
		_zoomInButton->hide();
		_zoomOutButton->hide();
	}

	// Maps tiles combo box
	if ( _mapsName.size() != 0 ) {

		QRect r(_comboBox->pos(), _comboBox->size());
		if ( r.contains(event->pos()) )
			_comboBox->show();
		else
			_comboBox->hide();
	}

	// Additional widgets
	OptionalWidgetList::const_iterator it = _optWidgets.begin();
	for (; it != _optWidgets.end(); ++it) {

		QRect r = QRect(it.key()->pos(), QSize(it.key()->width(), it.key()->height()));
		if ( r.contains(event->pos()) )
			it.key()->show();
		else
			it.key()->hide();
	}

	_decorator.mouseMoveEvent(event);

	if ( _mtm == mtm_onQMouseEvent )
	    _foregroundCanvas.mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setDecorator(Map::Decorator* newDecorator) {
	_decorator.setDecorator(newDecorator);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::appendDecorator(Map::Decorator* newDecorator) {
	_decorator.appendDecorator(newDecorator);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::insertDecorator(Map::Decorator* newDecorator) {
	_decorator.insertDecorator(newDecorator);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::leaveEvent(QEvent* event) {
	_decorator.leaveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::keyPressEvent(QKeyEvent* event) {

	// Add some fancy Key actions here to be checked
	/*
	 if ( event->key() == Qt::Key_G ) {

	 if ( _gridAction->isChecked() )
	 _gridAction->setChecked(false);
	 else
	 _gridAction->setChecked(true);

	 QListIterator<Layer*> it(_layers);
	 while ( it.hasNext() ) {
	 Layer* l = it.next();
	 if ( l->type() == Layer::Layer_Grid )
	 l->setVisible(_gridAction->isChecked());
	 }
	 }

	 if ( event->key() == Qt::Key_S )
	 emit showHideScale();

	 if ( event->key() == Qt::Key_C )
	 emit showHideCross();

	 if ( event->key() == Qt::Key_K )
	 emit showHideCoords();

	 update();
	 */

	//! @note This event should be installed in the parent widget aswell...
	if ( _rubberEnabled )
	    if ( event->key() == Qt::Key_Escape )
	        setRubberBandEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::updateMapsPath(QString str) {

	Q_UNUSED(str);

	QMapIterator<QString, QString> it(_mapList);
	while ( it.hasNext() ) {
		it.next();
		if ( it.key() == _comboBox->currentText() ) {
			_mapsPath = it.value().toStdString();
			_settings.tilePath = it.value();
			_backgroundCanvas.setMapSettings(_settings);
			_backgroundCanvas.updateSettings();
		}
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::wheelEvent(QWheelEvent* event) {


	int zoom = _zoomLevel + event->delta() / 120;

	if ( zoom < MIN_ZOOM ) return;
	if ( zoom > MAX_ZOOM ) return;

	qreal w = width() / (qreal) TILE_SIZE;
	qreal h = height() / (qreal) TILE_SIZE;
	qreal zoomFactor = pow(2, zoom - _zoomLevel);

	_tileX *= zoomFactor;
	_tileX -= 0.5 * w * (1.0 - zoomFactor);
	_tileY *= zoomFactor;
	_tileY -= 0.5 * h * (1.0 - zoomFactor);
	_zoomLevel = zoom;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::zoomInRequested() {
	setZoomLevel(_zoomLevel + 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::zoomOutRequested() {
	setZoomLevel(_zoomLevel - 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setTileLocation(const qreal& x, const qreal& y) {
	_tileX = x;
	_tileY = y;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setZoomLevel(const int& zoomLevel) {

	if ( (zoomLevel > MAX_ZOOM) || (zoomLevel < MIN_ZOOM) || (zoomLevel == _zoomLevel) )
	    return;

	_zoomLevel = zoomLevel;
	centerOn(_defaultLatitude, _defaultLongitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::centerOn(const QPointF& position) {
	centerOn(position.y(), position.x());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::centerOn(const qreal& lat, const qreal& lon) {

	qreal x;
	qreal y;

	if ( !coord2tile(lat, lon, _zoomLevel, x, y) ) {
		SEISCOMP_ERROR("%s failed. Bad geo coordinate (%f/%f)", __func__, lat, lon);
		return;
	}

	centerOnTile(x, y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::centerOnTile(qreal& x, qreal& y) {

	if ( !validCoordinatesTile(x, y, _zoomLevel) ) {
		SEISCOMP_ERROR("%s failed. Bad tile coordinates (%f/%f)", __func__, x, y);
		return;
	}

	x -= (qreal) width() / (TILE_SIZE * 2);
	y -= (qreal) height() / (TILE_SIZE * 2);

	qreal x_offset = (_tileX - x) * TILE_SIZE;
	qreal y_offset = (_tileY - y) * TILE_SIZE;

	_tileX -= x_offset / TILE_SIZE;
	_tileY -= y_offset / TILE_SIZE;

	if ( _tileX > pow2[_zoomLevel] ) {
		_tileX -= pow2[_zoomLevel];
	}
	else if ( _tileX < 0.0 ) {
		_tileX += pow2[_zoomLevel];
	}

	if ( _tileY > pow2[_zoomLevel] ) {
		_tileY -= pow2[_zoomLevel];
	}
	else if ( _tileY < 0.0 ) {
		_tileY += pow2[_zoomLevel];
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::log(const Client::LogMessage& lm, const QString& msg) {
	emit logMessage(static_cast<int>(lm), __func__, msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}// namespace Gui
} // namespace IPGP

