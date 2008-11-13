/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/



#include "debugdialog.h"
#include <QEvent>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QtDebug>
#include <QIcon>

DebugDialog* DebugDialog::singleton = NULL;
QFile DebugDialog::m_file;

QEvent::Type DebugEventType = (QEvent::Type) (QEvent::User + 1);

class DebugEvent : public QEvent
{
public:
	QString m_message;
	QObject * m_ancestor;

	DebugEvent(QString message, QObject * ancestor) : QEvent(DebugEventType) {
		this->m_message = message;
		this->m_ancestor = ancestor;
	}
};

DebugDialog::DebugDialog(QWidget *parent)
	: QDialog(parent)
{
	// Let's set the icon
	this->setWindowIcon(QIcon(QPixmap(":resources/images/fritzing_icon.png")));

	singleton = this;
	setWindowTitle(tr("for debugging"));
	resize(400, 300);
	m_textEdit = new QTextEdit(this);
	m_textEdit->setGeometry(QRect(10, 10, 381, 281));

    QString path = QCoreApplication::applicationDirPath();
    path += "/" + QObject::tr("debug.txt");

	m_file.setFileName(path);
	m_file.remove();
}

DebugDialog::~DebugDialog()
{
}

bool DebugDialog::event(QEvent *e) {
	if (e->type() == DebugEventType) {
		this->m_textEdit->append(((DebugEvent *) e)->m_message);
		emit debugBroadcast(((DebugEvent *) e)->m_message, ((DebugEvent *) e)->m_ancestor);
		// need to delete these events at some point...
		// but it's tricky if the message is being used elsewhere
		return true;
	}
	else {
		return QDialog::event(e);
	}
}

void DebugDialog::resizeEvent(QResizeEvent *e) {
	int w = this->width();
	int h = this->height();
	QRect geom = this->m_textEdit->geometry();
	geom.setWidth(w - geom.left() - geom.left());
	geom.setHeight( h - geom.top() - geom.top());
	this->m_textEdit->setGeometry(geom);
	return QDialog::resizeEvent(e);

}


void DebugDialog::debug(QString message, QObject * ancestor) {
	if (singleton == NULL) {
		new DebugDialog();
		//singleton->show();
	}

	qDebug() << message;

   	if (m_file.open(QIODevice::Append | QIODevice::Text)) {
   		QTextStream out(&m_file);
   		out << message << "\n";
		m_file.close();
	}
	DebugEvent* de = new DebugEvent(message, ancestor);
	QCoreApplication::postEvent(singleton, de);
}

void DebugDialog::hideDebug() {
	if (singleton != NULL) {
		singleton->hide();
	}
}

void DebugDialog::showDebug() {
	if (singleton == NULL) {
		new DebugDialog();
	}

	singleton->show();
}

void DebugDialog::closeDebug() {
	if (singleton != NULL) {
		singleton->close();
	}
}


bool DebugDialog::visible() {
	if (singleton == NULL) return false;

	return singleton->isVisible();
}

bool DebugDialog::connectToBroadcast(QObject * receiver, const char* slot) {
	if (singleton == NULL) {
		new DebugDialog();
	}

	return connect(singleton, SIGNAL(debugBroadcast(const QString &, QObject *)), receiver, slot );
}
