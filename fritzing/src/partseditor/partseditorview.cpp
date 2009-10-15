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

$Revision: 2676 $:
$Author: cohen@irascible.com $:
$Date: 2009-03-21 03:10:39 +0100 (Sat, 21 Mar 2009) $

********************************************************************/

#include <QMessageBox>
#include <QFileDialog>
#include <QtDebug>
#include <QSvgGenerator>
#include <QGraphicsProxyWidget>
#include <QVarLengthArray>

#include "partseditorview.h"
#include "partseditorconnectoritem.h"
#include "fixfontsdialog.h"
#include "../items/layerkinpaletteitem.h"
#include "../layerattributes.h"
#include "../fritzingwindow.h"
#include "../fsvgrenderer.h"
#include "../debugdialog.h"
#include "../fapplication.h"
#include "../utils/folderutils.h"
#include "../utils/textutils.h"
#include "../utils/graphicsutils.h"
#include "../svg/svgfilesplitter.h"


int PartsEditorView::ConnDefaultWidth = 5;
int PartsEditorView::ConnDefaultHeight = ConnDefaultWidth;

PartsEditorView::PartsEditorView(
		ViewIdentifierClass::ViewIdentifier viewId, QDir tempDir,
		bool showingTerminalPoints, QGraphicsItem *startItem,
		QWidget *parent, int size, bool deleteModelPartOnClearScene)
	: SketchWidget(viewId, parent, size, size)
{
	m_viewItem = NULL;
	m_item = NULL;
	m_deleteModelPartOnSceneClear = deleteModelPartOnClearScene;
	m_tempFolder = tempDir;
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setDefaultBackground();


	//spec
	m_svgFilePath = new SvgAndPartFilePath;
	m_startItem = startItem;
	if(m_startItem) {
		addFixedToCenterItem(startItem);
		ensureFixedToCenterItems();
	}
	addDefaultLayers();


	// conns
	m_showingTerminalPoints = showingTerminalPoints;
	m_lastSelectedConnId = "";

	setDragMode(QGraphicsView::ScrollHandDrag);

	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	m_terminalPointsTimer = new QTimer(this);
	connect(
		m_terminalPointsTimer,SIGNAL(timeout()),
		this,SLOT(recoverTerminalPointsState())
	);
	m_showingTerminalPointsBackup = m_showingTerminalPoints;
}

PartsEditorView::~PartsEditorView() {
	delete m_svgFilePath;
	clearScene();
}

void PartsEditorView::addDefaultLayers() {
	switch( m_viewIdentifier ) {
		case ViewIdentifierClass::BreadboardView: addBreadboardViewLayers(); break;
		case ViewIdentifierClass::SchematicView: addSchematicViewLayers(); break;
		case ViewIdentifierClass::PCBView: addPcbViewLayers(); break;
		default: break;
	}
}

void PartsEditorView::addItemInPartsEditor(ModelPart * modelPart, SvgAndPartFilePath * svgFilePath) {
	Q_ASSERT(modelPart);
	clearScene();

	m_item = newPartsEditorPaletteItem(modelPart, svgFilePath);
	this->addItem(modelPart, BaseCommand::CrossView, m_item->getViewGeometry(), m_item->id(), -1, -1, NULL, m_item);

	fitCenterAndDeselect();

	setItemProperties();

	/*foreach(QWidget* w, m_fixedWidgets) {
		QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
		proxy->setWidget(w);

		addFixedToBottomRightItem(proxy);
	}*/

	emit connectorsFound(this->m_viewIdentifier,m_item->connectors());
}

ItemBase * PartsEditorView::addItemAux(ModelPart * modelPart, const ViewGeometry &, long /*id*/, long /*originalModelIndex*/, AddDeleteItemCommand *, PaletteItem * paletteItemAux, bool doConnectors, ViewIdentifierClass::ViewIdentifier) {
	if(paletteItemAux == NULL) {
		paletteItemAux = newPartsEditorPaletteItem(modelPart);
	}
	PartsEditorPaletteItem *paletteItem = dynamic_cast<PartsEditorPaletteItem*>(paletteItemAux);
	Q_ASSERT(paletteItem);
	if(paletteItem) {
		modelPart->initConnectors();    // is a no-op if connectors already in place
		QString layerFileName = getLayerFileName(modelPart);
		if(layerFileName != ___emptyString___) {
			if(paletteItem->createSvgPath(modelPart->modelPartShared()->path(), layerFileName)) {
				paletteItem->createSvgFile(paletteItem->svgFilePath()->absolutePath());
				ViewLayer::ViewLayerID viewLayerID =
					ViewLayer::viewLayerIDFromXmlString(
						findConnectorLayerId(paletteItem->svgDom())
					);
				if(viewLayerID == ViewLayer::UnknownLayer) {
					viewLayerID = getViewLayerID(modelPart, m_viewIdentifier);
				}
				addDefaultLayers();
				if (m_viewItem != NULL) {
					QHash<QString, SvgFileSplitter *> svgHash;
					QString svg = m_viewItem->retrieveSvg(viewLayerID, svgHash, false, GraphicsUtils::StandardFritzingDPI);
					if (!svg.isEmpty()) {
						QSizeF size = m_viewItem->size();
						svg = makeSVGHeader(FSvgRenderer::printerScale(), GraphicsUtils::StandardFritzingDPI, size.width(), size.height()) + svg + "</svg>";
						paletteItem->setItemSVG(svg);
					}
				}

				if (paletteItem->renderImage(modelPart, m_viewIdentifier, m_viewLayers, viewLayerID, doConnectors)) {
					addToScene(paletteItemAux, paletteItemAux->viewLayerID());
					// layers are not needed on the parts editor (so far)
					/*paletteItem->loadLayerKin(m_viewLayers);
					for (int i = 0; i < paletteItem->layerKin().count(); i++) {
						LayerKinPaletteItem * lkpi = paletteItem->layerKin()[i];
						this->scene()->addItem(lkpi);
						lkpi->setHidden(!layerIsVisible(lkpi->viewLayerID()));
					}*/
					return paletteItemAux;
				}
			}
		}
	}
	return NULL;
}

void PartsEditorView::fitCenterAndDeselect() {
	if(m_item) {
		m_item->setSelected(false);
		m_item->setHidden(false);

		QRectF viewRect = rect();

		int zoomCorrection;
		if(m_viewIdentifier != ViewIdentifierClass::IconView) {
			qreal x = viewRect.center().x();
			qreal y = viewRect.center().y();
			m_item->setPos(x,y);
			zoomCorrection = 10;
		} else {
			zoomCorrection = 0;
		}

		QRectF itemsRect = scene()->itemsBoundingRect();

		qreal wRelation = viewRect.width()  / itemsRect.width();
		qreal hRelation = viewRect.height() / itemsRect.height();

		if(wRelation < hRelation) {
			m_scaleValue = (wRelation * 100);
		} else {
			m_scaleValue = (hRelation * 100);
		}

		absoluteZoom(m_scaleValue-zoomCorrection);
		centerOn(itemsRect.center());
	}
}

void PartsEditorView::setDefaultBackground() {
	QString bgColor = " PartsEditorView {background-color: rgb(%1,%2,%3);} ";
	if(m_bgcolors.contains(m_viewIdentifier)) {
		QColor c = m_bgcolors[m_viewIdentifier];
		setStyleSheet(styleSheet()+bgColor.arg(c.red()).arg(c.green()).arg(c.blue()));
	}
}

void PartsEditorView::clearScene() {
	if(m_item) {
		deleteItem(m_item, m_deleteModelPartOnSceneClear, true, false);

		scene()->clear();
		m_item = NULL;
	}
}

void PartsEditorView::removeConnectors() {
	QList<PartsEditorConnectorItem*> list;
	for (int i = m_item->childItems().count()-1; i >= 0; i--) {
		PartsEditorConnectorItem * connectorItem = dynamic_cast<PartsEditorConnectorItem *>(m_item->childItems()[i]);
		if (connectorItem == NULL) continue;

		list << connectorItem;
	}

	for(int i=0; i < list.size(); i++) {
		list[i]->removeFromModel();
		delete list[i];
	}
}

ModelPart *PartsEditorView::createFakeModelPart(SvgAndPartFilePath *svgpath) {
	const QHash<QString,StringPair*> connIds = getConnectorIds(svgpath->absolutePath());
	const QStringList layers = getLayers(svgpath->absolutePath());

	QString path = svgpath->relativePath() == ___emptyString___ ? svgpath->absolutePath() : svgpath->relativePath();
	ModelPart * mp = createFakeModelPart(connIds, layers, path);
	foreach(StringPair * sp, connIds.values()) {
		delete sp;
	}

	return mp;
}

ModelPart *PartsEditorView::createFakeModelPart(const QHash<QString,StringPair*> &conns, const QStringList &layers, const QString &svgFilePath) {
	QDomDocument *domDoc = new QDomDocument();
	QString errorStr;
	int errorLine;
	int errorColumn;
	QString fakeFzFile =
		QString("<module><views>\n")+
			QString("<%1><layers image='%2' >\n").arg(ViewIdentifierClass::viewIdentifierXmlName(m_viewIdentifier)).arg(svgFilePath);
		foreach(QString layer, layers) { fakeFzFile +=
			QString("    <layer layerId='%1' />\n").arg(layer);
		}
	fakeFzFile +=
			QString("</layers></%1>\n").arg(ViewIdentifierClass::viewIdentifierXmlName(m_viewIdentifier))+
			QString("</views><connectors>\n");

	foreach(QString id, conns.keys()) {
		QString terminalAttr = conns[id]->second != ___emptyString___ ? QString("terminalId='%1'").arg(conns[id]->second) : "";
		fakeFzFile += QString("<connector id='%1'><views>\n").arg(id)+
				QString("<%1>\n").arg(ViewIdentifierClass::viewIdentifierXmlName(m_viewIdentifier))+
				QString("<p layer='%1' svgId='%2' %3/>\n")
					.arg(ViewLayer::viewLayerXmlNameFromID(SketchWidget::defaultConnectorLayer(m_viewIdentifier)))
					.arg(conns[id]->first)
					.arg(terminalAttr)+
				QString("</%1>\n").arg(ViewIdentifierClass::viewIdentifierXmlName(m_viewIdentifier))+
				QString("</views></connector>\n");
	}
	fakeFzFile += QString("</connectors></module>\n");
  	domDoc->setContent(fakeFzFile, &errorStr, &errorLine, &errorColumn);



  	ModelPart *retval = m_sketchModel->root();
  	retval->modelPartShared()->setDomDocument(domDoc);
  	retval->modelPartShared()->resetConnectorsInitialization();
	retval->modelPartShared()->setPath(FolderUtils::getUserDataStorePath("parts")+"/svg/user");
  	retval->initConnectors(true /*redo connectors*/);
	return retval;
}

const QHash<QString,StringPair*> PartsEditorView::getConnectorIds(const QString &path) {
	QDomDocument dom ;
	QFile file(path);
	dom.setContent(&file);
	file.close();

	QHash<QString,StringPair*> retval;
	QDomElement docElem = dom.documentElement();
	getConnectorIdsAux(retval, docElem);

	return retval;
}

void PartsEditorView::getConnectorIdsAux(QHash<QString/*connectorId*/,StringPair*> &retval, QDomElement &docElem) {
	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull()) {
			QString id = e.attribute("id");
			if(id.startsWith("connector") && id.endsWith("terminal")) {
				QString conn = id.left(id.lastIndexOf(QRegExp("\\d"))+1);
				StringPair *pair = retval.contains(conn) ? retval[conn] : new StringPair();
				pair->second = id;
				retval[conn] = pair;
			}
			else if(id.startsWith("connector") /*&& id.endsWith("pin") */ ) {
				QString conn = id.left(id.lastIndexOf(QRegExp("\\d"))+1);
				StringPair *pair = retval.contains(conn) ? retval[conn] : new StringPair();
				pair->first = id;
				retval[conn] = pair;
			}
			else if(n.hasChildNodes()) {
				getConnectorIdsAux(retval, e);
			}
		}
		n = n.nextSibling();
	}
}

const QStringList PartsEditorView::getLayers(const QString &path) {
	if(m_viewIdentifier == ViewIdentifierClass::IconView) { // defaulting layer to icon for iconview
		QStringList retval; retval << defaultLayerAsStr();
		return retval;
	} else {
		QDomDocument dom;
		QFile file(path);
		dom.setContent(&file);
		file.close();
		return getLayers(&dom);
	}
}

const QStringList PartsEditorView::getLayers(const QDomDocument *dom, bool addDefaultIfNone) {
	QStringList retval;
	QDomElement docElem = dom->documentElement();

	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement();
		if(!e.isNull() && e.tagName() == "g") {
			QString id = e.attribute("id");
			retval << id;
		}
		n = n.nextSibling();
	}

	if(addDefaultIfNone && retval.isEmpty()) {
		retval << ViewIdentifierClass::viewIdentifierNaturalName(m_viewIdentifier);
	}

	return retval;
}

PartsEditorPaletteItem *PartsEditorView::newPartsEditorPaletteItem(ModelPart *modelPart) {
	return new PartsEditorConnectorsPaletteItem(this, modelPart, m_viewIdentifier);
}

PartsEditorPaletteItem *PartsEditorView::newPartsEditorPaletteItem(ModelPart * modelPart, SvgAndPartFilePath *path) {
	return new PartsEditorConnectorsPaletteItem(this, modelPart, m_viewIdentifier, path);
}

QDir PartsEditorView::tempFolder() {
	return m_tempFolder;
}

QString PartsEditorView::getOrCreateViewFolderInTemp() {
	QString viewFolder = ViewIdentifierClass::viewIdentifierNaturalName(m_viewIdentifier);

	if(!QFileInfo(m_tempFolder.absolutePath()+"/"+viewFolder).exists()) {
		Q_ASSERT(m_tempFolder.mkpath(m_tempFolder.absolutePath()+"/"+viewFolder));
	}

	return viewFolder;
}

bool PartsEditorView::isEmpty() {
	return m_item == NULL;
}

bool PartsEditorView::ensureFilePath(const QString &filePath) {
	QString svgFolder = FolderUtils::getUserDataStorePath("parts")+"/svg";

	Qt::CaseSensitivity cs = Qt::CaseSensitive;
#ifdef Q_WS_WIN
	cs = Qt::CaseInsensitive;
#endif
	if(!filePath.contains(svgFolder, cs)) {
		// This has to be here in order of all this, to work in release mode
		m_tempFolder.mkpath(QFileInfo(filePath).absoluteDir().path());
	}
	return true;
}

ViewLayer::ViewLayerID PartsEditorView::connectorLayerId() {
	//Q_ASSERT(m_item);
	ViewLayer::ViewLayerID viewLayerID = ViewLayer::UnknownLayer;
	if (m_item != NULL) {
		viewLayerID = ViewLayer::viewLayerIDFromXmlString(
			findConnectorLayerId(m_item->svgDom())
		);
	}
	if(viewLayerID == ViewLayer::UnknownLayer) {
		return SketchWidget::defaultConnectorLayer(m_viewIdentifier);
	} else {
		return viewLayerID;
	}
}

QString PartsEditorView::terminalIdForConnector(const QString &connId) {
	//Q_ASSERT(m_item)

	if (m_item == NULL) return "";

	QString result = "";
	QDomElement elem = m_item->svgDom()->documentElement();
	if(terminalIdForConnectorIdAux(result, connId, elem)) {
		return result;
	} else {
		return "";
	}
}

bool PartsEditorView::terminalIdForConnectorIdAux(QString &result, const QString &connId, QDomElement &docElem) {
	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement();
		if(!e.isNull()) {
			QString id = e.attribute("id");
			if(id.startsWith(connId) && id.endsWith("terminal")) {
				// the id is the one from the previous iteration
				result = id;
				return true;
			} else if(n.hasChildNodes()) {
				// potencial solution, if the next iteration returns true
				if(terminalIdForConnectorIdAux(result, connId, e)) {
					return true;
				}
			}
		}
		n = n.nextSibling();
	}
	return false;
}

QString PartsEditorView::findConnectorLayerId(QDomDocument *svgDom) {
	QString result = ___emptyString___;
	QStringList layers;
	QDomElement docElem = svgDom->documentElement();
	if(findConnectorLayerIdAux(result, docElem, layers)) {
		if(ViewLayer::viewLayerIDFromString(result) == ViewLayer::UnknownLayer) {
			foreach(QString layer, layers) {
				ViewLayer::ViewLayerID vlid = ViewLayer::viewLayerIDFromXmlString(layer);
				if(m_viewLayers.keys().contains(vlid)) {
					result = layer;
				}
			}
		}
		return result;
	} else {
		return defaultLayerAsStr();
	}
}

bool PartsEditorView::findConnectorLayerIdAux(QString &result, QDomElement &docElem, QStringList &prevLayers) {
	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement();
		if(!e.isNull()) {
			QString id = e.attribute("id");
			if(id.startsWith("connector")) {
				// the id is the one from the previous iteration
				return true;
			} else if(n.hasChildNodes()) {
				// potencial solution, if the next iteration returns true
				result = id;
				prevLayers << id;
				if(findConnectorLayerIdAux(result, e, prevLayers)) {
					return true;
				}
			}
		}
		n = n.nextSibling();
	}
	return false;
}

QString PartsEditorView::getLayerFileName(ModelPart * modelPart) {
	QDomElement layers = LayerAttributes::getSvgElementLayers(modelPart->modelPartShared()->domDocument(), m_viewIdentifier);
	if (layers.isNull()) return ___emptyString___;

	return layers.attribute("image");
}


// specs
void PartsEditorView::copySvgFileToDestiny(const QString &partFileName) {
	Qt::CaseSensitivity cs = Qt::CaseSensitive;
#ifdef Q_WS_WIN
	cs = Qt::CaseInsensitive;
#endif

	// if the svg file is in the temp folder, then copy it to destiny
	if(m_svgFilePath->absolutePath().startsWith(m_tempFolder.absolutePath(),cs)) {
		QString origFile = svgFilePath();
		setFriendlierSvgFileName(partFileName);
		QString destFile = FolderUtils::getUserDataStorePath("parts")+"/svg/user/"+m_svgFilePath->relativePath();

		ensureFilePath(origFile);
		QFile tempFile(origFile);
		DebugDialog::debug(QString("copying from %1 to %2")
				.arg(origFile)
				.arg(destFile));
		tempFile.copy(destFile);
		tempFile.close();

		// update the item info, to point to this file
		m_svgFilePath->setAbsolutePath(destFile);
	}
}

void PartsEditorView::loadFile() {
	QString origPath = QFileDialog::getOpenFileName(this,
		tr("Open Image"),
		m_originalSvgFilePath.isEmpty() ? FolderUtils::getUserDataStorePath("parts")+"/parts/svg/" : m_originalSvgFilePath,
		tr("Image Files (%1 %2 %3);;SVG Files (%1);;JPEG Files (%2);;PNG Files(%3)")
			.arg("*.svg").arg("*.jpg *.jpeg").arg("*.png")
	);

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

void PartsEditorView::updateModelPart(const QString& origPath) {
	m_undoStack->push(new QUndoCommand("Dummy parts editor command"));

	setSvgFilePath(origPath);
	copyToTempAndRenameIfNecessary(m_svgFilePath);
	m_item->setSvgFilePath(m_svgFilePath);

	ModelPart *mp = createFakeModelPart(m_svgFilePath);
	m_item->setModelPart(mp);
}

void PartsEditorView::loadSvgFile(const QString& origPath) {
	// back to an empty state
	m_drawnConns.clear();
	m_removedConnIds.clear();

	bool canceled = false;
	beforeSVGLoading(origPath, canceled);

	if(!canceled) {
		m_undoStack->push(new QUndoCommand("Dummy parts editor command"));
		setSvgFilePath(origPath);

		ModelPart * mp = createFakeModelPart(m_svgFilePath);
		loadSvgFile(mp);
	}
}

void PartsEditorView::beforeSVGLoading(const QString &filename, bool &canceled) {
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly )) {
    	QMessageBox::warning(
    		this,
    		tr("Couldn't open svg file"),
    		tr(
    		"The file couldn't be opened. If this file defines its dimensions \n"
    		"in non-real-world units (e.g. pixels), then they won't be translated \n"
    		"into real life ones.\n"
    		"Malformed font-family definitions won't be fixed either.")
    	);
        return;
    }

    QString fileContent(file.readAll());
	bool fileHasChanged = fixPixelDimensionsIn(fileContent,filename);
	fileHasChanged |= cleanXml(fileContent,filename);
	fileHasChanged |= fixViewboxOrigin(fileContent,filename);
	fileHasChanged |= fixFonts(fileContent,filename,canceled);

	if(fileHasChanged) {
		file.close();
		if(!file.open(QIODevice::WriteOnly )) {
			QMessageBox::warning(
				this,
				tr("Couldn't write into file"),
				tr(
				"This file needs to be fixed to fit fritzing needs, but it couldn't\n"
				"be written.\n"
				"Fritzing is not compatible with this kind of svg files. Please \n"
				"check your permissions, and try again.\n\n"

				"More information at http://fritzing.org/using-svg-images-new-parts/"
				)
			);
		} else {
			QTextStream out(&file);
			out << fileContent;
			file.close();
		}
	}

}

bool PartsEditorView::fixFonts(QString &fileContent, const QString &filename, bool &canceled) {
	bool changed = removeFontFamilySingleQuotes(fileContent, filename);
	changed |= fixUnavailableFontFamilies(fileContent, filename, canceled);

	return changed;
}

bool PartsEditorView::removeFontFamilySingleQuotes(QString &fileContent, const QString &filename) {
	QString pattern = "font-family=\"('.*')\"";
	QSet<QString> wrongFontFamilies = TextUtils::getRegexpCaptures(pattern,fileContent);

	foreach(QString ff, wrongFontFamilies) {
		QString wrongFF = ff;
		QString fixedFF = ff.remove('\'');
		fileContent.replace(wrongFF,fixedFF);
		DebugDialog::debug(
			QString("removing font-family single quotes: \"%1\" to \"%2\" in file '%3'")
				.arg(wrongFF).arg(fixedFF).arg(filename)
		);
	}

	return wrongFontFamilies.size() > 0;
}

bool PartsEditorView::fixUnavailableFontFamilies(QString &fileContent, const QString &filename, bool &canceled) {
	QSet<QString> definedFFs;
	definedFFs.unite(getAttrFontFamilies(fileContent));
	definedFFs.unite(getFontFamiliesInsideStyleTag(fileContent));

	FixedFontsHash fixedFonts = FixFontsDialog::fixFonts(this,definedFFs,canceled);

	if(!canceled) {
		foreach(QString oldF, fixedFonts.keys()) {
			QString newF = fixedFonts[oldF];
			fileContent.replace(oldF,newF);
			DebugDialog::debug(
				QString("replacing font-family: \"%1\" to \"%2\" in file '%3'")
					.arg(oldF).arg(newF).arg(filename)
			);
		}
	}

	return !canceled && fixedFonts.size() > 0;
}

QSet<QString> PartsEditorView::getAttrFontFamilies(const QString &fileContent) {
	/*
	 * font-family defined as attr example:

<text xmlns="http://www.w3.org/2000/svg" font-family="DroidSans"
id="text2732" transform="matrix(1 0 0 1 32.2012 236.969)"
font-size="9.9771" >A0</text>

	 */

	QString pattern = "font-family\\s*=\\s*\"(.|[^\"]*)\\s*\"";
	return TextUtils::getRegexpCaptures(pattern,fileContent);
}

QSet<QString> PartsEditorView::getFontFamiliesInsideStyleTag(const QString &fileContent) {
	/*
	 * regexp: font-family\s*:\s*(.|[^;"]*).*"
	 * font-family defined in a style attr example:

style="font-size:9;-inkscape-font-specification:Droid Sans;font-family:Droid Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal"

style="font-size:144px;font-style:normal;font-weight:normal;line-height:100%;fill:#ffffff;fill-opacity:1;stroke:none;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1;font-family:Bitstream Vera Sans" x="18.000002"

	 */

	QString pattern = "font-family\\s*:\\s*(.|[^;\"]*).*\"";
	return TextUtils::getRegexpCaptures(pattern,fileContent);
}

bool PartsEditorView::isIllustratorFile(const QString &fileContent) {
	return fileContent.contains("<!-- Generator: Adobe Illustrator");
}

bool PartsEditorView::fixPixelDimensionsIn(QString &fileContent, const QString &filename) {
	if(m_viewIdentifier == ViewIdentifierClass::IconView) return false;
	return false;

	QDomDocument *svgDom = new QDomDocument();

	QString errorMsg;
	int errorLine;
	int errorCol;
	if(!svgDom->setContent(fileContent, true, &errorMsg, &errorLine, &errorCol)) {
		qWarning() << QString("PartsEditorView::fixPixelDimensionsIn(filename) couldn't load svg: %1 (line %2, col %3)")
						.arg(errorMsg).arg(errorLine).arg(errorCol);
		return false;
	}

	QDomElement elem = svgDom->firstChildElement("svg");
	bool fileHasChanged = pxToInches(elem,"width",filename);
	fileHasChanged |= pxToInches(elem,"height",filename);

	if(fileHasChanged) {
		fileContent = removeXMLEntities(svgDom->toString());
	}

	delete svgDom;

	return fileHasChanged;
}

bool PartsEditorView::fixViewboxOrigin(QString &fileContent, const QString &filename) {
	QDomDocument *svgDom = new QDomDocument();

	bool fileHasChanged = false;
	if(isIllustratorFile(fileContent)) {
		QString *errorMsg = new QString("");
		int *errorLine = new int(0);
		int *errorCol = new int(0);
		if(!svgDom->setContent(fileContent, true, errorMsg, errorLine, errorCol)) {
			qWarning() << QString("PartsEditorView::fixViewboxOrigin(filename) couldn't load svg: %1 (line %2, col %3)")
							.arg(*errorMsg).arg(*errorLine).arg(*errorCol);
			return false;
		}
		delete errorMsg;
		delete errorLine;
		delete errorCol;

		QDomElement elem = svgDom->firstChildElement("svg");
		fileHasChanged = moveViewboxToTopLeftCorner(elem,filename);

		if(fileHasChanged) {
			fileContent = svgDom->toString();
		}

		delete svgDom;
	}

	return fileHasChanged;
}

bool PartsEditorView::moveViewboxToTopLeftCorner(QDomElement &elem, const QString &filename) {
	QString attrName = elem.hasAttribute("viewbox")? "viewbox": "viewBox";
	QStringList vals = elem.attribute(attrName).split(" ");
	if(vals.length() == 4 && (vals[0] != "0" || vals[1] != "0")) {
		QString newValue = QString("0 0 %1 %2").arg(vals[2]).arg(vals[3]);
		elem.setAttribute(attrName,newValue);
		DebugDialog::debug(
			QString("translating svg viewbox origin from '(%1,%2)' to '(0,0)' in file '%3'")
				.arg(vals[0]).arg(vals[1]).arg(filename)
		);

		return true;
	}
	return false;
}

bool PartsEditorView::pxToInches(QDomElement &elem, const QString &attrName, const QString &filename) {
	QString attrValue = elem.attribute(attrName);
	if(attrValue.endsWith("px")) {
		bool ok;
		qreal value = TextUtils::convertToInches(attrValue, &ok);
		if(ok) {
			QString newValue = QString("%1in").arg(value);
			elem.setAttribute(attrName,newValue);
			DebugDialog::debug(
				QString("translating svg attribute '%1' from '%2px' to '%3' in file '%4'")
					.arg(attrName).arg(attrValue).arg(newValue).arg(filename)
			);

			return true;
		}
	}
	return false;
}

void PartsEditorView::loadSvgFile(ModelPart * modelPart) {
	addItemInPartsEditor(modelPart, m_svgFilePath);
	copyToTempAndRenameIfNecessary(m_svgFilePath);
	m_item->setSvgFilePath(m_svgFilePath);
}

void PartsEditorView::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	clearScene();

	ViewGeometry viewGeometry;
	this->setPaletteModel(paletteModel);
	m_item = (PartsEditorPaletteItem*)SketchWidget::loadFromModel(modelPart, viewGeometry);
	fitCenterAndDeselect();

	setItemProperties();

	if(m_item) {
		if(m_startItem) {
			m_fixedToCenterItems.removeAll(m_startItem);
			m_startItem = NULL;
		}


		SvgAndPartFilePath *sp = m_item->svgFilePath();

		copyToTempAndRenameIfNecessary(sp);
		m_item->setSvgFilePath(m_svgFilePath);
	}
}

void PartsEditorView::copyToTempAndRenameIfNecessary(SvgAndPartFilePath *filePathOrig) {
	m_originalSvgFilePath = filePathOrig->absolutePath();
	QString userSvgFolderPath = FolderUtils::getUserDataStorePath("parts")+"/svg";
	QString coreSvgFolderPath = FolderUtils::getApplicationSubFolderPath("parts")+"/svg";

	if(!(filePathOrig->absolutePath().startsWith(userSvgFolderPath)
		|| filePathOrig->absolutePath().startsWith(coreSvgFolderPath))
		) { // it's outside the parts folder
		DebugDialog::debug(QString("copying from %1").arg(m_originalSvgFilePath));
		QString viewFolder = ViewIdentifierClass::viewIdentifierNaturalName(m_viewIdentifier);

		if(!QFileInfo(m_tempFolder.path()+"/"+viewFolder).exists()
		   && !m_tempFolder.mkdir(viewFolder)) return;
		if(!m_tempFolder.cd(viewFolder)) return;

		QString destFilePath = FritzingWindow::getRandText()+".svg";
		DebugDialog::debug(QString("dest file: %1").arg(m_tempFolder.absolutePath()+"/"+destFilePath));

		ensureFilePath(m_tempFolder.absolutePath()+"/"+destFilePath);

		QFile tempFile(m_originalSvgFilePath);
		tempFile.copy(m_tempFolder.absolutePath()+"/"+destFilePath);
		tempFile.close();

		if(!m_tempFolder.cd("..")) return; // out of view folder

		m_svgFilePath->setRelativePath(viewFolder+"/"+destFilePath);
		m_svgFilePath->setAbsolutePath(m_tempFolder.absolutePath()+"/"+m_svgFilePath->relativePath());

	} else {
		QString relPathAux = filePathOrig->relativePath();
		m_svgFilePath->setAbsolutePath(m_originalSvgFilePath);
		Q_ASSERT(relPathAux.count("/") <= 2);
		if(relPathAux.count("/") == 2) { // this means that core/user/contrib is still in the file name
			m_svgFilePath->setRelativePath(
				relPathAux.right(// remove user/core/contrib
					relPathAux.size() -
					relPathAux.indexOf("/") - 1
				)
			);
		} else { //otherwise, just leave it as it is
			m_svgFilePath->setRelativePath(relPathAux);
		}
	}
}

void PartsEditorView::setSvgFilePath(const QString &filePath) {
	ensureFilePath(filePath);
	m_originalSvgFilePath = filePath;

	QString userSvgFolder = FolderUtils::getUserDataStorePath("parts")+"/svg";
	QString coreSvgFolder = FolderUtils::getApplicationSubFolderPath("parts")+"/svg";

	QString tempFolder = m_tempFolder.absolutePath();

	QString relative;
	Qt::CaseSensitivity cs = Qt::CaseSensitive;
	QString filePathAux = filePath;

#ifdef Q_WS_WIN
	// seems to be necessary for Windows: getUserDataStorePath() returns a string starting with "c:"
	// but the file dialog returns a string beginning with "C:"
	cs = Qt::CaseInsensitive;
#endif
	if(filePath.contains(userSvgFolder, cs) || filePath.contains(coreSvgFolder, cs)) {
		QString svgFolder = filePath.contains(userSvgFolder,cs)? userSvgFolder: coreSvgFolder;
		// is core/user file
		relative = filePathAux.remove(svgFolder+"/", cs);
		//Mariano: I don't like this folder thing anymore
		relative = relative.mid(filePathAux.indexOf("/")+1); // remove core/user/contrib
	} else {
		// generated jpeg/png or file outside fritzing folder
		relative = "";
	}

	if (m_svgFilePath) delete m_svgFilePath;
	m_svgFilePath = new SvgAndPartFilePath(filePath,relative);
}


const QString PartsEditorView::svgFilePath() {
	return m_svgFilePath->absolutePath();
}

const SvgAndPartFilePath& PartsEditorView::svgFileSplit() {
	return *m_svgFilePath;
}

QString PartsEditorView::createSvgFromImage(const QString &origFilePath) {
	QString viewFolder = getOrCreateViewFolderInTemp();

	QString newFilePath = m_tempFolder.absolutePath()+"/"+viewFolder+"/"+FritzingWindow::getRandText()+".svg";
	ensureFilePath(newFilePath);

/* %1=witdh in mm
 * %2=height in mm
 * %3=width in local coords
 * %4=height in local coords
 * %5=binary data
 */
/*	QString svgTemplate =
"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
"	<svg width='%1mm' height='%2mm' viewBox='0 0 %3 %4' xmlns='http://www.w3.org/2000/svg'\n"
"		xmlns:xlink='http://www.w3.org/1999/xlink' version='1.2' baseProfile='tiny'>\n"
"		<g fill='none' stroke='black' vector-effect='non-scaling-stroke' stroke-width='1'\n"
"			fill-rule='evenodd' stroke-linecap='square' stroke-linejoin='bevel' >\n"
"			<image x='0' y='0' width='%3' height='%4'\n"
"				xlink:href='data:image/png;base64,%5' />\n"
"		</g>\n"
"	</svg>";

	QPixmap pixmap(origFilePath);
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	pixmap.save(&buffer,"png"); // writes pixmap into bytes in PNG format

	QString svgDom = svgTemplate
		.arg(pixmap.widthMM()).arg(pixmap.heightMM())
		.arg(pixmap.width()).arg(pixmap.height())
		.arg(QString("data:image/png;base64,%2").arg(QString(bytes.toBase64())));

	QFile destFile(newFilePath);
	if(!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::information(this, "", "file not created");
		if(!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
				QMessageBox::information(this, "", "file not created 2");
			}
	}
	QTextStream out(&destFile);
	out << svgDom;
	destFile.close();
	qDebug() << newFilePath;
	Q_ASSERT(QFileInfo(newFilePath).exists());
*/

	QImage imgOrig(origFilePath);

	QSvgGenerator svgGenerator;
	svgGenerator.setFileName(newFilePath);
    svgGenerator.setSize(imgOrig.size());
	QPainter svgPainter(&svgGenerator);
	svgPainter.drawImage(QPoint(0,0), imgOrig);
	svgPainter.end();

	return newFilePath;
}

QString PartsEditorView::setFriendlierSvgFileName(const QString &partFileName) {
	QString aux = partFileName;
	aux = aux
		.remove(FritzingPartExtension)
		.replace(" ","_");
	if(aux.length()>40) aux.truncate(40);
	aux+=QString("__%1__%2.svg")
			.arg(ViewIdentifierClass::viewIdentifierNaturalName(m_viewIdentifier))
			.arg(FritzingWindow::getRandText());
	int slashIdx = m_svgFilePath->relativePath().indexOf("/");
	QString relpath = m_svgFilePath->relativePath();
	QString relpath2 = relpath;
	QString abspath = m_svgFilePath->absolutePath();
	QString viewFolder = relpath.remove(slashIdx,relpath.size()-slashIdx+1);
	m_svgFilePath->setAbsolutePath(abspath.remove(relpath2)+viewFolder+"/"+aux);
	m_svgFilePath->setRelativePath(viewFolder+"/"+aux);
	return aux;
}


// conns
void PartsEditorView::wheelEvent(QWheelEvent* event) {
	if(m_showingTerminalPoints) {
		if(!m_terminalPointsTimer->isActive()) {
			m_showingTerminalPointsBackup = m_showingTerminalPoints;
			showTerminalPoints(false);
			m_terminalPointsTimer->start(50);
		}
	} else if(m_terminalPointsTimer->isActive()) {
		m_terminalPointsTimer->stop();
		m_terminalPointsTimer->start(50);
	}
	SketchWidget::wheelEvent(event);
}

void PartsEditorView::mousePressEvent(QMouseEvent *event) {
	SketchWidget::mousePressEvent(event);
}

void PartsEditorView::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
}

void PartsEditorView::mouseReleaseEvent(QMouseEvent *event) {
	SketchWidget::mouseReleaseEvent(event);
}

void PartsEditorView::drawConector(Connector *conn, bool showTerminalPoint) {
	QSize size(ConnDefaultWidth,ConnDefaultHeight);
	createConnector(conn,size,showTerminalPoint);
}

void PartsEditorView::createConnector(Connector *conn, const QSize &connSize, bool showTerminalPoint) {
	QString connId = conn->connectorSharedID();

	QRectF bounds = m_item
			? QRectF(m_item->boundingRect().center(),connSize)
			: QRectF(scene()->itemsBoundingRect().center(),connSize);
	PartsEditorConnectorsConnectorItem *connItem = new PartsEditorConnectorsConnectorItem(conn, m_item, m_showingTerminalPoints, bounds);
	m_drawnConns[connId] = connItem;
	connItem->setShowTerminalPoint(showTerminalPoint);

	m_undoStack->push(new QUndoCommand(
		QString("connector '%1' added to %2 view")
		.arg(connId).arg(ViewIdentifierClass::viewIdentifierName(m_viewIdentifier))
	));
}

void PartsEditorView::removeConnector(const QString &connId) {
	ConnectorItem *connToRemove = NULL;
	foreach(QGraphicsItem *item, items()) {
		ConnectorItem *connItem = dynamic_cast<ConnectorItem*>(item);
		if(connItem && connItem->connector()->connectorSharedID() == connId) {
			connToRemove = connItem;
			break;
		}
	}

	if(connToRemove) {
		scene()->removeItem(connToRemove);
		scene()->update();
		m_undoStack->push(new QUndoCommand(
			QString("connector '%1' removed from %2 view")
			.arg(connId).arg(ViewIdentifierClass::viewIdentifierName(m_viewIdentifier))
		));

		PartsEditorConnectorsConnectorItem *connToRemoveAux = dynamic_cast<PartsEditorConnectorsConnectorItem*>(connToRemove);
		m_drawnConns.remove(connToRemoveAux->connectorSharedID());
		m_removedConnIds << connId;
	}
}

void PartsEditorView::setItemProperties() {
	if(m_item) {
		m_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
		m_item->setFlag(QGraphicsItem::ItemIsMovable, false);
		m_item->setFlag(QGraphicsItem::ItemClipsToShape, true);
		//m_item->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
		myItem()->highlightConnectors(m_lastSelectedConnId);

		qreal size = 500; // just make sure the user get enough space to play
		setSceneRect(0,0,size,size);


		m_item->setPos((size-m_item->size().width())/2,(size-m_item->size().height())/2);
		centerOn(m_item);

	}
	//ensureFixedToBottomRight(m_zoomControls);
}

void PartsEditorView::informConnectorSelection(const QString &connId) {
	if(m_item) {
		m_lastSelectedConnId = connId;
		myItem()->highlightConnectors(connId);
	}
}

void PartsEditorView::informConnectorSelectionFromView(const QString &connId) {
	informConnectorSelection(connId);
	emit connectorSelected(connId);
}

void PartsEditorView::setMismatching(ViewIdentifierClass::ViewIdentifier viewId, const QString &id, bool mismatching) {
	if(m_item && viewId == m_viewIdentifier) {
		for (int i = 0; i < m_item->childItems().count(); i++) {
			PartsEditorConnectorsConnectorItem * connectorItem
				= dynamic_cast<PartsEditorConnectorsConnectorItem *>(m_item->childItems()[i]);
			if(connectorItem == NULL) continue;

			if(connectorItem->connector()->connectorSharedID() == id) {
				connectorItem->setMismatching(mismatching);
			}
		}
	}
}

void PartsEditorView::aboutToSave() {
	if(m_item) {
		FSvgRenderer renderer;
		if(renderer.load(m_item->flatSvgFilePath(), false)) {
			QRectF svgViewBox = renderer.viewBoxF();
			QSizeF sceneViewBox = renderer.defaultSizeF();
			QDomDocument *svgDom = m_item->svgDom();

			// this may change the layers defined in the file, so
			// let's get the connectorsLayer after it
			bool somethingChanged = addDefaultLayerIfNotIn(svgDom);

			QString connectorsLayerId = findConnectorLayerId(svgDom);
			QDomElement elem = svgDom->documentElement();

			somethingChanged |= removeConnectorsIfNeeded(elem);
			somethingChanged |= updateTerminalPoints(svgDom, sceneViewBox, svgViewBox, connectorsLayerId);
			somethingChanged |= addConnectorsIfNeeded(svgDom, sceneViewBox, svgViewBox, connectorsLayerId);
			somethingChanged |= (m_viewItem != NULL);

			if(somethingChanged) {
				QString viewFolder = getOrCreateViewFolderInTemp();

				QString tempFile = m_tempFolder.absolutePath()+"/"+viewFolder+"/"+FritzingWindow::getRandText()+".svg";

				ensureFilePath(tempFile);

				QFile file(tempFile);
				if(!file.open(QFile::WriteOnly)) {
					/*QMessageBox::information(this,"",
						QString("Couldn't open file for update, after drawing connectors: '%1'")
							.arg(tempFile)
					);*/
				}
				else {
					QTextStream out(&file);
					out << removeXMLEntities(svgDom->toString());

					file.close();
					updateModelPart(tempFile);
				}

			}
		} else {
			DebugDialog::debug("updating part view svg file: could not load file "+m_item->flatSvgFilePath());
		}
	}
}

QString PartsEditorView::removeXMLEntities(QString svgContent) {
	// remove the html entities
	svgContent.replace("&#xd;","");
	svgContent.replace("&#xa;","");
	svgContent.replace("&#x9;","");

	return svgContent;
}

bool PartsEditorView::addConnectorsIfNeeded(QDomDocument *svgDom, const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId) {
	bool changed = false;
	if(!m_drawnConns.isEmpty()) {
		QRectF bounds;
		QString connId;

		foreach(PartsEditorConnectorsConnectorItem* drawnConn, m_drawnConns.values()) {
			bounds = drawnConn->mappedRect();
			connId = drawnConn->connector()->connectorSharedID();

			QRectF svgRect = mapFromSceneToSvg(bounds,sceneViewBox,svgViewBox);
			QString svgId = svgIdForConnector(drawnConn->connector(), connId);
			addRectToSvg(svgDom,svgId,svgRect, connectorsLayerId);
		}
		changed = true;
	}

	return changed;
}

bool PartsEditorView::addDefaultLayerIfNotIn(QDomDocument *svgDom) {
	QString defaultLayer = defaultLayerAsStr();
	if( !getLayers(svgDom).contains(defaultLayer) ) {
		QDomElement docElem = svgDom->documentElement();

		QDomElement newTopLevel = svgDom->createElement("g");
		newTopLevel.setAttribute("id",defaultLayer);

		// place the child in a aux list, cause the
		// qdomnodelist takes care of the references
		QList<QDomNode> children;
		for(QDomNode child=docElem.firstChild(); !child.isNull(); child=child.nextSibling()) {
			children << child;
		}

		foreach(QDomNode child, children) {
			newTopLevel.appendChild(child);
		}

		docElem.appendChild(newTopLevel);

		return true;
	} else {
		return false;
	}
}

ViewLayer::ViewLayerID PartsEditorView::defaultLayer() {
	switch( m_viewIdentifier ) {
		case ViewIdentifierClass::IconView: return ViewLayer::Icon; break;
		case ViewIdentifierClass::BreadboardView: return ViewLayer::Breadboard; break;
		case ViewIdentifierClass::SchematicView: return ViewLayer::Schematic; break;
		case ViewIdentifierClass::PCBView: return ViewLayer::Copper0; break;
		default: break;
	}
	return ViewLayer::UnknownLayer;
}

QString PartsEditorView::defaultLayerAsStr() {
	return ViewLayer::viewLayerXmlNameFromID(defaultLayer());
}

QString PartsEditorView::svgIdForConnector(const QString &connId) {
	//Q_ASSERT(m_item)

	if (m_item == NULL) return "";

	foreach(Connector* conn, m_item->connectors()) {
		QString svgId = svgIdForConnector(conn, connId);
		if(connId != svgId) {
			return svgId;
		}
	}
	return connId;
}

QString PartsEditorView::svgIdForConnector(Connector* conn, const QString &connId) {
	foreach(SvgIdLayer *sil, conn->connectorShared()->pins().values(m_viewIdentifier)) {
		if(conn->connectorSharedID() == connId) {
			return sil->m_svgId;
		}
	}
	return connId;
}

bool PartsEditorView::updateTerminalPoints(QDomDocument *svgDom, const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId) {
	QList<PartsEditorConnectorsConnectorItem*> connsWithNewTPs;
	QStringList tpIdsToRemove;
	foreach(QGraphicsItem *item, items()) {
		PartsEditorConnectorsConnectorItem *citem =
			dynamic_cast<PartsEditorConnectorsConnectorItem*>(item);
		if(citem) {
			TerminalPointItem *tp = citem->terminalPointItem();
			QString connId = citem->connector()->connectorSharedID();
			QString terminalId = connId+"terminal";
			Q_ASSERT(tp);
			if(tp && !tp->isInTheCenter()) {
				if(tp->hasBeenMoved() || citem->hasBeenMoved()) {
					connsWithNewTPs << citem;
					tpIdsToRemove << terminalId;
					//DebugDialog::debug("<<<< MOVED! removing terminal "+terminalId+" in view: "+ViewIdentifierClass::viewIdentifierName(m_viewIdentifier));
					updateSvgIdLayer(connId, terminalId, connectorsLayerId);
				}
			} else {
				//DebugDialog::debug("<<<< removing terminal "+terminalId+" in view: "+ViewIdentifierClass::viewIdentifierName(m_viewIdentifier));
				tpIdsToRemove << terminalId;
				emit removeTerminalPoint(connId, m_viewIdentifier);
			}
		}
	}
	QDomElement elem = svgDom->documentElement();
	removeTerminalPoints(tpIdsToRemove,elem);
	addNewTerminalPoints(connsWithNewTPs, svgDom, sceneViewBox, svgViewBox, connectorsLayerId);
	return !tpIdsToRemove.isEmpty();
}

void PartsEditorView::updateSvgIdLayer(const QString &connId, const QString &terminalId, const QString &connectorsLayerId) {
	foreach(Connector *conn, m_item->connectors()) {
		foreach(SvgIdLayer *sil, conn->connectorShared()->pins().values(m_viewIdentifier)) {
			if(conn->connectorSharedID() == connId) {
				sil->m_terminalId = terminalId;
				ViewLayer::ViewLayerID viewLayerID =
					ViewLayer::viewLayerIDFromXmlString(connectorsLayerId);
				if(viewLayerID != ViewLayer::UnknownLayer) {
					sil->m_viewLayerID = viewLayerID;
				}
			}
		}
	}
}

void PartsEditorView::removeTerminalPoints(const QStringList &tpIdsToRemove, QDomElement &docElem) {
	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		bool doRemove = false;
		QDomElement e = n.toElement();
		if(!e.isNull()) {
			QString id = e.attribute("id");
			if(tpIdsToRemove.contains(id)) {
				doRemove = true;
			} else if(n.hasChildNodes()) {
				removeTerminalPoints(tpIdsToRemove,e);
			}
		}
		QDomElement e2;
		if(doRemove) {
			e2 = e;
		}
		n = n.nextSibling();
		if(doRemove) {
			e2.removeAttribute("id");
		}
	}
}

void PartsEditorView::addNewTerminalPoints(
			const QList<PartsEditorConnectorsConnectorItem*> &connsWithNewTPs, QDomDocument *svgDom,
			const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId
) {
	foreach(PartsEditorConnectorsConnectorItem* citem, connsWithNewTPs) {
		QString connId = citem->connector()->connectorSharedID();
		TerminalPointItem *tp = citem->terminalPointItem();
		Q_ASSERT(tp);
		if(tp) {
			QRectF tpointRect(tp->mappedPoint(), QPointF(0,0));
			QRectF svgTpRect = mapFromSceneToSvg(tpointRect,sceneViewBox,svgViewBox);

			qreal halfTPSize = 0.001; // a tiny rectangle
			svgTpRect.setSize(QSizeF(halfTPSize*2,halfTPSize*2));

			addRectToSvg(svgDom,connId+"terminal",svgTpRect, connectorsLayerId);
		} else {
			qWarning() << tr(
				"Parts Editor: couldn't save terminal "
				"point for connector %1 in %2 view")
				.arg(citem->connector()->connectorSharedID())
				.arg(ViewIdentifierClass::viewIdentifierNaturalName(m_viewIdentifier));
		}
	}
}

bool PartsEditorView::removeConnectorsIfNeeded(QDomElement &docElem) {
	if(!m_removedConnIds.isEmpty()) {
		//Q_ASSERT(docElem.tagName() == "svg");

		QDomNode n = docElem.firstChild();
		while(!n.isNull()) {
			bool doRemove = false;
			QDomElement e = n.toElement();
			if(!e.isNull()) {
				QString id = e.attribute("id");
				if(isSupposedToBeRemoved(id)) {
					doRemove = true;
				} else if(n.hasChildNodes()) {
					removeConnectorsIfNeeded(e);
				}
			}
			QDomElement e2;
			if(doRemove) {
				e2 = e;
			}
			n = n.nextSibling();
			if(doRemove) {
				e2.removeAttribute("id");
			}
		}
		return true;
	}
	return false;
}

QRectF PartsEditorView::mapFromSceneToSvg(const QRectF &itemRect, const QSizeF &sceneViewBox, const QRectF &svgViewBox) {
	qreal relationW = svgViewBox.width() / sceneViewBox.width();
	qreal relationH = svgViewBox.height() / sceneViewBox.height();

	qreal x = itemRect.x() * relationW;
	qreal y = itemRect.y() * relationH;
	qreal width = itemRect.width() * relationW;
	qreal height = itemRect.height() * relationH;

	return QRectF(x,y,width,height);
}

void PartsEditorView::addRectToSvg(QDomDocument* svgDom, const QString &id, const QRectF &rect, const QString &connectorsLayerId) {
	QDomElement connElem = svgDom->createElement("rect");
	connElem.setAttribute("id",id);
	connElem.setAttribute("x",rect.x());
	connElem.setAttribute("y",rect.y());
	connElem.setAttribute("width",rect.width());
	connElem.setAttribute("height",rect.height());
	connElem.setAttribute("fill","none");

	if(connectorsLayerId == ___emptyString___) {
		svgDom->firstChildElement("svg").appendChild(connElem);
	} else {
		QDomElement docElem = svgDom->documentElement();
		Q_ASSERT(addRectToSvgAux(docElem, connectorsLayerId, connElem));
	}
}

bool PartsEditorView::addRectToSvgAux(QDomElement &docElem, const QString &connectorsLayerId, QDomElement &rectElem) {
	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement();
		if(!e.isNull()) {
			QString id = e.attribute("id");
			if(id == connectorsLayerId) {
				e.appendChild(rectElem);
				return true;
			} else if(n.hasChildNodes()) {
				if(addRectToSvgAux(e, connectorsLayerId, rectElem)) {
					return true;
				}
			}
		}
		n = n.nextSibling();
	}
	return false;
}


bool PartsEditorView::isSupposedToBeRemoved(const QString& id) {
	foreach(QString toBeRemoved, m_removedConnIds) {
		if(id.startsWith(toBeRemoved)) {
			return true;
		}
	}
	return false;
}

PartsEditorConnectorsPaletteItem *PartsEditorView::myItem() {
	return dynamic_cast<PartsEditorConnectorsPaletteItem*>(m_item);
}

void PartsEditorView::showTerminalPoints(bool show) {
	m_showingTerminalPoints = show;
	foreach(QGraphicsItem *item, items()) {
		PartsEditorConnectorsConnectorItem *connItem
			= dynamic_cast<PartsEditorConnectorsConnectorItem*>(item);
		if(connItem) {
			connItem->setShowTerminalPoint(show);
		}
	}
	scene()->update();

	/*if(!m_showingTerminalPoints) {
		m_terminalPointsTimer->stop();
	}*/
}

bool PartsEditorView::showingTerminalPoints() {
	return m_showingTerminalPoints;
}

void PartsEditorView::inFileDefinedConnectorChanged(PartsEditorConnectorsConnectorItem *connItem) {
	QString connId = connItem->connectorSharedID();
	m_drawnConns[connId] = connItem;
	if(!m_removedConnIds.contains(connId)) {
		m_removedConnIds << connId;
	}
}


void PartsEditorView::addFixedToBottomRight(QWidget *widget) {
	m_fixedWidgets << widget;
	QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
	proxy->setWidget(widget);

	addFixedToBottomRightItem(proxy);
}

bool PartsEditorView::imageLoaded() {
	return m_item != NULL;
}

void PartsEditorView::drawBackground(QPainter *painter, const QRectF &rect) {
	SketchWidget::drawBackground(painter,rect);

	// 10mm spacing grid
	/*const int gridSize = 10*width()/widthMM();

	QRectF itemRect = m_item->mapToScene(m_item->boundingRect()).boundingRect();
	painter->drawRect(itemRect);
	qreal itemTop = itemRect.top();
	qreal itemLeft = itemRect.left();

	QVarLengthArray<QLineF, 100> lines;

	for (qreal x = itemLeft; x < rect.right(); x += gridSize) {
		lines.append(QLineF(x, rect.top(), x, rect.bottom()));
	}
	for (qreal x = itemLeft-gridSize; x > rect.left(); x -= gridSize) {
		lines.append(QLineF(x, rect.top(), x, rect.bottom()));
	}

	for (qreal y = itemTop; y < rect.bottom(); y += gridSize) {
		lines.append(QLineF(rect.left(), y, rect.right(), y));
	}
	for (qreal y = itemTop-gridSize; y > rect.top(); y -= gridSize) {
		lines.append(QLineF(rect.left(), y, rect.right(), y));
	}

	painter->drawLines(lines.data(), lines.size());*/
}

void PartsEditorView::recoverTerminalPointsState() {
	showTerminalPoints(m_showingTerminalPointsBackup);
	m_terminalPointsTimer->stop();
}

bool PartsEditorView::connsPosOrSizeChanged() {
	foreach(QGraphicsItem *item, items()) {
		PartsEditorConnectorsConnectorItem *citem =
			dynamic_cast<PartsEditorConnectorsConnectorItem*>(item);
		if(citem) {
			TerminalPointItem *tp = citem->terminalPointItem();
			if((tp && tp->hasBeenMoved()) || citem->hasBeenMoved() || citem->hasBeenResized()) {
				return true;
			}
		}
	}
	return false;
}

bool PartsEditorView::cleanXml(QString &content, const QString & filename)
{
	// clean out sodipodi stuff
	// TODO: don't bother with the core parts
	int l1 = content.length();
	content.remove(SvgFileSplitter::sodipodiDetector);
	if (content.length() != l1) {
		DebugDialog::debug(QString("sodipodi found in %1").arg(filename));
		/*
		QFileInfo f(filename);
		QString p = f.absoluteFilePath();
		p.remove(':');
		p.remove('/');
		p.remove('\\');
		QFile fi(QCoreApplication::applicationDirPath() + p);
		bool ok = fi.open(QFile::WriteOnly);
		if (ok) {
			QTextStream out(&fi);
   			out << str;
			fi.close();
		}
		*/
		return true;
	}
	return false;


	/*
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	bool result = doc.setContent(bytes, &errorStr, &errorLine, &errorColumn);
	m_svgXml.clear();
	if (!result) {
		return false;
	}

	SvgFlattener flattener;
	QDomElement root = doc.documentElement();
	flattener.flattenChildren(root);
	SvgFileSplitter::fixStyleAttributeRecurse(root);
	return doc.toByteArray();
	*/
}

void PartsEditorView::setViewItem(ItemBase * item) {
	m_viewItem = item;
}
