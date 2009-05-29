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
#include "version/version.h"

AboutBox* AboutBox::singleton = NULL;

static const int AboutWidth = 390;

AboutBox::AboutBox(QWidget *parent)
: QWidget(parent)
{
	singleton = this;
	// To make the application not quit when the window closes
	this->setAttribute(Qt::WA_QuitOnClose, FALSE);

	setFixedSize(AboutWidth, 430);

	// the background color
	setStyleSheet("background-color: #E8E8E8");

	// the new Default Font
	QFont smallFont;
	smallFont.setPointSize(11);
	QFont extraSmallFont;
	extraSmallFont.setPointSize(9);
	extraSmallFont.setLetterSpacing(QFont::PercentageSpacing, 92);

	// Big Icon
	QLabel *logoShield = new QLabel(this);
	logoShield->setPixmap(QPixmap(":/resources/images/AboutBoxLogoShield.png"));
	logoShield->setGeometry(131, 8, 128, 128);

	// Version String
	QLabel *versionMain = new QLabel(this);
	versionMain->setText(tr("Version %1.%2.%3 <small>(%4%5 %6)</small>")
						 .arg(Version::majorVersion())
						 .arg(Version::minorVersion())
						 .arg(Version::minorSubVersion())
						 .arg(Version::modifier())
						 .arg(Version::revision())
						 .arg(Version::date()) );
	versionMain->setFont(smallFont);
	versionMain->setGeometry(45, 150, 300, 20);
	versionMain->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

	// Link to website
	QLabel *linkToFritzing = new QLabel(this);
	linkToFritzing->setText(tr("<a href=\"http://www.fritzing.org\">www.fritzing.org</a>"));
	linkToFritzing->setOpenExternalLinks(TRUE);
	linkToFritzing->setFont(smallFont);
	linkToFritzing->setGeometry(45, 168, 300, 18);
	linkToFritzing->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	linkToFritzing->setOpenExternalLinks(TRUE);

	// Copyright messages
	

	QLabel *copyrightGNU = new QLabel(this);
	copyrightGNU->setText(tr("<b>GNU GPL v3 on the code and CreativeCommons:BY-SA on the rest"));
	copyrightGNU->setFont(extraSmallFont);
	copyrightGNU->setGeometry(0, 398, AboutWidth, 16);
	copyrightGNU->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

	QLabel *CC = new QLabel(this);
	QPixmap cc(":/resources/images/aboutbox_CC.png");
	CC->setPixmap(cc);
	CC->setGeometry(30, this->height() - cc.height(), cc.width(), cc.height());

	QLabel *FHP = new QLabel(this);
	QPixmap fhp(":/resources/images/aboutbox_FHP.png");
	FHP->setPixmap(fhp);
	FHP->setGeometry(360 - fhp.width(), this->height() - fhp.height(), fhp.width(), fhp.height());

	int w = qMax(fhp.width(), cc.width());

	QLabel *copyrightFHP = new QLabel(this);
	copyrightFHP->setText(tr("<b>2007-%1 Fachhochschule Potsdam</b>").arg(Version::year()));
	copyrightFHP->setFont(extraSmallFont);
	copyrightFHP->setGeometry(30 + w, 414, AboutWidth - 30 - 30 - w - w, 16);
	copyrightFHP->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

	// Scrolling Credits Text

	// creditsScroll as QLabel
	QLabel *creditsScroll = new QLabel(this);

	// moved data out of credits.txt so we could apply translation
	QString data = 
tr("<br /><br /><br /><br /><br /><br /><br /><br /><br />"
   "<p><br/>Fritzing is made by:<br />"
   "Prof. Reto Wettach, Andr&eacute; Kn&ouml;rig, Myriel Milicevic,<br/>"
   "Zach Eveland, Dirk van Oosterbosch,<br/>"
   "Jonathan Cohen, Marcus Paeschke, Omer Yosha,<br/>"
   "Travis Robertson, Stefan Hermann, Brendan Howell,<br/>"
   "Mariano Crowe, Johannes Landstorfer,<br/>"
   "Jenny Chowdhury, Lionel Michel and Jannis Leidel.</p>") +

tr("<p>Special thanks goes out to:<br />"
   "Jussi &Auml;ngeslev&auml;, Massimo Banzi, Ayah Bdeir,<br/>"
   "Durrell Bishop, David Cuartielles, Fabian Hemmert,<br />"
   "Gero Herkenrath, Jeff Hoefs, Tom Hulbert,<br/>"
   "Tom Igoe, Hans-Peter Kadel, Till Savelkoul,<br/>"
   "Jan Sieber, Yaniv Steiner, Olaf Val,<br/>"
   "Michaela Vieser and Julia Werner.</p>") +

tr("<p>Thanks to Kurt Badelt<br/>"
   "for the Spanish translation,<br/>") +

tr("and thanks to Gianluca Urgese<br/>"
   "for the Italian translation.</p>") +

tr("<p>Fritzing is made possible with funding from the<br/>"
   "MWFK Brandenburg, the sponsorship of the Design<br/>"
   "Department of Bauhaus-University Weimar and<br/>"
   "IxDS.</p>") +

tr("<p>Special thanks goes out as well to all the students<br/>"
   "and alpha testers who were brave enough to give<br/>"
   "Fritzing a test spin.</p>"
   "<br /><br /><br /><br /><br /><br /><br /><br /><br />"
   );

	creditsScroll->setText(data);
	creditsScroll->setFont(smallFont);
	creditsScroll->setGeometry(0, 0, AboutWidth, 800);
	creditsScroll->setWordWrap(false);
	creditsScroll->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

	int max = creditsScroll->sizeHint().height();
	creditsScroll->setGeometry(0, 0, AboutWidth, max);

	// set the creditsScroll inside our QScrollArea
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(creditsScroll);
	m_scrollArea->setGeometry(0, 210, AboutWidth, 160);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_scrollArea->setFrameStyle(QFrame::NoFrame);
	m_scrollArea->ensureVisible(0, 0);
	
	// Add a fade out and a fade in the scrollArea
	QLabel *scrollFade = new QLabel(this);
	scrollFade->setPixmap(QPixmap(":/resources/images/aboutbox_scrollfade.png"));
	scrollFade->setGeometry(0, 210, AboutWidth, 160);
	scrollFade->setStyleSheet("background-color: none");
	

	// auto scroll timer initialization
	m_restartAtTop = FALSE;
	m_startTime = QTime::currentTime();
	m_autoScrollTimer = new QTimer(this);
	connect(m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(scrollCredits()));
}

void AboutBox::resetScrollAnimation() {
	// Only called when the window is newly loaded
	m_autoScrollTimer->start(25);
	m_startTime.start();
}

void AboutBox::scrollCredits() {
	if (m_startTime.elapsed() >= 0 ) {
		//int max = m_scrollArea->verticalScrollBar()->maximum();
		//int v = m_scrollArea->widget()->sizeHint().height();
		if (m_restartAtTop) {
			// Reset at the top
			m_startTime.start();
			m_restartAtTop = false;
			m_scrollArea->verticalScrollBar()->setValue(0);
			return;
		}
		if (m_scrollArea->verticalScrollBar()->value() >= m_scrollArea->verticalScrollBar()->maximum()) {
			// go and reset
			// m_startTime.start();
			m_restartAtTop = true;
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

	// scroll text now to prevent a flash of text if text was visible the last time the about box was open
	singleton->m_scrollArea->verticalScrollBar()->setValue(0);

	singleton->show();
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
	if ((event->key() == Qt::Key_W) && (event->modifiers() & Qt::ControlModifier) ) {
		// We get the ctrl + W / command + W key event
		//DebugDialog::debug("W key!");
		this->closeAbout();
	}
}

void AboutBox::show() {
	QWidget::show();
	m_restartAtTop = true;
	resetScrollAnimation();
}
