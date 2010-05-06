/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef ALIGNSETTINGSDIALOG_H
#define ALIGNSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QRadioButton>

class AlignSettingsDialog : public QDialog
{
Q_OBJECT

public:
	AlignSettingsDialog(const QString & viewName, qreal defaultSize, QWidget *parent = 0);
	~AlignSettingsDialog();

protected slots:
	void acceptAnd();
	void units(bool);
	void restoreDefault();

protected:
	QLineEdit * m_lineEdit;
	QDoubleValidator * m_validator;
	QRadioButton * m_mmButton;
	QRadioButton * m_inButton;
	qreal m_defaultSize;
	QString m_viewName;

};


#endif
