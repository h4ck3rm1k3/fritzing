/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include <QGraphicsSceneHoverEvent>
#include <QMutex>
#include <QTimer>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>

#include "../items/itembase.h"
#include "../items/wire.h"
#include "../connectors/connectoritem.h"
#include "../referencemodel/referencemodel.h"

struct PropThing {
	QLabel * m_name;
	QLabel * m_value;
	QWidget * m_plugin;
	QVBoxLayout * m_layout;
};

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

	void hoverEnterItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item, bool swappingEnabled);
	void hoverLeaveItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ItemBase * item);

	void viewConnectorItemInfo(ConnectorItem* item);
	void hoverEnterConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item, bool swappingEnabled);
	void hoverLeaveConnectorItem(class InfoGraphicsView *, QGraphicsSceneHoverEvent * event, ConnectorItem * item);

	void unregisterCurrentItem();
	void unregisterCurrentItemIf(long id);

public:
	static const int STANDARD_ICON_IMG_WIDTH;
	static const int STANDARD_ICON_IMG_HEIGHT;

	static void cleanup();

protected slots:
	void setContent();
	void setInstanceTitle();
	void instanceTitleEnter();
	void instanceTitleLeave();
	void instanceTitleEditable(bool editable);
	void viewExpanded(bool);

protected:
	void appendStuff(ItemBase* item, bool swappingEnabled); //finds out if it's a wire or something else
	void appendWireStuff(Wire* wire, bool swappingEnabled);
	void appendItemStuff(ItemBase* base, bool swappingEnabled);
	void appendItemStuff(ItemBase * base, ModelPart * modelPart, bool swappingEnabled, bool labelIsVisible = false);

	void setInstanceTitleColors(class FLineEdit * edit, const QColor & base, const QColor & text);

	QString settingsBlockVisibilityName(const QString &blockId);

	void setCurrentItem(ItemBase *);
	void registerAsCurrentItem(ItemBase *item);
	void setNullContent();
	void viewItemInfoAux(class InfoGraphicsView *, ItemBase* item, bool swappingEnabled);
	void setUpTitle(ItemBase *);
	void setUpIcons(ModelPart *);
	void addTags(ModelPart * modelPart);
	void partTitle(const QString & title, const QString & version);
	void displayProps(ModelPart * modelPart, ItemBase * itemBase, bool swappingEnabled);
	void clearPropThingPlugin(PropThing * propThing);

protected:
	bool m_alreadyset;

	QPointer<ItemBase> m_currentItem;
	bool m_currentSwappingEnabled;					// previous item (possibly hovered over)

	QString m_content;
	QString m_savedContent;
	QTimer m_setContentTimer;
	QPointer<class InfoGraphicsView> m_infoGraphicsView;
	QPointer<ItemBase> m_lastItemBase;
	bool m_lastSwappingEnabled;						// previous item (selected)
	class FLineEdit * m_titleEdit;
	QLabel * m_icon1;
	QLabel * m_icon2;
	QLabel * m_icon3;
	QLabel * m_partTitle;
	QLabel * m_partVersion;
	QLabel * m_location;
	QLabel * m_tagsLabel;
	QLabel * m_connDescr;
	QLabel * m_connName;
	QLabel * m_connType;
	QGridLayout * m_propLayout;
	QList <PropThing *> m_propThings;

protected:
	static QString PropsBlockId;
	static QString TagsBlockId;
	static QString ConnsBlockId;
};

#endif
