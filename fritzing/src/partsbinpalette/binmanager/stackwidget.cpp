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

/*void StackWidget::tabDetached(QWidget *tab, const QPoint &pos) {
	int idx = closestIndexToPos(pos);
	DebugDialog::debug(QString("detached idx %1 %2").arg(idx).arg((long)tab));
	insertWidget(idx,new QLabel("detached!",this));
}*/

void StackWidget::setDragSource(StackTabWidget* tabWidget, int index) {
	m_dragSource = DragFromOrTo(tabWidget,index);
}

void StackWidget::setDropReceptor(QWidget* receptor, int index) {
	m_dropReceptor = DragFromOrTo(receptor,index);
}

void StackWidget::dropped() {
	int whereToInsert = indexOf(m_dropReceptor.first);
	StackTabWidget *oldTab = dynamic_cast<StackTabWidget*>(m_dragSource.first);
	if(oldTab) {
		int srcIndex = m_dragSource.second;
		QWidget *widgetToMove = oldTab->widget(srcIndex);
		QString title = oldTab->tabText(srcIndex);

		oldTab->removeTab(srcIndex);
		StackTabWidget *newTab = new StackTabWidget(this);
		newTab->addTab(widgetToMove,title);
		insertWidget(whereToInsert,newTab);

		if(oldTab->count() == 0) {
			StackWidgetSeparator *sepToRemove = m_separators[oldTab];
			Q_ASSERT(sepToRemove);
			removeWidget(oldTab);
			removeWidget(sepToRemove);
			oldTab->hide();
			sepToRemove->hide();
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
