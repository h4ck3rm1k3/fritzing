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

#include <QtGui>

#include "tabwindow.h"
#include "viewswitcher.h"

TabWindow::TabWindow(QWidget *parent)
    : QWidget(parent)
{
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
	resize(20, 20);
	m_hLayout = new QHBoxLayout;
	this->setLayout(m_hLayout);
	m_hLayout->setSpacing(0);
	m_hLayout->setMargin(0);

    m_toggleViewAction = new QAction(this);
    m_toggleViewAction->setCheckable(true);
    m_toggleViewAction->setText(windowTitle());
    QObject::connect(m_toggleViewAction, SIGNAL(triggered(bool)), this, SLOT(toggleMe(bool)));

}

void TabWindow::addViewSwitcher(ViewSwitcherPrivate * viewSwitcher) {
	viewSwitcher->setParent(this);
	m_hLayout->addWidget(viewSwitcher);
	QApplication::processEvents();
}

void TabWindow::mousePressEvent(QMouseEvent *event)
{
	m_inDrag = true;
	m_dragStartPos = event->globalPos() - this->pos();
}

void TabWindow::mouseReleaseEvent(QMouseEvent *   event )
{
	Q_UNUSED(event);
	m_inDrag = false;
}

void TabWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (m_inDrag) {
		QPoint pos = event->globalPos();
		this->move(pos - m_dragStartPos);
	}
} 

void TabWindow::toggleMe(bool b)
{
    if (b == isHidden()) {
        if (b)
            show();
        else
            close();
    }
}
