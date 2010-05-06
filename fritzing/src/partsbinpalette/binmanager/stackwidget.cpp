/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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


#include <QLabel>

#include "stackwidget.h"
#include "stacktabwidget.h"
#include "binmanager.h"
#include "../../fdockwidget.h"
#include "../../debugdialog.h"


///////////////////////////////////////////////////////////////////

StackWidget::StackWidget(QWidget *parent) : QFrame(parent) {
	m_current = NULL;
	m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(1);
    m_layout->setMargin(1);
}

int StackWidget::addWidget(QWidget *widget) {
	m_layout->addWidget(widget);
	if(!m_current) m_current = widget;
	return indexOf(widget);
}

void StackWidget::insertWidget(int index, QWidget *widget) {
	m_layout->insertWidget(index, widget);
	//if(!m_current) m_current = widget;
}

void StackWidget::removeWidget(QWidget *widget) {
	m_layout->removeWidget(widget);
	if(m_current == widget) m_current = NULL;
	widget->hide();
	//widget->setParent(NULL);
	//delete widget;
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

void StackWidget::setDock(FDockWidget* dock) {
	Q_UNUSED(dock)
}
