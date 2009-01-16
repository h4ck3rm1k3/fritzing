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



#include <QHBoxLayout>

#include "svgiconwidget.h"
#include "../infographicsview.h"
#include "../debugdialog.h"
#include "../misc.h"
#include "../fsvgrenderer.h"

#define SELECTED_STYLE "background-color: white;"
#define NON_SELECTED_STYLE "background-color: #BEBEBE;"

SvgIconWidget::SvgIconWidget(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, long id, QMenu * itemMenu)
	: QGraphicsProxyWidget() {
	setFlags(QGraphicsItem::ItemIsSelectable);
	m_moduleId = modelPart->moduleID();

	m_paletteItem = new PaletteItem(modelPart, viewIdentifier, ViewGeometry(), id, itemMenu);
	m_paletteItem->renderImage(modelPart, ItemBase::IconView, viewLayers,ViewLayer::Icon, false);

	m_container = new SvgIconWidgetContainer(m_paletteItem, this);
	m_container->setStyleSheet(NON_SELECTED_STYLE);
	m_styleSheetType = NONSELECTEDSTYLESHEET;
	m_container->setFixedSize(32,32);

	m_pixmapContainer = new QLabel(m_container);
	QPixmap * pixmap = FSvgRenderer::getPixmap(m_moduleId, ViewLayer::Icon, m_container->size());
	if (pixmap) {
		m_pixmapContainer->setPixmap(*pixmap);
		delete pixmap;
	}

	QHBoxLayout *lo = new QHBoxLayout(m_container);
	lo->setMargin(0);
	lo->setSpacing(0);
	lo->addWidget(m_pixmapContainer);
	m_container->setLayout(lo);

	setWidget(m_container);

	setToolTip(m_paletteItem->toolTip());
}

SvgIconWidget::~SvgIconWidget() {
	// should delete it
	//delete m_container;
}

ModelPart *SvgIconWidget::modelPart() const {
	return m_paletteItem->modelPart();
}

const QString &SvgIconWidget::moduleID() const {
	return m_moduleId;
}

void SvgIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	if(isSelected()) {
		if (m_styleSheetType != SELECTEDSTYLESHEET) {
			m_container->setStyleSheet(SELECTED_STYLE);
			m_styleSheetType = SELECTEDSTYLESHEET;
		}
	} else if (m_styleSheetType != NONSELECTEDSTYLESHEET) {
		m_container->setStyleSheet(NON_SELECTED_STYLE);
		m_styleSheetType = NONSELECTEDSTYLESHEET;
	}

	QGraphicsProxyWidget::paint(painter, option, widget);
}

QPoint SvgIconWidget::globalPos() {
	InfoGraphicsView *igv = dynamic_cast<InfoGraphicsView*>(this->scene()->parent());
	if(igv) {
		return igv->mapToGlobal(this->pos().toPoint());
	} else {
		return this->pos().toPoint();
	}
}
