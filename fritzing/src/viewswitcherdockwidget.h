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

$Revision: 2136 $:
$Author: merunga $:
$Date: 2009-01-09 18:26:29 +0100 (Fri, 09 Jan 2009) $

********************************************************************/


#ifndef VIEWSWITCHERDOCKWIDGET_H
#define VIEWSWITCHERDOCKWIDGET_H

#include "fdockwidget.h"


class ViewSwitcherDockWidget : public FDockWidget
{
    Q_OBJECT

public:
    ViewSwitcherDockWidget(const QString & title, QWidget * parent = 0);
	~ViewSwitcherDockWidget();
	
	void setViewSwitcher(class ViewSwitcher *);

public slots:
	void windowMoved(QWidget *);

protected:
	void calcWithin();
	//void enterEvent(QEvent *event);
	//void leaveEvent(QEvent *event);
	bool event(QEvent *event);
	void resizeEvent(QResizeEvent * event);


protected:
	//int m_viewIndex;
	class ViewSwitcher * m_viewSwitcher;
	QPoint m_offsetFromParent;
	bool m_within;
	QBitmap * m_bitmap;

protected slots:
	//void viewSwitched(int);
};

#endif
