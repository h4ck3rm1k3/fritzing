/*
 * (c) Fachhochschule Potsdam
 */

#include <QtGui>

#include "mainpartseditorwindow.h"
#include "../mainwindow.h"
#include "../debugdialog.h"
#include "../waitpushundostack.h"

const QString MainPartsEditorWindow::UntitledPartName = "Untitled Part";
int MainPartsEditorWindow::UntitledPartIndex = 1;


// TODO mariano: EXTRACT common functionality of both windows classes into an abstract ancestor
void MainPartsEditorWindow::saveAs() {
	DebugDialog::debug(tr("current path: %1").arg(QDir::currentPath()));
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

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);

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
		DebugDialog::debug(tr("copying from %1").arg(path->first+"/"+path->second));
		QFile tempFile(path->first+"/"+path->second);
		DebugDialog::debug(tr("copying to %1").arg(QDir::currentPath()+"/parts/svg/user/"+path->second));
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
