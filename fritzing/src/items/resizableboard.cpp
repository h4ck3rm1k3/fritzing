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

#include "resizableboard.h"
#include "../utils/resizehandle.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"
#include "moduleidnames.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <qmath.h>

static QString BoardLayerTemplate = "";
static QString SilkscreenLayerTemplate = "";
static const int LineThickness = 4;
static const QRegExp HeightExpr("height=\\'\\d*px");

QString ResizableBoard::customShapeTranslated;

ResizableBoard::ResizableBoard( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_keepAspectRatio = false;
	m_widthEditor = m_heightEditor = NULL;

	if (BoardLayerTemplate.isEmpty()) {
		QFile file(":/resources/templates/resizableBoard_boardLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		BoardLayerTemplate = file.readAll();
		file.close();
	}
	if (SilkscreenLayerTemplate.isEmpty()) {
		QFile file(":/resources/templates/resizableBoard_silkscreenLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		SilkscreenLayerTemplate = file.readAll();
		file.close();
	}

	m_resizeGripTL = m_resizeGripTR = m_resizeGripBL = m_resizeGripBR = NULL;

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
			if (this->scene()) {
				if (hasGrips()) {
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
				if (m_resizeGripBL) {
					setInitialSize();
				}
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

bool ResizableBoard::hasGrips() {
	return modelPart()->moduleID().compare(ModuleIDNames::rectangleModuleIDName) == 0;
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

		if (m_keepAspectRatio) {
			qreal w = (newY - oldY1) * m_aspectRatio.width() / m_aspectRatio.height();
			qreal h = (newX - oldX1) * m_aspectRatio.height() / m_aspectRatio.width();
			if (qAbs(w + oldX1 - newX) <= qAbs(h + oldY1 - newY)) {
				newX = oldX1 + w;
			}
			else {
				newY = oldY1 + h;
			}
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

		if (m_keepAspectRatio) {
			qreal w = (oldY2 - newY) * m_aspectRatio.width() / m_aspectRatio.height();
			qreal h = (oldX2 - newX) * m_aspectRatio.height() / m_aspectRatio.width();
			if (qAbs(w + newX - oldX2) <= qAbs(h + newY - oldY2)) {
				oldX2 = newX + w;
			}
			else {
				oldY2 = newY + h;
			}
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

		if (m_keepAspectRatio) {
			qreal w = (oldY2 - newY) * m_aspectRatio.width() / m_aspectRatio.height();
			qreal h = (newX - oldX1) * m_aspectRatio.height() / m_aspectRatio.width();
			if (qAbs(w + newX - oldX1) <= qAbs(h + newY - oldY2)) {
				newX = oldX1 + w;
			}
			else {
				oldY2 = newY + h;
			}
		}

		newR.setRect(0, 0, newX - oldX1, oldY2 - newY);
		//DebugDialog::debug(QString("new rect %1 %2 %3").arg(newY).arg(newR.height()).arg(newY + newR.height()));
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

		if (m_keepAspectRatio) {
			qreal w = (newY - oldY1) * m_aspectRatio.width() / m_aspectRatio.height();
			qreal h = (oldX2 - newX) * m_aspectRatio.height() / m_aspectRatio.width();
			if (qAbs(w + oldX2 - newX) <= qAbs(h + oldY1 - newY)) {
				oldX2 = newX + w;
			}
			else {
				newY = oldY1 + h;
			}
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
	resizeMM(GraphicsUtils::pixels2mm(w), GraphicsUtils::pixels2mm(h), viewLayers);
}

void ResizableBoard::resizeMM(qreal mmW, qreal mmH, const LayerHash & viewLayers) {
	if (mmW == 0 || mmH == 0) {
		setUpImage(modelPart(), m_viewIdentifier, viewLayers, m_viewLayerID, true);
		modelPart()->setProp("height", QVariant());
		modelPart()->setProp("width", QVariant());
		// do the layerkin
		positionGrips();
		return;
	}

	QRectF r = this->boundingRect();
	if (qAbs(GraphicsUtils::pixels2mm(r.width()) - mmW) < .001 &&
		qAbs(GraphicsUtils::pixels2mm(r.height()) - mmH) < .001) 
	{
		positionGrips();
		return;
	}


	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	qreal milsW = GraphicsUtils::mm2mils(mmW);
	qreal milsH = GraphicsUtils::mm2mils(mmH);

	QString s = makeBoardSvg(mmW, mmH, milsW, milsH);

	bool result = m_renderer->fastLoad(s.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("width", mmW);
		modelPart()->setProp("height", mmH);

		if (m_widthEditor) {
			m_widthEditor->setText(QString::number(qRound(mmW * 10) / 10.0));
		}
		if (m_heightEditor) {
			m_heightEditor->setText(QString::number(qRound(mmH * 10) / 10.0));
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
				itemBase->modelPart()->setProp("width", mmW);
				itemBase->modelPart()->setProp("height", mmH);
			}
			break;
		}
	}
}

void ResizableBoard::loadLayerKin( const LayerHash & viewLayers, const LayerList & notLayers) {
	PaletteItem::loadLayerKin(viewLayers, notLayers);
	qreal w = m_modelPart->prop("width").toDouble();
	if (w != 0) {
		resizeMM(w, m_modelPart->prop("height").toDouble(), viewLayers);
	}
}

void ResizableBoard::setInitialSize() {
	qreal w = m_modelPart->prop("width").toDouble();
	if (w == 0) {
		// set the size so the infoGraphicsView will display the size as you drag
		QSizeF sz = this->boundingRect().size();
		modelPart()->setProp("width", GraphicsUtils::pixels2mm(sz.width())); 
		modelPart()->setProp("height", GraphicsUtils::pixels2mm(sz.height())); 
	}
}

QString ResizableBoard::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	qreal w = m_modelPart->prop("width").toDouble();
	if (w != 0) {
		qreal h = m_modelPart->prop("height").toDouble();
		QString xml;
		switch (viewLayerID) {
			case ViewLayer::Board:
				xml = makeBoardSvg(w, h, GraphicsUtils::mm2mils(w), GraphicsUtils::mm2mils(h));
				break;
			case ViewLayer::Silkscreen:
				xml = makeSilkscreenSvg(w, h, GraphicsUtils::mm2mils(w), GraphicsUtils::mm2mils(h));
				break;
			default:
				break;
		}

		if (!xml.isEmpty()) {
			QString xmlName = ViewLayer::viewLayerXmlNameFromID(viewLayerID);
			SvgFileSplitter splitter;
			bool result = splitter.splitString(xml, xmlName);
			if (!result) {
				return "";
			}
			result = splitter.normalize(dpi, xmlName, blackOnly);
			if (!result) {
				return "";
			}
			return splitter.elementString(xmlName);
		}
	}

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

QString ResizableBoard::makeBoardSvg(qreal mmW, qreal mmH, qreal milsW, qreal milsH) {
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

	if (modelPart()->moduleID().compare(ModuleIDNames::rectangleModuleIDName) == 0) {
		if (degrees == 90 || degrees == -90) {
			QRectF r = this->boundingRect();
			r.moveTopLeft(pos());
			QPointF c = r.center();
			ViewGeometry vg;
			vg.setLoc(QPointF(c.x() - (r.height() / 2.0), c.y() - (r.width() / 2.0)));	
			qreal w = m_modelPart->prop("width").toDouble();
			qreal h = m_modelPart->prop("height").toDouble();
			LayerHash viewLayers;
			resizeMM(h, w, viewLayers);
			moveItem(vg);
		}
	}
	else {
		PaletteItem::rotateItem(degrees);
	}
}

void ResizableBoard::saveParams() {
	qreal w = modelPart()->prop("width").toDouble();
	qreal h = modelPart()->prop("height").toDouble();
	m_boardSize = QSizeF(w, h);
	m_boardPos = pos();
}

void ResizableBoard::getParams(QPointF & p, QSizeF & s) {
	p = m_boardPos;
	s = m_boardSize;
}

bool ResizableBoard::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

bool ResizableBoard::collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue) 
{
	bool result = PaletteItem::collectExtraInfoHtml(family, prop, value, swappingEnabled, returnProp, returnValue);

	if (prop.compare("shape", Qt::CaseInsensitive) == 0) {
		returnValue.replace(HeightExpr, "height='60px");
		returnProp = tr("shape");
	}

	return result;
}

QStringList ResizableBoard::collectValues(const QString & family, const QString & prop, QString & value) {
	QStringList result = PaletteItem::collectValues(family, prop, value);

	if (prop.compare("shape", Qt::CaseInsensitive) == 0) {
		if (customShapeTranslated.isEmpty()) {
			customShapeTranslated = tr("Import Shape...");
		}
		result.append(customShapeTranslated);
	}

	return result;
}

QObject * ResizableBoard::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {

	QObject * result = PaletteItem::createPlugin(parent, classid, url, paramNames, paramValues);

	if (classid.compare("shape", Qt::CaseInsensitive) != 0) {
		return result;
	}

	bool swappingEnabled = getSwappingEnabled(paramNames, paramValues);
	if (!m_modelPart->prop("height").isValid()) { 
		// display uneditable width and height
		QFrame * frame = new QFrame();
		QVBoxLayout * vboxLayout = new QVBoxLayout();
		vboxLayout->setAlignment(Qt::AlignLeft);
		vboxLayout->setSpacing(0);
		vboxLayout->setContentsMargins(0, 3, 0, 0);

		QRectF r = this->boundingRect();
		qreal w = qRound(GraphicsUtils::pixels2mm(r.width()) * 100) / 100.0;
		QLabel * l1 = new QLabel(tr("width: %1mm").arg(w));	
		l1->setMargin(0);

		qreal h = qRound(GraphicsUtils::pixels2mm(r.height()) * 100) / 100.0;
		QLabel * l2 = new QLabel(tr("height: %1mm").arg(h));
		l2->setMargin(0);

		vboxLayout->addWidget(qobject_cast<QWidget *>(result));
		vboxLayout->addWidget(l1);
		vboxLayout->addWidget(l2);

		frame->setLayout(vboxLayout);

		frame->setMaximumWidth(200);

		return frame;
	}

	qreal w = qRound(m_modelPart->prop("width").toDouble() * 10) / 10.0;	// truncate to 1 decimal point
	qreal h = qRound(m_modelPart->prop("height").toDouble() * 10) / 10.0;  // truncate to 1 decimal point

	QFrame * frame = new QFrame();
	QVBoxLayout * vboxLayout = new QVBoxLayout();
	vboxLayout->setAlignment(Qt::AlignLeft);
	vboxLayout->setSpacing(1);
	vboxLayout->setContentsMargins(0, 3, 0, 0);

	QFrame * subframe1 = new QFrame();
	QHBoxLayout * hboxLayout1 = new QHBoxLayout();
	hboxLayout1->setAlignment(Qt::AlignLeft);
	hboxLayout1->setContentsMargins(0, 0, 0, 0);
	hboxLayout1->setSpacing(2);

	QLabel * l1 = new QLabel(tr("width(mm)"));	
	l1->setMargin(0);
	QLineEdit * e1 = new QLineEdit();
	e1->setEnabled(swappingEnabled);
	QDoubleValidator * validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, 1);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e1->setValidator(validator);
	e1->setMaxLength(5);
	e1->setText(QString::number(w));
	m_widthEditor = e1;

	QFrame * subframe2 = new QFrame();
	QHBoxLayout * hboxLayout2 = new QHBoxLayout();
	hboxLayout2->setAlignment(Qt::AlignLeft);
	hboxLayout2->setContentsMargins(0, 0, 0, 0);
	hboxLayout2->setSpacing(2);

	QLabel * l2 = new QLabel(tr("height(mm)"));
	l2->setMargin(0);
	QLineEdit * e2 = new QLineEdit();
	e2->setEnabled(swappingEnabled);
	validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, 1);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e2->setValidator(validator);
	e2->setMaxLength(5);
	e2->setText(QString::number(h));
	m_heightEditor = e2;

	hboxLayout1->addWidget(l1);
	hboxLayout1->addWidget(e1);

	hboxLayout2->addWidget(l2);
	hboxLayout2->addWidget(e2);

	subframe1->setLayout(hboxLayout1);
	subframe2->setLayout(hboxLayout2);

	vboxLayout->addWidget(qobject_cast<QWidget *>(result));
	vboxLayout->addWidget(subframe1);
	vboxLayout->addWidget(subframe2);

	frame->setLayout(vboxLayout);

	connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));
	connect(e2, SIGNAL(editingFinished()), this, SLOT(heightEntry()));

	frame->setMaximumWidth(200);

	return frame;
}

void ResizableBoard::widthEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	double newVal = edit->text().toDouble();
	double oldVal =  m_modelPart->prop("height").toDouble();

	if (oldVal == newVal) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(oldVal, newVal, true);
	}
}

void ResizableBoard::heightEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	double newVal = edit->text().toDouble();
	double oldVal =  m_modelPart->prop("width").toDouble();

	if (oldVal == newVal) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(oldVal, newVal, true);
	}
}

bool ResizableBoard::stickyEnabled() {
	return false;
}

ItemBase::PluralType ResizableBoard::isPlural() {
	return Plural;
}
