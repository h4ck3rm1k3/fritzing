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

PaletteItem *InfoGraphicsView::selected() {
	return dynamic_cast<PaletteItem*>(selectedAux());
}

QGraphicsItem *InfoGraphicsView::selectedAux() {
	QList<QGraphicsItem*> selItems = scene()->selectedItems();
	DebugDialog::debug(QString("sel items count = %2").arg(selItems.size()));
	if(selItems.size() != 1) {
		return NULL;
	} else {
		return selItems[0];
	}
}
