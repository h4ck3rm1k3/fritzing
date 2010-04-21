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

#include "programmainwindow.h"
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
//		save message if stack dirty at closing
//		program (shell)
//		include in fz
//		include in fzz
//		* in tab representing dirty

static int UntitledIndex = 1;

ProgramMainWindow::ProgramMainWindow(QWidget *parent)
	: FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension(), parent)
{
	ProgramMainWindow::setTitle();
}

ProgramMainWindow::~ProgramMainWindow()
{
}

void ProgramMainWindow::initText() {
}

void ProgramMainWindow::setup()
{
    QFile styleSheet(":/resources/styles/programmingwindow.qss");
    QFrame * mainFrame = new QFrame(this);
    mainFrame->setObjectName("programmingWindow");

    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open :/resources/styles/programmingwindow.qss");
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
}

QFrame * ProgramMainWindow::createHeader() {
	QFrame * headerFrame = new QFrame();
	headerFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
	headerFrame->setObjectName("header");
	return headerFrame;
}

QFrame * ProgramMainWindow::createCenter() {

	QFrame * centerFrame = new QFrame();
	centerFrame->setObjectName("center");

	m_tabWidget = new PTabWidget(centerFrame);
	m_tabWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_tabWidget->setMovable(true);

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

bool ProgramMainWindow::save() {
	return FritzingWindow::save();
}

bool ProgramMainWindow::saveAs() {
	QString fileNameBU = m_fileName;

	QString title = "what's this";
	m_fileName = title != ___emptyString___ ? title+FritzingPartExtension : m_fileName;

	// TODO Mariano: This folder should be defined in the preferences... some day
	QString partsFolderPath = FolderUtils::getUserDataStorePath("parts")+"/";
	QString userPartsFolderPath = partsFolderPath+"user/";

	bool firstTime = true; // Perhaps the user wants to use the default file name, confirm first
	while(m_fileName.isEmpty()
		  || QFileInfo(userPartsFolderPath+m_fileName).exists()
		  || (FolderUtils::isEmptyFileName(m_fileName,untitledFileName()) && firstTime)
		) 
	{
		bool ok;
		m_fileName = QInputDialog::getText(
			this,
			tr("Save as new part"),
			tr("There's already a file with this name.\nPlease, specify a new filename"),
			QLineEdit::Normal,
			m_fileName,
			&ok
		);
		firstTime = false;
		if (!ok) {
			m_fileName = fileNameBU;
			return false;
		}
	}

	Qt::CaseSensitivity cs = Qt::CaseSensitive;
#ifdef Q_WS_WIN
	// seems to be necessary for Windows: getUserDataStorePath() returns a string starting with "c:"
	// but the file dialog returns a string beginning with "C:"
	cs = Qt::CaseInsensitive;
#endif

	QString filename = !m_fileName.startsWith(userPartsFolderPath, cs)
					&& !m_fileName.startsWith(partsFolderPath, cs)
			? userPartsFolderPath+m_fileName
			: m_fileName;
	QString guid = "__"+FolderUtils::getRandText()+FritzingPartExtension;
	if(!alreadyHasExtension(filename, FritzingPartExtension)) {
		filename += guid;
	} else {
		filename.replace(FritzingPartExtension,guid);
	}

	saveAsAux(filename);

	return false;
}

bool ProgramMainWindow::saveAsAux(const QString & fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    file.close();


    QApplication::setOverrideCursor(Qt::WaitCursor);


	QString fileNameAux = QFileInfo(fileName).fileName();
    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);

    m_fileName = fileName;
    //setCurrentFile(fileName);

   // mark the stack clean so we update the window dirty flag
    m_undoStack->setClean();
    m_tabWidget->setTabText(m_tabWidget->currentIndex(), fileNameAux);

	return true;
}


void ProgramMainWindow::cleanUp() {
}

void ProgramMainWindow::parentAboutToClose() {
	if(beforeClosing(false)) {
		cleanUp();
	}
}

void ProgramMainWindow::closeEvent(QCloseEvent *event) {
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

const QString ProgramMainWindow::untitledFileName() {
	return "Programming Window";
}

const QString ProgramMainWindow::fileExtension() {
	return FritzingPartExtension;
}

const QString ProgramMainWindow::defaultSaveFolder() {
	return FolderUtils::openSaveFolder();
}

bool ProgramMainWindow::eventFilter(QObject *object, QEvent *event) {
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

bool ProgramMainWindow::event(QEvent * e) {
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

int & ProgramMainWindow::untitledFileCount() {
	return UntitledIndex;
}

void ProgramMainWindow::setTitle() {
	setWindowTitle(tr("Programming Window"));
}

void ProgramMainWindow::addTab() {
	QFrame * editFrame = new ProgramTab(m_tabWidget);
	QString name = (UntitledIndex == 1) ? tr("Untitled") : tr("Untitled %1").arg(UntitledIndex);
	m_tabWidget->addTab(editFrame, name);
	UntitledIndex++;
}

///////////////////////////////////////////////

PTabWidget::PTabWidget(QWidget * parent) : QTabWidget(parent) {
}

QTabBar * PTabWidget::tabBar() {
	return QTabWidget::tabBar();
}
