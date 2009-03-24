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

$Revision: 2085 $:
$Author: cohen@irascible.com $:
$Date: 2009-01-06 12:15:02 +0100 (Tue, 06 Jan 2009) $

********************************************************************/

#include "autoclosemessagebox.h"
#include "../debugdialog.h"


AutoCloseMessageBox::AutoCloseMessageBox( QWidget * parent ) 
	: QMessageBox(parent)
{
	m_closeTimer = NULL;
	setAttribute(Qt::WA_DeleteOnClose, true);
}

void AutoCloseMessageBox::autoExec(long ms) {
	setUp(ms);
	exec();
}

void AutoCloseMessageBox::autoShow(long ms) {
	setUp(ms);
	this->setModal(false);
	show();
	//Qt::WindowFlags flags = windowFlags();
	//setWindowFlags(flags);
	QRect r = this->geometry();
	r.moveTo(r.left(), parentWidget()->height() - this->height() + parentWidget()->pos().y());
	this->setGeometry(r);
}

void AutoCloseMessageBox::setUp(long ms) {
	m_closeTimer = new QTimer(this);
	m_closeTimer->setSingleShot(true);
	connect(m_closeTimer, SIGNAL(timeout()), this, SLOT(autoClose()));
	m_closeTimer->start(ms);
}

void AutoCloseMessageBox::autoClose() {
	close();
}


void AutoCloseMessageBox::mousePressEvent(QMouseEvent * event) {
	Q_UNUSED(event);
	close();
}

void AutoCloseMessageBox::closeEvent(QCloseEvent * event) {
	if (m_closeTimer) {
		m_closeTimer->stop();
	}
	QDialog::closeEvent(event);
}

/*
//	example usage:
		AutoCloseMessageBox messageBox(this);
		messageBox.setIcon(QMessageBox::Information);
		messageBox.setWindowTitle(tr("Fritzing"));
		messageBox.setText(tr("Fritzing doesn't yet have a part that matches all the requested properties, so one that matches only some of the properties is being substituted."));
		messageBox.setStandardButtons(QMessageBox::Ok);
		messageBox.autoExec(10 * 1000);  // msec
*/