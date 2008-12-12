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

$Revision: 1794 $:
$Author: merunga $:
$Date: 2008-12-11 14:50:11 +0100 (Thu, 11 Dec 2008) $

********************************************************************/

#include <QSizeGrip>
#include <QStatusBar>

#include "dockmanager.h"
#include "triplenavigator.h"

DockManager::DockManager(MainWindow *mainWindow)
	: QObject(mainWindow)
{
	m_mainWindow = mainWindow;
	m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	m_mainWindow->setDockOptions(QMainWindow::AnimatedDocks);
	//QSizeGrip *sizeGrip = new QSizeGrip(m_mainWindow);
	//m_mainWindow->m_sizeGrip = sizeGrip;
}

void DockManager::dockChangeActivation(FDockWidget *) {
	if (!m_mainWindow->m_closing) {
		m_mainWindow->changeActivation(NULL);
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
	dockIt(m_mainWindow->m_paletteWidget, PartsBinMinHeight, PartsBinDefaultHeight);

    makeDock(tr("Part Inspector"), m_mainWindow->m_infoView, InfoViewMinHeight, InfoViewDefaultHeight);

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

    m_mainWindow->m_consoleView = new Console();
    FDockWidget * dock = makeDock(tr("Console"), m_mainWindow->m_consoleView, DockMinHeight, DockDefaultHeight, Qt::BottomDockWidgetArea);
	dock->hide();

    m_mainWindow->m_windowMenu->addSeparator();
    m_mainWindow->m_windowMenu->addAction(m_mainWindow->m_toggleDebuggerOutputAct);
    m_mainWindow->m_windowMenu->addSeparator();
}

FDockWidget * DockManager::makeDock(const QString & title, QWidget * widget, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area) {
    FDockWidget * dock = new FDockWidget(title, m_mainWindow);
    dock->setWidget(widget);
    widget->setParent(dock);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(ensureSpaceForSizeGrip(Qt::DockWidgetArea)));

	return dockIt(dock, dockMinHeight, dockDefaultHeight, area);
}

FDockWidget *DockManager::dockIt(FDockWidget* dock, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area) {
    //dock->setStyle(new QCleanlooksStyle());
	dock->setAllowedAreas(area);
	m_mainWindow->addDockWidget(area, dock);
    m_mainWindow->m_windowMenu->addAction(dock->toggleViewAction());

    dock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock->setMinimumSize(DockMinWidth, dockMinHeight);
	dock->resize(DockDefaultWidth, dockDefaultHeight);
    connect(dock, SIGNAL(dockChangeActivationSignal(FDockWidget *)), this, SLOT(dockChangeActivation(class FDockWidget *)));

    return dock;
}


void DockManager::ensureSpaceForSizeGrip(Qt::DockWidgetArea area) {
	//m_mainWindow->corner(Qt::BottomRightCorner)
}
