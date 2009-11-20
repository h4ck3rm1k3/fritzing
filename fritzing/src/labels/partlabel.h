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

#ifndef PARTLABEL_H
#define PARTLABEL_H

#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QXmlStreamWriter>
#include <QDomElement>
#include <QTextDocument>
#include <QKeyEvent>
#include <QPointer>
#include <QTimer>
#include <QMenu>

#include "../viewlayer.h"

class PartLabel : public QObject, public QGraphicsSimpleTextItem
{
	Q_OBJECT
public:
	PartLabel(class ItemBase * owner, QGraphicsItem * parent = 0 );   // itembase is not the parent
	~PartLabel();

	void showLabel(bool showIt, ViewLayer *);
	QPainterPath shape() const;
	void setPlainText(const QString & text);
	bool initialized();
	void ownerMoved(QPointF newPos);
	void setHidden(bool hide);
	bool hidden();
	ViewLayer::ViewLayerID viewLayerID();
	void saveInstance(QXmlStreamWriter & streamWriter);
	void restoreLabel(QDomElement & labelGeometry, ViewLayer::ViewLayerID);
	void moveLabel(QPointF newPos, QPointF newOffset);
	class ItemBase * owner();
	void rotateFlipLabel(qreal degrees, Qt::Orientations orientation);
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value);
	void ownerSelected(bool selected);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void transformLabel(QTransform currTransf);
	void setUpText();
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	void initMenu();
	void partLabelEdit();
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void setFontSize(int action);
	void rotateFlip(int action);
	void displayTexts();
	void setLabelDisplay(const QString & key);

protected:
	QPointer<class ItemBase> m_owner;
	bool m_initialized;
	bool m_spaceBarWasPressed;
	bool m_doDrag;
	QPointF m_initialPosition;
	QPointF m_initialOffset;
	QPointF m_offset;
	ViewLayer::ViewLayerID m_viewLayerID;
	bool m_hidden;
	QMenu m_menu;
	QString m_text;
	QStringList m_displayKeys;
};

#endif
