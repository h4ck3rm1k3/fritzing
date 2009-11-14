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


#include "svgiconwidget.h"
#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../utils/misc.h"
#include "../fsvgrenderer.h"
#include "iconwidgetpaletteitem.h"

#define SELECTED_STYLE "background-color: white;"
#define NON_SELECTED_STYLE "background-color: #C2C2C2;"

SvgIconWidget::SvgIconWidget(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, long id, QMenu * itemMenu)
	: QGraphicsWidget() 
{
	m_moduleId = modelPart->moduleID();

	setFlags(QGraphicsItem::ItemIsSelectable);

	this->setMaximumSize(QSize(32, 32));

	m_paletteItem = new IconWidgetPaletteItem(modelPart, viewIdentifier, ViewGeometry(), id, itemMenu);
	m_paletteItem->renderImage(modelPart, ViewIdentifierClass::IconView, viewLayers,ViewLayer::Icon, false);

	QPixmap * pixmap = FSvgRenderer::getPixmap(m_moduleId, ViewLayer::Icon, QSize(32, 32));
	if (pixmap) {
		m_pixmapItem = new QGraphicsPixmapItem(*pixmap, this);
		delete pixmap;
	}
	else {
		m_pixmapItem = new QGraphicsPixmapItem(this);
	}

	m_pixmapItem->setFlags(0);


	m_paletteItem->setTooltip();
	setToolTip(m_paletteItem->toolTip());
}

SvgIconWidget::~SvgIconWidget() {
	delete m_paletteItem;
}

ModelPart *SvgIconWidget::modelPart() const {
	return m_paletteItem->modelPart();
}

const QString &SvgIconWidget::moduleID() const {
	return m_moduleId;
}

void SvgIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	QColor c(0xc2, 0xc2,0xc2);
	if(isSelected()) {
		c.setRgb(255, 255, 255);
	} 

	QSizeF size = this->geometry().size();
	painter->fillRect(0, 0, size.width(), size.height(), c);

	QGraphicsWidget::paint(painter, option, widget);
}

QPoint SvgIconWidget::globalPos() {
	InfoGraphicsView *igv = dynamic_cast<InfoGraphicsView*>(this->scene()->parent());
	if(igv) {
		return igv->mapToGlobal(this->pos().toPoint());
	} else {
		return this->pos().toPoint();
	}
}
