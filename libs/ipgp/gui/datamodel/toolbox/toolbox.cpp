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




#include <ipgp/gui/datamodel/toolbox/toolbox.h>
#include <QDebug>


namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ToolBox::ToolBox(QWidget* parent, Qt::WFlags f) :
		QFrame(parent, f), _ui(new Ui::ToolBox), _enabled(false) {

	_ui->setupUi(this);

	connect(_ui->pushButtonPrint, SIGNAL(clicked()), this, SLOT(print()));
	connect(_ui->toolButtonShowHide, SIGNAL(clicked(bool)), this, SLOT(setFrameVisible(bool)));
	_ui->framePrint->setVisible(_enabled);

//	_ui->pushButtonReplot->setText("");
//	_ui->pushButtonReplot->setIcon(QIcon(QPixmap(":images/plot2.png")));

	qRegisterMetaType<ToolBox::ExportConfig>("ToolBox::ExportConfig");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ToolBox::~ToolBox() {
	delete _ui, _ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ToolBox::setFrameVisible(const bool& visible) {

	(_enabled) ? _enabled = false : _enabled = true;

	_ui->framePrint->setVisible(_enabled);

	(_enabled) ?
	        _ui->toolButtonShowHide->setArrowType(Qt::UpArrow) :
	        _ui->toolButtonShowHide->setArrowType(Qt::DownArrow);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ToolBox::print() {

	ExportFormat f;
	if ( _ui->comboBoxFileType->currentText().contains("BMP") )
		f = BMP;
	else if ( _ui->comboBoxFileType->currentText().contains("JPG") )
		f = JPG;
	else if ( _ui->comboBoxFileType->currentText().contains("PDF") )
		f = PDF;
	else if ( _ui->comboBoxFileType->currentText().contains("PNG") )
		f = PNG;
	else if ( _ui->comboBoxFileType->currentText().contains("PS") )
		f = PS;

	emit printRequested(ExportConfig(QSize(_ui->spinBoxXSize->value(), _ui->spinBoxYSize->value()), f));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP

