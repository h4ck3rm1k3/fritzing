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



#include "partseditorabstractviewimage.h"
#include "partseditorconnectoritem.h"
#include "../debugdialog.h"


PartsEditorAbstractViewImage::PartsEditorAbstractViewImage(ItemBase::ViewIdentifier viewId, QWidget *parent, int size)
	: SketchWidget(viewId, parent, size, size)
{
	m_item = NULL;
	setFixedSize(size,size);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PartsEditorAbstractViewImage::addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath) {
	Q_ASSERT(modelPart);
	clearScene();
	m_item = new PartsEditorPaletteItem(modelPart, m_viewIdentifier, svgFilePath, ItemBase::viewIdentifierNaturalName(m_viewIdentifier));
	this->addItem(modelPart, BaseCommand::CrossView, m_item->getViewGeometry(), m_item->id(), m_item);
	fitCenterAndDeselect();
}


void PartsEditorAbstractViewImage::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	ViewGeometry viewGeometry;
	this->setPaletteModel(paletteModel);
	m_item = (PartsEditorPaletteItem*)SketchWidget::loadFromModel(modelPart, viewGeometry);
	fitCenterAndDeselect();
}

ItemBase * PartsEditorAbstractViewImage::addItemAux(ModelPart * modelPart, const ViewGeometry & /*viewGeometry*/, long /*id*/, PaletteItem * paletteItem, bool doConnectors) {
	if(paletteItem == NULL) {
		paletteItem = new PartsEditorPaletteItem(modelPart, m_viewIdentifier);
	}
	modelPart->initConnectors();    // is a no-op if connectors already in place
	return addPartItem(modelPart, paletteItem, doConnectors);
}

void PartsEditorAbstractViewImage::fitCenterAndDeselect() {
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

void PartsEditorAbstractViewImage::wheelEvent(QWheelEvent* /*event*/) {
	return;
}

void PartsEditorAbstractViewImage::clearScene() {
	if(m_item) {
		m_item->removeLayerKin();
		m_item->removeFromModel();

		removeConnectors();

		scene()->removeItem(m_item);
		delete m_item;
		m_item = NULL;
	}
}

void PartsEditorAbstractViewImage::removeConnectors() {
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

ModelPart *PartsEditorAbstractViewImage::createFakeModelPart(const QString &svgpath, const QString &relativepath) {
	const QHash<QString,StringPair*> connIds = getConnectorIds(svgpath);
	const QStringList layers = getLayers(svgpath);

	return createFakeModelPart(connIds, layers, relativepath);
}

ModelPart *PartsEditorAbstractViewImage::createFakeModelPart(const QHash<QString,StringPair*> &conns, const QStringList &layers, QString svgFilePath) {
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
  	retval->initConnectors(true /*redo connectors*/);
	return retval;
}

const QHash<QString,StringPair*> PartsEditorAbstractViewImage::getConnectorIds(const QString &path) {
	QDomDocument *dom = new QDomDocument();
	QFile file(path);
	dom->setContent(&file);
	file.close();

	QHash<QString,StringPair*> retval;
	QDomElement docElem = dom->documentElement();
	getConnectorIdsAux(retval, docElem);

	return retval;
}

void PartsEditorAbstractViewImage::getConnectorIdsAux(QHash<QString/*connectorId*/,StringPair*> &retval, QDomElement &docElem) {
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

const QStringList PartsEditorAbstractViewImage::getLayers(const QString &path) {
	/*QStringList result;

	QXmlQuery query;
	query.setQuery(QString("doc('%1')/svg/g").arg(path));
	if(query.isValid()) {
		query.evaluateTo(&result);

		foreach(QString str, result) {
			DebugDialog::debug("<<< "+str);
		}
	}*/

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

	return retval;
}
