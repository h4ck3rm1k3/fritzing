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

$Revision: 1794 $:
$Author: merunga $:
$Date: 2008-12-11 14:50:11 +0100 (Thu, 11 Dec 2008) $

********************************************************************/

#include <QSizeGrip>
#include <QStatusBar>

#include "dockmanager.h"
#include "triplenavigator.h"
#include "fsizegrip.h"
#include "viewswitcher/viewswitcherdockwidget.h"
#include "utils/misc.h"
#include "partsbinpalette/partsbinpalettewidget.h"
#include "htmlinfoview.h"

FDockWidget * makeViewSwitcherDock(const QString & title, QWidget * parent) {
	return new ViewSwitcherDockWidget(title, parent);
}

DockManager::DockManager(MainWindow *mainWindow)
	: QObject(mainWindow)
{
	m_mainWindow = mainWindow;
	m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	m_mainWindow->setDockOptions(QMainWindow::AnimatedDocks);
	m_mainWindow->m_sizeGrip = new FSizeGrip(mainWindow);

	m_topDock = NULL;
	m_bottomDock = NULL;
	m_dontKeepMargins = true;

	m_oldTopDockStyle = ___emptyString___;
	m_oldBottomDockStyle = ___emptyString___;
}

void DockManager::dockChangeActivation(bool activate) {
	if (!m_mainWindow->m_closing) {
		m_mainWindow->changeActivation(activate);
		m_mainWindow->m_sizeGrip->rearrange();
	}

}

void DockManager::createBinAndInfoViewDocks() {
	m_mainWindow->m_infoView = new HtmlInfoView(m_mainWindow->m_refModel);

	m_mainWindow->m_paletteWidget = new PartsBinPaletteWidget(m_mainWindow->m_refModel, m_mainWindow->m_infoView, m_mainWindow->m_undoStack, m_mainWindow);
	connect(m_mainWindow->m_paletteWidget, SIGNAL(saved(bool)), m_mainWindow, SLOT(binSaved(bool)));
	connect(m_mainWindow, SIGNAL(alienPartsDismissed()), m_mainWindow->m_paletteWidget, SLOT(removeAlienParts()));

	if (m_mainWindow->m_paletteModel->loadedFromFile()) {
		m_mainWindow->m_paletteWidget->loadFromModel(m_mainWindow->m_paletteModel);
	} else {
		m_mainWindow->m_paletteWidget->setPaletteModel(m_mainWindow->m_paletteModel);
	}
}

void DockManager::createDockWindows()
{
	QWidget * widget = new QWidget();
	widget->setMinimumHeight(0);
	widget->setMaximumHeight(0);
	FDockWidget * dock = makeDock(tr("view switcher"), widget, 0,  0, Qt::RightDockWidgetArea, makeViewSwitcherDock);	
	static_cast<ViewSwitcherDockWidget *>(dock)->setViewSwitcher(m_mainWindow->m_viewSwitcher);
	connect(m_mainWindow, SIGNAL(mainWindowMoved(QWidget *)), dock, SLOT(windowMoved(QWidget *)));

#ifndef QT_NO_DEBUG
	//dock->setStyleSheet("background-color: red;");
	//m_mainWindow->m_viewSwitcher->setStyleSheet("background-color: blue;");
#endif

	makeDock(PartsBinPaletteWidget::Title, m_mainWindow->m_paletteWidget, PartsBinMinHeight, PartsBinDefaultHeight);
    makeDock(tr("Inspector"), m_mainWindow->m_infoView, InfoViewMinHeight, InfoViewDefaultHeight);

    m_mainWindow->m_navigators << (m_mainWindow->m_miniViewContainerBreadboard = new MiniViewContainer(m_mainWindow));
	m_mainWindow->m_miniViewContainerBreadboard->filterMousePress();
	connect(m_mainWindow->m_miniViewContainerBreadboard, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)),
								m_mainWindow, SLOT(currentNavigatorChanged(MiniViewContainer *)));

    m_mainWindow->m_navigators << (m_mainWindow->m_miniViewContainerSchematic = new MiniViewContainer(m_mainWindow));
	m_mainWindow->m_miniViewContainerSchematic->filterMousePress();
	connect(m_mainWindow->m_miniViewContainerSchematic, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)),
								m_mainWindow, SLOT(currentNavigatorChanged(MiniViewContainer *)));

    m_mainWindow->m_navigators << (m_mainWindow->m_miniViewContainerPCB = new MiniViewContainer(m_mainWindow));
	m_mainWindow->m_miniViewContainerPCB->filterMousePress();
	connect(m_mainWindow->m_miniViewContainerPCB, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)),
								m_mainWindow, SLOT(currentNavigatorChanged(MiniViewContainer *)));

    makeDock(tr("Undo History"), m_mainWindow->m_undoView, UndoHistoryMinHeight, UndoHistoryDefaultHeight)->hide();
    m_mainWindow->m_undoView->setMinimumSize(DockMinWidth, UndoHistoryMinHeight);

	m_mainWindow->m_tripleNavigator = new TripleNavigator(m_mainWindow);
	m_mainWindow->m_tripleNavigator->addView(m_mainWindow->m_miniViewContainerBreadboard, tr("Breadboard"));
	m_mainWindow->m_tripleNavigator->addView(m_mainWindow->m_miniViewContainerSchematic, tr("Schematic"));
	m_mainWindow->m_tripleNavigator->addView(m_mainWindow->m_miniViewContainerPCB, tr("PCB"));
	makeDock(tr("Navigator"), m_mainWindow->m_tripleNavigator, NavigatorMinHeight, NavigatorDefaultHeight);

	// comment out the console for nows
	// m_mainWindow->m_consoleView = new Console();
	// FDockWidget * dock = makeDock(tr("Console"), m_mainWindow->m_consoleView, DockMinHeight, DockDefaultHeight, Qt::BottomDockWidgetArea);
	// dock->hide();


#ifndef QT_NO_DEBUG
    m_mainWindow->m_windowMenu->addSeparator();
    m_mainWindow->m_windowMenu->addAction(m_mainWindow->m_toggleDebuggerOutputAct);
#endif

    m_mainWindow->m_windowMenu->addSeparator();
}

FDockWidget * DockManager::makeDock(const QString & title, QWidget * widget, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area, DockFactory dockFactory) {
	FDockWidget * dock = ((dockFactory) ? dockFactory(title, m_mainWindow) : new FDockWidget(title, m_mainWindow));
    dock->setObjectName(title);
    dock->setWidget(widget);
    widget->setParent(dock);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(dock, SIGNAL(positionChanged()), this, SLOT(keepMargins()));
    connect(dock, SIGNAL(topLevelChanged(bool)), this, SLOT(keepMargins()));
    connect(dock, SIGNAL(visibilityChanged(bool)), this, SLOT(keepMargins()));

	return dockIt(dock, dockMinHeight, dockDefaultHeight, area);
}

FDockWidget *DockManager::dockIt(FDockWidget* dock, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area) {
	dock->setAllowedAreas(area);
	m_mainWindow->addDockWidget(area, dock);
    m_mainWindow->m_windowMenu->addAction(dock->toggleViewAction());

    dock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock->setMinimumSize(DockMinWidth, dockMinHeight);
	dock->resize(DockDefaultWidth, dockDefaultHeight);
    connect(dock, SIGNAL(dockChangeActivationSignal(bool)), this, SLOT(dockChangeActivation(bool)));

    m_docks << dock;

    return dock;
}

FDockWidget *DockManager::newTopWidget() {
	int topMostY = 10000;
	FDockWidget *topWidget = NULL;
	foreach(FDockWidget* dock, m_docks) {
		if(/*!dock->isFloating() && dock->isVisible() &&*/
			m_mainWindow->dockWidgetArea(dock) == Qt::RightDockWidgetArea
			&& dock->pos().y() < topMostY) {
			topMostY = dock->pos().y();
			topWidget = dock;
		}
	}
	return topWidget;
}

FDockWidget *DockManager::newBottomWidget() {
	int bottomMostY = -1;
	FDockWidget *bottomWidget = NULL;
	foreach(FDockWidget* dock, m_docks) {
		if(!dock->isFloating() && dock->isVisible() &&
			m_mainWindow->dockWidgetArea(dock) == Qt::RightDockWidgetArea
			&& dock->pos().y() > bottomMostY) {
			bottomMostY = dock->pos().y();
			bottomWidget = dock;
		}
	}
	return bottomWidget;
}

void DockManager::keepMargins() {
	if (m_dontKeepMargins) return;

	/*FDockWidget* newTopWidget = this->newTopWidget();
	if(m_topDock != newTopWidget) {
		removeMargin(m_topDock);
		m_topDock = newTopWidget;
		if(m_topDock) m_oldTopDockStyle = m_topDock->styleSheet();
		addTopMargin(m_topDock);
	}*/

	FDockWidget* newBottomWidget = this->newBottomWidget();
	if(m_bottomDock != newBottomWidget) {
		removeMargin(m_bottomDock);
		m_bottomDock = newBottomWidget;
		if(m_bottomDock) m_oldBottomDockStyle = m_bottomDock->styleSheet();
		addBottomMargin(m_bottomDock);
		m_mainWindow->m_sizeGrip->raise();
	}
}


void DockManager::removeMargin(FDockWidget* dock) {
	if(dock) {
		TripleNavigator *tn = dynamic_cast<TripleNavigator*>(dock->widget());
		if(tn) {
			tn->showBottomMargin(false);
		} else {
			dockMarginAux(dock, "", m_oldBottomDockStyle);
		}
	}
}

void DockManager::addTopMargin(FDockWidget* dock) {
	if(dock) dockMarginAux(dock, "topMostDock", dock->widget()->styleSheet());
}

void DockManager::addBottomMargin(FDockWidget* dock) {
	if(dock) {
		TripleNavigator *tn = dynamic_cast<TripleNavigator*>(dock->widget());
		if(tn) {
			tn->showBottomMargin(true);
		} else {
			dockMarginAux(dock, "bottomMostDock", dock->widget()->styleSheet());
		}
	}
}


void DockManager::dockMarginAux(FDockWidget* dock, const QString &name, const QString &style) {
	Q_ASSERT(dock);

	dock->widget()->setObjectName(name);
	dock->widget()->setStyleSheet(style);
	dock->setStyleSheet(dock->styleSheet());

}

void DockManager::dontKeepMargins() {
	m_dontKeepMargins = true;
}
