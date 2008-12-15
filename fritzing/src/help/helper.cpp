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

QString Helper::PartsBinHelpText = tr("Drag out your <br> parts from here");
QString Helper::AutorouteHelpText = tr("When done with arranging, <br> use Autoroute to create <br> your copper traces");
QString Helper::SwitchButtonsHelpText = tr("Use these buttons to <br> toggle between views");


Helper::Helper(MainWindow *owner) : QObject(owner) {
	m_owner = owner;
	m_breadMainHelp = new SketchMainHelp("Breadboard", BreadboardHelpText);
	m_schemMainHelp = new SketchMainHelp("Schematic", SchematicHelpText);
	m_pcbMainHelp = new SketchMainHelp("PCB", PCBHelpText);

	m_partsBinHelp = new ToolHelp(PartsBinHelpText, QString("PartsBin"));
	m_autorouteHelp = new ToolHelp(AutorouteHelpText, QString("Autoroute"), QBoxLayout::RightToLeft);
	m_switchButtonsHelp = new ToolHelp(SwitchButtonsHelpText, QString("SwitchButtons"), QBoxLayout::RightToLeft);

	m_stillWaitingFirstDrop = true;

	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(init()));
	timer->start(400);
}

void Helper::init() {
	addItemToView(m_breadMainHelp, m_owner->m_breadboardGraphicsView);
	centerItemInView(m_breadMainHelp, m_owner->m_currentGraphicsView);

	addItemToView(m_schemMainHelp, m_owner->m_schematicGraphicsView);
	centerItemInView(m_schemMainHelp, m_owner->m_currentGraphicsView);

	addItemToView(m_pcbMainHelp, m_owner->m_pcbGraphicsView);
	centerItemInView(m_pcbMainHelp, m_owner->m_currentGraphicsView);

	addItemToView(m_partsBinHelp, m_owner->m_currentGraphicsView);
	moveItemBy(m_partsBinHelp, m_owner->m_breadboardGraphicsView->width()-m_partsBinHelp->widget()->width(), 0);

	addItemToView(m_autorouteHelp, m_owner->m_pcbGraphicsView);
	moveItemBy(m_autorouteHelp,110,m_owner->m_breadboardGraphicsView->height()-m_partsBinHelp->widget()->height());

	addItemToView(m_switchButtonsHelp, m_owner->m_breadboardGraphicsView);

	connect(m_owner->m_breadboardGraphicsView,SIGNAL(resizeSignal()),this,SLOT(viewChanged()));
	connect(m_owner->m_schematicGraphicsView,SIGNAL(resizeSignal()),this,SLOT(viewChanged()));
	connect(m_owner->m_pcbGraphicsView,SIGNAL(resizeSignal()),this,SLOT(viewChanged()));

	/*connect(m_owner->m_breadboardGraphicsView,SIGNAL(wheelSignal()),this,SLOT(viewChanged()));
	connect(m_owner->m_schematicGraphicsView,SIGNAL(wheelSignal()),this,SLOT(viewChanged()));
	connect(m_owner->m_pcbGraphicsView,SIGNAL(wheelSignal()),this,SLOT(viewChanged()));*/

	/*connectToScrollBar(m_owner->m_breadboardGraphicsView->verticalScrollBar());
	connectToScrollBar(m_owner->m_breadboardGraphicsView->horizontalScrollBar());*/
}

void Helper::addItemToView(QGraphicsWidget *item, SketchWidget* view) {
	// here we assume that when a view is resized, the no
	// visible ones, also get resized in the background

	connect(view, SIGNAL(dropSignal()), this, SLOT(somethingDroppedIntoView()));

	view->scene()->addItem(item);
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
}


void Helper::connectToScrollBar(QScrollBar *scrollBar) {
	connect(scrollBar, SIGNAL(valueChanged(int)),		this, SLOT(viewChanged()));
	connect(scrollBar, SIGNAL(rangeChanged(int,int)),	this, SLOT(viewChanged()));
	connect(scrollBar, SIGNAL(sliderPressed()),			this, SLOT(viewChanged()));
	connect(scrollBar, SIGNAL(sliderMoved(int)),		this, SLOT(viewChanged()));
	connect(scrollBar, SIGNAL(sliderReleased()),		this, SLOT(viewChanged()));
	connect(scrollBar, SIGNAL(actionTriggered(int)),	this, SLOT(viewChanged()));
}

void Helper::centerItemInView(SketchMainHelp *item, SketchWidget* view) {
	QWidget *w = item->widget();
	qreal xAux = (view->width() - w->width())/2 - w->pos().x();
	qreal yAux = (view->height() - w->height())/2 - w->pos().y();
	qreal x = (view->width() + view->mapToScene(w->pos()).x() - w->width())/2 - w->pos().x();
	qreal y = (view->height() + view->mapToScene(w->pos()).y() - w->height())/2 - w->pos().y();

	DebugDialog::debug(
			QString("<<<< %1 %2 - %3 %4")
				.arg(view->sceneRect().width())
				.arg(view->sceneRect().height())
				.arg(view->mapToScene(x,y).x())
				.arg(view->mapToScene(x,y).y())
		);

	moveItemBy(item,xAux,yAux);
}

void Helper::fixedX(ToolHelp *item, SketchWidget* view) {
	QScrollBar * sb = view->horizontalScrollBar();
	qreal hScroll = sb->isVisible() ? sb->height() : 0;
	qreal y = view->height()-item->widget()->height()-item->y()-hScroll;
	moveItemBy(item,0,y);
}

void Helper::fixedY(ToolHelp *item, SketchWidget* view) {
	QScrollBar * sb = view->verticalScrollBar();
	qreal wScroll = sb->isVisible() ? sb->width() : 0;
	qreal x = view->width()-item->widget()->width()-item->x()-wScroll;
	moveItemBy(item,x,0);
}

void Helper::viewChanged() {
	/*if(oldSize.width() > -1 && oldSize.height() > -1) { // don't apply on hide/show transition
		qreal dx = (newSize.width()-oldSize.width())/2;
		qreal dy = (newSize.height()-oldSize.height())/2;
		if(fabs(dx) >= 0.1 && fabs(dy) >= 0.1) {
			moveItemBy(m_breadMainHelp, dx, dy);
			moveItemBy(m_schemMainHelp, dx, dy);
			moveItemBy(m_pcbMainHelp, dx, dy);
			moveItemBy(m_partsBinHelp, dx, 0);
		}
	}*/
	centerItemInView(m_breadMainHelp, m_owner->m_currentGraphicsView);
	centerItemInView(m_schemMainHelp, m_owner->m_currentGraphicsView);
	centerItemInView(m_pcbMainHelp, m_owner->m_currentGraphicsView);
	fixedY(m_partsBinHelp, m_owner->m_currentGraphicsView);
	fixedX(m_autorouteHelp, m_owner->m_currentGraphicsView);
}

void Helper::somethingDroppedIntoView() {
	if(m_stillWaitingFirstDrop) {
		m_stillWaitingFirstDrop = false;
		m_breadMainHelp->setTransparent();
		m_schemMainHelp->setTransparent();
		m_pcbMainHelp->setTransparent();
	}
}

void Helper::moveItemBy(QGraphicsProxyWidget *item, qreal dx, qreal dy) {
	item->moveBy(dx,dy);
}
