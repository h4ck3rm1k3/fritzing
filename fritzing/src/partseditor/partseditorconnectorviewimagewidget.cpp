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



//#include <QXmlQuery>

#include "partseditorconnectorviewimagewidget.h"
#include "partseditorconnectoritem.h"
#include "../debugdialog.h"

PartsEditorConnectorViewImageWidget::PartsEditorConnectorViewImageWidget(ItemBase::ViewIdentifier viewId, QWidget *parent, int size)
	: PartsEditorAbstractViewImage(viewId, parent, size)
{
	m_connRubberBandOrigin = QPoint(-1,-1);
	m_connRubberBand = NULL;
}

void PartsEditorConnectorViewImageWidget::mousePressEvent(QMouseEvent *event) {
	m_connRubberBandOrigin = event->pos();
	if (!m_connRubberBand) {
		m_connRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	}

	m_connRubberBand->setGeometry(QRect(m_connRubberBandOrigin, QSize()));
	m_connRubberBand->show();
}

void PartsEditorConnectorViewImageWidget::mouseMoveEvent(QMouseEvent *event) {
	if(m_connRubberBandOrigin != QPoint(-1,-1)) {
		m_connRubberBand->setGeometry(QRect(m_connRubberBandOrigin, event->pos()).normalized());
	}
	return;
}

void PartsEditorConnectorViewImageWidget::mouseReleaseEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	m_connRubberBand->hide();
    createConnector(m_connRubberBand->geometry());
    m_connRubberBandOrigin = QPoint(-1,-1);
}

void PartsEditorConnectorViewImageWidget::createConnector(const QRect &connRect) {
	QGraphicsRectItem *rect = new QGraphicsRectItem(mapToScene(connRect).boundingRect());
	QPen pen(QColor::fromRgb(255,0,0));
	pen.setWidth(2);
	pen.setJoinStyle(Qt::MiterJoin);
	pen.setCapStyle(Qt::SquareCap);
	rect->setPen(pen);
	scene()->addItem(rect);
}

void PartsEditorConnectorViewImageWidget::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	PartsEditorAbstractViewImage::loadFromModel(paletteModel, modelPart);
	m_item->removeFromModel();
	m_item->highlightConnectors("");
}

void PartsEditorConnectorViewImageWidget::addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath) {
	if(!modelPart) {
		QString path = svgFilePath->first+(svgFilePath->second != ___emptyString___ ? "/" : "")+svgFilePath->second;
		modelPart = createFakeModelPart(path, svgFilePath->second);
	}

	PartsEditorAbstractViewImage::addItemInPartsEditor(modelPart,svgFilePath);
	m_item->removeFromModel();

	emit connectorsFound(this->m_viewIdentifier,m_item->connectors());

	m_item->highlightConnectors("");
}

void PartsEditorConnectorViewImageWidget::informConnectorSelection(const QString &connId) {
	if(m_item) {
		m_item->highlightConnectors(connId);
	}
}

void PartsEditorConnectorViewImageWidget::setMismatching(ItemBase::ViewIdentifier viewId, const QString &id, bool mismatching) {
	if(m_item && viewId == m_viewIdentifier) {
		for (int i = 0; i < m_item->childItems().count(); i++) {
			PartsEditorConnectorItem * connectorItem = dynamic_cast<PartsEditorConnectorItem *>(m_item->childItems()[i]);
			if(connectorItem == NULL) continue;

			if(connectorItem->connector()->connectorStuffID() == id) {
				connectorItem->setMismatching(mismatching);
			}
		}
	}
}
