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


#include <QLabel>

#include "stackwidget.h"
#include "stacktabwidget.h"
#include "stackwidgetseparator.h"
#include "../../debugdialog.h"


StackWidget::StackWidget(QWidget *parent) : QFrame(parent) {
	m_current = NULL;
	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing(1);
	m_layout->setMargin(1);
}

int StackWidget::addWidget(QWidget *widget) {
	m_layout->addWidget(widget);
	m_layout->addWidget(newSeparator(widget));
	if(!m_current) m_current = widget;
	return indexOf(widget);
}

StackWidgetSeparator *StackWidget::newSeparator(QWidget *widget) {
	StackWidgetSeparator *sep = new StackWidgetSeparator(this);
	connect(
		sep, SIGNAL(setDropReceptor(QWidget*,int)),
		this, SLOT(setDropReceptor(QWidget*,int))
	);
	connect(
		sep, SIGNAL(dropped()),
		this, SLOT(dropped())
	);
	m_separators[widget] = sep;

	return sep;
}

int StackWidget::count() const {
	return m_layout->count();
}

int StackWidget::currentIndex() const {
	return indexOf(currentWidget());
}

QWidget *StackWidget::currentWidget() const {
	return m_current;
}

int StackWidget::indexOf(QWidget *widget) const {
	return m_layout->indexOf(widget);
}

void StackWidget::insertWidget(int index, QWidget *widget) {
	m_layout->insertWidget(index, widget);
	m_layout->insertWidget(index, newSeparator(widget));
	if(!m_current) m_current = widget;
}

void StackWidget::removeWidget(QWidget *widget) {
	m_layout->removeWidget(widget);
	if(m_current == widget) m_current = NULL;
}

QWidget *StackWidget::widget(int index) const {
	QLayoutItem *item = m_layout->itemAt(index);
	if(item) {
		return item->widget();
	} else {
		return NULL;
	}
}

void StackWidget::setCurrentIndex(int index) {
	setCurrentWidget(widget(index));
}

void StackWidget::setCurrentWidget(QWidget *widget) {
	if(!contains(widget)) addWidget(widget);
	m_current = widget;
	//emit currentChanged(indexOf(m_current));
}

bool StackWidget::contains(QWidget *widget) const {
	return indexOf(widget) > -1;
}

void StackWidget::setDragSource(StackTabWidget* tabWidget, int index) {
	DebugDialog::debug(QString("setting drag source %1 index %2").arg((long)tabWidget).arg(index));
	m_dragSource = DragFromOrTo(tabWidget,index);
}

void StackWidget::setDropReceptor(QWidget* receptor, int index) {
	DebugDialog::debug(QString("setting drop receptor %1 index %2").arg((long)receptor).arg(index));
	m_dropReceptor = DragFromOrTo(receptor,index);
}

void StackWidget::dropped() {
	StackTabWidget *oldTab = dynamic_cast<StackTabWidget*>(m_dragSource.first);
	if(oldTab) {
		int fromIndex  = m_dragSource.second;
		QWidget *widgetToMove = oldTab->widget(fromIndex);
		QString text = oldTab->tabText(fromIndex);

		oldTab->removeTab(fromIndex);
		StackTabWidget *newTab = dynamic_cast<StackTabWidget*>(m_dropReceptor.first);
		if(!newTab) {
			int whereToInsert = indexOf(m_dropReceptor.first);
			newTab = new StackTabWidget(this);
			newTab->addTab(widgetToMove,text);
			insertWidget(whereToInsert,newTab);
		} else {
			int toIndex = m_dropReceptor.second;
			if(oldTab != newTab && fromIndex != toIndex) {
				DebugDialog::debug(QString("from: %1  to: %2").arg(fromIndex).arg(toIndex));
				QIcon icon = oldTab->tabIcon(fromIndex);

				oldTab->setCurrentIndex(-1);
				newTab->insertTab(toIndex, widgetToMove, icon, text);
				newTab->setCurrentIndex(toIndex);
			}
		}

		StackWidgetSeparator *curSeparator = m_separators[oldTab];
		curSeparator->shrink();
		if(oldTab->count() == 0) {
			removeWidget(oldTab);
			removeWidget(curSeparator);
			oldTab->hide();
			curSeparator->hide();
			m_separators.remove(oldTab);
			//delete oldTab;
			//delete sepToRemove;
		}
	}
}

int StackWidget::closestIndexToPos(const QPoint &pos) {
	for (int i = 0; i < count(); ++i) {
    	if (widget(i)->rect().contains(pos)) {
    		return i+1;
    	}
	}
    return -1;
}
