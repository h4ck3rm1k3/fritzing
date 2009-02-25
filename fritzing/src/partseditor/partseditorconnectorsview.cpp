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
#include <QMessageBox>
#include <QtDebug>

#include "partseditorconnectorsview.h"
#include "../fsvgrenderer.h"
#include "../fritzingwindow.h"
#include "../debugdialog.h"

int PartsEditorConnectorsView::ConnDefaultWidth = 5;
int PartsEditorConnectorsView::ConnDefaultHeight = ConnDefaultWidth;

PartsEditorConnectorsView::PartsEditorConnectorsView(ItemBase::ViewIdentifier viewId, QDir tempDir, bool showingTerminalPoints, QWidget *parent, int size)
	: PartsEditorAbstractView(viewId, tempDir, false, parent, size)
{
	m_showingTerminalPoints = showingTerminalPoints;
	m_lastSelectedConnId = "";

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
	QString connId = conn->connectorSharedID();

	QRectF bounds = m_item
			? QRectF(m_item->boundingRect().center(),connSize)
			: QRectF(scene()->itemsBoundingRect().center(),connSize);
	PartsEditorConnectorsConnectorItem *connItem = new PartsEditorConnectorsConnectorItem(conn, m_item, m_showingTerminalPoints, bounds);
	m_drawnConns << connItem;
	connItem->setShowTerminalPoint(showTerminalPoint);

	m_undoStack->push(new QUndoCommand(
		QString("connector '%1' added to %2 view")
		.arg(connId).arg(ItemBase::viewIdentifierName(m_viewIdentifier))
	));
}

void PartsEditorConnectorsView::removeConnector(const QString &connId) {
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
			.arg(connId).arg(ItemBase::viewIdentifierName(m_viewIdentifier))
		));

		PartsEditorConnectorsConnectorItem *connToRemoveAux = dynamic_cast<PartsEditorConnectorsConnectorItem*>(connToRemove);
		m_drawnConns.removeAll(connToRemoveAux);
		m_removedConnIds << connId;
	}
}

void PartsEditorConnectorsView::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	clearScene();
	PartsEditorAbstractView::loadFromModel(paletteModel, modelPart);
	setItemProperties();
}

void PartsEditorConnectorsView::addItemInPartsEditor(ModelPart * modelPart, SvgAndPartFilePath * svgFilePath) {
	Q_ASSERT(modelPart);

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

		qreal size = 500; // just make sure the user get enough space to play
		setSceneRect(0,0,size,size);

		if(m_prevTransform.isIdentity()) {
			m_item->setPos((size-m_item->size().width())/2,(size-m_item->size().height())/2);
			centerOn(m_item);
		} else {
			m_item->setTransform(m_prevTransform);
		}
	}
	//ensureFixedToBottomRight(m_zoomControls);
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

			if(connectorItem->connector()->connectorSharedID() == id) {
				connectorItem->setMismatching(mismatching);
			}
		}
	}
}

void PartsEditorConnectorsView::aboutToSave() {
	if(m_item) {
		m_prevTransform = m_item->transform();
		FSvgRenderer *renderer = new FSvgRenderer();
		if(renderer->load(m_item->flatSvgFilePath())) {
			QRectF svgViewBox = renderer->viewBoxF();
			QSizeF sceneViewBox = renderer->defaultSizeF();
			QDomDocument *svgDom = m_item->svgDom();

			QString connectorsLayerId = findConnectorLayerId(svgDom);
			QDomElement elem = svgDom->documentElement();
			bool somethingChanged = removeConnectorsIfNeeded(elem);
			somethingChanged |= updateTerminalPoints( svgDom, sceneViewBox, svgViewBox, connectorsLayerId);
			somethingChanged |= addConnectorsIfNeeded(svgDom, sceneViewBox, svgViewBox, connectorsLayerId);

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
				QTextStream out(&file);
				out << svgDom->toString();
				file.close();

				emit svgFileLoadNeeded(tempFile);
			}
		} else {
			DebugDialog::debug("updating part view svg file: could not load file "+m_item->flatSvgFilePath());
		}
	}
}

bool PartsEditorConnectorsView::addConnectorsIfNeeded(QDomDocument *svgDom, const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId) {
	bool changed = false;
	if(!m_drawnConns.isEmpty()) {
		QRectF bounds;
		QString connId;

		foreach(PartsEditorConnectorsConnectorItem* drawnConn, m_drawnConns) {
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

QString PartsEditorConnectorsView::svgIdForConnector(Connector* conn, const QString &connId) {
	foreach(SvgIdLayer *sil, conn->connectorShared()->pins().values(m_viewIdentifier)) {
		if(conn->connectorSharedID() == connId) {
			DebugDialog::debug("<<< "+sil->m_svgId);
			return sil->m_svgId;
		}
	}
	return connId;
}

bool PartsEditorConnectorsView::updateTerminalPoints(QDomDocument *svgDom, const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId) {
	QList<PartsEditorConnectorsConnectorItem*> connsWithNewTPs;
	QStringList tpIdsToRemove;
	foreach(QGraphicsItem *item, items()) {
		PartsEditorConnectorsConnectorItem *citem =
			dynamic_cast<PartsEditorConnectorsConnectorItem*>(item);
		if(citem) {
			TerminalPointItem *tp = citem->terminalPointItem();
			if(tp && tp->hasBeenMoved()) {
				connsWithNewTPs << citem;
				QString connId = citem->connector()->connectorSharedID();
				QString terminalId = connId+"terminal";
				tpIdsToRemove << terminalId;
				updateSvgIdLayer(connId, terminalId, connectorsLayerId);
			}
		}
	}
	QDomElement elem = svgDom->documentElement();
	removeTerminalPoints(tpIdsToRemove,elem);
	addNewTerminalPoints(connsWithNewTPs, svgDom, sceneViewBox, svgViewBox, connectorsLayerId);
	return !tpIdsToRemove.isEmpty();
}

void PartsEditorConnectorsView::updateSvgIdLayer(const QString &connId, const QString &terminalId, const QString &connectorsLayerId) {
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

void PartsEditorConnectorsView::removeTerminalPoints(const QStringList &tpIdsToRemove, QDomElement &docElem) {
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

void PartsEditorConnectorsView::addNewTerminalPoints(
			const QList<PartsEditorConnectorsConnectorItem*> &connsWithNewTPs, QDomDocument *svgDom,
			const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId
) {
	foreach(PartsEditorConnectorsConnectorItem* citem, connsWithNewTPs) {
		QString connId = citem->connector()->connectorSharedID();
		TerminalPointItem *tp = citem->terminalPointItem();
		Q_ASSERT(tp);
		QRectF tpointRect(tp->mappedPoint(), QPointF(0,0));
		QRectF svgTpRect = mapFromSceneToSvg(tpointRect,sceneViewBox,svgViewBox);

		qreal halfTPSize = 0.001; // a tiny rectangle
		svgTpRect.setSize(QSizeF(halfTPSize*2,halfTPSize*2));

		addRectToSvg(svgDom,connId+"terminal",svgTpRect, connectorsLayerId);
	}
}

bool PartsEditorConnectorsView::removeConnectorsIfNeeded(QDomElement &docElem) {
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

QRectF PartsEditorConnectorsView::mapFromSceneToSvg(const QRectF &itemRect, const QSizeF &sceneViewBox, const QRectF &svgViewBox) {
	qreal relationW = svgViewBox.width() / sceneViewBox.width();
	qreal relationH = svgViewBox.height() / sceneViewBox.height();

	qreal x = itemRect.x() * relationW;
	qreal y = itemRect.y() * relationH;
	qreal width = itemRect.width() * relationW;
	qreal height = itemRect.height() * relationH;

	return QRectF(x,y,width,height);
}

void PartsEditorConnectorsView::addRectToSvg(QDomDocument* svgDom, const QString &id, const QRectF &rect, const QString &connectorsLayerId) {
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

bool PartsEditorConnectorsView::addRectToSvgAux(QDomElement &docElem, const QString &connectorsLayerId, QDomElement &rectElem) {
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
	scene()->update();
}

bool PartsEditorConnectorsView::showingTerminalPoints() {
	return m_showingTerminalPoints;
}

PartsEditorPaletteItem *PartsEditorConnectorsView::newPartsEditorPaletteItem(ModelPart *modelPart) {
	return new PartsEditorConnectorsPaletteItem(this, modelPart, m_viewIdentifier);
}

PartsEditorPaletteItem *PartsEditorConnectorsView::newPartsEditorPaletteItem(ModelPart * modelPart, SvgAndPartFilePath *path) {
	return new PartsEditorConnectorsPaletteItem(this, modelPart, m_viewIdentifier, path);
}

void PartsEditorConnectorsView::inFileDefinedConnectorChanged(PartsEditorConnectorsConnectorItem *connItem) {
	m_drawnConns << connItem;
	m_removedConnIds << connItem->connector()->connectorSharedID();
}
