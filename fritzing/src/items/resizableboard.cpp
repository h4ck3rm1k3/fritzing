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

$Revision: 2829 $:
$Author: cohen@irascible.com $:
$Date: 2009-04-17 00:22:27 +0200 (Fri, 17 Apr 2009) $

********************************************************************/

#include "resizableboard.h"
#include "../utils/resizehandle.h"
#include "../fsvgrenderer.h"
#include "../infographicsview.h"

static QString BoardLayerTemplate = "";
static QString SilkscreenLayerTemplate = "";
static const int LineThickness = 4;

#define mm2mils(mm) (mm / 25.4 * 1000)
#define pixels2mm(p) (p / FSvgRenderer::printerScale() * 25.4)

ResizableBoard::ResizableBoard( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (BoardLayerTemplate.isEmpty()) {
		QFile file(":/resources/resizableBoard_boardLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		BoardLayerTemplate = file.readAll();
		file.close();
	}
	if (SilkscreenLayerTemplate.isEmpty()) {
		QFile file(":/resources/resizableBoard_silkscreenLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		SilkscreenLayerTemplate = file.readAll();
		file.close();
	}

	if (modelPart->moduleID().compare(ModelPart::RectangleModuleID) == 0) {
		m_resizeGripTL = new ResizeHandle(QPixmap(":/resources/images/itemselection/cornerHandlerActiveTopLeft.png"), Qt::SizeFDiagCursor, this);
		connect(m_resizeGripTL, SIGNAL(mousePressSignal(QGraphicsSceneMouseEvent *, ResizeHandle *)), this, SLOT(handleMousePressSlot(QGraphicsSceneMouseEvent *, ResizeHandle *)));
		m_resizeGripTR = new ResizeHandle(QPixmap(":/resources/images/itemselection/cornerHandlerActiveTopRight.png"), Qt::SizeBDiagCursor, this);
		connect(m_resizeGripTR, SIGNAL(mousePressSignal(QGraphicsSceneMouseEvent *, ResizeHandle *)), this, SLOT(handleMousePressSlot(QGraphicsSceneMouseEvent *, ResizeHandle *)));
		m_resizeGripBL = new ResizeHandle(QPixmap(":/resources/images/itemselection/cornerHandlerActiveBottomLeft.png"), Qt::SizeBDiagCursor, this);
		connect(m_resizeGripBL, SIGNAL(mousePressSignal(QGraphicsSceneMouseEvent *, ResizeHandle *)), this, SLOT(handleMousePressSlot(QGraphicsSceneMouseEvent *, ResizeHandle *)));
		m_resizeGripBR = new ResizeHandle(QPixmap(":/resources/images/itemselection/cornerHandlerActiveBottomRight.png"), Qt::SizeFDiagCursor, this);
		connect(m_resizeGripBR, SIGNAL(mousePressSignal(QGraphicsSceneMouseEvent *, ResizeHandle *)), this, SLOT(handleMousePressSlot(QGraphicsSceneMouseEvent *, ResizeHandle *)));
		connect(m_resizeGripTL, SIGNAL(zoomChangedSignal(qreal)), this, SLOT(handleZoomChangedSlot(qreal)));
	}
	else {
		m_resizeGripTL = m_resizeGripTR = m_resizeGripBL = m_resizeGripBR = NULL;
	}

	m_silkscreenRenderer = m_renderer = NULL;
	m_inResize = NULL;
}

ResizableBoard::~ResizableBoard() {
}


QVariant ResizableBoard::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSelectedChange:
			if (m_resizeGripBL) {
				m_resizeGripBL->setVisible(value.toBool());
				m_resizeGripBR->setVisible(value.toBool());
				m_resizeGripTL->setVisible(value.toBool());
				m_resizeGripTR->setVisible(value.toBool());
			}
			break;
		case ItemSceneHasChanged:
			if (m_resizeGripBL && this->scene()) {
				setInitialSize();
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

void ResizableBoard::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	if (m_inResize == NULL) {
		PaletteItem::mouseMoveEvent(event);
		return;
	}

	QRectF rect = boundingRect();
	rect.moveTopLeft(this->pos());

	qreal minWidth = 0.1 * FSvgRenderer::printerScale();			// .1 inch
	qreal minHeight = 0.1 * FSvgRenderer::printerScale();			// .1 inch

	qreal oldX1 = rect.x();
	qreal oldY1 = rect.y();
	qreal oldX2 = oldX1+rect.width();
	qreal oldY2 = oldY1+rect.height();
	qreal newX = event->scenePos().x() + m_inResize->resizeOffset().x();
	qreal newY = event->scenePos().y() + m_inResize->resizeOffset().y();
	QRectF newR;

	if (m_inResize == m_resizeGripBR) {
		if (newX - oldX1 < minWidth) {
			newX = oldX1 + minWidth;
		}
		if (newY - oldY1 < minHeight) {
			newY = oldY1 + minHeight;
		}
		newR.setRect(0, 0, newX - oldX1, newY - oldY1);
	}
	else if (m_inResize == m_resizeGripTL) {
		oldX2 = m_originalRect.left() + m_originalRect.width();
		oldY2 = m_originalRect.top() + m_originalRect.height();

		if (oldX2 - newX < minWidth) {
			newX = oldX2 - minWidth;
		}
		if (oldY2 - newY < minHeight) {
			newY = oldY2 - minHeight;
		}

		QPointF p(newX, newY);
		if (p != this->pos()) {
			this->setPos(p);
		}

		newR.setRect(0, 0, oldX2 - newX, oldY2 - newY);
	}
	else if (m_inResize == m_resizeGripTR) {
		if (newX - oldX1 < minWidth) {
			newX = oldX1 + minWidth;
		}

		oldY2 = m_originalRect.top() + m_originalRect.height();
		if (oldY2 - newY < minHeight) {
			newY = oldY2 - minHeight;
		}

		QPointF p(oldX1, newY);
		if (p != this->pos()) {
			this->setPos(p);
		}

		newR.setRect(0, 0, newX - oldX1, oldY2 - newY);
		QRectF tempR = newR;
		tempR.moveTopLeft(p);
		DebugDialog::debug(QString("new rect %1 %2 %3").arg(newY).arg(newR.height()).arg(newY + newR.height()));
	}
	else if (m_inResize == m_resizeGripBL) {
		oldX2 = m_originalRect.left() + m_originalRect.width();
		if (oldX2 - newX < minWidth) {
			newX = oldX2 - minWidth;
		}
		if (newY - oldY1 < minHeight) {
			newY = oldY1 + minHeight;
		}

		QPointF p(newX, oldY1);
		if (p != this->pos()) {
			this->setPos(p);
		}

		newR.setRect(0, 0, oldX2 - newX, newY - oldY1);
	}

	LayerHash lh;
	resizePixels(newR.width(), newR.height(), lh);
	event->accept();
}

void ResizableBoard::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	if (m_inResize == NULL) {
		PaletteItem::mouseReleaseEvent(event);
		return;
	}

	this->ungrabMouse();
	event->accept();
	m_inResize = NULL;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView) {
		infoGraphicsView->viewItemInfo(this);
	}
}

void ResizableBoard::handleMousePressSlot(QGraphicsSceneMouseEvent * event, ResizeHandle * resizeHandle)
{
	if (m_spaceBarWasPressed) return;

	m_originalRect = boundingRect();
	m_originalRect.moveTopLeft(this->pos());

	if (resizeHandle == m_resizeGripBR) {
		QSizeF sz = this->boundingRect().size();
		resizeHandle->setResizeOffset(this->pos() + QPointF(sz.width(), sz.height()) - event->scenePos());
	}
	else if (resizeHandle == m_resizeGripTL) {
		resizeHandle->setResizeOffset(this->pos() - event->scenePos());
	}
	else if (resizeHandle == m_resizeGripTR) {
		resizeHandle->setResizeOffset(QPointF(this->pos().x() + this->boundingRect().width(), this->pos().y())  - event->scenePos());
	}
	else if (resizeHandle == m_resizeGripBL) {
		resizeHandle->setResizeOffset(QPointF(this->pos().x(), this->pos().y() + this->boundingRect().height())  - event->scenePos());
	}

	m_inResize = resizeHandle;
	this->grabMouse();

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView) {
		setInitialSize();
		infoGraphicsView->viewItemInfo(this);
	}
}

void ResizableBoard::handleZoomChangedSlot(qreal scale) {
	Q_UNUSED(scale);
	positionGrips();
}

void ResizableBoard::positionGrips() {
	if (m_resizeGripBL == NULL) return;

	// TODO:  figure out how to position these on a rotated board

	QSizeF sz = this->boundingRect().size();
	qreal scale = m_resizeGripBL->currentScale();

	// assuming all the handles are the same size, offset to the center
	QSizeF hsz = m_resizeGripBL->boundingRect().size();
	qreal dx = hsz.width() / (scale * 2);
	qreal dy = hsz.height() / (scale * 2);

	m_resizeGripBR->setPos(sz.width() - dx, sz.height() - dy);
	m_resizeGripBL->setPos(-dx, sz.height() - dy);
	m_resizeGripTR->setPos(sz.width() - dx, -dy);
	m_resizeGripTL->setPos(-dx, -dy);
}

bool ResizableBoard::setUpImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors)
{
	bool result = PaletteItem::setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, doConnectors);
	if (result) {
		positionGrips();
	}

	return result;
}

void ResizableBoard::resizePixels(qreal w, qreal h, const LayerHash & viewLayers) {
	resizeMM(pixels2mm(w), pixels2mm(h), viewLayers);
}

void ResizableBoard::resizeMM(qreal mmW, qreal mmH, const LayerHash & viewLayers) {
	if (mmW == 0 || mmH == 0) {
		setUpImage(modelPart(), m_viewIdentifier, viewLayers, m_viewLayerID, true);
		modelPart()->setSize(QSizeF(0,0));
		// do the layerkin
		positionGrips();
		return;
	}

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	qreal milsW = mm2mils(mmW);
	qreal milsH = mm2mils(mmH);

	QString s = makeCopper0Svg(mmW, mmH, milsW, milsH);

	bool result = m_renderer->fastLoad(s.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setSize(QSizeF(mmW, mmH));

		InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
		if (infoGraphicsView) {
			infoGraphicsView->evaluateJavascript(QString("updateBoardSize(%1,%2);").arg(qRound(mmW * 10) / 10.0).arg(qRound(mmH * 10) / 10.0));
		}
	}
	//	DebugDialog::debug(QString("fast load result %1 %2").arg(result).arg(s));

	positionGrips();

	foreach (ItemBase * itemBase, m_layerKin) {
		if (itemBase->viewLayerID() == ViewLayer::Silkscreen) {
			if (m_silkscreenRenderer == NULL) {
				m_silkscreenRenderer = new FSvgRenderer(itemBase);
			}

			s = makeSilkscreenSvg(mmW, mmH, milsW, milsH);
			bool result = m_silkscreenRenderer->fastLoad(s.toUtf8());
			if (result) {
				dynamic_cast<PaletteItemBase *>(itemBase)->setSharedRenderer(m_silkscreenRenderer);
				itemBase->modelPart()->setSize(QSizeF(mmW, mmH));
			}
			break;
		}
	}
}

void ResizableBoard::loadLayerKin( const LayerHash & viewLayers) {
	PaletteItem::loadLayerKin(viewLayers);
	if (m_modelPart->size().width() != 0) {
		resizeMM(m_modelPart->size().width(), m_modelPart->size().height(), viewLayers);
	}
}

void ResizableBoard::setInitialSize() {
	QSizeF sz = modelPart()->size();
	if (sz.width() == 0) {
		// set the size so the infoGraphicsView will display the size as you drag
		sz = this->boundingRect().size();
		modelPart()->setSize(QSizeF(pixels2mm(sz.width()), pixels2mm(sz.height()))); 
	}
}

QString ResizableBoard::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	QSizeF sz = modelPart()->size();
	if (sz.width() != 0) {
		switch (viewLayerID) {
			case ViewLayer::Copper0:
				return makeCopper0Svg(sz.width(), sz.height(), mm2mils(sz.width()), mm2mils(sz.height()));
				break;
			case ViewLayer::Silkscreen:
				return makeSilkscreenSvg(sz.width(), sz.height(), mm2mils(sz.width()), mm2mils(sz.height()));
				break;
			default:
				break;
		}
	}

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

QString ResizableBoard::makeCopper0Svg(qreal mmW, qreal mmH, qreal milsW, qreal milsH) {
	return BoardLayerTemplate
		.arg(mmW).arg(mmH)			
		.arg(milsW).arg(milsH)
		.arg(milsW - LineThickness).arg(milsH - LineThickness);
}

QString ResizableBoard::makeSilkscreenSvg(qreal mmW, qreal mmH, qreal milsW, qreal milsH) {
	return SilkscreenLayerTemplate
		.arg(mmW).arg(mmH)
		.arg(milsW).arg(milsH)
		.arg(milsW - LineThickness).arg(milsH - LineThickness);
}

void ResizableBoard::rotateItem(qreal degrees) {
	// TODO: this hack only works for 90 degree rotations
	// eventually need to make this work for other angles
	// what gets screwed up is the drag handles

	if (modelPart()->moduleID().compare(ModelPart::RectangleModuleID) == 0) {
		QRectF r = this->boundingRect();
		r.moveTopLeft(pos());
		QPointF c = r.center();
		ViewGeometry vg;
		vg.setLoc(QPointF(c.x() - (r.height() / 2), c.y() - (r.width() / 2)));	
		QSizeF sz = modelPart()->size();
		LayerHash viewLayers;
		resizeMM(sz.height(), sz.width(), viewLayers);
		moveItem(vg);
	}
	else {
		PaletteItem::rotateItem(degrees);
	}
}
