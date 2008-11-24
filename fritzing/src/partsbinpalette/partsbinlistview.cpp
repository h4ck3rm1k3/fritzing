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


#include "../debugdialog.h"
#include "../htmlinfoview.h"
#include "../paletteitembase.h"
#include "../layerattributes.h"
#include "../rendererviewthing.h"

#include "partsbinlistview.h"

PartsBinListView::PartsBinListView( QWidget * parent ) : QListWidget( parent ) {
	m_infoView = NULL;
	m_hoverItem = NULL;
	setMouseTracking( true );
	setSpacing(2);
	setIconSize(QSize(16,16));
}
PartsBinListView::~PartsBinListView() {
}

void PartsBinListView::doClear() {
	clear();
}

void PartsBinListView::setItemAux(ModelPart * modelPart) {
	if (modelPart->modelPartStuff() == NULL) return;
	if (modelPart->itemType() == ModelPart::Module) {
		// don't want the empty root to appear in the view
		return;
	}

	QListWidgetItem * lwi = new QListWidgetItem(modelPart->modelPartStuff()->title(), this);
	lwi->setData(Qt::UserRole, qVariantFromValue( modelPart ) );

	LayerAttributes layerAttributes;
	FSvgRenderer * renderer = PaletteItemBase::setUpImage(modelPart, ItemBase::IconView, ViewLayer::Icon, layerAttributes);
	if (renderer != NULL) {
		QSize size(STANDARD_ICON_IMG_WIDTH, STANDARD_ICON_IMG_HEIGHT);
		QPixmap * pixmap = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
		lwi->setIcon(QIcon(*pixmap));
		delete pixmap;
		lwi->setData(Qt::UserRole + 1, renderer->defaultSize());
	}

	this->addItem(lwi);
}

void PartsBinListView::mouseMoveEvent ( QMouseEvent * event ) {
	if (m_infoView == NULL) return;

	QListWidgetItem * item = itemAt(event->pos());
	if (item == m_hoverItem) {
		// no change
		return;
	}

	if (m_hoverItem != NULL) {
		ModelPart * modelPart = m_hoverItem->data(Qt::UserRole).value<ModelPart *>();
		if (modelPart != NULL) {
			m_infoView->hoverLeaveItem(modelPart);
		}
	}

	m_hoverItem = item;
	if (item == NULL) {
		return;
	}

	ModelPart * modelPart = item->data(Qt::UserRole).value<ModelPart *>();
	if (modelPart == NULL) return;

	m_infoView->hoverEnterItem(modelPart, swappingEnabled(), NULL);
}


void PartsBinListView::mousePressEvent(QMouseEvent *event) {
	QListWidget::mousePressEvent(event);

	QListWidgetItem * current = currentItem ();
	if (current == NULL) return;

	ModelPart * modelPart = current->data(Qt::UserRole).value<ModelPart *>();
	if (modelPart == NULL) return;

	mousePressOnItem(modelPart->moduleID(), iconSize());
}

void PartsBinListView::setInfoView(HtmlInfoView * infoView) {
	m_infoView = infoView;
}

void PartsBinListView::removePart(const QString &moduleID) {
	int idxToRemove = -1;
	for(int i=0; i < count(); i++) {
		if(itemModuleID(item(i)) == moduleID) {
			idxToRemove = i;
			break;
		}
	}
	if(idxToRemove > -1) {
		delete takeItem(idxToRemove);
	}
}

const QString &PartsBinListView::itemModuleID(const QListWidgetItem *item) {
	return item->data(Qt::UserRole).value<ModelPart *>()->moduleID();
}

PaletteItem *PartsBinListView::selected() {
	// TODO Mariano
	return NULL;
}

bool PartsBinListView::swappingEnabled() {
	return false;
}
