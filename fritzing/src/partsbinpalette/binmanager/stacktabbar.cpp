/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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


#include <QApplication>
#include <QMouseEvent>
#include <QMenu>
#include <QStylePainter>
#include <QStyleOptionTabV2>

#include "stacktabbar.h"
#include "stacktabwidget.h"
#include "../partsbinpalettewidget.h"
#include "../partsbinview.h"
#include "../../debugdialog.h"


StackTabBar::StackTabBar(StackTabWidget *parent) : QTabBar(parent) {
	setAcceptDrops(true);
    //this->setContentsMargins(0,0,0,0);
    //this->setTabsClosable(true);
    setMovable(true);
	m_parent = parent;
	setProperty("current","false");
	setExpanding(false);
	setElideMode(Qt::ElideRight);
	setIconSize(QSize(32, 32));

	setContextMenuPolicy(Qt::CustomContextMenu);
 
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));

}

bool StackTabBar::mimeIsAction(const QMimeData* m, const QString& action) {
	if(m) {
		QStringList formats = m->formats();
		return formats.contains("action") && (m->data("action") == action);
	} else {
		return false;
	}
}

void StackTabBar::dragEnterEvent(QDragEnterEvent* event) {
    // Only accept if it's an part-reordering request
	const QMimeData *m = event->mimeData();
    if (mimeIsAction(m, "part-reordering")) {
		event->acceptProposedAction();
	}
}

void StackTabBar::dragMoveEvent(QDragMoveEvent* event) {
	const QMimeData *m = event->mimeData();
	int index = tabAt(event->pos());
    if ((event->source() != this) && mimeIsAction(m,"part-reordering")) {
		PartsBinPaletteWidget* bin = qobject_cast<PartsBinPaletteWidget*>(m_parent->widget(index));
		if(bin && bin->allowsChanges()) {
			event->acceptProposedAction();
			setCurrentIndex(index);
		}
	}
}

void StackTabBar::dropEvent(QDropEvent* event) {
	int toIndex = tabAt(event->pos());

	const QMimeData *m = event->mimeData();
    if(mimeIsAction(m, "part-reordering")) {
		PartsBinPaletteWidget* bin = qobject_cast<PartsBinPaletteWidget*>(m_parent->widget(toIndex));
		if(bin && bin->allowsChanges()) {
			bin->currentView()->dropEventAux(event,true);
		}
	}

	event->acceptProposedAction();
}
 
void StackTabBar::showContextMenu(const QPoint &point)
{
	if (point.isNull()) return;
 
	int tabIndex = this->tabAt(point);
	PartsBinPaletteWidget* bin = qobject_cast<PartsBinPaletteWidget*>(m_parent->widget(tabIndex));
	if (bin == NULL) return;

	setCurrentIndex(tabIndex);


	QMenu * binMenu = bin->getFileMenu();
	if (binMenu == NULL) return;

	QMenu * partMenu = bin->getPartMenu();
	if (partMenu == NULL) return;

	QMenu menu;
	menu.addMenu(partMenu);
	menu.addMenu(binMenu);
	menu.exec(this->mapToGlobal(point));
}


void StackTabBar::paintEvent(QPaintEvent *event)
{    
    QStylePainter painter(this);

    for(int i = 0; i < this->count(); ++i)
    {
        QStyleOptionTabV2 option;
        initStyleOption(&option, i);
        //printf("tab text: %s\n", option.text.toLatin1().data());
		option.text = "";
        painter.drawControl(QStyle::CE_TabBarTab, option);
    }
}