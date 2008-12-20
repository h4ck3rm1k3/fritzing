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



#include <QtGui>

#include "mainpartseditorwindow.h"
#include "../mainwindow.h"
#include "../debugdialog.h"
#include "../waitpushundostack.h"

const QString MainPartsEditorWindow::UntitledPartName = "Untitled Part";
int MainPartsEditorWindow::UntitledPartIndex = 1;


// TODO mariano: EXTRACT common functionality of both windows classes into an abstract ancestor
void MainPartsEditorWindow::saveAs() {
	DebugDialog::debug(QString("current path: %1").arg(QDir::currentPath()));
	QString fileExt;
    QString fileName = QFileDialog::getSaveFileName(
						this,
                        tr("Choose a file name"),
                        (m_fileName.isNull() || m_fileName.isEmpty()) ? QDir::currentPath()+"/parts/user/" : m_fileName,
                        tr("Fritzing (*%1)").arg(MainWindow::FritzingExtension),
                        &fileExt
                      );

    if (fileName.isEmpty()) return;

    /*if(!MainWindow::alreadyHasExtension(fileName)) {
		fileExt = MainWindow::getExtFromFileDialog(fileExt);
		fileName += fileExt;
	}*/
    saveAsAux(fileName);
}

void MainPartsEditorWindow::saveAsAux(const QString & fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    file.close();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ModelPartStuff * stuff = m_partInfoWidget->modelPartStuff();
    stuff->setConnectorsStuff(m_connWidget->connectorsInfo());
    m_sketchModel->root()->setModelPartStuff(stuff);
	m_sketchModel->save(fileName, true);

	copySvgFilesToDestiny();

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(QString("Saved '%1'").arg(fileName), 2000);

    emit partUpdated(fileName);
   // setCurrentFile(fileName);

   // mark the stack clean so we update the window dirty flag
    m_undoStack->setClean();
}

void MainPartsEditorWindow::copySvgFilesToDestiny() {
	copySvgFilesToDestinyAux(m_breadSvgFile);
	copySvgFilesToDestinyAux(m_schemSvgFile);
	copySvgFilesToDestinyAux(m_iconSvgFile);
}

void MainPartsEditorWindow::copySvgFilesToDestinyAux(StringPair *path) {
	if(!path->first.isEmpty() && !path->second.isEmpty() && path->first == QDir::tempPath()) {
		DebugDialog::debug(QString("copying from %1").arg(path->first+"/"+path->second));
		QFile tempFile(path->first+"/"+path->second);
		DebugDialog::debug(QString("copying to %1").arg(QDir::currentPath()+"/parts/svg/user/"+path->second));
		tempFile.copy(QDir::currentPath()+"/parts/svg/user/"+path->second);
	}
}

void MainPartsEditorWindow::save()
{
	if (m_fileName.isEmpty() || m_fileName.isNull() || m_fileName.startsWith(UntitledPartName) ) {
		saveAs();
	} else {
		saveAsAux(m_fileName);
	}
}
