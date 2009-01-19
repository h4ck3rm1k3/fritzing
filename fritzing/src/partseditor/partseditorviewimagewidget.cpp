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

#include "partseditorviewimagewidget.h"
#include "../layerkinpaletteitem.h"
#include "../debugdialog.h"
#include "../waitpushundostack.h"
#include "../fritzingwindow.h"
#include "../connectorstuff.h"

QT_BEGIN_NAMESPACE

PartsEditorViewImageWidget::PartsEditorViewImageWidget(ItemBase::ViewIdentifier viewId, QDir tempDir, QGraphicsItem *startItem, QWidget *parent, int size)
	: PartsEditorAbstractViewImage(viewId, false /*don't show terminal points*/, false, parent, size)
{
	m_svgFilePath = new StringPair;
	m_tempFolder = tempDir;
	if(startItem) {
		scene()->addItem(startItem);
	}
}

void PartsEditorViewImageWidget::mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *){
	DebugDialog::debug("got connector mouse press.  not yet implemented...");
	return;
}

void PartsEditorViewImageWidget::copySvgFileToDestiny() {
	// if the svg file is in the temp folder, then copy it to destiny
	if(!m_svgFilePath->first.isEmpty() && !m_svgFilePath->second.isEmpty() && m_svgFilePath->first == m_tempFolder.path()) {
		QFile tempFile(svgFilePath());
                DebugDialog::debug(QString("copying to %1").arg(getApplicationSubFolderPath("parts")+ "/parts/svg/user/"+m_svgFilePath->second));
                tempFile.copy(getApplicationSubFolderPath("parts")+"/svg/user/"+m_svgFilePath->second);
	}
}

void PartsEditorViewImageWidget::mousePressEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	loadFile();
}

void PartsEditorViewImageWidget::loadFile() {
	QString origPath = QFileDialog::getOpenFileName(this,
		tr("Open Image"),
		m_originalSvgFilePath.isEmpty() ? getApplicationSubFolderPath("parts")+"/parts/svg/" : m_originalSvgFilePath,
		//tr("SVG Files (*.svg);;JPEG (*.jpg);;PNG (*.png)"));
		tr("SVG Files (*.svg)"));

	if(origPath.isEmpty()) {
		return; // Cancel pressed
	} else {
		if(!origPath.endsWith(".svg")) {
			//DebugDialog::debug("<<< no es svg");
			origPath = createSvgFromImage(origPath);
		}
		if(origPath != ___emptyString___) {
			loadSvgFile(origPath);
		}
	}
}

void PartsEditorViewImageWidget::loadSvgFile(const QString& origPath) {
	m_undoStack->push(new QUndoCommand("Dummy parts editor command"));
	ModelPart * mp = static_cast<ModelPart *>(m_sketchModel->root());

	setSvgFilePath(origPath);

	mp = createFakeModelPart(origPath, m_svgFilePath->second);
	loadSvgFile(mp);
}

void PartsEditorViewImageWidget::loadSvgFile(ModelPart * modelPart) {
	addItemInPartsEditor(modelPart, m_svgFilePath);
	emit itemAddedToSymbols(0, m_svgFilePath);

	copyToTempAndRenameIfNecessary(m_svgFilePath);

	m_item->setSvgFilePath(m_svgFilePath);
}

void PartsEditorViewImageWidget::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	PartsEditorAbstractViewImage::loadFromModel(paletteModel, modelPart);

	StringPair *sp = m_item->svgFilePath();
	copyToTempAndRenameIfNecessary(sp);
	delete sp;
	m_item->setSvgFilePath(m_svgFilePath);

	emit loadedFromModel(paletteModel, modelPart);
}

void PartsEditorViewImageWidget::copyToTempAndRenameIfNecessary(StringPair *filePathOrig) {
	m_originalSvgFilePath = filePathOrig->first+(!filePathOrig->second.isEmpty()?"/"+filePathOrig->second:"");
	QString svgFolderPath = getApplicationSubFolderPath("parts")+"/svg";

	if(filePathOrig->first != svgFolderPath) {
		DebugDialog::debug(QString("copying from %1").arg(m_originalSvgFilePath));
		QString viewFolder = ItemBase::viewIdentifierNaturalName(m_viewIdentifier);

		if(!m_tempFolder.mkdir(viewFolder)) return;
		if(!m_tempFolder.cd(viewFolder)) return;

		QString destFilePath = FritzingWindow::getRandText()+".svg";
		DebugDialog::debug(QString("dest file: %1").arg(m_tempFolder.path()+"/"+destFilePath));
		QFile tempFile(m_originalSvgFilePath);
		tempFile.copy(m_tempFolder.path()+"/"+destFilePath);

		if(!m_tempFolder.cd("..")) return; // out of view folder
		m_svgFilePath->first = m_tempFolder.path();

		m_svgFilePath->second = viewFolder+"/"+destFilePath;
	} else {
		m_svgFilePath->first = svgFolderPath;
		m_svgFilePath->second = filePathOrig->second.right(// remove user/core/contrib
									filePathOrig->second.size() -
									filePathOrig->second.indexOf("/") - 1
								);
	}
}

void PartsEditorViewImageWidget::setSvgFilePath(const QString &filePath) {
	m_originalSvgFilePath = filePath;
	QString folder = getApplicationSubFolderPath("parts")+"/svg";

	QString first;
	QString second;

	Qt::CaseSensitivity cs = Qt::CaseSensitive;
#ifdef Q_WS_WIN
	// seems to be necessary for Windows: getApplicationSubFolderPath() returns a string starting with "c:"
	// but the file dialog returns a string beginning with "C:"
	cs = Qt::CaseInsensitive;
#endif
	if(filePath.contains(folder, cs)) {
		//DebugDialog::debug("<<< is in core");
		QString filePathAux = filePath;
		QString svgFile = filePathAux.remove(folder+"/", cs);
		first = folder;
		second = svgFile;
	} else {
		//DebugDialog::debug("<<< isn't in core");
		first = filePath;
		second = "";
	}

	delete m_svgFilePath;
	m_svgFilePath = new StringPair(first,second);
}


const QString PartsEditorViewImageWidget::svgFilePath() {
	return m_svgFilePath->first+"/"+m_svgFilePath->second;
}

const StringPair& PartsEditorViewImageWidget::svgFileSplit() {
	return *m_svgFilePath;
}

void PartsEditorViewImageWidget::fitCenterAndDeselect() {
	scene()->setSceneRect(0,0,width(),height());
	PartsEditorAbstractViewImage::fitCenterAndDeselect();
}

QString PartsEditorViewImageWidget::createSvgFromImage(const QString &origFilePath) {
	QString viewFolder = ItemBase::viewIdentifierNaturalName(m_viewIdentifier);

	if(!QFileInfo(m_tempFolder.path()+"/"+viewFolder).exists()) {
		if(!m_tempFolder.mkdir(viewFolder)) return ___emptyString___;
	}

	QString newFilePath = m_tempFolder.path()+"/"+viewFolder+"/"+FritzingWindow::getRandText()+".svg";
	QImage imgOrig(origFilePath);

	QSvgGenerator svgGenerator;
	svgGenerator.setFileName(newFilePath);
    svgGenerator.setSize(imgOrig.size());
	QPainter svgPainter(&svgGenerator);
	svgPainter.drawImage(QPoint(0,0), imgOrig);
	svgPainter.end();

	return newFilePath;

	/*

	QImage newImg(imgOrig.size(),QImage::Format_ARGB32);

	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	imgOrig.convertToFormat(QImage::Format_ARGB32).save(&buffer);
	QSvgRenderer renderer(bytes.toBase64());

	QPainter painter;
	painter.begin(&newImg);
	renderer.render(&painter);
	painter.end();


	QString newFilePath = m_tempFolder.path()+"/"+FritzingWindow::getRandText()+".svg";
	DebugDialog::debug("<<< nueva imagen "+newFilePath);

	bool result = newImg.save(newFilePath);
	if(!result) {
		DebugDialog::debug("<<< failed");
		return ___emptyString___;
	} else {
		DebugDialog::debug("<<< succes");
		return newFilePath;
	}

	*/
}
