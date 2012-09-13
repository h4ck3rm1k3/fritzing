/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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



#include <QMessageBox>
#include <QMenu>

#include "partseditorpaletteitem.h"
#include "partseditorconnectoritem.h"
#include "../sketch/sketchwidget.h"
#include "partseditorlayerkinpaletteitem.h"
#include "../fsvgrenderer.h"
#include "../debugdialog.h"
#include "../layerattributes.h"
#include "../items/layerkinpaletteitem.h"
#include "../items/partfactory.h"
#include "../utils/folderutils.h"
#include "../utils/graphicsutils.h"

typedef QPair<QString, QString> StringPair;

PartsEditorPaletteItem::PartsEditorPaletteItem(PartsEditorView *owner, ModelPart * modelPart, ViewLayer::ViewIdentifier viewIdentifier) :
	PaletteItem(modelPart, viewIdentifier, m_viewGeometry, ItemBase::getNextID(), NULL, false)
{
	m_itemSVG.clear();
	m_owner = owner;

	m_svgDom = NULL;

	m_svgStrings = NULL;
	m_shouldDeletePath = true;
}

PartsEditorPaletteItem::PartsEditorPaletteItem(PartsEditorView *owner, ModelPart * modelPart, ViewLayer::ViewIdentifier viewIdentifier, SvgAndPartFilePath *path) :
	PaletteItem(modelPart, viewIdentifier, m_viewGeometry, ItemBase::getNextID(), NULL, false)
{
	m_owner = owner;

	m_svgDom = NULL;
	createSvgFile(path->absolutePath());
	m_svgStrings = path;
	m_shouldDeletePath = false;


	setAcceptHoverEvents(false);
	setSelected(false);
}

PartsEditorPaletteItem::~PartsEditorPaletteItem()
{
	if (m_svgDom) {
		delete m_svgDom;
	}
	if (m_shouldDeletePath && m_svgStrings) {
		delete m_svgStrings;
	}
}

void PartsEditorPaletteItem::createSvgFile(QString path) {
	if (m_svgDom) {
		delete m_svgDom;
	}
    m_svgDom = new QDomDocument();
	if (!m_itemSVG.isEmpty() && m_svgDom->setContent(m_itemSVG)) {
		m_originalSvgPath = path;
		return;
	}

    QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
        return;
	}
    if (!m_svgDom->setContent(&file)) {
        return;
    }
    m_originalSvgPath = path;
}

bool PartsEditorPaletteItem::createSvgPath(const QString &modelPartSharedPath, const QString &layerFileName) {
	Q_UNUSED(modelPartSharedPath);

	if(m_shouldDeletePath && m_svgStrings) {
		delete m_svgStrings;
		m_svgStrings = NULL;
	}
	m_shouldDeletePath = true;

	if(QFileInfo(layerFileName).exists()) {
		m_svgStrings = new SvgAndPartFilePath();
		m_svgStrings->setAbsolutePath(layerFileName);
		return true; // nothing to do
	} else {
		StringPair tempPath;
		tempPath.first  = "%1/" + ItemBase::SvgFilesDir;
		tempPath.second = "%2/" + layerFileName;

		QStringList possibleRootFolders;
		possibleRootFolders << FolderUtils::getApplicationSubFolderPath("parts") << FolderUtils::getUserDataStorePath("parts") << PartFactory::folderPath();
		QStringList possibleFolders = ModelPart::possibleFolders();
		foreach(QString rootFolder, possibleRootFolders) {
			foreach(QString folder, possibleFolders) {
				QString path = tempPath.first.arg(rootFolder)+"/"+tempPath.second.arg(folder);
				if (QFileInfo(path).exists()) {
					m_svgStrings = new SvgAndPartFilePath();
					m_svgStrings->setAbsolutePath(path);
					m_svgStrings->setRelativePath(tempPath.second.arg(folder));
					m_svgStrings->setCoreContribOrUser(folder);
					return true;
				}
			}
		}
	}
	return false;
}

void PartsEditorPaletteItem::writeXmlLocation(QXmlStreamWriter & /*streamWriter*/) {
	return;
}

void PartsEditorPaletteItem::writeXml(QXmlStreamWriter & streamWriter) 
{
	if (m_svgStrings == NULL) return;

	streamWriter.writeStartElement(ViewLayer::viewIdentifierXmlName(m_viewIdentifier));
	streamWriter.writeStartElement("layers");
	streamWriter.writeAttribute("image",m_svgStrings->relativePath());
	streamWriter.writeStartElement("layer");
	streamWriter.writeAttribute("layerId",xmlViewLayerID());
	streamWriter.writeEndElement();

	// no layerkin were created, so use m_extraViewLayers
	// TODO: what if svg doesn't have any elements with the given layerid?

	/*
	foreach (ItemBase * lkpi, m_layerKin) {
		streamWriter.writeStartElement("layer");
		streamWriter.writeAttribute("layerId",ViewLayer::viewLayerXmlNameFromID(lkpi->viewLayerID()));
		streamWriter.writeEndElement();
	}
	*/

	foreach (ViewLayer::ViewLayerID vlid, m_extraViewLayers) {
		QString layername = ViewLayer::viewLayerXmlNameFromID(vlid);
		if (!layername.isEmpty()) {
			streamWriter.writeStartElement("layer");
			streamWriter.writeAttribute("layerId", layername);
			streamWriter.writeEndElement();
		}
	}

	streamWriter.writeEndElement();
	streamWriter.writeEndElement();
}

const QList< QPointer<Connector> > &PartsEditorPaletteItem::connectors() {
	if(m_connectors.size() == 0) {
		QList<QString> connNames = modelPart()->connectors().keys();
		qSort(connNames);
		foreach(QString connName, connNames) {
			m_connectors << modelPart()->connectors()[connName];
		}
	}
	m_connectorsTemp.clear();
	foreach (Connector * connector, m_connectors) {
		if (connector != NULL) {
			m_connectorsTemp.append(connector);
		}
	}
	return m_connectorsTemp;
}

void PartsEditorPaletteItem::setConnector(const QString &id, Connector *connector) {
	Q_UNUSED(id);
	Q_UNUSED(connector);
}

bool PartsEditorPaletteItem::setUpImage(ModelPart * modelPart, ViewLayer::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, ViewLayer::ViewLayerSpec viewLayerSpec, bool doConnectors, LayerAttributes & layerAttributes, QString & error)
{
	Q_UNUSED(viewLayerSpec);
	Q_UNUSED(error);

    ModelPartShared * modelPartShared = modelPart->modelPartShared();
    if (modelPartShared == NULL) return false;

	setViewLayerID(viewLayerID, viewLayers);

	if (m_svgStrings == NULL) {
		// TODO Mariano: Copied from paletteitembase::setUpImage (extract what's in common)
        QString filename = modelPartShared->imageFileName(viewIdentifier, viewLayerID);
		if (filename.isEmpty()) return false;


		if(!createSvgPath(modelPartShared->path(), filename)) {
			//QMessageBox::information( NULL, QObject::tr("Fritzing"),
				//					 QObject::tr("The file %1 is not a Fritzing file (6).").arg(tempPath.arg(possibleFolders[0])));
			return false;
		}
	}

    DebugDialog::debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! deal with flipsmd !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    LayerList layerList = modelPartShared->viewLayersFlipped(viewIdentifier);
    foreach (ViewLayer::ViewLayerID vlid, layerList) {
        if (vlid != viewLayerID) {
            m_extraViewLayers << vlid;
        }
    }

	FSvgRenderer * renderer = NULL;
	if (renderer == NULL) {
        QString fn = m_svgStrings->coreContribOrUser()+(!m_svgStrings->relativePath().isEmpty()?QString("/")+m_svgStrings->relativePath():QString(""));
		renderer = new FSvgRenderer();
		QByteArray loaded;
		if (!m_itemSVG.isEmpty()) {
			loaded = renderer->loadSvg(m_itemSVG.toUtf8(), m_svgStrings->absolutePath(), false);
		}
		if (loaded.isEmpty()) {
			loaded = renderer->loadSvg(m_svgStrings->absolutePath());
		}
		if (loaded.isEmpty()) {
			QMessageBox::information( NULL, QObject::tr("Fritzing"),
					QObject::tr("The file %1 is not a Fritzing file (7).").arg(m_svgStrings->absolutePath()));
			delete renderer;
			return false;
		}

		createSvgFile(m_svgStrings->absolutePath());
	}

	this->setZValue(this->z());

	this->setSharedRendererEx(renderer);

	m_svg = true;

	if (doConnectors) {
		setUpConnectors(renderer, modelPartShared->ignoreTerminalPoints());
	}

	return true;
}
SvgAndPartFilePath* PartsEditorPaletteItem::svgFilePath() {
	return m_svgStrings;
}

void PartsEditorPaletteItem::setSvgFilePath(SvgAndPartFilePath *path) {
	if(m_shouldDeletePath && m_svgStrings) {
		delete m_svgStrings;
	}
	m_shouldDeletePath = false;
	m_svgStrings = path;
}

QDomDocument *PartsEditorPaletteItem::svgDom() {
	return m_svgDom;
}

QString PartsEditorPaletteItem::flatSvgFilePath() {
	return m_svgStrings->absolutePath();
}

ConnectorItem* PartsEditorPaletteItem::newConnectorItem(Connector *connector) {
	return new PartsEditorConnectorItem(connector,this);
}

LayerKinPaletteItem * PartsEditorPaletteItem::newLayerKinPaletteItem(PaletteItemBase * chief, ModelPart * modelPart, 
																	 ViewLayer::ViewIdentifier viewIdentifier,
																	 const ViewGeometry & viewGeometry, long id,
																	 ViewLayer::ViewLayerID viewLayerID, 
																	 ViewLayer::ViewLayerSpec viewLayerSpec,
																	 QMenu* itemMenu, const LayerHash & viewLayers)
{
	LayerKinPaletteItem *lk = new
                PartsEditorLayerKinPaletteItem(chief, modelPart, viewIdentifier, viewGeometry, id, itemMenu);
	lk->init(viewLayerID, viewLayerSpec, viewLayers);
	return lk;
}


QString PartsEditorPaletteItem::xmlViewLayerID() {
	ViewLayer::ViewLayerID viewLayerIDAux = m_viewLayerID == ViewLayer::UnknownLayer
		? SketchWidget::defaultConnectorLayer(m_viewIdentifier)
		: m_viewLayerID;
	return ViewLayer::viewLayerXmlNameFromID(viewLayerIDAux);
}

void PartsEditorPaletteItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	setCursor(QCursor(Qt::ArrowCursor));
	QGraphicsSvgItem::hoverEnterEvent(event);
}

void PartsEditorPaletteItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsSvgItem::hoverLeaveEvent(event);
}

void PartsEditorPaletteItem::setItemSVG(const QString & itemSVG) {
	m_itemSVG = itemSVG;
}
