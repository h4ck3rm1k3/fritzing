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

/*
 *  aboutbox.cpp
 *  Fritzing
 *
 *  Created by Dirk van Oosterbosch on 18-10-08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include <QLabel>
#include <QScrollArea>
#include <QFont>
#include <QChar>
#include <QTime>
#include <QTimer>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QScrollBar>

#include "aboutbox.h"
#include "debugdialog.h"
#include "version.h"

AboutBox* AboutBox::singleton = NULL;

AboutBox::AboutBox(QWidget *parent)
: QWidget(parent)
{
	singleton = this;
	// To make the application not quit when the window closes
	this->setAttribute(Qt::WA_QuitOnClose, FALSE);

	setFixedSize(340, 358);

	// the background color
	//setStyleSheet("background-color: #F0F0F0");

	// the new Default Font
	QFont smallFont;
	smallFont.setPointSize(11);
	QFont extraSmallFont;
	extraSmallFont.setPointSize(9);
	extraSmallFont.setLetterSpacing(QFont::PercentageSpacing, 92);

	// Big Icon
	QLabel *logoShield = new QLabel(this);
	logoShield->setPixmap(QPixmap(":/resources/images/AboutBoxLogoShield.png"));
	logoShield->setGeometry(106, 8, 128, 128);

	// Version String
	QLabel *versionMain = new QLabel(this);
	versionMain->setText(tr("Version %1.%2 %3 <small>(revision %4)</small>")
						 .arg(Version::majorVersion())
						 .arg(Version::minorVersion())
						 .arg(Version::modifier())
						 .arg(Version::revision()));
	versionMain->setFont(smallFont);
	versionMain->setGeometry(20, 150, 300, 20);
	versionMain->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

	// Link to website
	QLabel *linkToFritzing = new QLabel(this);
	linkToFritzing->setText(tr("<a href=\"http://www.fritzing.org\">www.fritzing.org</a>"));
	linkToFritzing->setOpenExternalLinks(TRUE);
	linkToFritzing->setFont(smallFont);
	linkToFritzing->setGeometry(20, 168, 300, 18);
	linkToFritzing->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	linkToFritzing->setOpenExternalLinks(TRUE);

	// Copyright messages
	
	QLabel *copyrightNotice = new QLabel(this);
	copyrightNotice->setPixmap(QPixmap(":/resources/images/aboutbox_copyright_notice2008.png"));
	copyrightNotice->setGeometry(5, 326, 330, 32);
	
	/* Copyright notice is now an image, no text.
	QLabel *copyrightFHPText = new QLabel(this);
	copyrightFHPText->setText(tr("Open Source %1 2007-2008 University of Applied Sciences Potsdam FHP :-)").arg(QChar::QChar(169)));
	copyrightFHPText->setFont(extraSmallFont);
	copyrightFHPText->setGeometry(10, 322, 320, 16);
	copyrightFHPText->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

	QLabel *licenceText = new QLabel(this);
	licenceText->setText(tr("GNU GPL v2 on code and CreativeCommons:BY on the rest"));
	licenceText->setFont(extraSmallFont);
	licenceText->setGeometry(10, 336, 320, 16);
	licenceText->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	 */
	 
	// Scrolling Credits Text

	// creditsScroll as QLabel
	QLabel *creditsScroll = new QLabel(this);
	QFile creditsFile(":/resources/Credits.txt");
	if (creditsFile.open(QFile::ReadOnly)) {
		QString data(creditsFile.readAll());
		creditsScroll->setText(data);
	}

	creditsScroll->setFont(smallFont);
	creditsScroll->setGeometry(0, 0, 340, 900);
	creditsScroll->setWordWrap(false);
	creditsScroll->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

	int max = creditsScroll->sizeHint().height();
	creditsScroll->setGeometry(0, 0, 340, max);

	// set the creditsScroll inside our QScrollArea
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(creditsScroll);
	m_scrollArea->setGeometry(0, 208, 340, 100);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_scrollArea->setFrameStyle(QFrame::NoFrame);
	m_scrollArea->ensureVisible(0, 0);

	// auto scroll timer initialization
	m_restartAtTop = FALSE;
	m_startTime = QTime::currentTime();
	m_autoScrollTimer = new QTimer(this);
	connect(m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(scrollCredits()));
}

void AboutBox::resetScrollAnimation() {
	// Only called when the window is newly loaded
	m_autoScrollTimer->start(50);
	m_startTime.start();
}

void AboutBox::scrollCredits() {
	if (m_startTime.elapsed() >= 3000 ) {
		//int max = m_scrollArea->verticalScrollBar()->maximum();
		//int v = m_scrollArea->widget()->sizeHint().height();
		if (m_restartAtTop) {
			// Reset at the top
			m_startTime.start();
			m_restartAtTop = FALSE;
			m_scrollArea->verticalScrollBar()->setValue(0);
			return;
		}
		if (m_scrollArea->verticalScrollBar()->value() >= m_scrollArea->verticalScrollBar()->maximum()) {
			// go and reset
			// m_startTime.start();
			m_restartAtTop = TRUE;
		} else {
			m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() + 1);
		}
	}
}

void AboutBox::hideAbout() {
	//DebugDialog::debug("the AboutBox gets a hide action triggered");
	if (singleton != NULL) {
		singleton->hide();
	}
}

void AboutBox::showAbout() {
	//DebugDialog::debug("the AboutBox gets a show action triggered");
	if (singleton == NULL) {
		new AboutBox();
	}
	singleton->show();
	// Start the scroll timer
	singleton->resetScrollAnimation();
}

void AboutBox::closeAbout() {
	//DebugDialog::debug("the AboutBox gets a close action triggered");
	// Note: not every close triggers this function. we better listen to closeEvent
	if (singleton != NULL) {
		singleton->close();
	}
}

void AboutBox::closeEvent(QCloseEvent *event) {
	// called when the window is about to close
	//DebugDialog::debug("the AboutBox gets a closeEvent");
	m_autoScrollTimer->stop();
	event->accept();
}

void AboutBox::keyPressEvent ( QKeyEvent * event ) {
	if (event->key() == Qt::Key_W && event->modifiers() == Qt::CTRL ) {
		// We get the ctrl + W / command + W key event
		//DebugDialog::debug("W key!");
		this->closeAbout();
	}
}
