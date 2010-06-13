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

#ifndef RECOVERYDIALOG_H
#define RECOVERYDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QFileInfoList>

class RecoveryDialog : public QDialog {
        Q_OBJECT
public:
        RecoveryDialog(QFileInfoList fileList, QWidget *parent = 0, Qt::WindowFlags flags = 0);
        QList<QListWidgetItem*> getSelectedFiles();

protected:
		QString getOriginalName(const QString & path);

protected:
        QListWidget *m_recoveryList;
};

#endif // RECOVERYDIALOG_H