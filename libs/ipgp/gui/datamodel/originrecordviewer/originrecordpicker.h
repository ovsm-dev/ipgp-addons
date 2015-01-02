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

#ifndef __IPGP_GUI_WIDGETS_ORIGINRECORDPICKER_H__
#define __IPGP_GUI_WIDGETS_ORIGINRECORDPICKER_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>

#include <QWidget>
#include <QObject>
#include <QMap>
#include <QPointF>
#include <QPoint>
#include <QList>
#include <QPen>
#include <QStringList>
#include <QPixmap>
#include <QString>
#include <exception>

namespace Seiscomp {
namespace DataModel {
class Origin;
class Pick;
class Arrival;
}
}


namespace IPGP {
namespace Gui {

/**
 * @class   PickerException
 * @package IPGP::Gui::Widgets
 * @brief   Picker exception handler
 */
class SC_IPGP_GUI_API PickerException : public std::exception {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit PickerException(const char* msg) :
				_msg(msg) {}
		explicit PickerException(const std::string& msg) :
				_msg(msg) {}
		explicit PickerException(const QString& msg) :
				_msg(msg.toStdString()) {}
		~PickerException() throw () {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const char* what() const throw () {
			return this->_msg.c_str();
		}
		const std::string& whatString() const throw () {
			return this->_msg;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::string _msg;
};


DEFINE_IPGP_SMARTPOINTER(OriginRecordViewer);
DEFINE_IPGP_SMARTPOINTER(OriginRecordPicker);

/**
 * @class   OriginRecordPicker
 * @package IPGP::Gui::Widgets
 * @brief   Interactive pick interface
 *
 * This class provides a widget in which the user can create picks and associate
 * them to streams. Meant to be associated with an OriginRecordViewer widget,
 * this class is used to picture Seiscomp::DataModel::Origin, Picks, Arrivals
 * by using an over-painting method (the user must overlap the viewer with it).
 */
class SC_IPGP_GUI_API OriginRecordPicker : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(RefreshMethod)
	Q_ENUMS(MarkerType)
	Q_ENUMS(MarkerPolarity)

	Q_PROPERTY(Settings _settings READ settings WRITE setSettings)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum RefreshMethod {
			None,
			Markers,
			Tooltips,
			Components,
			RecordInformation,
			Default = Markers | Components | RecordInformation
		};

		enum MarkerType {
			mtPick,
			mtOrigin,
			mtUnknown
		};

		enum MarkerPolarity {
			Positive,
			Negative,
			Unknown
		};

		//! Settings
		struct Settings {

				enum MarkerPosition {
					mpTop,
					mpBottom
				};

				Settings() :
						originColor(Qt::red), pickAutoColor(Qt::darkRed),
						pickManualColor(QColor(0, 160, 0)), pickerPen(QPen()),
						textMargin(4), mpStandard(mpTop), mpAdditional(mpTop) {}

				QColor originColor;
				QColor pickAutoColor;
				QColor pickManualColor;
				QColor pickerTimeColor;
				QColor addPickColor;
				QPen pickerPen;
				int textMargin;

				MarkerPosition mpStandard;
				MarkerPosition mpAdditional;
		};

		//! Default marker
		struct Marker {
			public:
				Marker();
				~Marker() {}
				Marker& operator=(const Marker&);

			public:
				void setTime(const double& t) {
					_time = t;
				}
				void setPen(const QPen& p) {
					_pen = p;
				}
				void setFont(const QFont& f) {
					_font = f;
				}
				void setTooltip(const QString& s) {
					_tooltip = s;
				}
				void setCode(const QString& c) {
					_code = c;
				}
				void setID(const QString& id) {
					_id = id;
				}
				void setType(const MarkerType& t) {
					_type = t;
				}
				void setPolarity(const MarkerPolarity& p) {
					_polarity = p;
				}
				void setScreenStartPoint(const QPointF& p) {
					_ssp = p;
				}
				void setScreenEndPoint(const QPointF& p) {
					_sep = p;
				}

				const double& time() const {
					return _time;
				}
				const QPen& pen() const {
					return _pen;
				}
				const QFont& font() const {
					return _font;
				}
				const QString& tooltip() const {
					return _tooltip;
				}
				const QString& code() const {
					return _code;
				}
				const QString& ID() const {
					return _id;
				}
				const MarkerType& type() const {
					return _type;
				}
				const MarkerPolarity& polarity() const {
					return _polarity;
				}
				const QPointF& screenStartPoint() const {
					return _ssp;
				}
				const QPointF& screenEndPoint() const {
					return _sep;
				}

			private:
				double _time;
				QPen _pen;
				QFont _font;
				QString _tooltip;
				QString _code;
				QString _id;
				MarkerType _type;
				MarkerPolarity _polarity;
				QPointF _ssp;
				QPointF _sep;
		};
		typedef QMap<QString, Marker*> MarkerMap;

		//! Default component
		struct Component {
			public:
				Component() :
						_ampMin(.0), _ampMax(.0), _selected(false), _frequency(false) {}
				~Component() {}

			public:
				void setName(const QString& name) {
					_name = name;
				}
				void setLabel(const QString& s) {
					_label = s;
				}
				void setRect(const QRect& r) {
					_rect = r;
				}
				void setAmplitudeMin(const float& v) {
					_ampMin = v;
				}
				void setAmplitudeMax(const float& v) {
					_ampMax = v;
				}
				void setSelected(const bool& v) {
					_selected = v;
				}
				void setFrequency(const bool& v) {
					_frequency = v;
				}
				const QString& name() const {
					return _name;
				}
				const QString& label() const {
					return _label;
				}
				const QRect& rect() const {
					return _rect;
				}
				const float& amplitudeMin() const {
					return _ampMin;
				}
				const float& amplitudeMax() const {
					return _ampMax;
				}
				const bool& isSelected() const {
					return _selected;
				}
				const bool& isFrequency() const {
					return _frequency;
				}

			private:
				QString _name;
				QString _label;
				QRect _rect;
				float _ampMin;
				float _ampMax;
				bool _selected;
				bool _frequency;
		};
		typedef QList<Component> ComponentList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit OriginRecordPicker(OriginRecordViewer*, QWidget* parent = NULL,
		                            Qt::WFlags = 0);
		~OriginRecordPicker();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void leaveEvent(QEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void paintEvent(QPaintEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		Component& component(const QString&) throw (PickerException);
		void updateComponents();

		void setSettings(const Settings& set) {
			_settings = set;
		}
		const Settings& settings() const {
			return _settings;
		}

		const bool hasPick(const QString&);
		const bool& pickerIsActivated() const {
			return _mouseMarkerVisible;
		}

		QList<Marker> picks();
		void addPicks(const QList<Marker>&);
		void addPick(Seiscomp::DataModel::Pick*, Seiscomp::DataModel::Arrival*);
		void addOrigin(Seiscomp::DataModel::Origin*);
		void addComponent(const QString& name, const QString& label, const bool& isFrequency = false);
		void removeComponent(const QString& name);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		Marker* markerAt(const QPointF&);
		void removeMarkers(const QStringList& keys);

		void drawPicker(QPainter*);
		void drawPicks(QPainter*);
		void drawOrigins(QPainter*);
		void drawTooltips(QPainter*);
		void drawComponents();
		void drawRecordInformation(QPainter*);

		//! Adds the current pick marker at mouse position
		void addPick(const QPointF&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		//! Resets the picker to a semi original setup : picks and origins
		//! are deleted, components stay and are renamed with default names
		//! ( Z, 1 and 2 -> 3 components sensors)
		void reset();

		//! Resets the picker to its native setup : each object is deleted and
		//! components removed too
		void nativeReset();

		void timeLineUpdated();

		void clearPicks();
		void clearOrigins();

		void resizeRequested(const QSize&);

		void activatePickingMode(const QString&);
		void deactivatePickingMode();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		//! Signal emitted whenever a new pick has been created
		void newPick(const qreal&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		MarkerMap _markers;
		ComponentList _components;
		QPixmap _buffer;
		QPixmap _shadowPix;
		OriginRecordViewer* _viewer;
		QString _highLightedObjectID;
		QString _currentMarkerName;

		Settings _settings;

		QRect _currentRegion;
		QRect _lastRegion;
		QPointF _mousePosition;
		bool _mouseMarkerVisible;
		RefreshMethod _refreshMethod;
};

} // namespace Gui
} // namespace IPGP

#endif
