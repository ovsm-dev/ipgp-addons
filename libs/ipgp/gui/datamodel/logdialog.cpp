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

#include <ipgp/gui/datamodel/logdialog.h>
#include <ipgp/gui/datamodel/ui_logdialog.h>
#include <QtGui>

namespace IPGP {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogDialog::LogDialog(QWidget* parent, const QString& name, Qt::WFlags f) :
		QDialog(parent, f), _ui(new Ui::LogDialog) {

	_ui->setupUi(this);
	setWindowTitle(QString("%1 log's").arg(name));

	connect(_ui->pushButton_clear, SIGNAL(clicked()), this, SLOT(clearMessages()));
	connect(_ui->pushButton_okay, SIGNAL(clicked()), this, SLOT(hide()));

	_ui->tableWidget->verticalHeader()->setVisible(false);
	_ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	_ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	_ui->tableWidget->resizeColumnsToContents();
	_ui->tableWidget->resizeRowsToContents();
	_ui->tableWidget->setSelectionMode(QTreeView::ExtendedSelection);
	_ui->tableWidget->setSelectionBehavior(QTreeView::SelectRows);
	_ui->tableWidget->setAlternatingRowColors(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogDialog::~LogDialog() {
	delete _ui;
	_ui = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogDialog::addMessage(const Client::LogMessage& lm,
                           const QString& module, const QString& message) {

	QIcon ico;
	switch ( lm ) {
		case Client::LM_OK:
			ico = QIcon(":images/dialog-okay.png");
		break;
		case Client::LM_ERROR:
			ico = QIcon(":images/dialog-error.png");
		break;
		case Client::LM_INFO:
			ico = QIcon(":images/dialog-information.png");
		break;
		case Client::LM_WARNING:
			ico = QIcon(":images/dialog-warning.png");
		break;
	}

	int idx = _ui->tableWidget->rowCount();
	_ui->tableWidget->insertRow(idx);

	QFont bf(font());
	bf.setBold(true);
	bf.setPointSize(font().pointSize() - 1);

	QFont f(font());
	f.setPointSize(font().pointSize() - 1);

	QTableWidgetItem* type = new QTableWidgetItem(ico, "");
	type->setFont(f);

	QTableWidgetItem* ts = new QTableWidgetItem(QDateTime::currentDateTimeUtc()
	        .toString("yyyy/MM/dd hh:mm:ss"));
	ts->setFont(f);

	QTableWidgetItem* mod = new QTableWidgetItem(module);
	mod->setFont(bf);
//	mod->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QTableWidgetItem* msg = new QTableWidgetItem(message);
	msg->setFont(f);

	_ui->tableWidget->setItem(idx, 0, type);
	_ui->tableWidget->setItem(idx, 1, ts);
	_ui->tableWidget->setItem(idx, 2, mod);
	_ui->tableWidget->setItem(idx, 3, msg);

	_ui->tableWidget->resizeColumnsToContents();
	_ui->tableWidget->resizeRowsToContents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogDialog::setAppName(const QString& name) {
	_appName = name;
	setWindowTitle(QString("%1 log's").arg(name));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogDialog::clearMessages() {
	_ui->tableWidget->clearSelection();
	_ui->tableWidget->setRowCount(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}// namespace Gui
} // namespace IPGP
