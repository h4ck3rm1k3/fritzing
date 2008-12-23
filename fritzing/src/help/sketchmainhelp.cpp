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

#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QFile>
#include <QTimer>

#include "sketchmainhelp.h"
#include "../expandinglabel.h"

qreal SketchMainHelp::OpacityLevel = 0.5;

SketchMainHelpCloseButton::SketchMainHelpCloseButton(const QString &imagePath, QWidget *parent)
	:QLabel(parent)
{
	m_pixmap = QPixmap(
		QString(":/resources/images/inViewHelpCloseButton%1.png").arg(imagePath));
	setPixmap(m_pixmap);
	setFixedHeight(m_pixmap.height());
}

SketchMainHelp::~SketchMainHelp() {
}

void SketchMainHelpCloseButton::mousePressEvent(QMouseEvent * event) {
	emit clicked();
	QLabel::mousePressEvent(event);
}

void SketchMainHelpCloseButton::doShow() {
	setPixmap(m_pixmap);
}

void SketchMainHelpCloseButton::doHide() {
	setPixmap(0);
}


//////////////////////////////////////////////////////////////

SketchMainHelpPrivate::SketchMainHelpPrivate (
		const QString &viewString,
		const QString &htmlText,
		SketchMainHelp *parent)
	: QFrame()
{
	setObjectName("sketchMainHelp"+viewString);
	m_parent = parent;

	QFrame *main = new QFrame(this);
	QHBoxLayout *mainLayout = new QHBoxLayout(main);

	QLabel *imageLabel = new QLabel(this);
	QLabel *imageLabelAux = new QLabel(imageLabel);
	imageLabelAux->setObjectName("inviewHelpImage");
	QPixmap pixmap(QString(":/resources/images/helpImage%1.png").arg(viewString));
	imageLabelAux->setPixmap(pixmap);
	imageLabel->setFixedWidth(pixmap.width());
	imageLabel->setFixedHeight(pixmap.height());
	imageLabelAux->setFixedWidth(pixmap.width());
	imageLabelAux->setFixedHeight(pixmap.height());

	ExpandingLabel *textLabel = new ExpandingLabel(this);
	textLabel->setLabelText(htmlText);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	textLabel->allTextVisible();
	textLabel->setToolTip("");
	textLabel->setAlignment(Qt::AlignLeft);

	mainLayout->setSpacing(4);
	mainLayout->setMargin(2);
	mainLayout->addWidget(imageLabel);
	mainLayout->addWidget(textLabel);
	setFixedWidth(430);

	QVBoxLayout *layout = new QVBoxLayout(this);
	m_closeButton = new SketchMainHelpCloseButton(viewString,this);
	connect(m_closeButton, SIGNAL(clicked()), this, SLOT(doClose()));
	layout->addWidget(m_closeButton);
	layout->addWidget(main);
	layout->setSpacing(0);
	layout->setMargin(2);

	m_shouldGetTransparent = false;
	m_closeButton->doHide();

	QFile styleSheet(":/resources/styles/inviewhelp.qss");
    if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/inviewhelp.qss");
	} else {
		setStyleSheet(styleSheet.readAll());
	}
}

void SketchMainHelpPrivate::doClose() {
	emit aboutToClose();
	m_parent->doClose();
}

void SketchMainHelpPrivate::enterEvent(QEvent * event) {
	if(m_shouldGetTransparent) {
		setWindowOpacity(1.0);
		QTimer *timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(timer, SIGNAL(timeout()), this, SLOT(setTransparent()));
		timer->start(2000);
	}
	m_closeButton->doShow();
	QFrame::enterEvent(event);
}

void SketchMainHelpPrivate::setTransparent() {
	setWindowOpacity(SketchMainHelp::OpacityLevel);
}

void SketchMainHelpPrivate::leaveEvent(QEvent * event) {
	if(m_shouldGetTransparent) {
		setTransparent();
	}
	m_closeButton->doHide();
	QFrame::leaveEvent(event);
}


//////////////////////////////////////////////////////////////

SketchMainHelp::SketchMainHelp (
		const QString &viewString,
		const QString &htmlText
	) : QGraphicsProxyWidget()
{
	setObjectName(viewString);
	m_son = new SketchMainHelpPrivate(viewString, htmlText, this);
	setWidget(m_son);
}


void SketchMainHelp::doClose() {
	if(scene()) {
		scene()->removeItem(this);
	}
}

void SketchMainHelp::setTransparent() {
	m_son->setWindowOpacity(OpacityLevel);
	m_son->m_shouldGetTransparent = true;
}
