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



#include "partseditorabstractview.h"
#include "partseditorconnectoritem.h"
#include "../debugdialog.h"


PartsEditorAbstractView::PartsEditorAbstractView(ItemBase::ViewIdentifier viewId, QDir tempDir, QWidget *parent, int size)
	: SketchWidget(viewId, parent, size, size)
{
	m_item = NULL;
	m_tempFolder = tempDir;
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PartsEditorAbstractView::addItemInPartsEditor(ModelPart * modelPart, SvgAndPartFilePath * svgFilePath) {
	clearScene();

	m_item = newPartsEditorPaletteItem(modelPart, svgFilePath);
	this->addItem(modelPart, BaseCommand::CrossView, m_item->getViewGeometry(), m_item->id(), -1, m_item);

	fitCenterAndDeselect();
}


void PartsEditorAbstractView::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	ViewGeometry viewGeometry;
	this->setPaletteModel(paletteModel);
	m_item = (PartsEditorPaletteItem*)SketchWidget::loadFromModel(modelPart, viewGeometry);
	fitCenterAndDeselect();
}

ItemBase * PartsEditorAbstractView::addItemAux(ModelPart * modelPart, const ViewGeometry & /*viewGeometry*/, long /*id*/, PaletteItem * paletteItem, bool doConnectors) {
	if(paletteItem == NULL) {
		paletteItem = newPartsEditorPaletteItem(modelPart);
	}
	modelPart->initConnectors();    // is a no-op if connectors already in place
	return addPartItem(modelPart, paletteItem, doConnectors);
}

void PartsEditorAbstractView::fitCenterAndDeselect() {
	m_item->setSelected(false);
	m_item->setHidden(false);

	QRectF viewRect = rect();

	int zoomCorrection;
	if(m_viewIdentifier != ItemBase::IconView) {
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

void PartsEditorAbstractView::wheelEvent(QWheelEvent* /*event*/) {
	return;
}

void PartsEditorAbstractView::clearScene() {
	if(m_item) {
		deleteItem(m_item, false, true);

		//delete m_item;
		scene()->clear();
		m_item = NULL;
	}
}

void PartsEditorAbstractView::removeConnectors() {
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

ModelPart *PartsEditorAbstractView::createFakeModelPart(SvgAndPartFilePath *svgpath) {
	const QHash<QString,StringPair*> connIds = getConnectorIds(svgpath->absolutePath());
	const QStringList layers = getLayers(svgpath->absolutePath());

	QString path = svgpath->relativePath() == ___emptyString___ ? svgpath->absolutePath() : svgpath->relativePath();
	return createFakeModelPart(connIds, layers, path);
}

ModelPart *PartsEditorAbstractView::createFakeModelPart(const QHash<QString,StringPair*> &conns, const QStringList &layers, QString svgFilePath) {
	QString fakePath = svgFilePath.mid(svgFilePath.indexOf("/")+1); // remove core/user/contrib TODO Mariano: I don't like this folder thing anymore

	QDomDocument *domDoc = new QDomDocument();
	QString errorStr;
	int errorLine;
	int errorColumn;
	QString fakeFzFile =
		QString("<module><views>\n")+
			QString("<%1><layers image='%2' >\n").arg(ItemBase::viewIdentifierXmlName(m_viewIdentifier)).arg(fakePath);
		foreach(QString layer, layers) { fakeFzFile +=
			QString("    <layer layerId='%1' />\n").arg(layer);
		}
	fakeFzFile +=
			QString("</layers></%1>\n").arg(ItemBase::viewIdentifierXmlName(m_viewIdentifier))+
			QString("</views><connectors>\n");

	foreach(QString id, conns.keys()) {
		QString terminalAttr = conns[id]->second != ___emptyString___ ? QString("terminalId='%1'").arg(conns[id]->second) : "";
		fakeFzFile += QString("<connector id='%1'><views>\n").arg(id)+
				QString("<%1>\n").arg(ItemBase::viewIdentifierXmlName(m_viewIdentifier))+
				QString("<p layer='%1' svgId='%2' %3/>\n")
					.arg(ViewLayer::viewLayerXmlNameFromID(ItemBase::defaultConnectorLayer(m_viewIdentifier)))
					.arg(conns[id]->first)
					.arg(terminalAttr)+
				QString("</%1>\n").arg(ItemBase::viewIdentifierXmlName(m_viewIdentifier))+
				QString("</views></connector>\n");
	}
	fakeFzFile += QString("</connectors></module>\n");

  	domDoc->setContent(fakeFzFile, &errorStr, &errorLine, &errorColumn);

  	ModelPart *retval = m_sketchModel->root();
  	retval->modelPartStuff()->setDomDocument(domDoc);
  	//retval->setParent(m_sketchModel->root());
  	retval->initConnectors(true /*redo connectors*/);
	return retval;
}

const QHash<QString,StringPair*> PartsEditorAbstractView::getConnectorIds(const QString &path) {
	QDomDocument *dom = new QDomDocument();
	QFile file(path);
	dom->setContent(&file);
	file.close();

	QHash<QString,StringPair*> retval;
	QDomElement docElem = dom->documentElement();
	getConnectorIdsAux(retval, docElem);

	return retval;
}

void PartsEditorAbstractView::getConnectorIdsAux(QHash<QString/*connectorId*/,StringPair*> &retval, QDomElement &docElem) {
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

const QStringList PartsEditorAbstractView::getLayers(const QString &path) {
	QDomDocument *dom = new QDomDocument();
	QFile file(path);
	dom->setContent(&file);
	file.close();

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

	if(retval.isEmpty()) {
		retval << ItemBase::viewIdentifierNaturalName(m_viewIdentifier);
	}

	return retval;
}

PartsEditorPaletteItem *PartsEditorAbstractView::newPartsEditorPaletteItem(ModelPart *modelPart) {
	return new PartsEditorPaletteItem(this, modelPart, m_viewIdentifier);
}

PartsEditorPaletteItem *PartsEditorAbstractView::newPartsEditorPaletteItem(ModelPart *modelPart, SvgAndPartFilePath *path) {
	return new PartsEditorPaletteItem(this, modelPart, m_viewIdentifier, path);
}

QDir PartsEditorAbstractView::tempFolder() {
	return m_tempFolder;
}

QString PartsEditorAbstractView::getOrCreateViewFolderInTemp() {
	QString retval = ItemBase::viewIdentifierNaturalName(m_viewIdentifier);

	if(!QFileInfo(m_tempFolder.path()+"/"+retval).exists()) {
		Q_ASSERT(m_tempFolder.mkdir(retval));
	}

	return retval;
}
