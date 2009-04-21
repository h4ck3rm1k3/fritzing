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

	m_dragSource = DragFromOrTo(NULL,-1);
	m_dropSink = DragFromOrTo(NULL,-1);
	m_potentialDropSink = DragFromOrTo(NULL,-1);

	m_layout->addWidget(newSeparator(this));
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
		sep, SIGNAL(setDropSink(DropSink*,int)),
		this, SLOT(setDropSink(DropSink*,int))
	);
	connect(
		sep, SIGNAL(setPotentialDropSink(DropSink*,int)),
		this, SLOT(setPotentialDropSink(DropSink*,int))
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
	m_layout->insertWidget(index+1, newSeparator(widget));
	//if(!m_current) m_current = widget;
}

void StackWidget::removeWidget(QWidget *widget) {
	m_layout->removeWidget(widget);
	if(m_current == widget) m_current = NULL;
	if(m_separators.contains(widget)) {
		StackWidgetSeparator* sep = m_separators[widget];
		m_separators.remove(widget);
		m_layout->removeWidget(sep);
		sep->hide();
		//sep->setParent(NULL);
		//delete sep;
	}
	widget->hide();
	//widget->setParent(NULL);
	//delete widget;
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
	m_dragSource = DragFromOrTo(tabWidget,index);
}

void StackWidget::setDropSink(DropSink* receptor, int index) {
	m_dropSink = DragFromOrTo(receptor,index);
}

void StackWidget::setPotentialDropSink(DropSink* receptor, int index) {
	DropSink *oldOne = m_potentialDropSink.first;
	if(oldOne && (oldOne != receptor || m_potentialDropSink.second != index)) {
		oldOne->showFeedback(m_potentialDropSink.second, false);
	}
	m_potentialDropSink = DragFromOrTo(receptor,index);
	m_potentialDropSink.first->showFeedback(m_potentialDropSink.second, true);
}

void StackWidget::dropped() {
	StackTabWidget *oldTabWidget = dynamic_cast<StackTabWidget*>(m_dragSource.first);
	if(oldTabWidget && m_dropSink.first) {
		m_potentialDropSink.first->showFeedback(m_potentialDropSink.second, false);
		int fromIndex  = m_dragSource.second;
		int toIndex = m_dropSink.second;
		QWidget *widgetToMove = oldTabWidget->widget(fromIndex);
		QIcon icon = oldTabWidget->tabIcon(fromIndex);
		QString text = oldTabWidget->tabText(fromIndex);

		StackTabWidget *newTabWidget = dynamic_cast<StackTabWidget*>(m_dropSink.first);
		if(!newTabWidget) { // dropping into a container, not an existing tabwidget
			int whereToInsert = indexOf(dynamic_cast<QWidget*>(m_dropSink.first))+1;
			newTabWidget = new StackTabWidget(this);
// this functions are only available on 4.5.0 or later
#if QT_VERSION >= 0x040500
			newTabWidget->setTabsClosable(true);
			//newTabWidget->setMovable(true);
#endif
			oldTabWidget->removeTab(fromIndex);
			newTabWidget->addTab(widgetToMove, icon, text);
			insertWidget(whereToInsert,newTabWidget);
		} else {
			if(oldTabWidget != newTabWidget || fromIndex != toIndex) { // is the user really rearranging?
				oldTabWidget->removeTab(fromIndex);
				oldTabWidget->setCurrentIndex(-1);
				newTabWidget->insertTab(toIndex, widgetToMove, icon, text);
				newTabWidget->setCurrentIndex(toIndex);
			}
		}

		StackWidgetSeparator *curSeparator = m_separators[oldTabWidget];
		curSeparator->shrink();
		if(oldTabWidget->count() == 0) { // if the tabwidget is now empty, remove it
			removeWidget(oldTabWidget);
			removeWidget(curSeparator);

			oldTabWidget->hide();
			curSeparator->hide();

			m_separators.remove(oldTabWidget);

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
