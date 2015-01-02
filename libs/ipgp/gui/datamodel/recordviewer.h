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

#ifndef __IPGP_GUI_DATAMODEL_RECORDVIEWER_H__
#define __IPGP_GUI_DATAMODEL_RECORDVIEWER_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/misc/misc.h>
#include <ipgp/gui/datamodel/recordentity.h>
#include <ipgp/gui/3rd-party/qcustomplot/qcustomplot.h>

#include <QWidget>
#include <QObject>
#include <QStack>
#include <QQueue>
#include <QScrollArea>
#include <QHBoxLayout>

#include <vector>

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/timewindow.h>


namespace Seiscomp {
class RecordSequence;
}

QT_FORWARD_DECLARE_CLASS(QGridLayout);

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(RecordViewer);
DEFINE_IPGP_SMARTPOINTER(TimeLine);

/**
 * @class   TimeLine
 * @package IPGP::Gui::DataModel
 * @brief   This class provides an interactive time line.
 *
 * This object is an QCustomPlot stripped of its base QCPGraph, and with
 * default margins set to 0. This allows the use of the xAxis as a ruler.
 */
class SC_IPGP_GUI_API TimeLine : public QCustomPlot {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		enum LastAction {
			laMouseWheel,
			laMouseRelease,
			laMouseMove,
			laKeyPressed,
			None
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit TimeLine(QWidget* parent = NULL, Qt::WFlags = 0);
		~TimeLine() {}

		friend class RecordViewer;

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void wheelEvent(QWheelEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void decreaseTimespan(const qreal& = 1.);
		void increaseTimespan(const qreal& = 1.);
		void setTimeWindow(const Seiscomp::Core::Time& start,
		                   const Seiscomp::Core::Time& end);
		void setTimeWindow(const Seiscomp::Core::TimeWindow&);

		const Seiscomp::Core::TimeWindow& timeWindow() const {
			return _tw;
		}

		QCPAxis* ruler() {
			return _timeScale;
		}

		const LastAction& lastAction() const {
			return _la;
		}

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void newTimeWindow(const QCPRange&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Seiscomp::Core::TimeWindow _tw;
		QCPAxis* _timeScale;
		LastAction _la;
		double _timeMargin;
};


/**
 * @class   ScrollArea
 * @package IPGP::Gui::DataModel
 * @brief   This class provides a scroll-able area widget which emits a signal
 *          whenever it gets resized.
 */
class SC_IPGP_GUI_API ScrollArea : public QScrollArea {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ScrollArea(QWidget* parent = NULL) :
				QScrollArea(parent) {}
		~ScrollArea() {}

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void resizeEvent(QResizeEvent* event) {
			emit resizeRequested(size());
			QScrollArea::resizeEvent(event);
		}

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void resizeRequested(const QSize&);
};

/**
 * @class   Container
 * @package IPGP::Gui::DataModel
 * @brief   This class provides a widget which emits a signal whenever it gets
 *          resized.
 */
class SC_IPGP_GUI_API Container : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Container(QWidget* parent = NULL, Qt::WFlags = 0) :
				QWidget(parent) {}
		~Container() {}

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void resizeEvent(QResizeEvent* event) {
			emit resizeRequested(size());
			QWidget::resizeEvent(event);
		}

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void resizeRequested(const QSize&);
};


/**
 * @class   RecordViewer
 * @package IPGP::Gui::DataModel
 * @brief   Viewer base class!
 *
 * This class provides a main viewer widget for displaying streams. The user has
 * two way for doing so:
 *  + pile streams up without scroll bar: the default record height is 50pixels,
 *  + pile streams up with scroll bar: it would be possible to store a great
 *    number of record entities inside a small window ;).
 *
 * Two stream normalization methods are possible:
 *  + onMouseRelease: the streams are normalized from peak min to peak max
 *    when the timeline gets released,
 *  + onTheFly: the streams are normalized whenever an event occurs (wheel
 *    event, timelineChanged, etc).
 *    The user must take in account the capabilities of its system before
 *    selecting this policy because it is quite heavy resources demanding.
 */
class SC_IPGP_GUI_API RecordViewer : public QWidget {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	Q_ENUMS(NormalizeAmplitudePolicy)
	Q_ENUMS(WidgetMode)

	Q_PROPERTY(NormalizeAmplitudePolicy _nap READ normalizeAmplitudePolicy WRITE setNormalizeAmplitudePolicy)
	Q_PROPERTY(WidgetMode _wm READ widgetMode)
	Q_PROPERTY(Settings _settings READ settings WRITE setSettings)

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum NormalizeAmplitudePolicy {
			onMouseRelease = 0x1, //! Normalize streams after mouse release
			onTheFly = 0x2        //! Normalize streams while the mouse moves
		};

		enum WidgetMode {
			Scroll,  //! Streams are scrollable thru ScrollArea
			NoScroll //! Streams are not scrollable at all
		};

		enum StreamsSelectionPolicy {
			StreamsSelectable,
			StreamsNotSelectable
		};

		typedef QPair<QString, RecordEntity> NamedRecordEntity;
		typedef QQueue<NamedRecordEntity> RecordEntityMap;

		struct Settings {
				Settings();
				QColor streamTraceColor;
				QColor streamBackgroundColor;
		};

		enum StreamMovement {
			Left = 0,
			Right = 1,
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit RecordViewer(QWidget* parent = NULL, Qt::WFlags = 0,
		                      const NormalizeAmplitudePolicy& nap = onTheFly,
		                      const WidgetMode& mode = NoScroll,
		                      const Settings& set = Settings());
		virtual ~RecordViewer();

	protected:
		void resizeEvent(QResizeEvent*);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setupLayout();
		void setupRecordEntities() throw (RecordEntityException);
		const RecordEntityMap& recordEntities() const {
			return _recordEntities;
		}
		RecordEntity& recordEntity(const QString& name) throw (RecordEntityException);

		TimeLine* timeLine() const {
			return _timeLine;
		}

		void setNormalizeAmplitudePolicy(const NormalizeAmplitudePolicy& nap) {
			_nap = nap;
		}
		void setStreamsSelectionPolicy(const StreamsSelectionPolicy& ssp) {
			_ssp = ssp;
		}
		const NormalizeAmplitudePolicy& normalizeAmplitudePolicy() const {
			return _nap;
		}
		const WidgetMode& widgetMode() const {
			return _wm;
		}
		const StreamsSelectionPolicy& streamsSelectionPolicy() const {
			return _ssp;
		}

		void setSettings(const Settings& set) {
			_settings = set;
		}
		const Settings& settings() const {
			return _settings;
		}

		const float minAmplitude(const QString&) throw (RecordEntityException);
		const float maxAmplitude(const QString&) throw (RecordEntityException);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void updatePlots();
		void addRecordEntity(const QString& name) throw (RecordEntityException);
		bool removeRecordEntity(const QString& name) throw (RecordEntityException);
		const int removeAllRecordEntities();

		void decreaseVisibleTimespan(const qreal& = 1.);
		void increaseVisibleTimespan(const qreal& = 1.);

		void moveVisibleRegion(const qreal& = 1., const StreamMovement& = Left);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		/**
		 * @brief Updates the streams. Gaps and min/max peak values are updated
		 *        too from the visible region. Streams are rescaled to fit the
		 *        component box only if normalization policy is onTheFly or if
		 *        the action performed on thru the timeline is a wheelEvent.
		 * @param range The new QCPRange value
		 */
		void timeLineRangeChanged(const QCPRange&);

		/**
		 * @brief Rescales streams amplitudes based on current visible region.
		 * @param event The mouse event
		 */
		void timeLineReleased(QMouseEvent*);

		void streamClicked(QMouseEvent*);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void contextMenuRequest(QPoint);
		void requestPickerUpdate();
		void idling();
		void working();
		void loadingPercentage(const int&, const QString&, const QString&);

	protected:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Container* _sc;
		ScrollArea* _scrollArea;
		QGridLayout* _streamLayout;
		TimeLine* _timeLine;
		Settings _settings;
		NormalizeAmplitudePolicy _nap;
		WidgetMode _wm;
		StreamsSelectionPolicy _ssp;
		RecordEntityMap _recordEntities;
		QWidget* _hSpacer;
};

} // namespace Gui
} // namespace IPGP

#endif
