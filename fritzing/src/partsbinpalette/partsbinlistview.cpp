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


#include "../debugdialog.h"
#include "../htmlinfoview.h"
#include "../itembase.h"
#include "../layerattributes.h"
#include "../fsvgrenderer.h"

#include "partsbinlistview.h"

PartsBinListView::PartsBinListView( QWidget * parent ) : QListWidget( parent ) {
	m_infoView = NULL;
	m_hoverItem = NULL;
	m_infoViewOnHover = false;
	setMouseTracking( true );
	setSpacing(2);
	setIconSize(QSize(16,16));
}
PartsBinListView::~PartsBinListView() {
}

void PartsBinListView::doClear() {
	PartsBinView::doClear();
	clear();
}

void PartsBinListView::setItemAux(ModelPart * modelPart, int position) {
	if (modelPart->modelPartShared() == NULL) return;
	if (modelPart->itemType() == ModelPart::Unknown) {
		// don't want the empty root to appear in the view
		return;
	}

	QString moduleID = modelPart->moduleID();
	if(!alreadyIn(moduleID)) {
		QListWidgetItem * lwi = new QListWidgetItem(modelPart->modelPartShared()->title(), this);
		lwi->setData(Qt::UserRole, qVariantFromValue( modelPart ) );

		LayerAttributes layerAttributes;
		FSvgRenderer * renderer = ItemBase::setUpImage(modelPart, ItemBase::IconView, ViewLayer::Icon, layerAttributes);
		if (renderer != NULL) {
			QSize size(STANDARD_ICON_IMG_WIDTH, STANDARD_ICON_IMG_HEIGHT);
			QPixmap * pixmap = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
			lwi->setIcon(QIcon(*pixmap));
			delete pixmap;
			lwi->setData(Qt::UserRole + 1, renderer->defaultSize());
		}

		if(position > -1 && position < count()) {
			insertItem(position, lwi);
			update();
		} else {
			addItem(lwi);
		}

		m_partHash[moduleID] = modelPart;
	} else {
		m_partHash[moduleID]->copy(modelPart);
	}
}

void PartsBinListView::mouseMoveEvent ( QMouseEvent * event ) {
	if (m_infoView == NULL) return;

	if(m_infoViewOnHover) {
		QListWidgetItem * item = itemAt(event->pos());
		showInfo(item);
	}
}

void PartsBinListView::showInfo(QListWidgetItem * item) {
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

	m_infoView->hoverEnterItem(modelPart, swappingEnabled());
}


void PartsBinListView::mousePressEvent(QMouseEvent *event) {
	QListWidget::mousePressEvent(event);

	QListWidgetItem * current = currentItem ();
	if (current == NULL) return;

	ModelPart * modelPart = current->data(Qt::UserRole).value<ModelPart *>();
	if (modelPart == NULL) return;

	if(!m_infoViewOnHover) {
		showInfo(current);
	}

	mousePressOnItem(modelPart->moduleID(), iconSize());
}

void PartsBinListView::setInfoView(HtmlInfoView * infoView) {
	m_infoView = infoView;
}

void PartsBinListView::removePart(const QString &moduleID) {
	int idxToRemove = position(moduleID);
	if(idxToRemove > -1) {
		m_partHash.remove(moduleID);
		delete takeItem(idxToRemove);
	}
}

int PartsBinListView::position(const QString &moduleID) {
	for(int i=0; i < count(); i++) {
		if(itemModuleID(item(i)) == moduleID) {
			return i;
		}
	}
	return -1;
}

ModelPart *PartsBinListView::itemModelPart(const QListWidgetItem *item) {
	return item->data(Qt::UserRole).value<ModelPart *>();
}

const QString &PartsBinListView::itemModuleID(const QListWidgetItem *item) {
	return itemModelPart(item)->moduleID();
}

ModelPart *PartsBinListView::selected() {
	if(selectedItems().size()==1) {
		return itemModelPart(selectedItems()[0]);
	}
	return NULL;
}

bool PartsBinListView::swappingEnabled() {
	return false;
}

void PartsBinListView::setSelected(int position) {
	if(position > -1 && position < count()) {
		item(position)->setSelected(true);
	} else {
		setCurrentRow(position);
	}
}
