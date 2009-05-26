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
#include <QUuid>
#include <QCryptographicHash>
#include <QIcon>
#include <QAction>
#include <QAbstractButton>

#include "fritzingwindow.h"
#include "debugdialog.h"
#include "utils/misc.h"
#include "fapplication.h"

#include "lib/quazip/quazip.h"
#include "lib/quazip/quazipfile.h"
#include "utils/fileprogressdialog.h"

QString FritzingWindow::ReadOnlyPlaceholder(" [READ-ONLY] ");
const QString FritzingWindow::CoreBinLocation = ":/resources/bins/bin" + FritzingBinExtension;
static QString ___fritzingTitle___;

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
	if (isEmptyFileName(m_fileName,untitledFileName())) {
		return saveAs();
	} else if (m_readOnly) {
		return saveAs();
	} else {
		saveAsAux(m_fileName);
		return true;
	}
}

bool FritzingWindow::isEmptyFileName(const QString &fileName, const QString &untitledFileName) {
	return (fileName.isEmpty() || fileName.isNull() || fileName.startsWith(untitledFileName));
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
    QString fileName = FApplication::getSaveFileName(
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
		//fileExt = getExtFromFileDialog(fileExt);
		fileName += fileExtension();
	}

    saveAsAux(fileName);
    return true;
}

//void FritzingWindow::saveAsAux(const QString & fileName) {
	//FApplication::setOpenSaveFolder(fileName);
//}

void FritzingWindow::undoStackCleanChanged(bool isClean) {
	setWindowModified(!isClean);
}

void FritzingWindow::replicateDir(QDir srcDir, QDir targDir) {
	// copy all files from srcDir source to tagDir
	QStringList files = srcDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for(int i=0; i < files.size(); i++) {
		QFile tempFile(srcDir.path() + "/" +files.at(i));
		DebugDialog::debug(tr("Copying file %1").arg(tempFile.fileName()));
		QFileInfo fi(files.at(i));
		QString newFilePath = targDir.path() + "/" + fi.fileName();
		if(QFileInfo(tempFile.fileName()).isDir()) {
			QDir newTargDir = QDir(newFilePath);
			newTargDir.mkpath(newTargDir.absolutePath());
			newTargDir.cd(files.at(i));
			replicateDir(QDir(tempFile.fileName()),newTargDir);
		} else {
			if(!tempFile.copy(newFilePath)) {
				DebugDialog::debug(tr("File %1 already exists: it won't be overwritten").arg(newFilePath));
			}
		}
	}
}

bool FritzingWindow::alreadyHasExtension(const QString &fileName, const QString &fileExt) {
	// TODO: Make something preattier to manage all the supported formats at once
	if(fileExt != ___emptyString___) {
		return fileName.endsWith(fileExt);
	} else {
		foreach (QString extension, fritzingExtensions()) {
			if (fileName.endsWith(extension)) return true;
		}

		return fileName.endsWith(".pdf")
			|| fileName.endsWith(".ps")
			|| fileName.endsWith(".png")
			|| fileName.endsWith(".jpg");
	}
}

QString FritzingWindow::getRandText() {
	QString rand = QUuid::createUuid().toString();
	QString randext = QCryptographicHash::hash(rand.toAscii(),QCryptographicHash::Md4).toHex();
	return randext;
}

/*QString FritzingWindow::getBase64RandText() {
	QString rand = QUuid::createUuid().toString();
	QString randext = QCryptographicHash::hash(rand.toAscii(),QCryptographicHash::Md4).toHex();
	return randext;
}*/

/**
 * Is assumed that the options of the possible extensions are defined this way:
 * <Description of the file type> (*.<extension>)
 */
QString FritzingWindow::getExtFromFileDialog(const QString &extOpt) {
	return extOpt.mid(
			extOpt.indexOf("*")+1,
			extOpt.indexOf(")")-extOpt.indexOf("(")-2);
}

bool FritzingWindow::beforeClosing(bool showCancel) {
	if (this->isWindowModified()) {
     	QMessageBox::StandardButton reply;
     	QMessageBox *messageBox = new QMessageBox(
     			tr("Save \"%1\"").arg(QFileInfo(m_fileName).baseName()),
     			tr("Do you want to save the changes you made in the document \"%1\"?")
					.arg(QFileInfo(m_fileName).baseName()),
     			QMessageBox::Warning,
				showCancel ? QMessageBox::Yes : QMessageBox::Yes | QMessageBox::Default,
     			QMessageBox::No,
     			showCancel ? QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default : QMessageBox::NoButton,
     			this, Qt::Sheet);

		messageBox->setButtonText(QMessageBox::Yes,
			m_fileName.startsWith(untitledFileName()) ? tr("Save...") : tr("Save"));
		messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));
		messageBox->button(QMessageBox::No)->setShortcut(tr("Ctrl+D"));
		messageBox->setInformativeText(tr("Your changes will be lost if you don't save them."));
		if (showCancel) {
			messageBox->setButtonText(QMessageBox::Cancel, tr("Cancel"));
		}
		reply = (QMessageBox::StandardButton)messageBox->exec();

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

bool FritzingWindow::createFolderAnCdIntoIt(QDir &dir, QString newFolder) {
	if(!dir.mkdir(newFolder)) return false;
	if(!dir.cd(newFolder)) return false;

	return true;
}

void FritzingWindow::rmdir(const QString &dirPath) {
	QDir dir = QDir(dirPath);
	rmdir(dir);
}

void FritzingWindow::rmdir(QDir & dir) {
	DebugDialog::debug(QString("removing folder: %1").arg(dir.path()));

	QStringList files = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for(int i=0; i < files.size(); i++) {
		QFile tempFile(dir.path() + "/" +files.at(i));
		DebugDialog::debug(QString("removing from original folder: %1").arg(tempFile.fileName()));
		if(QFileInfo(tempFile.fileName()).isDir()) {
			QDir dir = QDir(tempFile.fileName());
			rmdir(dir);
		} else {
			tempFile.remove(tempFile.fileName());
		}
	}
	dir.rmdir(dir.path());
}

bool FritzingWindow::createZipAndSaveTo(const QDir &dirToCompress, const QString &filepath) {
	DebugDialog::debug("zipping "+dirToCompress.path()+" into "+filepath);

	QString tempZipFile = QDir::temp().path()+"/"+getRandText()+".zip";
	DebugDialog::debug("temp file: "+tempZipFile);
	QuaZip zip(tempZipFile);
	if(!zip.open(QuaZip::mdCreate)) {
		qWarning("zip.open(): %d", zip.getZipError());
		return false;
	}

	QFileInfoList files=dirToCompress.entryInfoList();
	QFile inFile;
	QuaZipFile outFile(&zip);
	char c;

	QString currFolderBU = QDir::currentPath();
	QDir::setCurrent(dirToCompress.path());
	foreach(QFileInfo file, files) {
		if(!file.isFile()||file.fileName()==filepath) continue;

		inFile.setFileName(file.fileName());

		if(!inFile.open(QIODevice::ReadOnly)) {
			qWarning("inFile.open(): %s", inFile.errorString().toLocal8Bit().constData());
			return false;
		}
		if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(inFile.fileName(), inFile.fileName()))) {
			qWarning("outFile.open(): %d", outFile.getZipError());
			return false;
		}

		while(inFile.getChar(&c)&&outFile.putChar(c)){}

		if(outFile.getZipError()!=UNZ_OK) {
			qWarning("outFile.putChar(): %d", outFile.getZipError());
			return false;
		}
		outFile.close();
		if(outFile.getZipError()!=UNZ_OK) {
			qWarning("outFile.close(): %d", outFile.getZipError());
			return false;
		}
		inFile.close();
	}
	zip.close();
	QDir::setCurrent(currFolderBU);

	if(QFileInfo(filepath).exists()) {
		// if we're here the usr has already accepted to overwrite
		QFile::remove(filepath);
	}
	QFile file(tempZipFile);
	file.copy(filepath);
	file.remove();

	if(zip.getZipError()!=0) {
		qWarning("zip.close(): %d", zip.getZipError());
		return false;
	}
	return true;
}


bool FritzingWindow::unzipTo(const QString &filepath, const QString &dirToDecompress) {
	QuaZip zip(filepath);
	if(!zip.open(QuaZip::mdUnzip)) {
		qWarning("zip.open(): %d", zip.getZipError());
		return false;
	}
	zip.setFileNameCodec("IBM866");
	DebugDialog::debug(QString("unzipping %1 entries from %2").arg(zip.getEntriesCount()).arg(filepath));
	QuaZipFileInfo info;
	QuaZipFile file(&zip);
	QFile out;
	QString name;
	char c;
	for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
		if(!zip.getCurrentFileInfo(&info)) {
			qWarning("getCurrentFileInfo(): %d\n", zip.getZipError());
			return false;
		}

		if(!file.open(QIODevice::ReadOnly)) {
			qWarning("file.open(): %d", file.getZipError());
			return false;
		}
		name=file.getActualFileName();
		if(file.getZipError()!=UNZ_OK) {
			qWarning("file.getFileName(): %d", file.getZipError());
			return false;
		}

		out.setFileName(dirToDecompress+"/"+name);
		// this will fail if "name" contains subdirectories, but we don't mind that
		if(!out.open(QIODevice::WriteOnly)) {
			qWarning("out.open(): %s", out.errorString().toLocal8Bit().constData());
			return false;
		}

		// Slow like hell (on GNU/Linux at least), but it is not my fault.
		// Not ZIP/UNZIP package's fault either.
		// The slowest thing here is out.putChar(c).
		// TODO: now that out.putChar has been replaced with a buffered write, is it still slow under Linux?

#define BUFFERSIZE 1024
		char buffer[BUFFERSIZE];
		int ix = 0;
		while(file.getChar(&c)) {
			buffer[ix++] = c;
			if (ix == BUFFERSIZE) {
				out.write(buffer, ix);
				ix = 0;
			}
		}
		if (ix > 0) {
			out.write(buffer, ix);
		}

		out.close();
		if(file.getZipError()!=UNZ_OK) {
			qWarning("file.getFileName(): %d", file.getZipError());
			return false;
		}
		if(!file.atEnd()) {
			qWarning("read all but not EOF");
			return false;
		}
		file.close();
		if(file.getZipError()!=UNZ_OK) {
			qWarning("file.close(): %d", file.getZipError());
			return false;
		}
	}
	zip.close();
	if(zip.getZipError()!=UNZ_OK) {
		qWarning("zip.close(): %d", zip.getZipError());
		return false;
	}
	return true;
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
