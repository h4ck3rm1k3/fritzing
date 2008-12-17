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

$Revision$:
$Author$:
$Date$

********************************************************************/


#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QAction>


class TabWindow : public QWidget
{
    Q_OBJECT

public:
    TabWindow(QWidget *parent = 0);
	void addViewSwitcher(class ViewSwitcher * viewSwitcher);
	QAction * toggleViewAction() const;
	void setWindowTitle(const QString & title);
	void saveState();
	void restoreState();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	bool event(QEvent *event);

protected:
	QHBoxLayout * m_hLayout;
	bool m_inDrag;
	QPoint m_dragStartPos;
	QAction * m_toggleViewAction;
	int m_viewIndex;
	bool m_movedEnough;
	bool m_state;
	
protected slots:
	void toggleMe(bool);
	void viewSwitched(int);
};

#endif
