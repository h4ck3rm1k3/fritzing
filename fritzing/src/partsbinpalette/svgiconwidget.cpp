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

#include <QPixmap>

#include "svgiconwidget.h"
#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../utils/misc.h"
#include "../fsvgrenderer.h"
#include "iconwidgetpaletteitem.h"

#define SELECTED_STYLE "background-color: white;"
#define NON_SELECTED_STYLE "background-color: #C2C2C2;"

#define SELECTION_THICKNESS 3
#define ICON_SIZE 32

static QPixmap * PluralImage = NULL;

SvgIconWidget::SvgIconWidget(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, long id, QMenu * itemMenu, bool isPlural)
	: QGraphicsWidget() 
{
	setAcceptHoverEvents(true);
	m_moduleId = modelPart->moduleID();
	m_isPlural = isPlural;

	setFlags(QGraphicsItem::ItemIsSelectable);

	this->setMaximumSize(QSize(ICON_SIZE + (2 * SELECTION_THICKNESS), ICON_SIZE + (2 * SELECTION_THICKNESS)));

	m_paletteItem = new IconWidgetPaletteItem(modelPart, viewIdentifier, ViewGeometry(), id, itemMenu);
	m_paletteItem->renderImage(modelPart, ViewIdentifierClass::IconView, viewLayers,ViewLayer::Icon, false);

	QPixmap * pixmap = FSvgRenderer::getPixmap(m_moduleId, ViewLayer::Icon, QSize(ICON_SIZE, ICON_SIZE));
	if (pixmap) {
		m_pixmapItem = new QGraphicsPixmapItem(*pixmap, this);
		delete pixmap;
	}
	else {
		m_pixmapItem = new QGraphicsPixmapItem(this);
	}

	m_pixmapItem->setFlags(0);
	m_pixmapItem->setPos(SELECTION_THICKNESS, SELECTION_THICKNESS);

	m_paletteItem->setTooltip();
	setToolTip(m_paletteItem->toolTip());
}

void SvgIconWidget::initNames() {
	PluralImage = new QPixmap(":/resources/images/icons/plural.png");
}

SvgIconWidget::~SvgIconWidget() {
	delete m_paletteItem;
}

void SvgIconWidget::cleanup() {
	if (PluralImage) {
		delete PluralImage;
		PluralImage = NULL;
	}
}

ModelPart *SvgIconWidget::modelPart() const {
	return m_paletteItem->modelPart();
}

const QString &SvgIconWidget::moduleID() const {
	return m_moduleId;
}

void SvgIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

	QColor c(0xc2, 0xc2,0xc2);
	QSizeF size = this->geometry().size();
	painter->fillRect(0, 0, size.width(), size.height(), c);

	if (m_isPlural ) {
		QSize p = PluralImage->size();
		painter->drawPixmap((size.width() - p.width()) / 2, (size.height() - p.height()) / 2, *PluralImage);
	}

	if (isSelected()) {
		painter->save();
		QPen pen = painter->pen();
		pen.setColor(QColor(122, 15, 49));
		pen.setWidth(SELECTION_THICKNESS);
		painter->setPen(pen);
		painter->drawRect(0, 0, size.width(), size.height());
		painter->restore();
	} 

	QGraphicsWidget::paint(painter, option, widget);
}

void SvgIconWidget::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ){
	QGraphicsWidget::hoverEnterEvent(event);
	InfoGraphicsView * igv = InfoGraphicsView::getInfoGraphicsView(this);
	if (igv) {
		igv->hoverEnterItem(this->modelPart());
	}
}

void SvgIconWidget::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	QGraphicsWidget::hoverLeaveEvent(event);
	InfoGraphicsView * igv = InfoGraphicsView::getInfoGraphicsView(this);
	if (igv) {
		igv->hoverLeaveItem(this->modelPart());
	}
}
