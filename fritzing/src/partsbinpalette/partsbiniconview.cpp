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




#include <QtGui>
#include <QGraphicsScene>
#include <QPoint>
#include <QSet>
#include <QSvgWidget>

#include "partsbiniconview.h"
#include "graphicsflowlayout.h"
#include "../paletteitem.h"
#include "../debugdialog.h"

PartsBinIconView::PartsBinIconView(QWidget *parent)
    : InfoGraphicsView(parent)
{
    setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    setAcceptDrops(true);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QGraphicsScene* scene = new QGraphicsScene(this);
    this->setScene(scene);

    m_layouter = NULL;
    m_layout = NULL;
    setupLayout();
}

void PartsBinIconView::setupLayout() {
	// TODO Mariano: smells like leak, but if this two lines are uncommented there's a crash
	// Also tried to iterate through layout children, but didn't work
	//delete m_layouter;
	//delete m_layout;

    m_layouter = new QGraphicsWidget;
    m_layout = new GraphicsFlowLayout(m_layouter);
    scene()->addItem(m_layouter);
}

void PartsBinIconView::updateSize(QSize newSize) {
	updateSizeAux(newSize.width());
}

void PartsBinIconView::updateSize() {
	updateSizeAux(width());
}

void PartsBinIconView::updateSizeAux(int width) {
	setSceneRect(0, 0, width, m_layout->heightForWidth(width));
}

void PartsBinIconView::resizeEvent(QResizeEvent * event) {
	InfoGraphicsView::resizeEvent(event);
	updateSize(event->size());
}

void PartsBinIconView::mousePressEvent(QMouseEvent *event)
{
	QGraphicsItem* item = this->itemAt(event->pos());
	if (item == NULL) {
		return QGraphicsView::mousePressEvent(event);
	}

	SvgIconWidget* icon = dynamic_cast<SvgIconWidget *>(item);
	if (icon != NULL) {
		QList<QGraphicsItem *> items = scene()->selectedItems();
		for (int i = 0; i < items.count(); i++) {
			// not sure why clearSelection doesn't do the update, but whatever...
			items[i]->setSelected(false);
			items[i]->update();
		}
		icon->setSelected(true);
		icon->update();

		QPointF mts = this->mapToScene(event->pos());
		QString moduleID = icon->moduleID();
		QPoint hotspot = (mts.toPoint()-icon->pos().toPoint());

		mousePressOnItem( moduleID, icon->size().toSize(), (mts - icon->pos()), hotspot );
	}
}

void PartsBinIconView::doClear() {
	PartsBinView::doClear();
	scene()->clear();
	setupLayout();
}

void PartsBinIconView::addPart(ModelPart * model) {
	PartsBinView::addPart(model);
	updateSize();
}

void PartsBinIconView::removePart(const QString &moduleID) {
	SvgIconWidget *itemToRemove = NULL;
	foreach(QGraphicsItem *gIt, m_layouter->childItems()) {
		SvgIconWidget *it = dynamic_cast<SvgIconWidget*>(gIt);
		if(it && it->moduleID() == moduleID) {
			itemToRemove = it;
			break;
		}
	}
	if(itemToRemove) {
		m_partHash.remove(moduleID);
		itemToRemove->setParentItem(NULL);
		m_layout->removeItem(itemToRemove);
		delete itemToRemove;
	}

	setFirstSelected();
	updateSize();
}

void PartsBinIconView::setItemAux(ModelPart * modelPart) {
	if (!modelPart || modelPart->itemType() == ModelPart::Module) {
		// don't want the empty root item to appear in the view
		return;
	}

	QString moduleID = modelPart->moduleID();

	if(!alreadyIn(moduleID)) {
		SvgIconWidget* svgicon = new SvgIconWidget(modelPart, ItemBase::IconView, m_viewLayers, ItemBase::getNextID(), NULL);
		m_layout->addItem(svgicon);
		m_partHash[moduleID] = svgicon->modelPart();
	} else {
		m_partHash[moduleID]->copy(modelPart);
	}
}

void PartsBinIconView::setPaletteModel(PaletteModel *model, bool clear) {
	PartsBinView::setPaletteModel(model, clear);
	updateSize();
}

void PartsBinIconView::loadFromModel(PaletteModel * model) {
	ModelPart* root = model->root();
	QList<QObject *>::const_iterator i;
    for (i = root->children().constBegin(); i != root->children().constEnd(); ++i) {
		ModelPart* mp = qobject_cast<ModelPart *>(*i);
		if (mp == NULL) continue;

		QDomElement instance = mp->instanceDomElement();
		if (instance.isNull()) continue;

		QDomElement views = instance.firstChildElement("views");
		if (views.isNull()) continue;

		QString name = ItemBase::viewIdentifierXmlName(ItemBase::IconView);
		QDomElement view = views.firstChildElement(name);
		if (view.isNull()) continue;

		QDomElement geometry = view.firstChildElement("geometry");
		if (geometry.isNull()) continue;

		setItemAux(mp);
	}
}

ModelPart *PartsBinIconView::selected() {
	SvgIconWidget *icon = dynamic_cast<SvgIconWidget *>(selectedAux());
	if(icon) {
		return icon->modelPart();
	} else {
		return NULL;
	}
}

void PartsBinIconView::setFirstSelected() {
	foreach(QGraphicsItem *gIt, m_layouter->childItems()) {
		SvgIconWidget *it = dynamic_cast<SvgIconWidget*>(gIt);
		if(it) {
			it->setSelected(true);
			break;
		}
	}
}

bool PartsBinIconView::swappingEnabled() {
	return false;
}
