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

$Revision: 1490 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 13:10:48 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#include <QGraphicsScene>
#include <QImage>
#include <QRgb>
#include <QFontMetrics>
#include <QPainter>

#include "viewswitcher.h"
#include "debugdialog.h"
#include "help/sketchmainhelp.h"

QHash<QString, QPixmap *> ViewSwitcherButton::Pixmaps;
QString ViewSwitcherButton::ResourcePathPattern = (":/resources/images/icons/segmentedSwitcher%1%2.png");
QBitmap * ViewSwitcher::m_mask = NULL;

ViewSwitcherButton::ViewSwitcherButton(const QString &view, const QString & text, int index, ViewSwitcher *parent) : QLabel(parent)
{
	m_focus = false;
	m_active = false;
	m_hover = false;
	m_index = index;
	m_resourcePath = ResourcePathPattern.arg(view);
	m_parent = parent;
	
	QFontMetrics fm(this->font());

	QList<QString> actives;
	actives << "Active" << "Inactive";
	QList<QString> focuses;
	focuses << "Focus" << "Dimmed";
	QList<QString> hovers;
	hovers << "Hover" << "";

	foreach (QString active, actives) {
		foreach (QString focus, focuses) {
			foreach (QString hover, hovers) {
				if (focus.compare("Dimmed") == 0 && hover.compare("Hover") == 0) {
					// this state is never visible;
					continue;
				}

				QString name = m_resourcePath.arg(active+focus+hover);
				if (Pixmaps.value(name, NULL) != NULL) {
					// already set this one up
					continue;	
				}

				QString pixmapName = ResourcePathPattern.arg("X").arg(active+focus+hover);
				QPixmap bgPixmap(pixmapName);
				if (bgPixmap.isNull()) continue;

				int textWidth = fm.width(text);

				QPixmap * buttonPixmap = new QPixmap(textWidth + bgPixmap.width() - 1, bgPixmap.height());
				QPainter painter(buttonPixmap);
				QRect r = bgPixmap.rect();
				r.setLeft(bgPixmap.width() / 2);
				r.setWidth(1);
				painter.drawPixmap(buttonPixmap->rect(), bgPixmap, r);
				painter.setFont(this->font());
				painter.drawText(buttonPixmap->rect(), Qt::AlignCenter, text);
				painter.end();

				Pixmaps.insert(name, buttonPixmap);
			}
		}
	}
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
	QPixmap * pixmap = Pixmaps.value(m_resourcePath.arg(activeText+focusText+hoverText));
	if (pixmap != NULL) {
		setPixmap(*pixmap);
	}
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

ViewSwitcher::ViewSwitcher() : QFrame()
{
	// TODO Mariano: couldn't get this applied with the qss file
	setStyleSheet("ViewSwitcher {border: 0px; background-color: transparent; margin-top: 0px; margin-left: 0px; } ViewSwitcherButton {	margin: 0px;}");

	m_mask = NULL;
	m_closeButton = NULL;

	m_layout = new QHBoxLayout(this);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);

	//m_closeButton = new SketchMainHelpCloseButton("PCB" ,this);
	//m_layout->addWidget(m_closeButton);

	m_buttons << createButton("Breadboard", tr("Breadboard"));
	m_buttons << createButton("Schematic", tr("Schematic"));
	m_buttons << createButton("PCB", tr("PCB"));

}

ViewSwitcherButton *ViewSwitcher::createButton(const QString &view, const QString & text) {
	ViewSwitcherButton *btn = new ViewSwitcherButton(view, text, m_buttons.size(), this);
	connect(btn, SIGNAL(clicked(ViewSwitcherButton*)), this, SLOT(updateState(ViewSwitcherButton*)));
	m_layout->addWidget(btn);
	return btn;
}

void ViewSwitcher::enterEvent(QEvent *event) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setFocus(true);
	}
	QFrame::enterEvent(event);

}

void ViewSwitcher::leaveEvent(QEvent *event) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setFocus(false);
	}
	QFrame::leaveEvent(event);
}

void ViewSwitcher::updateHoverState(ViewSwitcherButton* hoverOne) {
	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setHover(btn==hoverOne);
	}
}
void ViewSwitcher::updateState(ViewSwitcherButton* clickedOne, bool doEmit) {


	foreach(ViewSwitcherButton *btn, m_buttons) {
		btn->setActive(btn == clickedOne);
	}
	if(doEmit) emit viewSwitched(clickedOne->index());
}

void ViewSwitcher::viewSwitchedTo(int index) {
	updateState(m_buttons[index],false);
}

void ViewSwitcher::createMask() 
{
	


	if (m_mask != NULL) return;

	setStyleSheet("ViewSwitcher {border: 0px; background-color: rgb(0,255,255); margin-top: 0px; margin-left: 0px; } ViewSwitcherButton {	margin: 0px;}");

	QPixmap pixmap(this->size());
	this->render(&pixmap);
	QImage image = pixmap.toImage();

	QBitmap bitmap(this->size());
	bitmap.fill(Qt::white);
	QImage bImage = bitmap.toImage();

	QRgb value = qRgb(0, 0, 0); 
	bImage.setColor(0, value);
	value = qRgb(255, 255, 255); 
	bImage.setColor(1, value);

	for (int y = 0; y < pixmap.height(); y++) {
		QRgb* scanLine = (QRgb *) image.scanLine(y);
		for (int x = 0; x < pixmap.width(); x++, scanLine++) {
			if (qRed(*scanLine) == 0 && qGreen(*scanLine) == 255 && qBlue(*scanLine) == 255) {
				bImage.setPixel(x, y, 1);
			}
			else {
				bImage.setPixel(x, y, 0);
			}
		}

	}

	setStyleSheet("ViewSwitcher {border: 0px; background-color: transparent; margin-top: 0px; margin-left: 0px; } ViewSwitcherButton {	margin: 0px;}");
	
	m_mask = new QBitmap(QBitmap::fromImage(bImage));
}


const QBitmap * ViewSwitcher::getMask() {
	if (m_mask == NULL) {
		createMask();
	}

	return m_mask;
}

void ViewSwitcher::connectClose(QObject * target, const char* slot) {
	if (m_closeButton) {
		connect(m_closeButton, SIGNAL(clicked()), target, slot);
	}
}

/////////////////////////////////////////

/*
ViewSwitcher::ViewSwitcher(QWidget *parent) : QGraphicsProxyWidget()
{
	ViewSwitcherPrivate *d = new ViewSwitcherPrivate();
	connect(d, SIGNAL(viewSwitched(int)), parent, SLOT(viewSwitchedTo(int)));
	connect(parent, SIGNAL(viewSwitched(int)), d, SLOT(viewSwitchedTo(int)));
	setFlags(QGraphicsItem::ItemIgnoresTransformations);
	setWidget(d);
	setZValue(10000);
}
*/