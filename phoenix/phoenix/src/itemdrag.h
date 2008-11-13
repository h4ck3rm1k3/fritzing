/*
 *  itemdrag.h
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/20/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#ifndef ITEMDRAG_H
#define ITEMDRAG_H

#include <QDrag>
#include <QHash>
#include <QPixmap>


class ItemDrag : public QObject {
	
Q_OBJECT
	
	
protected:	
	ItemDrag(QObject * parent = 0);
	QHash<QObject *, QObject *> & cache();
	void dragIsDone();
	
public:
	static ItemDrag * _itemDrag();
	static QHash<QObject *, QObject *> & _cache();
	static void _dragIsDone();

signals:
	void dragIsDoneSignal(ItemDrag *);

protected:
	QHash<QObject *, QObject *> m_cache;
	
protected:
	static ItemDrag * singleton;
};

#endif
