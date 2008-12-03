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

#include "infographicsview.h"
#include "debugdialog.h"
#include "bettertriggeraction.h"
#include "commands.h"

#include <QMessageBox>

#include <math.h>

InfoGraphicsView::InfoGraphicsView( QWidget * parent )
	: QGraphicsView(parent)
{
	m_infoView = NULL;
}

void InfoGraphicsView::viewItemInfo(ItemBase * item) {
	if (m_infoView == NULL) return;

	m_infoView->viewItemInfo(item, swappingEnabled());
}

void InfoGraphicsView::hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item) {
	if (m_infoView == NULL) return;

	m_infoView->hoverEnterItem(this, event, item, swappingEnabled());
}

void InfoGraphicsView::hoverEnterItem(ModelPart* modelPart, QPixmap *pixmap) {
	if (m_infoView == NULL) return;

	m_infoView->hoverEnterItem(modelPart, swappingEnabled(), pixmap);
}

void InfoGraphicsView::hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item){
	if (m_infoView == NULL) return;

	m_infoView->hoverLeaveItem(this, event, item);
}

void InfoGraphicsView::hoverLeaveItem(ModelPart* modelPart) {
	if (m_infoView == NULL) return;

	m_infoView->hoverLeaveItem(modelPart);
}

void InfoGraphicsView::viewConnectorItemInfo(ConnectorItem * item) {
	if (m_infoView == NULL) return;

	m_infoView->viewConnectorItemInfo(item, swappingEnabled());
}

void InfoGraphicsView::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	if (m_infoView == NULL) return;

	m_infoView->hoverEnterConnectorItem(this, event, item, swappingEnabled());
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

void InfoGraphicsView::setItemTooltip(long id, const QString &newTooltip) {
	Q_UNUSED(id);
	Q_UNUSED(newTooltip);
}

ModelPart *InfoGraphicsView::selected() {
	PaletteItem* pi =  dynamic_cast<PaletteItem*>(selectedAux());
	if(pi) {
		return pi->modelPart();
	} else {
		return NULL;
	}
}

QGraphicsItem *InfoGraphicsView::selectedAux() {
	QList<QGraphicsItem*> selItems = scene()->selectedItems();
	if(selItems.size() != 1) {
		return NULL;
	} else {
		return selItems[0];
	}
}
