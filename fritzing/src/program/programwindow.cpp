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

#include "programwindow.h"
#include "highlighter.h"
#include "syntaxer.h"
#include "programtab.h"

#include "../debugdialog.h"
#include "../waitpushundostack.h"
#include "../utils/folderutils.h"

#include <QFileInfoList>
#include <QFileInfo>
#include <QRegExp>
#include <QtGui>
#include <QSettings>
#include <QFontMetrics>
#include <QTextStream>

#ifdef Q_WS_WIN
#include "windows.h"
#endif

// TODO: 
//		text search
//		serial port plugins?
//		numbers, string escape chars...
//		where to store program files when opening fzz
//		update serial port list (when?)

static int UntitledIndex = 1;				

ProgramWindow::ProgramWindow(QWidget *parent)
	: FritzingWindow("", untitledFileCount(), "", parent)
{
	m_savingProgramTab = NULL;
	UntitledIndex--;						// incremented by FritzingWindow
	ProgramWindow::setTitle();				// set to something weird by FritzingWindow
}

ProgramWindow::~ProgramWindow()
{
}

void ProgramWindow::initText() {
}

void ProgramWindow::setup(const QStringList & programs, const QString & alternativePath)
{
    QFile styleSheet(":/resources/styles/programwindow.qss");
    QFrame * mainFrame = new QFrame(this);
    mainFrame->setObjectName("programmingWindow");

    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open :/resources/styles/programwindow.qss");
    } else {
    	mainFrame->setStyleSheet(styleSheet.readAll());
    }

    resize(500,700);

	setAttribute(Qt::WA_DeleteOnClose, true);

	QFrame * headerFrame = createHeader();
	QFrame * centerFrame = createCenter();

	layout()->setMargin(0);
	layout()->setSpacing(0);

	QGridLayout *layout = new QGridLayout(mainFrame);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(headerFrame,0,0);
	layout->addWidget(centerFrame,1,0);

	setCentralWidget(mainFrame);

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	QSettings settings;
	//if (!settings.value("peditor/state").isNull()) {
		//restoreState(settings.value("peditor/state").toByteArray());
	//}
	//if (!settings.value("peditor/geometry").isNull()) {
		//restoreGeometry(settings.value("peditor/geometry").toByteArray());
	//}

	installEventFilter(this);

	if (programs.count() == 0) return;

	bool firstTime = true;
	foreach (QString program, programs) {
		ProgramTab * programTab = NULL;
		if (firstTime) {
			firstTime = false;
			programTab = dynamic_cast<ProgramTab *>(m_tabWidget->widget(0));
		}
		else {
			programTab = addTab();
		}
		QDir dir(alternativePath);
		QFileInfo fileInfo(program);
		programTab->loadProgramFile(program, dir.absoluteFilePath(fileInfo.fileName()));
	}
}

QFrame * ProgramWindow::createHeader() {
	QFrame * headerFrame = new QFrame();
	headerFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
	headerFrame->setObjectName("header");
	return headerFrame;
}

QFrame * ProgramWindow::createCenter() {

	QFrame * centerFrame = new QFrame();
	centerFrame->setObjectName("center");

	m_tabWidget = new PTabWidget(centerFrame);
	m_tabWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_tabWidget->setMovable(true);
	m_tabWidget->setUsesScrollButtons(false);
	m_tabWidget->setElideMode(Qt::ElideLeft);

	m_addButton = new QPushButton("+", m_tabWidget);
	m_addButton->setObjectName("addButton");
	connect(m_addButton, SIGNAL(clicked()), this, SLOT(addTab()));
	m_tabWidget->setCornerWidget(m_addButton, Qt::TopRightCorner);

	addTab();

	QGridLayout *tabLayout = new QGridLayout(m_tabWidget);
	tabLayout->setMargin(0);
	tabLayout->setSpacing(0);

	QGridLayout *mainLayout = new QGridLayout(centerFrame);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(m_tabWidget,0,0,1,1);

	return centerFrame;
}


void ProgramWindow::cleanUp() {
}

void ProgramWindow::parentAboutToClose() {
	if(beforeClosing(false)) {
		cleanUp();
	}
}

void ProgramWindow::closeEvent(QCloseEvent *event) {
	if(beforeClosing()) {
		cleanUp();
		QMainWindow::closeEvent(event);
		emit closed();
	} else {
		event->ignore();
	}

	QSettings settings;
	//settings.setValue("peditor/state",saveState());
	//settings.setValue("peditor/geometry",saveGeometry());
}

const QString ProgramWindow::untitledFileName() {
	return "Untitled";
}

const QString ProgramWindow::fileExtension() {
	return "";
}

const QString ProgramWindow::defaultSaveFolder() {
	return FolderUtils::openSaveFolder();
}

bool ProgramWindow::eventFilter(QObject *object, QEvent *event) {
	if (object == this && event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
		if(keyEvent && keyEvent->matches(QKeySequence::Close)) {
			this->close();
			event->ignore();
			QCoreApplication::processEvents();
#ifdef Q_WS_MAC
			FritzingWindow *parent = dynamic_cast<FritzingWindow*>(parentWidget());
			if(parent) {
				parent->notClosableForAWhile();
			}
#endif
			return true;
		}
	}
	return QMainWindow::eventFilter(object, event);
}

bool ProgramWindow::event(QEvent * e) {
	switch (e->type()) {
		case QEvent::WindowActivate:
			emit changeActivationSignal(true, this);
			break;
		case QEvent::WindowDeactivate:
			emit changeActivationSignal(false, this);
			break;
		default:
			break;
	}
	return FritzingWindow::event(e);
}

int & ProgramWindow::untitledFileCount() {
	return UntitledIndex;
}

void ProgramWindow::setTitle() {
	setWindowTitle(tr("Programming Window"));
}

ProgramTab * ProgramWindow::addTab() {
	ProgramTab * programTab = new ProgramTab(m_tabWidget);
	connect(programTab, SIGNAL(wantToSaveAs(int)), this, SLOT(tabSaveAs(int)));
	connect(programTab, SIGNAL(wantToSave(int)), this, SLOT(tabSave(int)));
	connect(programTab, SIGNAL(wantBeforeClosing(int, bool &)), this, SLOT(tabBeforeClosing(int, bool &)), Qt::DirectConnection);
	connect(programTab, SIGNAL(wantToDelete(int, bool)), this, SLOT(tabDelete(int, bool)), Qt::DirectConnection);
	connect(programTab, SIGNAL(wantToLink(const QString &, bool)), this, SLOT(tabLinkTo(const QString &, bool)));
	QString name = (UntitledIndex == 1) ? untitledFileName() : tr("%1 %2").arg(untitledFileName()).arg(UntitledIndex);
	programTab->setFilename(name);
	int ix = m_tabWidget->addTab(programTab, name);
	m_tabWidget->setCurrentIndex(ix);
	UntitledIndex++;

	return programTab;
}

bool ProgramWindow::beforeClosing(bool showCancel) {
	for (int i = 0; i < m_tabWidget->count(); i++) {
		if (!beforeClosingTab(i, showCancel)) {
			return false;
		}
	}

	return true;
}

bool ProgramWindow::beforeClosingTab(int index, bool showCancel) 
{
	ProgramTab * programTab = dynamic_cast<ProgramTab *>(m_tabWidget->widget(index));
	if (programTab == NULL) return true;

	if (!programTab->isModified()) return true;

	QMessageBox::StandardButton reply = beforeClosingMessage(programTab->filename(), showCancel);
	if (reply == QMessageBox::Yes) {
		return prepSave(index, programTab, false);
	} 
	
	if (reply == QMessageBox::No) {
		return true;
	}
 		
	return false;
}

bool ProgramWindow::saveAsAux(const QString & fileName) {
	if (!m_savingProgramTab) return false;

	bool result = m_savingProgramTab->save(fileName);
	m_savingProgramTab = NULL;
	return result;
}


void ProgramWindow::tabDelete(int index, bool deleteFile) {
	ProgramTab * programTab = dynamic_cast<ProgramTab *>(m_tabWidget->widget(index));
	QString fname = programTab->filename();
	if (programTab != NULL) {
		emit linkToProgramFile(fname, false);
	}
	m_tabWidget->removeTab(index);
	if (m_tabWidget->count() == 0) {
		addTab();
	}

	if (deleteFile) {
		QFile file(fname);
		file.remove();
	}
}

void ProgramWindow::tabSave(int index) {
	ProgramTab * programTab = dynamic_cast<ProgramTab *>(m_tabWidget->widget(index));
	if (programTab == NULL) return;

    prepSave(index, programTab, false);
}

void ProgramWindow::tabSaveAs(int index) {
	ProgramTab * programTab = dynamic_cast<ProgramTab *>(m_tabWidget->widget(index));
	if (programTab == NULL) return;

	QString formerFilename = programTab->filename();
	if (prepSave(index, programTab, true)) {
		emit linkToProgramFile(formerFilename, false);
	}
}

void ProgramWindow::tabBeforeClosing(int index, bool & ok) {
	ok = beforeClosingTab(index, true);
}


bool ProgramWindow::prepSave(int index, ProgramTab * programTab, bool saveAsFlag) 
{
	m_savingProgramTab = programTab;				// need this for the saveAsAux call

	bool result = (saveAsFlag) 
		? saveAs(programTab->filename(), programTab->extension(), programTab->readOnly())
		: save(programTab->filename(), programTab->extension(), programTab->readOnly());

	if (result) {
		m_tabWidget->setTabText(index, programTab->filename());
		programTab->setClean();
		emit linkToProgramFile(programTab->filename(), true);
	}
	return result;
}

void ProgramWindow::tabLinkTo(const QString & filename, bool link) 
{
	emit linkToProgramFile(filename, link);
}

///////////////////////////////////////////////

PTabWidget::PTabWidget(QWidget * parent) : QTabWidget(parent) {
}

QTabBar * PTabWidget::tabBar() {
	return QTabWidget::tabBar();
}
