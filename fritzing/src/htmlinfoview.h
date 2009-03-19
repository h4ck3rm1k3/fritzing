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



#ifndef HTMLINFOVIEW_H
#define HTMLINFOVIEW_H

#include <QFrame>
#include <QWebView>
#include <QGraphicsSceneHoverEvent>
#include <QMutex>

#include "itembase.h"
#include "wire.h"
#include "connectoritem.h"
#include "referencemodel/referencemodel.h"

class HtmlInfoView : public QFrame
{
Q_OBJECT
public:
	HtmlInfoView(ReferenceModel *refModel, QWidget * parent = 0);
	~HtmlInfoView();

	QSize sizeHint() const;
	void setContent(const QString& html);

	ItemBase *currentItem();
	void reloadContent(class InfoGraphicsView *);

	void viewItemInfo(class InfoGraphicsView *, ItemBase* item, bool swappingEnabled);
	void hoverEnterItem(ModelPart *, bool swappingEnabled);
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
	void setBlockVisibility(const QString &blockId, bool value);

protected:
	QString appendStuff(ItemBase* item, bool swappingEnabled); //finds out if it's a wire or something else
	QString appendViewGeometry(ItemBase * base, bool doLine);
	QString appendCurrentGeometry(ItemBase *, bool doLine);
	QString appendWireStuff(Wire* wire, long itemID);
	QString appendItemStuff(ItemBase* base, long itemID, bool swappingEnabled);
	QString appendItemStuff(ModelPart * modelPart, long itemID, bool swappingEnabled, const QString title = "", bool labelIsVisible = false);

	void prepareTitleStuff(ItemBase *base, QString &title);
	QString propertyHtml(const QString& name, const QString& value, const QString& family, const QString& displayName, bool dynamic);
	QString toHtmlImage(QPixmap *pixmap, const char* format = "PNG");
	QString wireColorsSelect(Wire *wire);
	QString wireWidthSelect(Wire *wire);

	QString blockHeader(const QString &title, const QString &blockId);
	QString blockVisibility(const QString &blockId);
	QString blockContainer(const QString &blockId);
	QString settingsBlockVisibilityName(const QString &blockId);

	void registerJsObjects(const QString &parentName);
	void registerCurrentAgain();
	void registerRefModel();
	bool registerAsCurrentItem(ItemBase *item);
	void registerInfoGraphicsView(class InfoGraphicsView *, const QString & parentName);
	void setNullContent();

protected:
	QString m_includes;
	bool m_alreadyset;

	ReferenceModel *m_refModel;
	ItemBase *m_currentItem;
	bool m_currentSwappingEnabled;
	int m_maxPropCount;
	QMutex m_setContentMutex;

	QWebView *m_webView;
	QHash<QString /**/, bool> m_blocksVisibility;

protected:
	static QString PropsBlockId;
	static QString TagsBlockId;
	static QString ConnsBlockId;
};

#endif
