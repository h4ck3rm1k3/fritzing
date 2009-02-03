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


#include <QInputDialog>

#include "partseditorconnectorsview.h"
#include "../fsvgrenderer.h"
#include "../fritzingwindow.h"
#include "../debugdialog.h"

int PartsEditorConnectorsView::ConnDefaultWidth = 5;
int PartsEditorConnectorsView::ConnDefaultHeight = ConnDefaultWidth;

PartsEditorConnectorsView::PartsEditorConnectorsView(ItemBase::ViewIdentifier viewId, bool showingTerminalPoints, QWidget *parent, int size)
	: PartsEditorAbstractView(viewId, parent, size)
{
	m_showingTerminalPoints = showingTerminalPoints;
	m_lastSelectedConnId = "";

	m_zoomControls = new ZoomControls(this);
	addFixedToBottomRightItem(m_zoomControls);

	setDragMode(QGraphicsView::ScrollHandDrag);

	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

void PartsEditorConnectorsView::wheelEvent(QWheelEvent* event) {
	SketchWidget::wheelEvent(event);
}

void PartsEditorConnectorsView::mousePressEvent(QMouseEvent *event) {
	PartsEditorAbstractView::mousePressEvent(event);
}

void PartsEditorConnectorsView::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
}

void PartsEditorConnectorsView::mouseReleaseEvent(QMouseEvent *event) {
	PartsEditorAbstractView::mouseReleaseEvent(event);
}

void PartsEditorConnectorsView::drawConector(Connector *conn, bool showTerminalPoint) {
	QSize size(ConnDefaultWidth,ConnDefaultHeight);
	createConnector(conn,size,showTerminalPoint);
}

void PartsEditorConnectorsView::createConnector(Connector *conn, const QSize &connSize, bool showTerminalPoint) {
	Q_ASSERT(m_item);
	QString connId = conn->connectorStuffID();

	QRectF bounds = QRectF(m_item->boundingRect().center(),connSize);
	PartsEditorConnectorsConnectorItem *connItem = new PartsEditorConnectorsConnectorItem(conn, m_item, m_showingTerminalPoints, bounds);
	m_drawnConns << connItem;
	connItem->setShowTerminalPoint(showTerminalPoint);

	m_undoStack->push(new QUndoCommand(
		QString("connector '%1' added to %2 view")
		.arg(connId).arg(ItemBase::viewIdentifierName(m_viewIdentifier))
	));
}

void PartsEditorConnectorsView::removeConnector(const QString &connId) {
	PartsEditorConnectorsConnectorItem *connToRemove = NULL;
	foreach(QGraphicsItem *item, items()) {
		PartsEditorConnectorsConnectorItem *connItem = dynamic_cast<PartsEditorConnectorsConnectorItem*>(item);
		if(connItem && connItem->connector()->connectorStuffID() == connId) {
			connToRemove = connItem;
			break;
		}
	}

	if(connToRemove) {
		scene()->removeItem(connToRemove);
		scene()->update();
		m_undoStack->push(new QUndoCommand(
			QString("connector '%1' removed from %2 view")
			.arg(connId).arg(ItemBase::viewIdentifierName(m_viewIdentifier))
		));
		m_drawnConns.removeAll(connToRemove);
		m_removedConnIds << connId;
	}
}

void PartsEditorConnectorsView::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	PartsEditorAbstractView::loadFromModel(paletteModel, modelPart);
	setItemProperties();
}

void PartsEditorConnectorsView::addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath) {
	QString imagePath = svgFilePath->first+(svgFilePath->second != ___emptyString___ ? "/" : "")+svgFilePath->second;
	if(!modelPart) {
		modelPart = createFakeModelPart(imagePath, svgFilePath->second);
	}

	PartsEditorAbstractView::addItemInPartsEditor(modelPart,svgFilePath);
	setItemProperties();
	//m_item->removeFromModel();

	emit connectorsFound(this->m_viewIdentifier,m_item->connectors());

	//m_item->highlightConnectors("");
}

void PartsEditorConnectorsView::setItemProperties() {
	if(m_item) {
		m_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
		m_item->setFlag(QGraphicsItem::ItemIsMovable, false);
		m_item->setFlag(QGraphicsItem::ItemClipsToShape, true);
		//m_item->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
		m_item->removeFromModel();
		myItem()->highlightConnectors(m_lastSelectedConnId);

		qreal size = 500; // just make sure the user get enough place to play
		setSceneRect(0,0,size,size);

		m_item->setPos((size-m_item->size().width())/2,(size-m_item->size().height())/2);
		centerOn(m_item);
	}
	ensureFixedToBottomRight(m_zoomControls);
}

void PartsEditorConnectorsView::informConnectorSelection(const QString &connId) {
	if(m_item) {
		m_lastSelectedConnId = connId;
		myItem()->highlightConnectors(connId);
	}
}

void PartsEditorConnectorsView::informConnectorSelectionFromView(const QString &connId) {
	informConnectorSelection(connId);
	emit connectorSelected(connId);
}

void PartsEditorConnectorsView::setMismatching(ItemBase::ViewIdentifier viewId, const QString &id, bool mismatching) {
	if(m_item && viewId == m_viewIdentifier) {
		for (int i = 0; i < m_item->childItems().count(); i++) {
			PartsEditorConnectorsConnectorItem * connectorItem
				= dynamic_cast<PartsEditorConnectorsConnectorItem *>(m_item->childItems()[i]);
			if(connectorItem == NULL) continue;

			if(connectorItem->connector()->connectorStuffID() == id) {
				connectorItem->setMismatching(mismatching);
			}
		}
	}
}

void PartsEditorConnectorsView::updateDomIfNeeded() {
	if(m_item) {
		FSvgRenderer *renderer = new FSvgRenderer();
		if(renderer->load(m_item->flatSvgFilePath())) {
			QRectF viewBox = renderer->viewBoxF();
			QSizeF defaultSize = renderer->defaultSizeF();
			QDomDocument *svgDom = m_item->svgDom();

			//bool somethingChanged = addConnectorsIfNeeded(svgDom, defaultSize, viewBox);
			bool somethingChanged = removeConnectorsIfNeeded(svgDom);
			somethingChanged |= addConnectorsIfNeeded(svgDom, defaultSize, viewBox);

			if(somethingChanged) {
				QString tempFile = QDir::tempPath()+"/"+FritzingWindow::getRandText()+".svg";
				QFile file(tempFile);
				Q_ASSERT(file.open(QFile::WriteOnly));
				QTextStream out(&file);
				out << svgDom->toString();
				file.close();

				emit svgFileLoadNeeded(tempFile);
				QFile::remove(tempFile);
			}
		} else {
			DebugDialog::debug("updating part view svg file: could not load file "+m_item->flatSvgFilePath());
		}
	}
}

bool PartsEditorConnectorsView::addConnectorsIfNeeded(QDomDocument *svgDom, const QSizeF &defaultSize, const QRectF &viewBox) {
	if(!m_drawnConns.isEmpty()) {
		DebugDialog::debug(QString("<<<< dsW %1  dsH %2  vbW %3  vbH %4")
				.arg(defaultSize.width()).arg(defaultSize.height())
				.arg(viewBox.width()).arg(viewBox.height()));

		QRectF bounds;
		QString connId;

		foreach(PartsEditorConnectorsConnectorItem* drawnConn, m_drawnConns) {
			/*QRectF rectAux = drawnConn->mapToParent(drawnConn->boundingRect()).boundingRect();
			QPointF posAux = drawnConn->m_handlers->mapToItem(m_item,QPointF(rectAux.x(),rectAux.y()));
			QPointF posAux2 = posAux;
			DebugDialog::debug(QString("<<<<< pos %1 %2 \n").arg(posAux2.x()).arg(posAux2.y()));
			qreal xAux = posAux.x();
			qreal yAux = posAux.y();
			bounds = QRectF(xAux, yAux, rectAux.width(), rectAux.height());*/

			bounds = drawnConn->mappedRect();//mapToParent(drawnConn->boundingRect()).boundingRect();
			DebugDialog::debug(QString("<<< x=%1 y=%2 w=%3 h=%4")
					.arg(bounds.x()).arg(bounds.y()).arg(bounds.width()).arg(bounds.height())
			);
			connId = drawnConn->connectorStuffID();

			QRectF svgRect = mapFromSceneToSvg(bounds,defaultSize,viewBox);
			addRectToSvg(svgDom,connId/*+"pin"*/,svgRect);

			/*TerminalPointItem *tp = drawnConn->terminalPointItem();
			if(tp && tp->hasBeenMoved()) {
				QRectF rectTPAux = tp->boundingRect();
				QPointF posTPAux = QPointF(
					rectTPAux.x()+rectTPAux.width()/2,
					rectTPAux.y()+rectTPAux.height()/2
				);
				qreal halfTPSize = 0.001; // a tiny rectangle
				QRectF tpointRect(
					tp->mapToParent(posTPAux).x()-halfTPSize,
					tp->mapToParent(posTPAux).y()-halfTPSize,
					halfTPSize*2, halfTPSize*2
				);
				QRectF svgTpRect = mapFromSceneToSvg(tpointRect,defaultSize,viewBox);
				addRectToSvg(svgDom,connId+"terminal",svgTpRect);
			}*/
		}
		return true;
	}
	return false;
}

bool PartsEditorConnectorsView::removeConnectorsIfNeeded(QDomDocument *svgDom) {
	if(!m_removedConnIds.isEmpty()) {
		QDomElement docEle = svgDom->documentElement();
		Q_ASSERT(docEle.tagName() == "svg");
		QString result;
		QList<QDomNode> nodesToRemove;
		for (int i = 0; i < docEle.childNodes().count(); ++i) {
			QDomNode n = docEle.childNodes().at(i);
			if (n.nodeType() == QDomNode::ElementNode) {
				if (isSupposedToBeRemoved(n.toElement().attribute("id"))) {
					docEle.removeChild(n);
					continue;
				}

				QDomNodeList children = n.toElement().childNodes();
				for (int c = 0; c < children.count(); ++c) {
					QDomNode child = children.at(c);
					if (child.nodeType() == QDomNode::ElementNode
						&& isSupposedToBeRemoved(child.toElement().attribute("id"))) {
						n.removeChild(child);
						continue;
					}
				}
			}
		}
		return true;
	}
	return false;
}

QRectF PartsEditorConnectorsView::mapFromSceneToSvg(const QRectF &sceneRect, const QSizeF &defaultSize, const QRectF &viewBox) {
	qreal relationW = defaultSize.width() / viewBox.width();
	qreal relationH = defaultSize.height() / viewBox.height();

	qreal x = sceneRect.x() * relationW;
	qreal y = sceneRect.y() * relationH;
	qreal width = sceneRect.width() * relationW;
	qreal height = sceneRect.height() * relationH;

	return QRectF(x,y,width,height);
}

void PartsEditorConnectorsView::addRectToSvg(QDomDocument* svgDom, const QString &id, const QRectF &rect) {
	QDomElement connElem = svgDom->createElement("rect");
	connElem.setAttribute("id",id);
	connElem.setAttribute("x",rect.x());
	connElem.setAttribute("y",rect.y());
	connElem.setAttribute("width",rect.width());
	connElem.setAttribute("height",rect.height());
	connElem.setAttribute("fill","none");
	Q_ASSERT(!svgDom->firstChildElement("svg").isNull());
	svgDom->firstChildElement("svg").appendChild(connElem);
}


bool PartsEditorConnectorsView::isSupposedToBeRemoved(const QString& id) {
	foreach(QString toBeRemoved, m_removedConnIds) {
		if(id.startsWith(toBeRemoved)) {
			return true;
		}
	}
	return false;
}

PartsEditorConnectorsPaletteItem *PartsEditorConnectorsView::myItem() {
	return dynamic_cast<PartsEditorConnectorsPaletteItem*>(m_item);
}

void PartsEditorConnectorsView::showTerminalPoints(bool show) {
	m_showingTerminalPoints = show;
	foreach(QGraphicsItem *item, items()) {
		PartsEditorConnectorsConnectorItem *connItem
			= dynamic_cast<PartsEditorConnectorsConnectorItem*>(item);
		if(connItem) {
			connItem->setShowTerminalPoint(show);
		}
	}
}

bool PartsEditorConnectorsView::showingTerminalPoints() {
	return m_showingTerminalPoints;
}

PartsEditorPaletteItem *PartsEditorConnectorsView::newPartsEditorPaletteItem(ModelPart *modelPart) {
	return new PartsEditorConnectorsPaletteItem(this, modelPart, m_viewIdentifier);
}

PartsEditorPaletteItem *PartsEditorConnectorsView::newPartsEditorPaletteItem(ModelPart * modelPart, StringPair *path) {
	return new PartsEditorConnectorsPaletteItem(this, modelPart, m_viewIdentifier, path, ItemBase::viewIdentifierNaturalName(m_viewIdentifier));
}

void PartsEditorConnectorsView::inFileDefinedConnectorChanged(PartsEditorConnectorsConnectorItem *connItem) {
	m_drawnConns << connItem;
	m_removedConnIds << connItem->connector()->connectorStuffID();
}
