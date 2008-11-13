/*
 *  layerattributes.cpp
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/20/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include "itemdrag.h"
#include "debugdialog.h"

ItemDrag * ItemDrag::singleton = new ItemDrag();

ItemDrag::ItemDrag(QObject * parent) :
	QObject(parent)
{
}

QHash<QObject *, QObject *> & ItemDrag::cache() {
	return m_cache;
}

void ItemDrag::dragIsDone() {
	m_cache.clear();
	emit dragIsDoneSignal(this);
}

QHash<QObject *, QObject *> & ItemDrag::_cache() {
	return singleton->m_cache;
}


ItemDrag * ItemDrag::_itemDrag() {
	return singleton;
}

void ItemDrag::_dragIsDone() {
	singleton->dragIsDone();
}
