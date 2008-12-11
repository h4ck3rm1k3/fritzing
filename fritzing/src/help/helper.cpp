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


#include "helper.h"
#include "../debugdialog.h"

QString Helper::BreadboardHelpText = tr(
	"Sketches and Prototypes usally start in the <b>Breadboard View</b>."
	"<br/><br/>"
	"Begin by dragging out a Bradboard Part from the Parts Bin."
	"Then populate the breadboard with the components, just like they are arranged in the real world."
	"<br/><br/>"
	"The Arduino part wil turn into an Arduino Shield.");
QString Helper::SchematicHelpText = tr(
	"Engineers and teachers typically like a <b>Schematic View</b>"
	"<br/><br/>"
	"Here you have the same components as you have on your breadboard, "
	"they just look different and more ... ehm schematic"
	"<br/><br/>"
	"Use this Schematic View to check your connections with the ones "
	"the datasheet will tell you need");
QString Helper::PCBHelpText = tr(
	"Use the <b>PCB View</b> for producing your prototype or product."
	"<br/><br/>"
	"You can use different (typically smaller) sizes of the parts you have in the "
	"breadboard, in order to make production faster, lighter and cheaper."
	"<br/><br/>"
	"First rearrange all the components so they fit nicely on the board. Then use "
	"Autoroute &darr; to generate the optimal traces between components.");

QString Helper::BreadboardHelpImage = ":/resources/images/helpImageBreadboard.png";
QString Helper::SchematicHelpImage = ":/resources/images/helpImageSchematic.png";
QString Helper::PCBHelpImage = ":/resources/images/helpImagePCB.png";

Helper::Helper(MainWindow *owner) : QObject(owner) {
	m_owner = owner;
	m_breadMainHelp = new SketchMainHelp("Breadboard", BreadboardHelpImage, BreadboardHelpText);
	m_schemMainHelp = new SketchMainHelp("Schematic", SchematicHelpImage, SchematicHelpText);
	m_pcbMainHelp = new SketchMainHelp("PCB", PCBHelpImage, PCBHelpText);

	m_stillWaitingFirstDrop = true;

	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(init()));
	timer->start(400);
}

void Helper::init() {
	addAndCenterItemInView(m_breadMainHelp, m_owner->m_breadboardGraphicsView);
	addAndCenterItemInView(m_schemMainHelp, m_owner->m_schematicGraphicsView);
	addAndCenterItemInView(m_pcbMainHelp, m_owner->m_pcbGraphicsView);
}

void Helper::addAndCenterItemInView(SketchMainHelp *item, SketchWidget* view) {
	// here we assume that when a view is resized, the no
	// visible ones, also get resized in the background
	connect(
		view,
		SIGNAL(resizeSignal(const QSize&, const QSize&)),
		this,
		SLOT(viewResized(const QSize&, const QSize&))
	);
	connect(view, SIGNAL(dropSignal()), this, SLOT(somethingDroppedIntoView()));

	view->scene()->addItem(item);
	centerItemInView(item, view);
}

void Helper::centerItemInView(SketchMainHelp *item, SketchWidget* view) {
	moveItemBy(item,
		(view->width()-item->widget()->width())/2,
		(view->height()-item->widget()->height())/2
	);
}

void Helper::viewResized(const QSize& oldSize, const QSize& newSize) {
	if(oldSize.width() > -1 || oldSize.height() > -1) { // don't apply on hide/show transition
		qreal dx = (newSize.width()-oldSize.width())/2;
		qreal dy = (newSize.height()-oldSize.height())/2;
		moveItemBy(m_breadMainHelp, dx, dy);
		moveItemBy(m_schemMainHelp, dx, dy);
		moveItemBy(m_pcbMainHelp, dx, dy);
	}
}

void Helper::somethingDroppedIntoView() {
	if(m_stillWaitingFirstDrop) {
		m_stillWaitingFirstDrop = false;
		m_breadMainHelp->applyAlpha();
		m_schemMainHelp->applyAlpha();
		m_pcbMainHelp->applyAlpha();
	}
}

void Helper::moveItemBy(SketchMainHelp *item, qreal dx, qreal dy) {
	item->moveBy(dx,dy);
}
