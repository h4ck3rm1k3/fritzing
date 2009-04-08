/*
 * (c) Fachhochschule Potsdam
 */

#include <QMouseEvent>
#include <QLabel>

#include "stackwidget.h"
#include "../debugdialog.h"

StackTabBar::StackTabBar(StackTabWidget *parent) : QTabBar(parent) {
	m_pressedIndex = -1;
	m_parent = parent;
}

int StackTabBar::tabIndexAtPos(const QPoint &p) const {
	for (int i = 0; i < m_parent->count(); ++i) {
    	if (m_parent->isTabEnabled(i) && tabRect(i).contains(p)) {
    		return i;
    	}
	}
    return -1;
}

void StackTabBar::mouseMoveEvent(QMouseEvent *event) {
	/*if(!rect().contains(event->pos())) {
		setMovable(false);
	} else {
		setMovable(true);
	}*/
	QTabBar::mouseMoveEvent(event);
}

void StackTabBar::mousePressEvent(QMouseEvent *event) {
	m_pressedIndex = tabIndexAtPos(event->pos());
	QTabBar::mousePressEvent(event);
}

void StackTabBar::mouseReleaseEvent(QMouseEvent *event) {
	if(!rect().contains(event->pos()) && m_pressedIndex > -1) {
		QWidget *tab = m_parent->widget(m_pressedIndex);
		m_parent->removeTab(m_pressedIndex);
		emit tabDetached(tab, event->pos());
	}
	QTabBar::mouseReleaseEvent(event);
}

////////////////////////////////////////////////////////////

StackTabWidget::StackTabWidget(QWidget *parent) : QTabWidget(parent) {
	setTabBar(new StackTabBar(this));
	connect(
		tabBar(),SIGNAL(tabDetached(QWidget*, const QPoint&)),
		parent,SLOT(tabDetached(QWidget*, const QPoint&))
	);
}

////////////////////////////////////////////////////////////

StackWidgetSeparator::StackWidgetSeparator(QWidget *parent)
	:QFrame(parent)
{
	setMinimumHeight(10);
}

void StackWidgetSeparator::enterEvent(QEvent *event) {
	DebugDialog::debug("separator enter");
	expand();
	emit setReceptor(this);
	QFrame::enterEvent(event);
}

void StackWidgetSeparator::leaveEvent(QEvent *event) {
	DebugDialog::debug("separator leave");
	shrink();
	emit setReceptor(NULL);
	QFrame::leaveEvent(event);
}

void StackWidgetSeparator::expand() {
	setMinimumHeight(200);
	resize(width(),200);
}

void StackWidgetSeparator::shrink() {
	setMinimumHeight(10);
	resize(width(),10);
}

////////////////////////////////////////////////////////////

StackWidget::StackWidget(QWidget *parent) : QFrame(parent) {
	m_current = NULL;
	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing(1);
	m_layout->setMargin(1);
}

int StackWidget::addWidget(QWidget *widget) {
	m_layout->addWidget(widget);
	m_layout->addWidget(newSeparator());
	if(!m_current) m_current = widget;
	return indexOf(widget);
}

StackWidgetSeparator *StackWidget::newSeparator() {
	StackWidgetSeparator *sep = new StackWidgetSeparator(this);
	connect(
		sep, SIGNAL(setReceptor(StackWidgetSeparator*)),
		this, SLOT(setReceptor(StackWidgetSeparator*))
	);
	return sep;
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

void StackWidget::insertWidget(int index, QWidget *widget) {
	m_layout->insertWidget(index, widget);
	if(!m_current) m_current = widget;
}

void StackWidget::removeWidget(QWidget *widget) {
	m_layout->removeWidget(widget);
	if(m_current == widget) m_current = NULL;
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

void StackWidget::tabDetached(QWidget *tab, const QPoint &pos) {
	int idx = closestIndexToPos(pos);
	DebugDialog::debug(QString("detached idx %1 %2").arg(idx).arg((long)tab));
	insertWidget(idx,new QLabel("detached!",this));
}

int StackWidget::closestIndexToPos(const QPoint &pos) {
	for (int i = 0; i < count(); ++i) {
    	if (widget(i)->rect().contains(pos)) {
    		return i+1;
    	}
	}
    return -1;
}

void StackWidget::setReceptor(StackWidgetSeparator* receptor) {
	m_dropReceptor = receptor;
}
