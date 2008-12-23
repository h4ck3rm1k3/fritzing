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
#include <QScrollBar>
#include <math.h>

#include "helper.h"
#include "../debugdialog.h"

QString Helper::BreadboardHelpText = tr(
	"Sketches and prototypes usally start in the <b>Breadboard View</b>."
	"<br/><br/>"
	"Begin by dragging out a part from the Parts Bin, which is over at the top right. "
	"Then populate the breadboard with the parts, just as you had arranged them in the physical world. "
	"<br/><br/>"
	"After you're finished creating your sketch in the breadboard view, switch to the PCB view. "
	"You can switch by clicking 'PCB View' in either the View Switcher widget or the Navigator Widget. "
	"In the PCB view the Arduino part will turn into an Arduino Shield, "
	"and other parts will also appear differently in PCB View.");
QString Helper::SchematicHelpText = tr(
	"Engineers and teachers typically work with a <b>Schematic View</b>"
	"<br/><br/>"
	"This is a more abstract way to look at parts and wires than the Breadboard View. "
	"You have the same parts as you have on your breadboard, "
	"they just look different. "
	"<br/><br/>"
	"If you are using a datasheet, the Schematic View can help you check your connections with the ones "
	"the datasheet will suggest.");
QString Helper::PCBHelpText = tr(
	"Use the <b>PCB View</b> for producing your prototype or product."
	"<br/><br/>"
	"You can use different (typically smaller) sizes of the parts you have in the "
	"breadboard, in order to make production faster, lighter and cheaper. "
	"<br/><br/>"
	"First rearrange all the components so they fit nicely on the board. Then use "
	"Autoroute to generate the copper traces between parts. "
	"The Autoroute button <img src=\":resources/images/icons/toolbarAutorouteEnabled_icon.png\" /> is at the bottom left.");

QString Helper::PartsBinHelpText = tr("Drag out your <br> parts from here");
QString Helper::AutorouteHelpText = tr("When done with arranging, <br> use Autoroute to create <br> your copper traces");
QString Helper::SwitchButtonsHelpText = tr("Use these buttons to <br> toggle between views");


Helper::Helper(MainWindow *owner) : QObject(owner) {
	m_owner = owner;
	m_breadMainHelp = new SketchMainHelp("Breadboard", BreadboardHelpText);
	connect(m_breadMainHelp->widget(), SIGNAL(aboutToClose()), this, SLOT(removePartsBinHelp()));
	connect(m_breadMainHelp->widget(), SIGNAL(aboutToClose()), this, SLOT(removeSwitchButtonsHelp()));

	m_schemMainHelp = new SketchMainHelp("Schematic", SchematicHelpText);

	m_pcbMainHelp = new SketchMainHelp("PCB", PCBHelpText);
	connect(m_pcbMainHelp->widget(), SIGNAL(aboutToClose()), this, SLOT(removeAutorouteHelp()));

	m_partsBinHelp = new ToolHelp(PartsBinHelpText, QString("PartsBin"));
	m_autorouteHelp = new ToolHelp(AutorouteHelpText, QString("Autoroute"), QBoxLayout::RightToLeft);
	m_switchButtonsHelp = new ToolHelp(SwitchButtonsHelpText, QString("SwitchButtons"), QBoxLayout::BottomToTop);

	m_stillWaitingFirstDrop = true;
	m_stillWaitingFirstViewSwitch = true;
	m_stillWaitingFirstAutoroute = true;

	m_prevVScroolW = 0;
	m_prevHScroolH = 0;

	connectToView(m_owner->m_breadboardGraphicsView);
	connectToView(m_owner->m_schematicGraphicsView);
	connectToView(m_owner->m_pcbGraphicsView);

	m_owner->m_breadboardGraphicsView->addFixedToCenterItem(m_breadMainHelp);
	m_owner->m_schematicGraphicsView->addFixedToCenterItem(m_schemMainHelp);
	m_owner->m_pcbGraphicsView->addFixedToCenterItem(m_pcbMainHelp);

	//m_owner->m_breadboardGraphicsView->addFixedToTopRightItem(m_partsBinHelp);
	//m_owner->m_breadboardGraphicsView->addFixedToTopLeftItem(m_switchButtonsHelp);
	//m_owner->m_pcbGraphicsView->addFixedToBottomLeftItem(m_autorouteHelp);
}

Helper::~Helper() {
	// m_...MainHelp objects  are deleted by the scene when the scene is deleted
	// when the sketch is closed, so calling doClose crashes at this point

	//m_breadMainHelp->doClose();
	//m_schemMainHelp->doClose();
	//m_pcbMainHelp->doClose();
}

void Helper::connectToView(SketchWidget* view) {
	connect(view, SIGNAL(dropSignal()), this, SLOT(somethingDroppedIntoView()));
	//connect(m_owner->m_breadViewSwitcher->widget(), SIGNAL(viewSwitched(int)), this, SLOT(viewSwitched()));
	connect(m_owner, SIGNAL(autorouted()), this, SLOT(autorouted()));
}


void Helper::somethingDroppedIntoView() {
	if(m_stillWaitingFirstDrop) {
		m_stillWaitingFirstDrop = false;
		m_breadMainHelp->setTransparent();
		m_schemMainHelp->setTransparent();
		m_pcbMainHelp->setTransparent();
		removePartsBinHelp();
	} else {
		disconnect(m_owner->m_currentGraphicsView, SIGNAL(dropSignal()), this, SLOT(somethingDroppedIntoView()));
	}
}

void Helper::removePartsBinHelp() {
	m_owner->m_breadboardGraphicsView->scene()->removeItem(m_partsBinHelp);
}

void Helper::removeSwitchButtonsHelp() {
	m_owner->m_breadboardGraphicsView->scene()->removeItem(m_switchButtonsHelp);
}

void Helper::removeAutorouteHelp() {
	m_owner->m_pcbGraphicsView->scene()->removeItem(m_autorouteHelp);
}

void Helper::viewSwitched() {
	if(m_stillWaitingFirstViewSwitch) {
		m_stillWaitingFirstViewSwitch = false;

		QTimer *timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(timer, SIGNAL(timeout()), this, SLOT(removeSwitchButtonsHelp()));
		timer->start(400);
	} else {
		//disconnect(m_owner->m_breadViewSwitcher->widget(), SIGNAL(viewSwitched(int)), this, SLOT(viewSwitched()));
	}
}

void Helper::autorouted() {
	if(m_stillWaitingFirstAutoroute) {
		m_stillWaitingFirstAutoroute = false;
		removeAutorouteHelp();
	} else {
		disconnect(m_owner, SIGNAL(autorouteSignal()), this, SLOT(autorouted()));
	}
}

void Helper::moveItemBy(QGraphicsProxyWidget *item, qreal dx, qreal dy) {
	item->moveBy(dx,dy);
}
