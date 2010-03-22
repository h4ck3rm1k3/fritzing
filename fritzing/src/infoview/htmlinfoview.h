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
#include <QTimer>
#include <QLabel>
#include <QScrollArea>

#include "../items/itembase.h"
#include "../items/wire.h"
#include "../connectors/connectoritem.h"
#include "../referencemodel/referencemodel.h"

class HtmlInfoView : public QScrollArea
{
Q_OBJECT
public:
	HtmlInfoView(QWidget * parent = 0);
	~HtmlInfoView();

	QSize sizeHint() const;
	void setContent(const QString& html);

	ItemBase *currentItem();
	void reloadContent(class InfoGraphicsView *);

	void viewItemInfo(class InfoGraphicsView *, ItemBase* item, bool swappingEnabled);
	void viewModelPartInfo(class InfoGraphicsView *, ModelPart * modelPart, bool swappingEnabled);

	void hoverEnterItem(class InfoGraphicsView *, ModelPart *, bool swappingEnabled);
	void hoverLeaveItem(class InfoGraphicsView *, ModelPart *);
	void hoverEnterItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item, bool swappingEnabled);
	void hoverLeaveItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item);

	void viewConnectorItemInfo(class InfoGraphicsView *, ConnectorItem* item, bool swappingEnabled);
	void hoverEnterConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item, bool swappingEnabled);
	void hoverLeaveConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item);

	void unregisterCurrentItem();
	void unregisterCurrentItemIf(long id);

public:
	static const int STANDARD_ICON_IMG_WIDTH;
	static const int STANDARD_ICON_IMG_HEIGHT;

	static void cleanup();

protected slots:
	void jsRegister();
	void setBlockVisibility(const QString &blockId, bool value);
	void setContent();
	void setInstanceTitle();
	void instanceTitleEnter();
	void instanceTitleLeave();
	void instanceTitleEditable(bool editable);

protected:
	QString appendStuff(ItemBase* item, bool swappingEnabled); //finds out if it's a wire or something else
	QString appendWireStuff(Wire* wire, long itemID);
	QString appendItemStuff(ItemBase* base, long itemID, bool swappingEnabled);
	QString appendItemStuff(ItemBase * base, ModelPart * modelPart, long itemID, bool swappingEnabled, const QString title = "", bool labelIsVisible = false);

	void prepareTitleStuff(ItemBase *base, QString &title);
	void setInstanceTitleColors(class FLineEdit * edit, const QColor & base, const QColor & text);


	QString blockHeader(const QString &title, const QString &blockId);
	QString blockVisibility(const QString &blockId);
	QString blockContainer(const QString &blockId);
	QString settingsBlockVisibilityName(const QString &blockId);

	void setCurrentItem(ItemBase *);
	void registerAsCurrentItem(ItemBase *item);
	void setNullContent();
	void viewItemInfoAux(class InfoGraphicsView *, ItemBase* item, bool swappingEnabled);
	void viewModelPartInfoAux(class InfoGraphicsView *, ModelPart * modelPart, bool swappingEnabled);
	void setUpTitle(const QString & title);
	void setUpIcons(ModelPart *);
	void addTags(ModelPart * modelPart, QString & s);

protected:
	QString m_includes;
	bool m_alreadyset;

	QPointer<ItemBase> m_currentItem;
	bool m_currentSwappingEnabled;
	int m_maxPropCount;

	QWebView *m_webView;
	QHash<QString, bool> m_blocksVisibility;
	QPointer<class InfoViewWebPage> m_infoViewWebPage;
	QString m_content;
	QString m_savedContent;
	QTimer m_setContentTimer;
	QPointer<class InfoGraphicsView> m_infoGraphicsView;
	QPointer<ModelPart> m_modelPart;
	QPointer<ModelPart> m_lastModelPart;
	QPointer<ItemBase> m_lastItemBase;
	QPointer<class InfoGraphicsView> m_lastItemBaseInfoGraphicsView;
	QPointer<class InfoGraphicsView> m_lastModelPartInfoGraphicsView;
	class FLineEdit * m_titleEdit;
	QLabel * m_icon1;
	QLabel * m_icon2;
	QLabel * m_icon3;

protected:
	static QString PropsBlockId;
	static QString TagsBlockId;
	static QString ConnsBlockId;
};

#endif
