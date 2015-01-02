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

#ifndef __IPGP_GUI_DATAMODEL_MAINWINDOW_H__
#define __IPGP_GUI_DATAMODEL_MAINWINDOW_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QMainWindow>
#include <QMessageBox>
#include <QCloseEvent>
#include <QObject>

namespace IPGP {
namespace Gui {

DEFINE_IPGP_SMARTPOINTER(MainWindow);

/**
 * @class   MainWindow
 * @package IPGP::Gui::DataModel
 * @brief   A mainwindow macro
 *
 * This class provides a base QMainWindow interface from which the user can
 * catch a closing signal.
 */
class SC_IPGP_GUI_API MainWindow : public QMainWindow {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MainWindow(QWidget* parent = NULL, Qt::WFlags flags = 0) :
				QMainWindow(parent, flags), _confirmOnClose(false) {}

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void closeEvent(QCloseEvent* event) {

			if ( _confirmOnClose ) {
				if ( QMessageBox::question(this, _appName,
				    tr("Are you sure ?"),
				    QMessageBox::Cancel | QMessageBox::Yes,
				    QMessageBox::Yes) != QMessageBox::Yes )
					event->ignore();
				else {
					emit mainWindowClosing();
					event->accept();
				}
			}
			else {
				emit mainWindowClosing();
				QMainWindow::closeEvent(event);
			}
		}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setConfirmOnClose(const bool& value) {
			_confirmOnClose = value;
		}
		inline const bool& confirmOnClose() const {
			return _confirmOnClose;
		}
		void setAppName(const QString& name) {
			_appName = name;
		}
		inline const QString& appName() const {
			return _appName;
		}

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void mainWindowClosing();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		bool _confirmOnClose;
		QString _appName;
};


} // namespace Gui
} // namespace IPGP

#endif
