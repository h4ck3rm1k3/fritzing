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


#ifndef STACKWIDGET_H_
#define STACKWIDGET_H_

#include <QFrame>
#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QHash>

class StackWidgetSeparator;
class StackTabWidget;

#include "dropsink.h"

struct DragFromOrTo {
	DragFromOrTo(DropSink* _sink = NULL, int _index = -1, QTabBar::ButtonPosition _side = QTabBar::LeftSide) {
		sink = _sink;
		index = _index;
		side = _side;
	}
	DropSink* sink;
	int index;
	QTabBar::ButtonPosition side;
};

class StackWidgetDockTitleBar : public QFrame {
	Q_OBJECT
	public:
		StackWidgetDockTitleBar(class StackWidget*, class FDockWidget*);

	signals:
		void draggingCloseToSeparator(QWidget*,bool);
		void dropToSeparator(QWidget*);

	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dragLeaveEvent(QDragLeaveEvent *event);
		void dragMoveEvent(QDragMoveEvent *event);
		void dropEvent(QDropEvent *event);
};

class StackWidget : public QFrame {
	Q_OBJECT
	public:
		StackWidget(QWidget *parent=0);

		int addWidget(QWidget *widget);
		int count() const;
		int currentIndex() const;
		QWidget *currentWidget() const;
		int indexOf(QWidget *widget) const;
		void insertWidget(int index, QWidget *widget);
		void removeWidget(QWidget *widget);
		QWidget *widget(int index) const;
		bool contains(QWidget *widget) const;

		void setDock(class FDockWidget*);

	public slots:
		void setCurrentIndex(int index);
		void setCurrentWidget(QWidget *widget);

		void setDragSource(StackTabWidget*, int index);
		void setDropSink(DropSink* receptor, QTabBar::ButtonPosition, int index);
		void setPotentialDropSink(DropSink* receptor, QTabBar::ButtonPosition side, int index);
		void dropped();

		void draggingCloseToSeparator(class QWidget*, bool);
		void dropToSeparator(QWidget*);

	signals:
		void currentChanged(int index);
		void widgetRemoved(int index);
		void widgetChangedTabParent(
			QWidget* widgetToMove, StackTabWidget* oldTabWidget, StackTabWidget* newTabWidget
		);


	protected:
		int closestIndexToPos(const QPoint &pos);
		StackWidgetSeparator *newSeparator(QWidget *widget);

		QVBoxLayout *m_layout;
		QWidget *m_current;

		DragFromOrTo m_dragSource;
		DragFromOrTo m_dropSink;
		DragFromOrTo m_potentialDropSink;

		QHash<QWidget*,StackWidgetSeparator*> m_separators;
		StackWidgetSeparator* m_topSeparator;
};

#endif /* STACKWIDGET_H_ */
