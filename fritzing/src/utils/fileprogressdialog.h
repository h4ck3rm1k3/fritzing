/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 2597 $:
$Author: cohen@irascible.com $:
$Date: 2009-03-10 12:44:55 +0100 (Tue, 10 Mar 2009) $

********************************************************************/


#ifndef FILEPROGRESSDIALOG_H
#define FILEPROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>

class FileProgressDialog : public QDialog
{
Q_OBJECT

public:
	FileProgressDialog(QWidget *parent = 0);
	FileProgressDialog(const QString & title, QWidget *parent = 0);
	~FileProgressDialog();

protected:
	void closeEvent(QCloseEvent *);

public slots:
	void setMinimum(int);
	void setMaximum(int);
	void setValue(int);
	void setMessage(const QString & message);
	void sendCancel();

signals:
	void cancel();

protected:
	void init(const QString & title);

protected:
	QProgressBar * m_progressBar;	
	QLabel * m_message;
};


#endif 
