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

$Revision: 2776 $:
$Author: merunga $:
$Date: 2009-04-02 13:54:08 +0200 (Thu, 02 Apr 2009) $

********************************************************************/


#include <QApplication>
#include <QMouseEvent>

#include "stacktabbar.h"
#include "stacktabwidget.h"

StackTabBar::StackTabBar(StackTabWidget *parent) : QTabBar(parent) {
	setAcceptDrops(true);
	m_pressedIndex = -1;
	m_parent = parent;
}

int StackTabBar::tabIndexAtPos(const QPoint &p) const {
	for (int i = 0; i < m_parent->count(); ++i) {
    	if (m_parent->isTabEnabled(i) && tabRect(i).contains(p)) {
    		return i;
    	}
	}
    return -1;
}

void StackTabBar::mouseMoveEvent(QMouseEvent *event) {
	// If the left button isn't pressed anymore then return
	if (!(event->buttons() & Qt::LeftButton))
		return;

	// If the distance is too small then return
	if ((event->pos() - m_dragStartPos).manhattanLength()< QApplication::startDragDistance())
		return;

	// initiate Drag
	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData;
	// a crude way to distinguish tab-reodering drops from other ones
	mimeData->setData("action", "tab-reordering") ;
	drag->setMimeData(mimeData);
	drag->exec();
}

void StackTabBar::mousePressEvent(QMouseEvent *event) {
	if(event->button() == Qt::LeftButton) {
		m_dragStartPos = event->pos();
	}

    QTabBar::mousePressEvent(event);
}

void StackTabBar::dragEnterEvent(QDragEnterEvent* event) {
	// Only accept if it's an tab-reordering request
	const QMimeData* m = event->mimeData();
	QStringList formats = m->formats();
	if (formats.contains("action") && (m->data("action") == "tab-reordering")) {
		event->acceptProposedAction();
	}
}

void StackTabBar::dropEvent(QDropEvent* event) {
	int fromIndex = tabAt(m_dragStartPos);
	int toIndex = tabAt(event->pos());

	// Tell interested objects that
	if (fromIndex != toIndex) {
		emit tabMoveRequested(fromIndex, toIndex);
	}

	event->acceptProposedAction();
}


void StackTabBar::mouseReleaseEvent(QMouseEvent *event) {
	if(!rect().contains(event->pos()) && m_pressedIndex > -1) {
		QWidget *tab = m_parent->widget(m_pressedIndex);
		m_parent->removeTab(m_pressedIndex);
		emit tabDetached(tab, event->pos());
	}
	QTabBar::mouseReleaseEvent(event);
}

