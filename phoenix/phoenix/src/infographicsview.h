#ifndef INFOGRAPHICSVIEW_H
#define INFOGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMenu>
#include <QHash>
#include <QList>

#include "wire.h"
#include "paletteitembase.h"
#include "htmlinfoview.h"
#include "viewlayer.h"
#include "itembase.h"

class InfoGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	InfoGraphicsView(QWidget* parent = 0);

	void viewItemInfo(ItemBase * item);
	virtual void hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	virtual void hoverEnterItem(ModelPart * modelPart, QPixmap *pixmap=NULL);
	virtual void hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	virtual void hoverLeaveItem(ModelPart * modelPart);

	virtual bool swappingEnabled() = 0;

	void viewConnectorItemInfo(ConnectorItem * item);
	virtual void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	virtual void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);

	void setInfoView(HtmlInfoView *);
	HtmlInfoView * infoView();

	virtual void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);

	virtual void setItemTooltip(long id, const QString &newTooltip);
	virtual PaletteItem *selected();

protected:
	QGraphicsItem *selectedAux();
	HtmlInfoView *m_infoView;

};

#endif
