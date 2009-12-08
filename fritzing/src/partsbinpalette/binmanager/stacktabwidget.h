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


#ifndef STACKTABWIDGET_H_
#define STACKTABWIDGET_H_

#include <QTabWidget>
#include <QTabBar>
#include <QPushButton>
#include <QPointer>

#include "dropsink.h"

// originally extracted from http://wiki.qtcentre.org/index.php?title=Movable_Tabs
class StackTabWidget : public QTabWidget, public DropSink {
	Q_OBJECT
	public:
		StackTabWidget(class StackWidget *parent);
		void showFeedback(int index, QTabBar::ButtonPosition side, bool doShow=true);
		class StackTabBar *stackTabBar();

	public slots:
		void moveTab(int fromIndex, int toIndex);
		void informCurrentChanged(int index);
		void informTabCloseRequested(int index);
		void showFeedback();

	signals:
		void currentChanged(StackTabWidget *, int index);
		void tabCloseRequested(StackTabWidget *, int index);

	protected:
		void dragEnterEvent(QDragEnterEvent* event);

		QPointer<QPushButton> m_feedback;
};

#endif /* STACKTABWIDGET_H_ */
