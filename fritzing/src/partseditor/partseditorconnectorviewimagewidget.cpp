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
#include <QInputDialog>

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
	if(m_item) {
		m_connRubberBandOrigin = event->pos();
		if (!m_connRubberBand) {
			m_connRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
		}

		m_connRubberBand->setGeometry(QRect(m_connRubberBandOrigin, QSize()));
		m_connRubberBand->show();
	}
}

void PartsEditorConnectorViewImageWidget::mouseMoveEvent(QMouseEvent *event) {
	if(m_item && m_connRubberBandOrigin != QPoint(-1,-1)) {
		m_connRubberBand->setGeometry(QRect(m_connRubberBandOrigin, event->pos()).normalized());
	}
}

void PartsEditorConnectorViewImageWidget::mouseReleaseEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	if(m_item) {
		m_connRubberBand->hide();
		createConnector(m_connRubberBand->geometry());
		m_connRubberBandOrigin = QPoint(-1,-1);
	}
}

void PartsEditorConnectorViewImageWidget::createConnector(const QRect &connRect) {
	Q_ASSERT(m_item);
	bool ok;
	QString connId =
		QInputDialog::getText(
			this,
			tr("Please, give this connector an id"),
			tr("Connector id:"),
			QLineEdit::Normal,
			"connector",
			&ok);

	if(ok) {
		QRectF bounds = mapToScene(connRect).boundingRect();

		QGraphicsRectItem *rect = new QGraphicsRectItem(bounds);
		QPen pen(QColor::fromRgb(255,0,0));
		int penWidth = 1;
		pen.setWidth(penWidth);
		pen.setJoinStyle(Qt::MiterJoin);
		pen.setCapStyle(Qt::SquareCap);
		rect->setPen(pen);
		scene()->addItem(rect);

		QSvgRenderer *renderer = new QSvgRenderer(m_item->flatSvgFilePath());
		QRectF viewBox = renderer->viewBoxF();
		QSize defaultSize = renderer->defaultSize();

		qreal x = bounds.x() * defaultSize.width() / viewBox.width();
		qreal y = bounds.y() * defaultSize.height() / viewBox.height();
		qreal width = bounds.width() * defaultSize.width() / viewBox.width(); width += penWidth; //rect pen width
		qreal height = bounds.height() * defaultSize.height() / viewBox.height(); height += penWidth; //rect pen width

		QDomDocument *svgDom = m_item->svgDom();
		QDomElement connElem = svgDom->createElement("rect");
		connElem.setAttribute("id",connId+"pin");
		connElem.setAttribute("x",x);
		connElem.setAttribute("y",y);
		connElem.setAttribute("width",width);
		connElem.setAttribute("height",height);
		connElem.setAttribute("style","fill:transparent");
		Q_ASSERT(!svgDom->firstChildElement("svg").isNull());
		svgDom->firstChildElement("svg").appendChild(connElem);

		QFile file("/home/merun/out.svg");
		Q_ASSERT(file.open(QFile::WriteOnly));
		QTextStream out(&file);
		out << svgDom->toString();
		file.close();
	}
}

void PartsEditorConnectorViewImageWidget::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	PartsEditorAbstractViewImage::loadFromModel(paletteModel, modelPart);
	m_item->removeFromModel();
	m_item->highlightConnectors("");
}

void PartsEditorConnectorViewImageWidget::addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath) {
	QString imagePath = svgFilePath->first+(svgFilePath->second != ___emptyString___ ? "/" : "")+svgFilePath->second;
	if(!modelPart) {
		modelPart = createFakeModelPart(imagePath, svgFilePath->second);
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
