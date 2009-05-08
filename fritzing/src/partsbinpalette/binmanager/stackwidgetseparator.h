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


#ifndef STACKWIDGETSEPARATOR_H_
#define STACKWIDGETSEPARATOR_H_

#include <QFrame>
#include <QLabel>
#include <QTabBar>

#include "dropsink.h"

class StackWidgetSeparator : public QFrame, public DropSink {
	Q_OBJECT
	public:
		StackWidgetSeparator(QWidget *parent=0);
		~StackWidgetSeparator();
		void setDragging(bool);
		void expand();
		void shrink();
		void showFeedback(int index, QTabBar::ButtonPosition side, bool doShow=true);

	signals:
		void setPotentialDropSink(DropSink*, QTabBar::ButtonPosition=QTabBar::RightSide, int index=-1);
		void setDropSink(DropSink*, QTabBar::ButtonPosition=QTabBar::LeftSide, int index=-1);
		void dropped();

	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dragLeaveEvent(QDragLeaveEvent *event);
		void dropEvent(QDropEvent* event);
		void resizeEvent(QResizeEvent *event);

		void setPixmapWidth(int newWidth);

		QPixmap *m_feedbackPixmap;
		QLabel *m_feedbackIcon;
};

#endif /* STACKWIDGETSEPARATOR_H_ */
