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

class StackTabBar : public QTabBar {
	Q_OBJECT
	public:
		StackTabBar(class StackTabWidget *parent);

	signals:
		void tabDetached(QWidget *tab, const QPoint &pos);
		void tabMoveRequested(int fromIndex, int toIndex);

	protected:
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent( QMouseEvent *event);
		void dragEnterEvent(QDragEnterEvent* event);
        //void dragMoveEvent(QDragMoveEvent* event);
		void dropEvent(QDropEvent* event);

		int tabIndexAtPos(const QPoint &p) const;


		QPoint m_dragStartPos;
		int m_dragCurrentIndex;
		int m_pressedIndex;
		class StackTabWidget* m_parent;
};

#endif /* STACKTABBAR_H_ */
