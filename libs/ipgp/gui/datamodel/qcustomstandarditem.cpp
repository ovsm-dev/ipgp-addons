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

#include <ipgp/gui/datamodel/qcustomstandarditem.h>
#include <QDebug>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QCustomStandardItem::QCustomStandardItem(const QString& text,
                                         const DataRole& role,
                                         const QString& pattern) :
		QStandardItem(text), _dataRole(role), _pattern(pattern) {
	setData(QVariant(text));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QCustomStandardItem::setDataRole(DataRole role) {
	_dataRole = role;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& QCustomStandardItem::pattern() const {
	return _pattern;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QCustomStandardItem::setPattern(const QString& pattern) {
	_pattern = pattern;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


//"[^\\d+\\.?\\d+]"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QCustomStandardItem::operator<(const QStandardItem& other) const {

	if ( _dataRole == NumberUnitRole ) {

		QString str1 = data(NumberUnitRole).toString();
		QString str2 = other.data(NumberUnitRole).toString();

		str1.remove(QRegExp("[^\\d+\\.?\\d+]"));
		str2.remove(QRegExp("[^\\d+\\.?\\d+]"));

		if ( str1.left(1) == "-" || str1.isEmpty() )
			return false;

		if ( str2.left(1) == "-" || str2.isEmpty() )
			return true;

		return str1.toDouble() < str2.toDouble();
	}

	if ( _dataRole == MixedRole ) {

		QString str1 = data(MixedRole).toString();
		QString str2 = other.data(MixedRole).toString();

		str1.remove(QRegExp(_pattern));
		str2.remove(QRegExp(_pattern));

		if ( str1.left(1) == "-" || str1.isEmpty() )
			return false;

		if ( str2.left(1) == "-" || str2.isEmpty() )
			return true;

		return str1.toDouble() < str2.toDouble();
	}

	return QStandardItem::operator <(other);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


