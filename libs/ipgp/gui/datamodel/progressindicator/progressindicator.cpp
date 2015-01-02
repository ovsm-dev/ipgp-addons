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




#include <ipgp/gui/datamodel/progressindicator/progressindicator.h>
#include <ipgp/gui/datamodel/progressindicator/ui_progressindicator.h>
#include <QtGui>




namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProgressIndicator::ProgressIndicator(QWidget* parent, Qt::WFlags f) :
		QDialog(parent, f), _ui(new Ui::ProgressIndicator) {

	QPixmap logo(":images/smlogo.png");
	logo = logo.scaled(100, 100, Qt::KeepAspectRatio, Qt::FastTransformation);

	_ui->setupUi(this);
	_ui->labelLogo->setPixmap(logo);
	_ui->progressBar->setValue(-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProgressIndicator::~ProgressIndicator() {
	delete _ui;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProgressIndicator::setValue(const int& value, const QString& entity,
                                 const QString& message) {

	QString oldmsg = _ui->labelMessage->text();
	setWindowTitle(entity);
	_ui->labelMessage->setText(message);
	_ui->progressBar->setValue(value);
	if ( oldmsg != message )
		qApp->processEvents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
