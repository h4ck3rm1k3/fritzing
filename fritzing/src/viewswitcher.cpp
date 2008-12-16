/*
 * (c) Fachhochschule Potsdam
 */

#include <QGraphicsScene>

#include "viewswitcher.h"
#include "debugdialog.h"

QString ViewSwitcherButton::ResourcePathPattern = tr(":/resources/images/icons/segmentedSwitcher%1%2.png");

ViewSwitcherButton::ViewSwitcherButton(const QString &view, int index, ViewSwitcherPrivate *parent) : QLabel(parent)
{
	m_focus = false;
	m_active = false;
	m_hover = false;
	m_index = index;
	m_resourcePath = ResourcePathPattern.arg(view);
	m_parent = parent;
}

void ViewSwitcherButton::setFocus(bool focus) {
	m_focus = focus;
	updateImage();
}

void ViewSwitcherButton::setActive(bool active) {
	m_active = active;
	updateImage();
}

void ViewSwitcherButton::setHover(bool hover) {
	m_hover = hover;
	updateImage();
}

int ViewSwitcherButton::index() {
	return m_index;
}

void ViewSwitcherButton::updateImage() {
	QString activeText = m_active ? "Active" : "Inactive";
	QString focusText = m_focus ? "Focus" : "Dimmed";
	QString hoverText = m_hover ? "Hover" : "";
	setPixmap(QPixmap(m_resourcePath.arg(activeText+focusText+hoverText)));
}

void ViewSwitcherButton::mousePressEvent(QMouseEvent *event) {
	emit clicked(this);
	QLabel::mousePressEvent(event);
}

void ViewSwitcherButton::enterEvent(QEvent *event) {
	m_parent->updateHoverState(this);
	QLabel::enterEvent(event);
}

void ViewSwitcherButton::leaveEvent(QEvent *event) {
	m_parent->updateHoverState();
	QLabel::leaveEvent(event);
}

/////////////////////////////////////////////////////////////////////

ViewSwitcherPrivate::ViewSwitcherPrivate() : QFrame()
{
	// TODO Mariano: couldn't get this applied with the qss file
	setStyleSheet("ViewSwitcherPrivate {border: 0px; background-color: transparent; margin-top: 10px; margin-left: 10px; } ViewSwitcherButton {	margin: 0px;}");

	m_layout = new QHBoxLayout(this);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);

	m_buttons << createButton("Breadboard");
	m_buttons << createButton("Schematic");
	m_buttons << createButton("PCB");
}

ViewSwitcherButton *ViewSwitcherPrivate::createButton(const QString &view) {
	ViewSwitcherButton *btn = new ViewSwitcherButton(view, m_buttons.size(), this);
	connect(btn, SIGNAL(clicked(ViewSwitcherButton*)), this, SLOT(updateState(ViewSwitcherButton*)));
	m_layout->addWidget(btn);
	return btn;
}

void ViewSwitcherPrivate::enterEvent(QEvent *event) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setFocus(true);
	}
	QFrame::enterEvent(event);
}

void ViewSwitcherPrivate::leaveEvent(QEvent *event) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setFocus(false);
	}
	QFrame::leaveEvent(event);
}

void ViewSwitcherPrivate::updateHoverState(ViewSwitcherButton* hoverOne) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setHover(btn==hoverOne);
	}
}


void ViewSwitcherPrivate::updateState(ViewSwitcherButton* clickedOne, bool doEmit) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setActive(btn == clickedOne);
	}
	if(doEmit) emit viewSwitched(clickedOne->index());
}

void ViewSwitcherPrivate::viewSwitchedTo(int index) {
	updateState(m_buttons[index],false);
}

ViewSwitcher::ViewSwitcher(QWidget *parent) : QGraphicsProxyWidget()
{
	ViewSwitcherPrivate *d = new ViewSwitcherPrivate();
	connect(d, SIGNAL(viewSwitched(int)), parent, SLOT(viewSwitchedTo(int)));
	connect(parent, SIGNAL(viewSwitched(int)), d, SLOT(viewSwitchedTo(int)));
	setFlags(QGraphicsItem::ItemIgnoresTransformations);
	setWidget(d);
	setZValue(10000);
}
