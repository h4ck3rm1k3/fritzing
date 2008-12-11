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

#include <QHBoxLayout>
#include <QGraphicsScene>

#include "sketchmainhelp.h"
#include "../expandinglabel.h"

SketchMainHelpCloseButton::SketchMainHelpCloseButton(const QString &imagePath, QWidget *parent)
	:QLabel(parent)
{
	setPixmap(QPixmap(imagePath));
}

void SketchMainHelpCloseButton::mousePressEvent(QMouseEvent * event) {
	emit clicked();
	QLabel::mousePressEvent(event);
}

SketchMainHelpPrivate::SketchMainHelpPrivate (const QString &imagePath, const QString &htmlText, SketchMainHelp *parent)
	: QFrame()
{
	m_parent = parent;

	QFrame *main = new QFrame(this);
	QHBoxLayout *mainLayout = new QHBoxLayout(main);
	QLabel *imageLabel = new QLabel(this);
	imageLabel->setPixmap(QPixmap(imagePath));
	ExpandingLabel *textLabel = new ExpandingLabel(this);
	textLabel->setLabelText(htmlText);
	textLabel->setToolTip("");
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->addWidget(imageLabel);
	mainLayout->addWidget(textLabel);
	setFixedWidth(430);

	QVBoxLayout *layout = new QVBoxLayout(this);
	SketchMainHelpCloseButton *closeBtn = new SketchMainHelpCloseButton(":/resources/images/inViewHelpCloseButtonBreadboard.png",this);
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(doClose()));
	layout->addWidget(closeBtn);
	layout->addWidget(main);
	layout->setSpacing(0);
	layout->setMargin(0);
}

void SketchMainHelpPrivate::doClose() {
	m_parent->doClose();
}

SketchMainHelp::SketchMainHelp (
		const QString &imagePath,
		const QString &htmlText
	) : QGraphicsProxyWidget()
{
	SketchMainHelpPrivate *son = new SketchMainHelpPrivate(imagePath, htmlText, this);
	setWidget(son);
}


void SketchMainHelp::doClose() {
	scene()->removeItem(this);
}
