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

$Revision$:
$Author$:
$Date$

********************************************************************/



#include <QFileInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QIcon>
#include <QAction>
#include <QAbstractButton>
#include <QSettings>

#include "fritzingwindow.h"
#include "debugdialog.h"
#include "utils/misc.h"
#include "utils/folderutils.h"

#include "utils/fileprogressdialog.h"

const QString FritzingWindow::QtFunkyPlaceholder("[*]");  // this is some weird hack Qt uses in window titles as a placeholder to setr the modified state
QString FritzingWindow::ReadOnlyPlaceholder(" [READ-ONLY] ");
static QString ___fritzingTitle___;
QStringList FritzingWindow::OtherKnownExtensions;

FritzingWindow::FritzingWindow(const QString &untitledFileName, int &untitledFileCount, QString fileExt, QWidget * parent, Qt::WFlags f)
	: QMainWindow(parent, f)
{
	___fritzingTitle___ = QObject::tr("Fritzing");
	m_readOnly = false;

	// Let's set the icon
	this->setWindowIcon(QIcon(QPixmap(":resources/images/fritzing_icon.png")));

	m_fileName = untitledFileName;

	if(untitledFileCount > 1) {
		m_fileName += " " + QString::number(untitledFileCount);
	}
	m_fileName += fileExt;
	untitledFileCount++;

	setTitle();

	m_undoStack = new WaitPushUndoStack(this);
	connect(m_undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoStackCleanChanged(bool)) );
}

void FritzingWindow::createCloseAction() {
	m_closeAct = new QAction(tr("&Close Window"), this);
	m_closeAct->setShortcut(tr("Ctrl+W"));
	m_closeAct->setStatusTip(tr("Close the current sketch"));
	connect(m_closeAct, SIGNAL(triggered()), this, SLOT(close()));
}

void FritzingWindow::setTitle() {
	setWindowTitle(tr("%1 - %2")
		.arg(QFileInfo(m_fileName).fileName()+(m_readOnly?ReadOnlyPlaceholder:"")+QtFunkyPlaceholder)
		.arg(fritzingTitle()));
}

// returns true if the user wanted to save the file
bool FritzingWindow::save() {
	if (FolderUtils::isEmptyFileName(m_fileName,untitledFileName())) {
		return saveAs();
	} else if (m_readOnly) {
		return saveAs();
	} else {
		saveAsAux(m_fileName);
		return true;
	}
}

bool FritzingWindow::saveAs() {
	DebugDialog::debug(QString("current path: %1").arg(QDir::currentPath()));
	QString fileExt;
    QString path;
    QString untitledBase = untitledFileName();

	if(m_readOnly) {
		path = defaultSaveFolder() + "/" + QFileInfo(m_fileName).fileName();
    }
	else if(m_fileName.startsWith(untitledBase, Qt::CaseInsensitive)) {
		path = defaultSaveFolder() + "/" + m_fileName;
	}
	else if(m_fileName.isNull() || m_fileName.isEmpty()) {
		path = defaultSaveFolder();
	}
	else {
		path = m_fileName;
	}
	DebugDialog::debug(QString("current file: %1").arg(m_fileName));
    QString fileName = FolderUtils::getSaveFileName(
						this,
                        tr("Specify a file name"),
                        path,
                        tr("Fritzing (*%1)").arg(fileExtension()),
                        &fileExt
                      );

    if (fileName.isEmpty()) return false; // Cancel pressed

	if (m_readOnly && (fileName.compare(m_fileName, Qt::CaseInsensitive) == 0)) {
        QMessageBox::warning(NULL, QObject::tr("Fritzing"),
                     QObject::tr("The file '%1' is read-only; please use a different filename.")
                     .arg(fileName) );
		return false;

	}

	FileProgressDialog progress("Saving...", 0, this);

    if(!alreadyHasExtension(fileName, fileExtension())) {
		fileName += fileExtension();
	}

	if (saveAsAux(fileName)) {
		QSettings settings;
		settings.setValue("lastOpenSketch",m_fileName);
	}

    return true;
}


void FritzingWindow::undoStackCleanChanged(bool isClean) {
	setWindowModified(!isClean);
}

bool FritzingWindow::alreadyHasExtension(const QString &fileName, const QString &fileExt) {
	// TODO: Make something preattier to manage all the supported formats at once
	if(fileExt != ___emptyString___) {
		return fileName.endsWith(fileExt);
	} else {
		foreach (QString extension, fritzingExtensions()) {
			if (fileName.endsWith(extension)) return true;
		}
		foreach (QString extension, OtherKnownExtensions) {
			if (fileName.endsWith(extension)) return true;
		}
		
		return false;
	}
}

bool FritzingWindow::beforeClosing(bool showCancel) {
	if (this->isWindowModified()) {
     	QMessageBox messageBox(
     			tr("Save \"%1\"").arg(QFileInfo(m_fileName).baseName()),
     			tr("Do you want to save the changes you made in the document \"%1\"?")
					.arg(QFileInfo(m_fileName).baseName()),
     			QMessageBox::Warning,
				showCancel ? QMessageBox::Yes : QMessageBox::Yes | QMessageBox::Default,
     			QMessageBox::No,
     			showCancel ? QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default : QMessageBox::NoButton,
     			this, Qt::Sheet);

		messageBox.setButtonText(QMessageBox::Yes,
			m_fileName.startsWith(untitledFileName()) ? tr("Save...") : tr("Save"));
		messageBox.setButtonText(QMessageBox::No, tr("Don't Save"));
		messageBox.button(QMessageBox::No)->setShortcut(tr("Ctrl+D"));
		messageBox.setInformativeText(tr("Your changes will be lost if you don't save them."));
		if (showCancel) {
			messageBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
		}
		QMessageBox::StandardButton reply = (QMessageBox::StandardButton)messageBox.exec();
     	if (reply == QMessageBox::Yes) {
     		return save();
    	} else if (reply == QMessageBox::No) {
     		return true;
        }
     	else {
         	return false;
        }
	} else {
		return true;
	}
}

void FritzingWindow::setReadOnly(bool readOnly) {
	bool hasChanged = m_readOnly != readOnly;
	m_readOnly = readOnly;
	if(hasChanged) {
		emit readOnlyChanged(readOnly);
	}
}

const QString FritzingWindow::fritzingTitle() {
	return ___fritzingTitle___;
}

const QString & FritzingWindow::fileName() {
	return m_fileName;
}

void FritzingWindow::notClosableForAWhile() {
}
