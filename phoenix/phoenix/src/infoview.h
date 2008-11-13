#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QTextEdit>
#include <QGraphicsSceneHoverEvent>

#include "itembase.h"
#include "connectoritem.h"

class InfoView : public QTextEdit
{
Q_OBJECT
public:
	InfoView(QWidget * parent = 0);
	void hoverEnterItem(ModelPart *);
	void hoverLeaveItem(ModelPart *);
	void hoverEnterItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverLeaveItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverEnterConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void hoverLeaveConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item);

protected:
	QString appendViewGeometry(ItemBase * base, bool doLine);
	QString appendCurrentGeometry(ItemBase *, bool doLine);
	QString appendItemStuff(ItemBase* base, long itemID);	
	QString appendItemStuff(ModelPart * modelPart, long itemID);
};

#endif
