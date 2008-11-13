/*
 * (c) Fachhochschule Potsdam
 */

#ifndef HTMLINFOVIEW_H
#define HTMLINFOVIEW_H

#include <QWebView>
#include <QGraphicsSceneHoverEvent>

#include "itembase.h"
#include "wire.h"
#include "paletteitem.h"
#include "connectoritem.h"
#include "referencemodel/referencemodel.h"

class HtmlInfoView : public QWebView
{
Q_OBJECT
public:
	HtmlInfoView(ReferenceModel *refModel, QWidget * parent = 0);
	QSize sizeHint() const;
	void setContent(const QString& html);

	ItemBase *currentItem();
	void reloadContent();

	void viewItemInfo(ItemBase* item, bool swappingEnabled);
	void hoverEnterItem(ModelPart *, bool swappingEnabled, QPixmap *pixmap=NULL);
	void hoverLeaveItem(ModelPart *);
	void hoverEnterItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item, bool swappingEnabled);
	void hoverLeaveItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item);

	void viewConnectorItemInfo(ConnectorItem* item, bool swappingEnabled);
	void hoverEnterConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item, bool swappingEnabled);
	void hoverLeaveConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item);

	void unregisterCurrentItem();
	void unregisterCurrentItemIf(long id);

protected slots:
	void jsRegister();

protected:
	QString appendStuff(ItemBase* item, bool swappingEnabled); //finds out if it's a wire or something else
	QString appendViewGeometry(ItemBase * base, bool doLine);
	QString appendCurrentGeometry(ItemBase *, bool doLine);
	QString appendWireStuff(Wire* wire, long itemID);
	QString appendItemStuff(ItemBase* base, long itemID, bool swappingEnabled);
	QString appendItemStuff(ModelPart * modelPart, long itemID, bool swappingEnabled, QPixmap *pixmap = NULL, const QString title = "");

	void prepareTitleStuff(ItemBase *base, QString &title, QString &instanceTitle, QString &defaultTitle);
	void ensureUniqueTitle(ItemBase* item, QString &title);
	int getNextTitle(QList<QGraphicsItem*> items, const QString &title);
	QString propertyHtml(const QString& name, const QString& value, const QString& family, bool dynamic);
	QString toHtmlImage(QPixmap *pixmap, const char* format = "PNG");
	QString wireColorsSelect(Wire *wire);

	void registerJsObjects(const QString &parentName);
	void registerCurrentAgain();
	void registerRefModel();
	bool registerAsCurrentItem(ItemBase *item);

protected:
	QString m_includes;
	bool m_alreadyset;

	ReferenceModel *m_refModel;
	ItemBase *m_currentItem;
	bool m_currentSwappingEnabled;
};

#endif
