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

$Revision: 1490 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 13:10:48 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#include <QVBoxLayout>

#include "zoomcontrols.h"
#include "../zoomcombobox.h"
#include "../debugdialog.h"

ZoomButton::ZoomButton(ZoomControls::ZoomType type, SketchWidget* view, QWidget *parent) : QLabel(parent)
{
	QString imgPath = ":/resources/images/icons/partsEditorZoom%1Button.png";
	if(type == ZoomControls::ZoomIn) {
		imgPath = imgPath.arg("In");
		m_step = 5*ZoomComboBox::ZoomStep;
	} else if(type == ZoomControls::ZoomOut) {
		imgPath = imgPath.arg("Out");
		m_step = -5*ZoomComboBox::ZoomStep;
	}

	m_owner = view;
	connect(this, SIGNAL(clicked()), this, SLOT(zoom()) );
	setPixmap(QPixmap(imgPath));
}

void ZoomButton::zoom() {
	m_owner->relativeZoom(m_step);
	m_owner->ensureFixedToBottomRightItems();
}

void ZoomButton::mousePressEvent(QMouseEvent *event) {
	//QLabel::mousePressEvent(event);
	Q_UNUSED(event);
	emit clicked();
}

void ZoomButton::enterEvent(QEvent *event) {
	QLabel::enterEvent(event);
}

void ZoomButton::leaveEvent(QEvent *event) {
	QLabel::leaveEvent(event);
}


///////////////////////////////////////////////////////////

ZoomControlsPrivate::ZoomControlsPrivate(SketchWidget* view) : QFrame()
{
	//setObjectName("zoomControls");

	m_zoomInButton = new ZoomButton(ZoomControls::ZoomIn, view, this);
	m_zoomOutButton = new ZoomButton(ZoomControls::ZoomOut, view, this);

	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->addWidget(m_zoomInButton);
	lo->addWidget(m_zoomOutButton);
	lo->setMargin(2);
	lo->setSpacing(2);

	setStyleSheet("background-color: transparent;");
}

///////////////////////////////////////////////////////////

ZoomControls::ZoomControls(SketchWidget *view) : QGraphicsProxyWidget()
{
	ZoomControlsPrivate *d = new ZoomControlsPrivate(view);
	setFlags(QGraphicsItem::ItemIgnoresTransformations);
	setWidget(d);
	setZValue(10000);
}

