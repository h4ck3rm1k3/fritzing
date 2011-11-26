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

$Revision: 5380 $:
$Author: cohen@irascible.com $:
$Date: 2011-08-10 00:15:35 +0200 (Wed, 10 Aug 2011) $

********************************************************************/


#ifndef PINLABELDIALOG_H
#define PINLABELDIALOG_H

#include <QDialog>
#include <QList>
#include <QFrame>
#include <QStringList>
#include <QGridLayout>

class PinLabelDialog : public QDialog
{
	Q_OBJECT

public:
	PinLabelDialog(const QStringList & labels, bool singleRow, const QString & chipLabel, QWidget *parent = 0);
	~PinLabelDialog();

	const QStringList & labels();

protected slots:
	void labelChanged();

protected:
	QFrame * initLabels(const QStringList & labels, bool singleRow, const QString & chipLabel);
	void makeOnePinEntry(int index, const QString & label, Qt::Alignment alignment, int row, QGridLayout *);


protected:
	QStringList m_labels;
};

#endif 
