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
#include "../debugdialog.h"
#include "../waitpushundostack.h"
#include "../utils/folderutils.h"

#include <QFileInfoList>
#include <QFileInfo>
#include <QRegExp>
#include <QtGui>
#include <QSettings>
#include <QComboBox>

// TODO: 
//		undo
//		text search
//		serial port plugins
//		numbers, string escape chars...
//		hook up char formats for highlighting
//		linking vs loading?
//		save as part of fzz

// linux: finding serial ports using the shell:  "dmesg | grep tty", then clean up the response

QHash<QString, QString> ProgramMainWindow::m_languages;
QHash<QString, class Syntaxer *> ProgramMainWindow::m_syntaxers;

ProgramMainWindow::ProgramMainWindow(QWidget *parent)
	: FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension(), parent)
{
	m_updateEnabled = false;
	if (m_languages.count() == 0) {
		QDir dir(FolderUtils::getApplicationSubFolderPath("translations"));
		dir.cd("syntax");
		QStringList nameFilters;
		nameFilters << "*.xml";
		QFileInfoList list = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks);
		foreach (QFileInfo fileInfo, list) {
			if (fileInfo.baseName().compare("styles") == 0) {
				Highlighter::loadStyles(fileInfo.absoluteFilePath());
			}
			else {
				QString name = Syntaxer::parseForName(fileInfo.absoluteFilePath());
				if (!name.isEmpty()) {
					m_languages.insert(name, fileInfo.absoluteFilePath());
				}
			}
		}
	}
}

ProgramMainWindow::~ProgramMainWindow()
{
}

void ProgramMainWindow::initText() {
}

void ProgramMainWindow::setup()
{
    QFile styleSheet(":/resources/styles/partseditor.qss");
    m_mainFrame = new QFrame(this);
    m_mainFrame->setObjectName("partsEditor");

    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open :/resources/styles/partseditor.qss");
    } else {
    	m_mainFrame->setStyleSheet(styleSheet.readAll());
    }

    resize(500,700);

	setAttribute(Qt::WA_DeleteOnClose, true);

	createHeader();
	createCenter();
	createFooter();

	layout()->setMargin(0);
	layout()->setSpacing(0);

	QGridLayout *layout = new QGridLayout(m_mainFrame);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_headerFrame,0,0);
	layout->addWidget(m_centerFrame,1,0);
	layout->addWidget(m_footerFrame,2,0);
	setCentralWidget(m_mainFrame);


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

void ProgramMainWindow::createHeader() {
	m_headerFrame = new QFrame();
	m_headerFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
	m_headerFrame->setObjectName("header");
	m_headerFrame->setStyleSheet("padding: 2px; padding-bottom: 0;");
}

void ProgramMainWindow::createCenter() {

	m_centerFrame = new QFrame();
	m_centerFrame->setObjectName("center");

	m_textEdit = new QTextEdit;
	m_highlighter = new Highlighter(m_textEdit);

	m_textEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	QGridLayout *tabLayout = new QGridLayout(m_textEdit);
	tabLayout->setMargin(0);
	tabLayout->setSpacing(0);

	QSplitter *splitter = new QSplitter(Qt::Vertical,this);
	splitter->addWidget(m_textEdit);

	QGridLayout *mainLayout = new QGridLayout(m_centerFrame);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(splitter,0,0,1,1);
}

void ProgramMainWindow::createFooter() {
	m_footerFrame = new QFrame();
	m_footerFrame->setObjectName("footer");
	m_footerFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

	QPushButton * loadButton = new QPushButton(tr("Open..."));
	loadButton->setObjectName("loadButton");
	loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadProgramFile()));

	m_saveAsNewPartButton = new QPushButton(tr("save as"));
	m_saveAsNewPartButton->setObjectName("saveAsPartButton");
	m_saveAsNewPartButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_saveAsNewPartButton, SIGNAL(clicked()), this, SLOT(saveAs()));

	m_saveButton = new QPushButton(tr("save"));
	m_saveButton->setObjectName("saveAsButton");
	m_saveButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()));

	QComboBox * comboBox = new QComboBox();
	comboBox->setEditable(false);
	comboBox->setEnabled(true);
	connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeLanguage(const QString &)));
	comboBox->addItems(m_languages.keys());

	updateSaveButton();

	m_cancelCloseButton = new QPushButton(tr("cancel"));
	m_cancelCloseButton->setObjectName("cancelButton");
	m_cancelCloseButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_cancelCloseButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *footerLayout = new QHBoxLayout;

	footerLayout->setMargin(0);
	footerLayout->setSpacing(0);
	footerLayout->addSpacerItem(new QSpacerItem(15,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(comboBox);
	footerLayout->addSpacerItem(new QSpacerItem(15,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(loadButton);
	footerLayout->addSpacerItem(new QSpacerItem(15,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_saveAsNewPartButton);
	footerLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
	footerLayout->addWidget(m_saveButton);
	footerLayout->addSpacerItem(new QSpacerItem(15,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_cancelCloseButton);
	footerLayout->addSpacerItem(new QSpacerItem(15,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	m_footerFrame->setLayout(footerLayout);
}

bool ProgramMainWindow::save() {
	bool result = FritzingWindow::save();
	if(result) {
		m_cancelCloseButton->setText(tr("close"));
	}
	return result;
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
    setTitle();


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
	return "What is this?";
}

const QString ProgramMainWindow::fileExtension() {
	return FritzingPartExtension;
}

const QString ProgramMainWindow::defaultSaveFolder() {
	return QDir::currentPath()+"/parts/user/";
}

void ProgramMainWindow::updateSaveButton() {
	if(m_saveButton) m_saveButton->setEnabled(m_updateEnabled);
}

void ProgramMainWindow::updateButtons() {
	m_saveAsNewPartButton->setEnabled(false);
	m_cancelCloseButton->setText(tr("close"));
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

const QString ProgramMainWindow::fritzingTitle() {
	return FritzingWindow::fritzingTitle();
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
	static int whatever = 1;
	return whatever;
}

void ProgramMainWindow::changeLanguage(const QString & newLanguage) {
	Syntaxer * syntaxer = m_syntaxers.value(newLanguage, NULL);
	if (syntaxer == NULL) {
		syntaxer = new Syntaxer();
		if (syntaxer->loadSyntax(m_languages.value(newLanguage))) {
			m_syntaxers.insert(newLanguage, syntaxer);
		}
	}
	m_highlighter->setSyntaxer(syntaxer);
}

void ProgramMainWindow::loadProgramFile() {
	QString fileName = FolderUtils::getOpenFileName(
							this,
							tr("Select an programming file to load"),
							defaultSaveFolder(),
							m_highlighter->syntaxer()->extensions()
		);

	if (fileName.isEmpty()) return;

	QFile file(fileName);
	if (file.open(QFile::ReadOnly)) {
		QString text = file.readAll();
		m_textEdit->setText(text);
		m_fileName = fileName;
		this->setTitle();
	}
}
