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



#include <QFileDialog>
#include <QFrame>
#include <QBuffer>
#include <QSvgGenerator>
#include <QGraphicsProxyWidget>

#include "partseditorspecificationsview.h"
#include "../layerkinpaletteitem.h"
#include "../debugdialog.h"
#include "../waitpushundostack.h"
#include "../fritzingwindow.h"
#include "../connectorstuff.h"

QT_BEGIN_NAMESPACE

PartsEditorSpecificationsView::PartsEditorSpecificationsView(ItemBase::ViewIdentifier viewId, QDir tempDir, QGraphicsItem *startItem, QWidget *parent, int size)
	: PartsEditorAbstractView(viewId, tempDir, parent, size)
{
	m_svgFilePath = new SvgAndPartFilePath;
	m_startItem = startItem;
	if(m_startItem) {
		scene()->addItem(startItem);
		addFixedToCenterItem(startItem);
		ensureFixedToCenterItems();
	}
}

void PartsEditorSpecificationsView::mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *){
	DebugDialog::debug("got connector mouse press.  not yet implemented...");
	return;
}

void PartsEditorSpecificationsView::copySvgFileToDestiny() {
	QString origFile = "";
	QString destFile = "";
	bool doIt = false;

	Qt::CaseSensitivity cs = Qt::CaseSensitive;

#ifdef Q_WS_WIN
	// seems to be necessary for Windows: getApplicationSubFolderPath() returns a string starting with "c:"
	// but the file dialog returns a string beginning with "C:"
	cs = Qt::CaseInsensitive;
#endif

	// if the svg file is in the temp folder, then copy it to destiny
	if(m_svgFilePath->absolutePath().startsWith(QDir::tempPath(),cs)) {
		origFile = svgFilePath();
		destFile = getApplicationSubFolderPath("parts")+"/svg/user/"+m_svgFilePath->relativePath();
		doIt = true;
	}

	// jpeg / png turned into svg (is in temp )
	/*if(m_svgFilePath->second.isEmpty() && m_svgFilePath->first.startsWith(m_tempFolder.path())) {
		origFile = m_svgFilePath->first;
		destFile = getApplicationSubFolderPath("parts")+"/svg/user/"+m_svgFilePath->first.remove(m_tempFolder.path()+"/");
		doIt = true;
	}*/

	if(doIt) {
		QFile tempFile(origFile);
		DebugDialog::debug(QString("copying from %1 to %2")
				.arg(origFile)
				.arg(destFile));
		tempFile.copy(destFile);
	}
}

void PartsEditorSpecificationsView::mousePressEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	loadFile();
}

void PartsEditorSpecificationsView::loadFile() {
	QString origPath = QFileDialog::getOpenFileName(this,
		tr("Open Image"),
		m_originalSvgFilePath.isEmpty() ? getApplicationSubFolderPath("parts")+"/parts/svg/" : m_originalSvgFilePath,
		tr("SVG Files (*.svg);;JPEG (*.jpg);;PNG (*.png)"));

	if(origPath.isEmpty()) {
		return; // Cancel pressed
	} else {
		if(!origPath.endsWith(".svg")) {
			origPath = createSvgFromImage(origPath);
		}
		if(origPath != ___emptyString___) {
			if(m_startItem) {
				m_fixedToCenterItems.removeAll(m_startItem);
				scene()->removeItem(m_startItem);
				//delete m_startItem;
				m_startItem = NULL;
			}
			loadSvgFile(origPath);
		}
	}
}

void PartsEditorSpecificationsView::updateModelPart(const QString& origPath) {
	m_undoStack->push(new QUndoCommand("Dummy parts editor command"));
	setSvgFilePath(origPath);

	ModelPart *mp = createFakeModelPart(origPath, m_svgFilePath->relativePath());
	m_item->setModelPart(mp);
	copyToTempAndRenameIfNecessary(m_svgFilePath);
	m_item->setSvgFilePath(m_svgFilePath);
}

void PartsEditorSpecificationsView::loadSvgFile(const QString& origPath) {
	m_undoStack->push(new QUndoCommand("Dummy parts editor command"));

	setSvgFilePath(origPath);

	ModelPart * mp = createFakeModelPart(origPath, m_svgFilePath->relativePath());
	loadSvgFile(mp);
}

void PartsEditorSpecificationsView::loadSvgFile(ModelPart * modelPart) {
	addItemInPartsEditor(modelPart, m_svgFilePath);
	emit itemAddedToSymbols(0, m_svgFilePath);

	copyToTempAndRenameIfNecessary(m_svgFilePath);

	m_item->setSvgFilePath(m_svgFilePath);
}

void PartsEditorSpecificationsView::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	if(m_startItem) {
		m_fixedToCenterItems.removeAll(m_startItem);
		m_startItem = NULL;
	}

	PartsEditorAbstractView::loadFromModel(paletteModel, modelPart);

	SvgAndPartFilePath *sp = m_item->svgFilePath();

	copyToTempAndRenameIfNecessary(sp);
	delete sp;
	m_item->setSvgFilePath(m_svgFilePath);

	emit loadedFromModel(paletteModel, modelPart);
}

void PartsEditorSpecificationsView::copyToTempAndRenameIfNecessary(SvgAndPartFilePath *filePathOrig) {
	m_originalSvgFilePath = filePathOrig->absolutePath();
	QString svgFolderPath = getApplicationSubFolderPath("parts")+"/svg";

	if(!filePathOrig->absolutePath().startsWith(svgFolderPath)) { // it's outside the parts folder
		DebugDialog::debug(QString("copying from %1").arg(m_originalSvgFilePath));
		QString viewFolder = ItemBase::viewIdentifierNaturalName(m_viewIdentifier);

		if(!m_tempFolder.mkdir(viewFolder)) return;
		if(!m_tempFolder.cd(viewFolder)) return;

		QString destFilePath = FritzingWindow::getRandText()+".svg";
		DebugDialog::debug(QString("dest file: %1").arg(m_tempFolder.path()+"/"+destFilePath));
		QFile tempFile(m_originalSvgFilePath);
		tempFile.copy(m_tempFolder.path()+"/"+destFilePath);

		if(!m_tempFolder.cd("..")) return; // out of view folder

		m_svgFilePath->setRelativePath(viewFolder+"/"+destFilePath);
		m_svgFilePath->setAbsolutePath(m_tempFolder.path()+"/"+m_svgFilePath->relativePath());

	} else {
		QString relPathAux = filePathOrig->relativePath();
		m_svgFilePath->setAbsolutePath(m_originalSvgFilePath);
		m_svgFilePath->setRelativePath(
				relPathAux.right(// remove user/core/contrib
						relPathAux.size() -
						relPathAux.indexOf("/") - 1
				)
		);
	}
}

void PartsEditorSpecificationsView::setSvgFilePath(const QString &filePath) {
	m_originalSvgFilePath = filePath;
	QString svgFolder = getApplicationSubFolderPath("parts")+"/svg";
	QString tempFolder = m_tempFolder.path();

	QString relative;
	Qt::CaseSensitivity cs = Qt::CaseSensitive;
	QString filePathAux = filePath;

#ifdef Q_WS_WIN
	// seems to be necessary for Windows: getApplicationSubFolderPath() returns a string starting with "c:"
	// but the file dialog returns a string beginning with "C:"
	cs = Qt::CaseInsensitive;
#endif
	if(filePath.contains(svgFolder, cs)) {
		// is core file
		relative = filePathAux.remove(svgFolder+"/", cs);
	} else if(filePath.startsWith(tempFolder,cs)) {
		// is generated file that currently lives inside the temp folder
		relative = filePathAux.remove(tempFolder+"/", cs);
	} else {
		// generated jpeg/png
		relative = m_svgFilePath->relativePath();
	}

	delete m_svgFilePath;
	m_svgFilePath = new SvgAndPartFilePath(filePath,relative);
}


const QString PartsEditorSpecificationsView::svgFilePath() {
	return m_svgFilePath->absolutePath();
}

const SvgAndPartFilePath& PartsEditorSpecificationsView::svgFileSplit() {
	return *m_svgFilePath;
}

void PartsEditorSpecificationsView::fitCenterAndDeselect() {
	scene()->setSceneRect(0,0,width(),height());
	PartsEditorAbstractView::fitCenterAndDeselect();
}

QString PartsEditorSpecificationsView::createSvgFromImage(const QString &origFilePath) {
	QString viewFolder = getOrCreateViewFolderInTemp();

	QString newFilePath = m_tempFolder.path()+"/"+viewFolder+"/"+FritzingWindow::getRandText()+".svg";
	QImage imgOrig(origFilePath);


	QSvgGenerator svgGenerator;
	svgGenerator.setFileName(newFilePath);
    svgGenerator.setSize(imgOrig.size());
	QPainter svgPainter(&svgGenerator);
	svgPainter.drawImage(QPoint(0,0), imgOrig);
	svgPainter.end();

	return newFilePath;
}
