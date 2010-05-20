/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include <QFont>
#include <QChar>
#include <QTime>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QScrollBar>

#include "aboutbox.h"
#include "../debugdialog.h"
#include "../version/version.h"
#include "../utils/expandinglabel.h"

AboutBox* AboutBox::singleton = NULL;

static const int AboutWidth = 390;
static const int AboutText = 210;

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
	QFont smallFont("Droid Sans", 11);
	QFont extraSmallFont("Droid Sans", 9);
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

	// moved data out of credits.txt so we could apply translation
	QString data = 
tr("<br /><br /><br /><br /><br /><br /><br /><br /><br />"
   "<p>Fritzing is made by: "
   "Prof. Reto Wettach, Andr&eacute; Kn&ouml;rig, Myriel Milicevic, "
   "Zach Eveland, Dirk van Oosterbosch, "
   "Jonathan Cohen, Marcus Paeschke, Omer Yosha, "
   "Travis Robertson, Stefan Hermann, Brendan Howell, "
   "Mariano Crowe, Johannes Landstorfer, "
   "Jenny Chowdhury, Lionel Michel and Jannis Leidel.</p>") +

tr("<p>Special thanks goes out to: "
   "Jussi &Auml;ngeslev&auml;, Massimo Banzi, Ayah Bdeir, "
   "Durrell Bishop, David Cuartielles, Fabian Hemmert, "
   "Gero Herkenrath, Jeff Hoefs, Tom Hulbert, "
   "Tom Igoe, Hans-Peter Kadel, Till Savelkoul, "
   "Jan Sieber, Yaniv Steiner, Olaf Val, "
   "Michaela Vieser and Julia Werner.</p>") +

tr("<p>Thanks to Kurt Badelt for the Spanish translation, "
	"to Gianluca Urgese for the Italian translation, "
	"to Nuno Pessanha Santos for the Portuguese translation, "
	"to Yuelin and Ninjia  for the Chinese (Simplified) translation, "
	"to Hiroshi Suzuki for the Japanese translation, "
	"to Robert Lee for the Chinese (Traditional) translation, "
	"to Vladimir Savinov for the Russian translation," 
	"and to Steven Noppe for the Dutch translation." 
	"</p>") +

tr("<p>Fritzing is made possible with funding from the "
   "MWFK Brandenburg, the sponsorship of the Design "
   "Department of Bauhaus-University Weimar and "
   "IxDS.</p>") +

tr("<p>Special thanks goes out as well to all the students "
   "and alpha testers who were brave enough to give "
   "Fritzing a test spin. "
   "<br /><br /><br /><br /><br /><br /><br /><br /><br />"
   );

	QPixmap fadepixmap(":/resources/images/aboutbox_scrollfade.png");

	m_expandingLabel = new ExpandingLabel(this, AboutWidth);
	m_expandingLabel->setLabelText(data);
	m_expandingLabel->setFont(smallFont);
	m_expandingLabel->setGeometry(0, AboutText, AboutWidth, fadepixmap.height());
	m_expandingLabel->setStyleSheet("border: 0px; background-color: transparent; margin-top: 0px; margin-bottom: 0px;");
	
	// setAlignment only aligns the "current paragraph"
	// the QTextCursor code aligns all paragraphs
	QTextCursor cursor(m_expandingLabel->document());
	cursor.select(QTextCursor::Document);
	QTextBlockFormat fmt;
	fmt.setAlignment(Qt::AlignCenter);
	cursor.mergeBlockFormat(fmt);
	
	// Add a fade out and a fade in the scrollArea
	QLabel *scrollFade = new QLabel(this);
	scrollFade->setPixmap(fadepixmap);
	scrollFade->setGeometry(0, AboutText, AboutWidth, fadepixmap.height());
	scrollFade->setStyleSheet("background-color: none");
	

	// auto scroll timer initialization
	m_restartAtTop = FALSE;
	m_startTime = QTime::currentTime();
	m_autoScrollTimer = new QTimer(this);
	connect(m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(scrollCredits()));
}

void AboutBox::resetScrollAnimation() {
	// Only called when the window is newly loaded
	m_autoScrollTimer->start(35);
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
			m_expandingLabel->verticalScrollBar()->setValue(0);
			return;
		}
		if (m_expandingLabel->verticalScrollBar()->value() >= m_expandingLabel->verticalScrollBar()->maximum()) {
			// go and reset
			// m_startTime.start();
			m_restartAtTop = true;
		} else {
			m_expandingLabel->verticalScrollBar()->setValue(m_expandingLabel->verticalScrollBar()->value() + 1);
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
	singleton->m_expandingLabel->verticalScrollBar()->setValue(0);

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
