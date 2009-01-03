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
        "The <b>Breadboard View</b> is meant to look like a <i>real-life</i> breadboard prototype."
	"<br/><br/>"
        "Begin by dragging a part from the Parts Bin, which is over at the top right. "
        "Then pull in more parts, connecting them by clicking on the connectors and dragging wires. "
        "The process is similar to how you would arrange things in the physical world. "
	"<br/><br/>"
        "After you're finished creating your sketch in the breadboard view, try the other views. "
        "You can switch by clicking the other views in either the View Switcher or the Navigator on the lower right. "
        "Because different views have different purposes, parts will look different in the other views.");
QString Helper::SchematicHelpText = tr(
        "Welcome to the <b>Schematic View</b>"
	"<br/><br/>"
        "This is a more abstract way to look at components and connections than the Breadboard View. "
        "You have the same elements as you have on your breadboard, "
        "they just look different. This representation is closer to the traditional diagrams used by engineers."
        "<br/><br/>"
        "You can press &lt;Shift&gt;-click with the mouse to create bend points and tidy up your connections. "
        "The Schematic View can help you check that you have made the right connections between components. "
        "You can also print out your schematic for documentation.");
QString Helper::PCBHelpText = tr(
        "The <b>PCB View</b> is where you design how the components will appear on a physical PCB (Printed Circuit Board)."
	"<br/><br/>"
        "PCBs can be made at home or in a small lab using DIY etching processes. "
        "They also can be sent to professional PCB manufacturing services for more precise fabrication. "
	"<br/><br/>"
        "To lay out your PCB, first rearrange all the components so they fit nicely on the board. "
        "Then try to shift them around to minimize the length and confusion of connections. "
        "Once the parts are sorted out, you can right-click on individual connections or use "
        "Autoroute to generate the copper traces between parts. "
        "The Autoroute button <img src=\":resources/images/icons/toolbarAutorouteEnabled_icon.png\" /> is on the bottom left.");

QString Helper::PartsBinHelpText = tr("Drag out your <br> parts from here");
QString Helper::AutorouteHelpText = tr("When done with arranging, <br> use Autoroute to create <br> your copper traces");
QString Helper::SwitchButtonsHelpText = tr("Use these buttons to <br> toggle between views");


Helper::Helper(MainWindow *owner, bool doShow) : QObject(owner) {
	m_owner = owner;
	m_breadMainHelp = new SketchMainHelp("Breadboard", BreadboardHelpText, doShow);
	connect(m_breadMainHelp->widget(), SIGNAL(aboutToClose()), this, SLOT(removePartsBinHelp()));
	connect(m_breadMainHelp->widget(), SIGNAL(aboutToClose()), this, SLOT(removeSwitchButtonsHelp()));

	m_schemMainHelp = new SketchMainHelp("Schematic", SchematicHelpText, doShow);

	m_pcbMainHelp = new SketchMainHelp("PCB", PCBHelpText, doShow);
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
	connect(view, SIGNAL(dropSignal(const QPoint &)), this, SLOT(somethingDroppedIntoView(const QPoint &)));
	//connect(m_owner->m_breadViewSwitcher->widget(), SIGNAL(viewSwitched(int)), this, SLOT(viewSwitched()));
	connect(m_owner, SIGNAL(autorouted()), this, SLOT(autorouted()));
}


void Helper::somethingDroppedIntoView(const QPoint & pos) {
	Q_UNUSED(pos);
	/*SketchAreaWidget * widgetParent = dynamic_cast<SketchAreaWidget *>(
		m_owner->m_tabWidget->currentWidget()
	);
	SketchWidget *currSketch = widgetParent->graphicsView();

	int currIdx = m_owner->m_tabWidget->currentIndex();
	SketchMainHelp *mainHelp = helpForIndex(currIdx);

	bool doHide = false;
	if(currSketch && mainHelp) {
		QPointF posAux = currSketch->mapFrom(m_owner,pos);
		doHide = mainHelp->boundingRect().contains(posAux);
		DebugDialog::debug(QString("<<<< %1,%2").arg(posAux.x()).arg(posAux.y()));
		DebugDialog::debug(doHide ? "<<< hide" : "<<< show");
	}*/

	if(m_stillWaitingFirstDrop) {
		m_stillWaitingFirstDrop = false; //false;
		/*m_breadMainHelp->setTransparent();
		m_schemMainHelp->setTransparent();
		m_pcbMainHelp->setTransparent();*/
		m_breadMainHelp->doSetVisible(false);
		m_schemMainHelp->doSetVisible(false);
		m_pcbMainHelp->doSetVisible(false);
		removePartsBinHelp();
	} else {
		m_breadMainHelp->setTransparent();
		m_schemMainHelp->setTransparent();
		m_pcbMainHelp->setTransparent();
		disconnect(m_owner->m_currentGraphicsView, SIGNAL(dropSignal(const QPoint &)), this, SLOT(somethingDroppedIntoView(const QPoint &)));
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

void Helper::toggleHelpVisibility(int index) {
	SketchMainHelp * which = helpForIndex(index);
	if (which == NULL) return;

	which->doSetVisible(!which->isVisible());
}

void Helper::setHelpVisibility(int index, bool show) {
	SketchMainHelp * which = helpForIndex(index);
	if (which == NULL) return;

	which->doSetVisible(show);
}

bool Helper::helpVisible(int index) {
	SketchMainHelp * which = helpForIndex(index);
	if (which == NULL) return false;

	return which->isVisible();
}

SketchMainHelp *Helper::helpForIndex(int index) {
	SketchMainHelp * which = NULL;
	switch (index) {
		case 0:
			which = m_breadMainHelp;
			break;
		case 1:
			which = m_schemMainHelp;
			break;
		case 2:
			which = m_pcbMainHelp;
			break;
		default:
			break;
	}

	return which;
}
