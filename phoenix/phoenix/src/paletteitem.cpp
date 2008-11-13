/*
 * (c) Fachhochschule Potsdam
 */

#include "paletteitem.h"
#include "debugdialog.h"
#include "viewgeometry.h"
#include "infographicsview.h"
#include "layerkinpaletteitem.h"
#include "rendererviewthing.h"
#include "layerattributes.h"
#include "connectorstuff.h"

#include <math.h>

#include <QGraphicsSceneMouseEvent>
#include <QSvgRenderer>
#include <QtDebug>
#include <QPainter>
#include <QDomElement>
#include <QDir>
#include <QMessageBox>

PaletteItem::PaletteItem( ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu)
	: PaletteItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu, true)
{
	m_pixmap = NULL;
}

bool PaletteItem::renderImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool renderPixmap, bool doConnectors) {
	Q_UNUSED(viewLayerID);

	bool result = setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, doConnectors);
    if (result && renderPixmap) {
		RendererViewThing * viewThing = dynamic_cast<RendererViewThing *>(modelPart->modelPartStuff()->viewThing());
		if (viewThing != NULL) {
			QSvgRenderer * renderer = viewThing->get((long)  /*m_viewIdentifier*/  m_viewLayerID);
			if (renderer != NULL) {
				m_pixmap = new QPixmap(m_size);
				m_pixmap->fill(Qt::transparent);
				QPainter painter(m_pixmap);
				renderer->render(&painter);
				painter.end();
			}
		}
   	}

	m_syncMoved = this->pos();
	return result;
}

void PaletteItem::loadLayerKin( const LayerHash & viewLayers) {

	if (m_modelPart == NULL) return;

	ModelPartStuff * modelPartStuff = m_modelPart->modelPartStuff();
	if (modelPartStuff == NULL) return;
	if (modelPartStuff->domDocument() == NULL) return;

	qint64 id = m_id + 1;
	ViewGeometry viewGeometry = m_viewGeometry;
	viewGeometry.setZ(-1);

	// the palette item already used the zeroth child "layer" element
	foreach (ViewLayer::ViewLayerID viewLayerID, viewLayers.keys()) {
		if (viewLayerID == m_viewLayerID) continue;

		LayerKinPaletteItem * lkpi = new LayerKinPaletteItem(this, m_modelPart, m_viewIdentifier, viewGeometry, id, viewLayerID, m_itemMenu, viewLayers);
		if (lkpi->ok()) {
			DebugDialog::debug(QString("add layer kin %1 %2").arg(id).arg(m_viewIdentifier) );
			addLayerKin(lkpi);
			id++;
		}
		else {
			delete lkpi;
		}
	}
}

QPixmap * PaletteItem::pixmap() const {
	return m_pixmap;
}

void PaletteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	PaletteItemBase::paint(painter, option, widget);
	if (!m_svg && (m_pixmap != NULL)) {
		painter->drawPixmap ( 0, 0, *m_pixmap );
	}

}


void PaletteItem::addLayerKin(LayerKinPaletteItem * lkpi) {
	m_layerKin.append(lkpi);
}

void PaletteItem::removeLayerKin() {
	// assumes paletteitem is still in scene
	for (int i = 0; i < m_layerKin.size(); i++) {
		DebugDialog::debug(QObject::tr("removing kin %1 %2").arg(m_layerKin[i]->id()).arg(m_layerKin[i]->z()));
		this->scene()->removeItem(m_layerKin[i]);
	}

	m_layerKin.clear();
	// free the items?
}

void PaletteItem::syncKinSelection(bool selected, PaletteItemBase * originator) {
	PaletteItemBase::syncKinSelection(selected, originator);

	for (int i = 0; i < m_layerKin.count(); i++) {
		if (m_layerKin[i] != originator && m_layerKin[i]->isSelected() != selected) {
				m_layerKin[i]->blockItemSelectedChange(selected);
				m_layerKin[i]->setSelected(selected);
		}
	}

	if (this != originator && this->isSelected() != selected) {
		this->blockItemSelectedChange(selected);
		this->setSelected(selected);
	}
}

QVariant PaletteItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//DebugDialog::debug(QString("chief item change %1 %2").arg(this->id()).arg(change));
	if (m_layerKin.count() > 0) {
	    if (change == ItemSelectedChange) {
	       	bool selected = value.toBool();
	    	if (m_blockItemSelectedChange && m_blockItemSelectedValue == selected) {
	    		m_blockItemSelectedChange = false;
	   		}
			else {
	       		syncKinSelection(selected, this);
			}
	    }
	    //else if (change == ItemVisibleHasChanged && value.toBool()) {
	    	//this->setSelected(syncSelected());
	    	//this->setPos(m_offset + syncMoved());
	    //}
	    else if (change == ItemPositionHasChanged) {
	    	this->syncKinMoved(this->m_offset, value.toPointF());
	   	}
   	}

    return PaletteItemBase::itemChange(change, value);
}

QList<class LayerKinPaletteItem *> & PaletteItem::layerKin()
{
	return m_layerKin;
}

void PaletteItem::rotateItemAnd(qreal degrees) {
	this->rotateItem(degrees);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->rotateItem(degrees);
	}
}

void PaletteItem::flipItemAnd(Qt::Orientations orientation) {
	this->flipItem(orientation);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->flipItem(orientation);
	}
}


void PaletteItem::restoreConnections(QDomElement & instance, QHash<long, ItemBase *> & newItems) {
	PaletteItemBase::restoreConnections(instance, newItems);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->restoreConnections(instance, newItems);
	}
}

void PaletteItem::setTransforms() {
	setTransform(getViewGeometry().transform());
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setTransform(m_layerKin[i]->getViewGeometry().transform());
	}
}

void PaletteItem::moveItem(ViewGeometry & viewGeometry) {
	PaletteItemBase::moveItem(viewGeometry);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->moveItem(viewGeometry);
	}
}

void PaletteItem::setItemPos(QPointF & loc) {
	PaletteItemBase::setItemPos(loc);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setItemPos(loc);
	}
}

ItemBase * PaletteItem::layerKinChief() {
	return this;
}

void PaletteItem::sendConnectionChangedSignal(ConnectorItem * from, ConnectorItem * to, bool connect) {
	Q_UNUSED(connect);
	emit connectionChangedSignal(from, to, true);
}

void PaletteItem::updateConnections() {
	updateConnectionsAux();
	foreach (LayerKinPaletteItem * lkpi, m_layerKin) {
		lkpi->updateConnectionsAux();
	}
}

void PaletteItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	DebugDialog::debug("layerkinchief got mouse press event");
	if (isBuriedConnectorHit(event)) return;

	foreach(LayerKinPaletteItem * lkpi, m_layerKin) {
		if (lkpi->isBuriedConnectorHit(event)) return;
	}

	PaletteItemBase::mousePressEvent(event);
}


void PaletteItem::syncKinMoved(QPointF offset, QPointF newPos) {
	Q_UNUSED(offset);    // ignore offset--should all be zeros now

	//DebugDialog::debug(QString("sync kin moved %1 %2").arg(offset.x()).arg(offset.y()) );
	//m_syncMoved = pos - offset;
	//if (newPos != pos()) {
		setPos(newPos);
		foreach (LayerKinPaletteItem * lkpi, m_layerKin) {
			lkpi->setPos(newPos);
		}
	//}
}

void PaletteItem::setInstanceTitleAndTooltip(const QString& title) {
	ItemBase::setInstanceTitleAndTooltip(title);
	foreach (LayerKinPaletteItem * lkpi, m_layerKin) {
		lkpi->setInstanceTitleAndTooltip(title);
	}
}

void PaletteItem::updateTooltip() {
	ItemBase::updateTooltip();
	foreach (LayerKinPaletteItem * lkpi, m_layerKin) {
		lkpi->updateTooltip();
	}
}

QString PaletteItem::family() {
	return modelPartStuff()->family();
}

bool PaletteItem::swap(PaletteItem* other, const LayerHash &layerHash) {
	return swap(other->modelPart(), layerHash);
}

bool PaletteItem::swap(ModelPart* newModelPart, const LayerHash &layerHash) {
	bool sameFamily = family() == newModelPart->modelPartStuff()->family();
	if(sameFamily) {
		invalidateConnectors();
		m_modelPart->copy(newModelPart, true);
		renderImage(m_modelPart,m_viewIdentifier,layerHash,m_viewLayerID,false,true);
		cleanupConnectors();
		updateTooltip();
	}
	return sameFamily;
}

void PaletteItem::invalidateConnectors() {
	foreach(QGraphicsItem* child, childItems()) {
		ConnectorItem *conn = dynamic_cast<ConnectorItem*>(child);
		if(conn) {
			conn->setDirty(true);
		}
	}
}

void PaletteItem::cleanupConnectors() {
//	RendererViewThing * viewThing = dynamic_cast<RendererViewThing *>(modelPart()->modelPartStuff()->viewThing());
//	QSvgRenderer * renderer = NULL;
//	if (viewThing != NULL) {
//		renderer = viewThing->get((long)  /*m_viewIdentifier*/  m_viewLayerID);
//	} else {
//		return;
//	}
//	foreach(QGraphicsItem* child, childItems()) {
//		ConnectorItem *connIt = dynamic_cast<ConnectorItem*>(child);
//		if(connIt) {
//			QRectF connectorRect;
//			QPointF terminalPoint;
//
//			connIt->connector()->setUpConnector(renderer, m_viewIdentifier, m_viewLayerID, connectorRect, terminalPoint, false);
//			connIt->setRect(connectorRect);
//			connIt->setTerminalPoint(terminalPoint);
//
//			connIt->adjustConnectedItems();
//		}
//	}


	QList<ConnectorItem*> newOnes;
	QHash<QString /*name*/, ConnectorItem*> oldOnes;

	foreach(QGraphicsItem* child, childItems()) {
		ConnectorItem *conn = dynamic_cast<ConnectorItem*>(child);
		if(conn) {
			if(conn->isDirty()) {
				oldOnes[conn->connector()->connectorStuff()->name()] = conn;
			} else {
				newOnes << conn;
			}
		}
	}

	foreach(ConnectorItem* newOne, newOnes) {
		QString name = newOne->connector()->connectorStuff()->name();
		ConnectorItem *oldOne = oldOnes[name];
		if(oldOne) {
			foreach(ConnectorItem* oldConnectedTo, oldOne->connectedToItems()) {
				oldOne->tempRemove(oldConnectedTo);
				oldConnectedTo->tempRemove(oldOne);
				newOne->tempConnectTo(oldConnectedTo);
				oldConnectedTo->tempConnectTo(newOne);

				newOne->adjustConnectedItems();
			}
			oldOnes.remove(name);
			scene()->removeItem(oldOne);
			delete oldOne;
		} else {
			// nothing to do with this one
		}
	}

	// not working old ones
	foreach(QString name, oldOnes.keys()) {
		ConnectorItem *toRemove = oldOnes[name];
		if(toRemove) {
			scene()->removeItem(toRemove);
			delete toRemove;
		}
	}
}
