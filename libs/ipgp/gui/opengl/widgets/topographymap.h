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

#ifndef __IPGP_OPENGL_WIDGETS_TOPOGRAPHYMAP_H__
#define __IPGP_OPENGL_WIDGETS_TOPOGRAPHYMAP_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QMainWindow>
#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QAction>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <ipgp/core/datamodel/types.h>


namespace Ui {
class TopographyMap;
class TopographyMapDialog;
}

QT_FORWARD_DECLARE_CLASS(QDialog);


namespace IPGP {
namespace Gui {
namespace OpenGL {

class SC_IPGP_GUI_API CustomAction : public QAction {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		CustomAction(QObject* parent = NULL);
		explicit CustomAction(const QIcon& checked, const QIcon& unchecked,
		                      QObject* parent = NULL);
		~CustomAction();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setChecked(const bool&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void updateIcon(const bool&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QIcon _checked;
		QIcon _unchecked;
};

DEFINE_IPGP_SMARTPOINTER(Triangle);
DEFINE_IPGP_SMARTPOINTER(Hypocenter);
DEFINE_IPGP_SMARTPOINTER(Station);
DEFINE_IPGP_SMARTPOINTER(Arrival);
DEFINE_IPGP_SMARTPOINTER(CrossSection);
DEFINE_IPGP_SMARTPOINTER(Triangle);
DEFINE_IPGP_SMARTPOINTER(TopographyRenderer);

/**
 * @class   TopographyMap
 * @package IPGP::Gui
 * @brief   Topography rendering user interface.
 *
 * This object provides a simple interface in which topography data can be
 * drawn/plotted. The user can also choose to incorporate Seiscomp's objects
 * like origins, stations, etc.
 */
class SC_IPGP_GUI_API TopographyMap : public QMainWindow {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit TopographyMap(QWidget* = NULL, Qt::WFlags = 0);
		~TopographyMap();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void init();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		TopographyRenderer* renderer() {
			return _renderer;
		}
		void updateInterface();

		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);
		void setOrigin(Seiscomp::DataModel::Origin*);
		void setOrigins(Core::OriginList*);

		void setCrossSection(CrossSection*);

		void setTopographies(const QStringList& l) {
			_topographies = l;
		}
		const QStringList& topographies() const {
			return _topographies;
		}
		const bool& inventoryVisible() const {
			return _inventoryVisible;
		}

	private:
		bool latIsOutOfResolution(const double&) const;
		bool lonIsOutOfResolution(const double&) const;
		bool depthIsOutOfResolution(const double&) const;

		void setNewLatResolution(const double&);
		void setNewLonResolution(const double&);
		void setNewDepthResolution(const double&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setInventoryVisible(const bool&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void renderingTypeChanged(const bool&);

		void rendererSettingsChanged();
		void changeRendererSettings(const QString&);

		void showConfigDialog();
		void configDialogAccepted();
		void updateConfigDialog();

		void showStatusMessage(const QString&, const int& timeout = 0);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void loadingPercentage(const int&, const QString&, const QString&);
		void nullifyQObject(QObject*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		TopographyRenderer* _renderer;
		QDialog* _configDialog;
		Ui::TopographyMap* _ui;
		Ui::TopographyMapDialog* _dialogUi;

		Seiscomp::DataModel::DatabaseQueryPtr _query;
		Seiscomp::DataModel::OriginPtr _origin;

		//! General actions
		QAction* _aAxis;
		QAction* _aInfo;
		QAction* _aGrid;
		QAction* _aBbox;
		CustomAction* _aLight;

		//! Rendering type actions
		QAction* _aPointCloud;
		QAction* _aMesh;
		QAction* _aFilledMesh;
		QAction* _aFilled;
		QAction* _aSmooth;
		QAction* _aTexture;

		QStringList _topographies;

		bool _paintOrphanOrigin;
		bool _inventoryVisible;
};

} // namespace OpenGL
} // namespace Gui
} // namespace IPGP

#endif
