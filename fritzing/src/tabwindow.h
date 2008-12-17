/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 1666 $:
$Author: merunga $:
$Date: 2008-11-27 17:11:05 +0100 (Thu, 27 Nov 2008) $

********************************************************************/


#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <QWidget>
#include <QHBoxLayout>

class TabWindow : public QWidget
{
    Q_OBJECT

public:
    TabWindow(QWidget *parent = 0);
	void addViewSwitcher(class ViewSwitcherPrivate * viewSwitcher);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

protected:
	QHBoxLayout * m_hLayout;
	bool m_inDrag;
	QPoint m_dragStartPos;
};


#endif
