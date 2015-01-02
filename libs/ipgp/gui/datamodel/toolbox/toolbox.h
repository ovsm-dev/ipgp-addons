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




#ifndef __IPGP_GUI_DATAMODEL_TOOLBOX_H__
#define __IPGP_GUI_DATAMODEL_TOOLBOX_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>
#include <ipgp/gui/datamodel/toolbox/ui_toolbox.h>
#include <QFrame>
#include <QMetaType>
#include <QVariant>
#include <seiscomp3/core/enumeration.h>


using namespace Seiscomp::Core;

namespace IPGP {
namespace Gui {


MAKEENUM(ExportFormat, EVALUES(PDF,PS,PNG,JPG,BMP),
    ENAMES("PDF","PS","PNG","JPG","BMP"));


/**
 * @brief Printing toolbox
 * @package IPGP::Gui::Datamodel
 */
class SC_IPGP_GUI_API ToolBox : public QFrame {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct ExportConfig {
				ExportConfig() :
						printSize(QSize()), format(PNG) {}
				ExportConfig(const QSize& s, const ExportFormat& ef) :
						printSize(s), format(ef) {}
				operator QVariant() const {
					return QVariant::fromValue(*this);
				}
				QSize printSize;
				ExportFormat format;
		};


	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ToolBox(QWidget* parent = NULL, Qt::WFlags = 0);
		~ToolBox();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		Ui::ToolBox* ui() {
			return _ui;
		}

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void setFrameVisible(const bool&);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void print();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void printRequested(const ToolBox::ExportConfig&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Ui::ToolBox* _ui;
		bool _enabled;
};

} // namespace Gui
} // namespace IPGP

Q_DECLARE_METATYPE(IPGP::Gui::ToolBox::ExportConfig);

#endif
