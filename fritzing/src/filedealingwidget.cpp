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

#include "filedealingwidget.h"
#include "debugdialog.h"
#include "misc.h"


DirtyFilesManager::DirtyFilesManager(
		QString *fileName,
		const QString &untitledFileName,
		const QString &defaultSaveFolder,
		SetTitleFunc setTitleFunc,
		IsModifiedFunc isModifiedFunc,
		SetModifiedFunc setModifiedFunc,
		SaveAsAuxFunc saveAsAuxFunc,
		int *fileCount,
		QUndoStack *undoStack,
		QWidget *parent,
		bool appendExtensionOnTitle)
	: QObject(parent)
{
	Q_ASSERT(parent!=NULL);
	Q_ASSERT(undoStack!=NULL);

	m_fileName = fileName;
	m_widgetParent = parent;
	m_appendExtensionOnTitle = appendExtensionOnTitle;

	m_setTitleFunc = setTitleFunc;
	m_isModifiedFunc = isModifiedFunc;
	m_setModifiedFunc = setModifiedFunc;
	m_saveAsFunc = saveAsAuxFunc;

	m_fileCount = fileCount;

	m_untitledFileName = untitledFileName;
	m_defaultSaveFolder = defaultSaveFolder;

	*m_fileName = m_untitledFileName;

	int untitledFileCount = *m_fileCount;

	if(untitledFileCount > 1) {
		*m_fileName += " " + QString::number(untitledFileCount);
	}
	*m_fileName += FritzingSketchExtension;
	*m_fileCount++;

	setTitle();

	m_undoStack = undoStack;
	connect(m_undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoStackCleanChanged(bool)) );
}

void DirtyFilesManager::setModified(bool modified) {
	((*m_widgetParent).*m_setModifiedFunc)(modified);
}

bool DirtyFilesManager::isModified() {
	return ((*m_widgetParent).*m_isModifiedFunc)();
}

void DirtyFilesManager::setTitle() {
	QString title;
	if(m_appendExtensionOnTitle) {
		title = tr("%1 - %2")
			.arg(QFileInfo(*m_fileName).fileName() + QtFunkyPlaceholder)
			.arg(tr("Fritzing"));
	} else {
		title = tr("%1").arg(QFileInfo(*m_fileName).fileName() + QtFunkyPlaceholder);
	}
	((*m_widgetParent).*m_setTitleFunc)(title);
}

void DirtyFilesManager::saveAsAux(const QString& filename) {
	(*(dynamic_cast<FileDealingWidget*>(m_widgetParent)).*m_saveAsFunc)(filename);
}

// returns true if the user wanted to save the file
bool DirtyFilesManager::save() {
	if (isEmptyFile()) {
		return saveAs();
	} else {
		saveAsAux(*m_fileName);
		return true;
	}
}

bool DirtyFilesManager::isEmptyFile() {
	return (m_fileName->isEmpty() || m_fileName->isNull() || m_fileName->startsWith(m_untitledFileName));
}

bool DirtyFilesManager::saveAs() {
	DebugDialog::debug(QString("current path: %1").arg(QDir::currentPath()));
	QString fileExt;
    QString fileName = QFileDialog::getSaveFileName(
						m_widgetParent,
                        tr("Specify a file name"),
                        (m_fileName->isNull() || m_fileName->isEmpty()) ? m_defaultSaveFolder : *m_fileName,
                        tr("Fritzing (*%1)").arg(FritzingSketchExtension),
                        &fileExt
                      );

    if (fileName.isEmpty()) return false; // Cancel pressed

    if(!alreadyHasExtension(fileName)) {
		fileExt = getExtFromFileDialog(fileExt);
		fileName += fileExt;
	}
    saveAsAux(fileName);
    return true;
}

void DirtyFilesManager::undoStackCleanChanged(bool isClean) {
	setModified(!isClean);
}

bool DirtyFilesManager::alreadyHasExtension(const QString &fileName) {
	// TODO: Make something preattier to manage all the supported formats at once
	return fileName.indexOf(FritzingSketchExtension)  != -1 || fileName.indexOf(".pdf")  != -1 || fileName.indexOf(".ps")  != -1 || fileName.indexOf(".png")  != -1 || fileName.indexOf(".jpg")  != -1;
}

/**
 * It's assumed that the options of the possible extensions are defined this way:
 * <Description of the file type> (*.<extension>)
 */
QString DirtyFilesManager::getExtFromFileDialog(const QString &extOpt) {
	return extOpt.mid(
			extOpt.indexOf("*")+1,
			extOpt.indexOf(")")-extOpt.indexOf("(")-2);
}

bool DirtyFilesManager::beforeClosing() {
	if (this->isModified()) {
     	QMessageBox::StandardButton reply;
     	reply = QMessageBox::question(
					m_widgetParent,
					tr("Save %1").arg(QFileInfo(*m_fileName).baseName()),
                    tr("Do you want to save the changes you made in the document %1? Your changes will be lost if you don't save them.")
						.arg(QFileInfo(*m_fileName).baseName()),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
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



FileDealingWidget::FileDealingWidget(QWidget *meAsWidget, const QString& untitledFileName, const QString &defaultSaveFolder ) {
	m_meAsWidget = meAsWidget;
	m_untitledFileName = untitledFileName;
	m_defaultSaveFolder = defaultSaveFolder;
}

void FileDealingWidget::setUpDirtyFilesManager(QUndoStack *undoStack, bool appendFileExt) {
	m_undoStack = undoStack;

	m_dirtyFilesManager = new DirtyFilesManager(
		&m_filename, m_untitledFileName, m_defaultSaveFolder,
		&QWidget::setWindowTitle, &QWidget::isWindowModified,
		&QWidget::setWindowModified, &FileDealingWidget::saveAsAux,
		&m_untitledFileCount, m_undoStack, m_meAsWidget, appendFileExt
	);
}
