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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include <QMouseEvent>
#include <QTimer>

#include "stacktabwidget.h"
#include "stackwidget.h"
#include "stacktabbar.h"
#include "../../debugdialog.h"

StackTabWidget::StackTabWidget(StackWidget *parent) : QTabWidget(parent) {
	setTabBar(new StackTabBar(this));

	connect(
		tabBar(),SIGNAL(setDragSource(StackTabWidget*, int)),
		parent,SLOT(setDragSource(StackTabWidget*, int)),
		Qt::DirectConnection
	);
	connect(
		tabBar(),SIGNAL(setDropSink(DropSink*,QTabBar::ButtonPosition, int)),
		parent,SLOT(setDropSink(DropSink*,QTabBar::ButtonPosition, int)),
		Qt::DirectConnection
	);
	connect(
		tabBar(),SIGNAL(setPotentialDropSink(DropSink*, QTabBar::ButtonPosition, int)),
		parent,SLOT(setPotentialDropSink(DropSink*, QTabBar::ButtonPosition, int)),
		Qt::DirectConnection
	);
	connect(
		tabBar(),SIGNAL(dropped()),
		parent,SLOT(dropped())
	);

	QPixmap pixmap = QPixmap(":/resources/images/icons/binRearrangeTabs.png");
	QIcon icon(pixmap);
	m_feedback = new QPushButton(this);
	m_feedback->setIcon(icon);
	m_feedback->hide();
	m_feedback->setEnabled(false);
	m_feedback->setFlat(true);
	m_feedback->setFixedWidth(10);
	m_feedback->setStyleSheet("background-color: transparent;");

#if QT_VERSION >= 0x040500
	//this->setMovable(true);
	//this->setTabsClosable(true);
#endif

	connect(
		this, SIGNAL(currentChanged(int)),
		this, SLOT(informCurrentChanged(int))
	);
	connect(
		this, SIGNAL(tabCloseRequested(int)),
		this, SLOT(informTabCloseRequested(int))
	);
}

void StackTabWidget::dragEnterEvent(QDragEnterEvent* event) {
	// Only accept if it's an tab-reordering request
	const QMimeData* m = event->mimeData();
	QStringList formats = m->formats();
	if (formats.contains("action") && (m->data("action") == "tab-reordering")) {
		event->acceptProposedAction();
	}
}

void StackTabWidget::moveTab(int fromIndex, int toIndex) {
	QWidget* w = widget(fromIndex);
	QIcon icon = tabIcon(fromIndex);
	QString text = tabText(fromIndex);

	removeTab(fromIndex);
	insertTab(toIndex, w, icon, text);
	setCurrentIndex(toIndex);
}

void StackTabWidget::showFeedback(int index, QTabBar::ButtonPosition side, bool doShow) {
	QTimer * timer = new QTimer();
	timer->setSingleShot(true);
	timer->setInterval(10);
	timer->setProperty("index", index);
	timer->setProperty("side", side);
	timer->setProperty("show", doShow);
	connect(timer, SIGNAL(timeout()), this, SLOT(showFeedback()));
	timer->start();
}

void StackTabWidget::showFeedback() {
	QObject * sndr = sender();
	if (sndr == NULL) return;

	int index = sndr->property("index").toInt();
	bool doShow = sndr->property("show").toBool();
	QTabBar::ButtonPosition side = (QTabBar::ButtonPosition) sndr->property("side").toInt();

	if (m_feedback) {
		m_feedback->setVisible(doShow);
	}
	if(doShow) {
		tabBar()->setTabButton(index,side,m_feedback);
		QTabBar::ButtonPosition otherSide = side == QTabBar::RightSide? QTabBar::LeftSide: QTabBar::RightSide;
		tabBar()->setTabButton(index,otherSide,NULL);
	} else {
		if (m_feedback) {
			m_feedback->setParent(this);
		}
		tabBar()->setTabButton(index,QTabBar::LeftSide,NULL);
		tabBar()->setTabButton(index,QTabBar::RightSide,NULL);
	}

	sndr->deleteLater();
}

StackTabBar *StackTabWidget::stackTabBar() {
	return dynamic_cast<StackTabBar*>(tabBar());
}

void StackTabWidget::informCurrentChanged(int index) {
	emit currentChanged(this,index);
}

void StackTabWidget::informTabCloseRequested(int index) {
	emit tabCloseRequested(this, index);
}

