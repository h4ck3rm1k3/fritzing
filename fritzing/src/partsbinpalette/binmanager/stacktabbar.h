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


#ifndef STACKTABBAR_H_
#define STACKTABBAR_H_

#include <QTabBar>
#include "dropsink.h"

class StackTabBar : public QTabBar {
	Q_OBJECT
	public:
		StackTabBar(class StackTabWidget *parent);

	signals:
		void setDragSource(StackTabWidget*, int index=-1);
		void setDropSink(DropSink*, QTabBar::ButtonPosition=LeftSide, int index=-1);
		void setPotentialDropSink(DropSink*, QTabBar::ButtonPosition, int index=-1);
		void dropped();

	protected:
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent( QMouseEvent *event);
		void dragEnterEvent(QDragEnterEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
		void dropEvent(QDropEvent* event);

		bool mimeIsAction(const class QMimeData* m, const QString& action);
		int tabIndexAtPos(const QPoint &p) const;
		QTabBar::ButtonPosition getButtonPos(int index, const QPoint &pos);
		bool posCloserToTheEnd(const QPoint &pos);


		QPoint m_dragStartPos;
		class StackTabWidget* m_parent;
};

#endif /* STACKTABBAR_H_ */
