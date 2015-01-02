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




#ifndef __IPGP_GUI_DATAMODEL_PROGRESSINDICATOR_H__
#define __IPGP_GUI_DATAMODEL_PROGRESSINDICATOR_H__


#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <QDialog>
#include <QString>

namespace Ui {
class ProgressIndicator;
}

namespace IPGP {
namespace Gui {

class SC_IPGP_GUI_API ProgressIndicator : public QDialog {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ProgressIndicator(QWidget* parent = NULL, Qt::WFlags f = 0);
		~ProgressIndicator();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setValue(const int&, const QString& = "Application",
		              const QString& = "Loading in progress");

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::ProgressIndicator* _ui;
};

} // namespace Gui
} // namespace IPGP

#endif
