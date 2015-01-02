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

#ifndef __IPGP_GUI_DATAMODEL_LOGWIDGET_H__
#define __IPGP_GUI_DATAMODEL_LOGWIDGET_H__

#include <QWidget>
#include <QDialog>
#include <ipgp/gui/client/misc.h>
#include <ipgp/gui/api.h>


namespace Ui {
class LogDialog;
}

namespace IPGP {
namespace Gui {

/**
 * @class   LogDialog
 * @package IPGP::Gui::DataModel
 * @brief   Logger window
 *
 * This class provides a standardized application dialog in which information
 * can be logged in three levels:
 *  + LM_OK: the message announces an OKAY operation,
 *  + LM_WARNING: the message raises a concern,
 *  + LM_ERROR: the message shouts an error,
 *  + LM_INFO: the message signal some information.
 */
class SC_IPGP_GUI_API LogDialog : public QDialog {

	Q_OBJECT

	Q_CLASSINFO( "Author", "IPGP" )
	Q_CLASSINFO( "Version", "1.0.0" )
	Q_CLASSINFO( "URL", "www.ipgp.fr" )

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit LogDialog(QWidget* parent = NULL,
		                   const QString& name = QString("Unknown"),
		                   Qt::WFlags f = 0);
		~LogDialog();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Adds a new message to the log.
		 * @param lm the message's type @see LogMessage enum
		 * @param module the name of the module sending the message
		 * @param message the message itself
		 */
		void addMessage(const Client::LogMessage&, const QString& module,
		                const QString& message);
		void setAppName(const QString&);
		const QString& appName() const {
			return _appName;
		}

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void clearMessages();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::LogDialog* _ui;
		QString _appName;
};

} // namespace Gui
} // namespace IPGP

#endif
