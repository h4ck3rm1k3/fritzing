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

#include "infographicsview.h"
#include "../debugdialog.h"
#include "../commands.h"
#include "../infoview/htmlinfoview.h"

#include <QMessageBox>

static LayerHash ViewLayers;

InfoGraphicsView::InfoGraphicsView( QWidget * parent )
	: ZoomableGraphicsView(parent)
{
	m_infoView = NULL;
}

void InfoGraphicsView::viewItemInfo(ItemBase * item) {
	if (m_infoView == NULL) return;

	m_infoView->viewItemInfo(this, item ? item->layerKinChief() : item, swappingEnabled(item));
}

void InfoGraphicsView::hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item) {
	if (m_infoView == NULL) return;

	m_infoView->hoverEnterItem(this, event, item ? item->layerKinChief() : item, swappingEnabled(item));
}

void InfoGraphicsView::hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item){
	if (m_infoView == NULL) return;

	m_infoView->hoverLeaveItem(this, event, item ? item->layerKinChief() : item);
}

void InfoGraphicsView::viewConnectorItemInfo(ConnectorItem * item) {
	if (m_infoView == NULL) return;

	m_infoView->viewConnectorItemInfo(this, item, swappingEnabled(item->attachedTo()));
}

void InfoGraphicsView::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	if (m_infoView == NULL) return;

	m_infoView->hoverEnterConnectorItem(this, event, item, swappingEnabled(item->attachedTo()));
}

void InfoGraphicsView::hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item){
	if (m_infoView == NULL) return;

	m_infoView->hoverLeaveConnectorItem(this, event, item);
}

void InfoGraphicsView::setInfoView(HtmlInfoView * infoView) {
	m_infoView = infoView;
}

HtmlInfoView * InfoGraphicsView::infoView() {
	return m_infoView;
}

void InfoGraphicsView::mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
}

void InfoGraphicsView::partLabelChanged(ItemBase * item, const QString &oldText, const QString & newText) {
	Q_UNUSED(item);
	Q_UNUSED(oldText);
	Q_UNUSED(newText);
}

void InfoGraphicsView::noteChanged(ItemBase * item, const QString &oldText, const QString & newText, QSizeF oldSize, QSizeF newSize) {
	Q_UNUSED(item);
	Q_UNUSED(oldText);
	Q_UNUSED(newText);
	Q_UNUSED(oldSize);
	Q_UNUSED(newSize);
}

QGraphicsItem *InfoGraphicsView::selectedAux() {
	QList<QGraphicsItem*> selItems = scene()->selectedItems();
	if(selItems.size() != 1) {
		return NULL;
	} else {
		return selItems[0];
	}
}

void InfoGraphicsView::partLabelMoved(ItemBase * itemBase, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset) 
{
	Q_UNUSED(itemBase);
	Q_UNUSED(oldPos);
	Q_UNUSED(oldOffset);
	Q_UNUSED(newPos);
	Q_UNUSED(newOffset);
}

void InfoGraphicsView::rotateFlipPartLabel(ItemBase * itemBase, qreal degrees, Qt::Orientations flipDirection) {
	Q_UNUSED(itemBase);
	Q_UNUSED(degrees);
	Q_UNUSED(flipDirection);
}

void InfoGraphicsView::noteSizeChanged(ItemBase * itemBase, const QRectF & oldRect, const QRectF & newRect) {
	Q_UNUSED(itemBase);
	Q_UNUSED(oldRect);
	Q_UNUSED(newRect);
}

bool InfoGraphicsView::spaceBarIsPressed() {
	return false;
}

InfoGraphicsView * InfoGraphicsView::getInfoGraphicsView(QGraphicsItem * item)
{
	if (item == NULL) return NULL;

	QGraphicsScene * scene = item->scene();
	if (scene == NULL) return NULL;

	return dynamic_cast<InfoGraphicsView *>(scene->parent());
}

void InfoGraphicsView::initWire(Wire *, int penWidth) {
	Q_UNUSED(penWidth);
}


void InfoGraphicsView::getBendpointWidths(class Wire *, qreal w, qreal & w1, qreal & w2) {
	Q_UNUSED(w);
	Q_UNUSED(w1);
	Q_UNUSED(w2);
}

void InfoGraphicsView::getLabelFont(QFont &, QColor &, const LayerList &) {
}

qreal InfoGraphicsView::getLabelFontSizeSmall() {
	return 7;
}

qreal InfoGraphicsView::getLabelFontSizeMedium() {
	return 9;
}

qreal InfoGraphicsView::getLabelFontSizeLarge() {
	return 14;
}

bool InfoGraphicsView::hasBigDots() {
	return false;
}

void InfoGraphicsView::setVoltage(qreal v, bool doEmit) {
	if (doEmit) {
		emit setVoltageSignal(v, false);
	}
}

void InfoGraphicsView::resizeBoard(qreal w, qreal h, bool doEmit) {
	Q_UNUSED(w);
	Q_UNUSED(h);
	Q_UNUSED(doEmit);
}

void InfoGraphicsView::setResistance(QString resistance, QString pinSpacing)
{
	Q_UNUSED(resistance);
	Q_UNUSED(pinSpacing);
}

void InfoGraphicsView::setChipLabel(QString label)
{
	Q_UNUSED(label);
}

void InfoGraphicsView::changeWireWidthMils(const QString newWidth) {
	Q_UNUSED(newWidth);
}

void InfoGraphicsView::changeWireColor(const QString newColor) {
	Q_UNUSED(newColor);
}

void InfoGraphicsView::swap(const QString & family, const QString & prop, QMap<QString, QString> & propsMap) {
	emit swapSignal(family, prop, propsMap);
}

void InfoGraphicsView::setInstanceTitle(long id, const QString & title, bool isUndoable, bool doEmit) {
	Q_UNUSED(id);
	Q_UNUSED(title);
	Q_UNUSED(isUndoable);
	Q_UNUSED(doEmit);
}

LayerHash & InfoGraphicsView::viewLayers() {
	return ViewLayers;
}

void InfoGraphicsView::loadLogoImage(long itemID, const QString & oldSvg, const QSizeF oldAspectRatio, const QString & oldFilename, const QString & newFilename, bool addName) {
	Q_UNUSED(itemID);
	Q_UNUSED(oldSvg);
	Q_UNUSED(oldAspectRatio);
	Q_UNUSED(oldFilename);
	Q_UNUSED(newFilename);
	Q_UNUSED(addName);
}

void InfoGraphicsView::setNoteFocus(QGraphicsItem * item, bool inFocus) {
	Q_UNUSED(item);
	Q_UNUSED(inFocus);
}
