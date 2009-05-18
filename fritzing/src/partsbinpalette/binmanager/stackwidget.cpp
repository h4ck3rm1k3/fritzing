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
#include "binmanager.h"
#include "../partsbinpalettewidget.h"
#include "../../fdockwidget.h"
#include "../../debugdialog.h"


StackWidgetDockTitleBar::StackWidgetDockTitleBar(class StackWidget* stackW, FDockWidget* dock)
	: QFrame(stackW)
{
	Q_UNUSED(dock);
	setAcceptDrops(true);
	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->setMargin(0);
	lo->setSpacing(0);
	//lo->addWidget(dock->layout()->itemAt(2)->widget());

	//DebugDialog::debug(dock->layout()->itemAt(2)->widget()->metaObject()->className());

	/*QLayout *dlo = dock->layout();
	DebugDialog::debug(QString("%1").arg(dlo->count()));*/

	connect(
		this, SIGNAL(draggingCloseToSeparator(QWidget*,bool)),
		stackW, SLOT(draggingCloseToSeparator(QWidget*,bool))
	);

	connect(
		this, SIGNAL(dropToSeparator(QWidget*)),
		stackW, SLOT(dropToSeparator(QWidget*))
	);
}

void StackWidgetDockTitleBar::dragEnterEvent(QDragEnterEvent *event) {
	if(BinManager::isTabReorderingEvent(event)) {
		event->acceptProposedAction();
	}
	QFrame::dragEnterEvent(event);
}

void StackWidgetDockTitleBar::dragLeaveEvent(QDragLeaveEvent *event) {
	emit draggingCloseToSeparator(this,false);
	QFrame::dragLeaveEvent(event);
}

void StackWidgetDockTitleBar::dragMoveEvent(QDragMoveEvent *event) {
	if(BinManager::isTabReorderingEvent(event)) {
		event->acceptProposedAction();
		emit draggingCloseToSeparator(this,true);
	}
	QFrame::dragMoveEvent(event);
}

void StackWidgetDockTitleBar::dropEvent(QDropEvent *event) {
	if(BinManager::isTabReorderingEvent(event)) {
		emit dropToSeparator(this);
	}
	QFrame::dropEvent(event);
}

///////////////////////////////////////////////////////////////////

StackWidget::StackWidget(QWidget *parent) : QFrame(parent) {
	m_current = NULL;
	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing(1);
	m_layout->setMargin(1);

	m_dragSource = DragFromOrTo();
	m_dropSink = DragFromOrTo();
	m_potentialDropSink = DragFromOrTo();

	m_topSeparator = newSeparator(this);
	m_layout->addWidget(m_topSeparator);
}

int StackWidget::addWidget(QWidget *widget) {
	m_layout->addWidget(widget);
	m_layout->addWidget(newSeparator(widget));
	if(!m_current) m_current = widget;
	return indexOf(widget);
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

StackWidgetSeparator *StackWidget::newSeparator(QWidget *widget) {
	StackWidgetSeparator *sep = new StackWidgetSeparator(this);
	connect(
		sep, SIGNAL(setDropSink(DropSink*,QTabBar::ButtonPosition,int)),
		this, SLOT(setDropSink(DropSink*,QTabBar::ButtonPosition,int))
	);
	connect(
		sep, SIGNAL(setPotentialDropSink(DropSink*,QTabBar::ButtonPosition,int)),
		this, SLOT(setPotentialDropSink(DropSink*,QTabBar::ButtonPosition,int))
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

void StackWidget::setDropSink(DropSink* receptor, QTabBar::ButtonPosition side, int index) {
	m_dropSink = DragFromOrTo(receptor,index,side);
}

void StackWidget::setPotentialDropSink(DropSink* receptor, QTabBar::ButtonPosition side, int index) {
	DropSink *oldOne = m_potentialDropSink.sink;
	if(oldOne && (oldOne != receptor || m_potentialDropSink.index != index)) {
		oldOne->showFeedback(m_potentialDropSink.index, side, false);
	}
	m_potentialDropSink = DragFromOrTo(receptor,index,side);
	if(m_potentialDropSink.sink) {
		m_potentialDropSink.sink->showFeedback(m_potentialDropSink.index, side, true);
	}
}

void StackWidget::dropped() {
	StackTabWidget *oldTabWidget = dynamic_cast<StackTabWidget*>(m_dragSource.sink);
	if(oldTabWidget && m_dropSink.sink) {
		m_potentialDropSink.sink->showFeedback(m_potentialDropSink.index, QTabBar::LeftSide, false);
		int fromIndex  = m_dragSource.index;
		int toIndex = m_dropSink.index;
		QWidget *widgetToMove = oldTabWidget->widget(fromIndex);
		QIcon icon = oldTabWidget->tabIcon(fromIndex);
		QString text = oldTabWidget->tabText(fromIndex);

		StackTabWidget *newTabWidget = dynamic_cast<StackTabWidget*>(m_dropSink.sink);
		if(!newTabWidget) { // dropping into a container, not an existing tabwidget
			int whereToInsert = indexOf(dynamic_cast<QWidget*>(m_dropSink.sink))+1;
			newTabWidget = new StackTabWidget(this);
			oldTabWidget->removeTab(fromIndex);
			newTabWidget->addTab(widgetToMove, icon, text);
			insertWidget(whereToInsert,newTabWidget);
		} else {
			if(oldTabWidget != newTabWidget || fromIndex != toIndex) { // is the user really rearranging?
				oldTabWidget->removeTab(fromIndex);
				oldTabWidget->setCurrentIndex(-1);
				int realToIndex = m_dropSink.side == QTabBar::RightSide? -1: toIndex;
				newTabWidget->insertTab(realToIndex, widgetToMove, icon, text);
				newTabWidget->setCurrentIndex(realToIndex);
			}
		}
		emit widgetChangedTabParent(widgetToMove, oldTabWidget, newTabWidget);

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

void StackWidget::draggingCloseToSeparator(QWidget* w, bool really) {
	if(m_separators.contains(w)) {
		if(really) {
			setPotentialDropSink(m_separators[w],QTabBar::LeftSide,-1);
		} else {
			setPotentialDropSink(NULL,QTabBar::LeftSide,-1);
		}
	}
}

void StackWidget::dropToSeparator(QWidget* w) {
	if(m_separators.contains(w)) {
		setDropSink(m_separators[w],QTabBar::LeftSide,true);
		dropped();
	}
}

void StackWidget::setDock(FDockWidget* dock) {
	Q_UNUSED(dock)
	/*StackWidgetDockTitleBar* dtb = new StackWidgetDockTitleBar(this,dock);
	m_separators.remove(this);
	m_separators[dtb] = m_topSeparator;
	dock->setTitleBarWidget(dtb);*/
}
