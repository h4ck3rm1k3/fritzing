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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include "fapplication.h"
#include "debugdialog.h"
#include "misc.h"

#include <QKeyEvent>
#include <QFileInfo>
#include <QDesktopServices>

bool FApplication::m_spaceBarIsPressed = false;
QString FApplication::m_openSaveFolder = ___emptyString___;

FApplication::FApplication( int & argc, char ** argv) : QApplication(argc, argv)
{
	installEventFilter(this);
}

FApplication::~FApplication(void)
{
}

bool FApplication::spaceBarIsPressed() {
	return m_spaceBarIsPressed;
}

void FApplication::setOpenSaveFolder(const QString& path) {
	QFileInfo fileInfo(path);
	if(fileInfo.isFile()) {
		m_openSaveFolder = path;
	} else {
		m_openSaveFolder = fileInfo.path().remove(fileInfo.fileName());
	}
}

const QString FApplication::openSaveFolder() {
	if(m_openSaveFolder == ___emptyString___) {
		DebugDialog::debug(tr("default save location: %1").arg(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
		return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
	} else {
		return m_openSaveFolder;
	}
}

bool FApplication::eventFilter(QObject *obj, QEvent *event)
{
	// check whether the space bar is down.

	Q_UNUSED(obj);

	switch (event->type()) {
		case QEvent::KeyPress:
			{
				QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
				if (!kevent->isAutoRepeat() && (kevent->key() == Qt::Key_Space)) {
					m_spaceBarIsPressed = true;
					emit spaceBarIsPressedSignal(true);
				}
			}
			break;
		case QEvent::KeyRelease:
			{
				QKeyEvent * kevent = static_cast<QKeyEvent *>(event);
				if (!kevent->isAutoRepeat() && (kevent->key() == Qt::Key_Space)) {
					m_spaceBarIsPressed = false;
					emit spaceBarIsPressedSignal(false);
				}
			}
			break;
		default:
			break;
	}

	return false;
}

