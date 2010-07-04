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

#include "programtab.h"
#include "highlighter.h"
#include "syntaxer.h"
#include "../debugdialog.h"
#include "../utils/folderutils.h"

#include <QFileInfoList>
#include <QFileInfo>
#include <QRegExp>
#include <QtGui>
#include <QSettings>
#include <QFontMetrics>
#include <QTextStream>
#include <QMessageBox>
#include <QSplitter>

/////////////////////////////////////////

SerialPortComboBox::SerialPortComboBox() : QComboBox() {
}

void SerialPortComboBox::showPopup() {
	emit aboutToShow();
	QComboBox::showPopup();
}

/////////////////////////////////////////

DeleteDialog::DeleteDialog(const QString & title, const QString & text, bool deleteFileCheckBox, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
	this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |  Qt::WindowCloseButtonHint);

// code borrowed from QMessageBox

    QLabel * label = new QLabel;
    label->setObjectName(QLatin1String("qt_msgbox_label"));
    label->setTextInteractionFlags(Qt::TextInteractionFlags(this->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this)));
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setOpenExternalLinks(true);
#if defined(Q_WS_MAC)
    label->setContentsMargins(16, 0, 0, 0);
#elif !defined(Q_WS_QWS)
    label->setContentsMargins(2, 0, 0, 0);
    label->setIndent(9);
#endif
    QLabel * iconLabel = new QLabel;
    iconLabel->setObjectName(QLatin1String("qt_msgboxex_icon_label"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->setObjectName(QLatin1String("qt_msgbox_buttonbox"));
    m_buttonBox->setCenterButtons(this->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));
    QObject::connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));


    QGridLayout *grid = new QGridLayout;
#ifndef Q_WS_MAC
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    grid->addWidget(label, 0, 1, 1, 1);
    grid->addWidget(m_buttonBox, 2, 0, 1, 2);
#else
    grid->setMargin(0);
    grid->setVerticalSpacing(8);
    grid->setHorizontalSpacing(0);
    this->setContentsMargins(24, 15, 24, 20);
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    grid->addWidget(label, 0, 1, 1, 1);
    // -- leave space for information label --
    grid->setRowStretch(1, 100);
    grid->setRowMinimumHeight(2, 6);
    grid->addWidget(m_buttonBox, 3, 1, 1, 1);
#endif

	m_checkBox = NULL;
	if (deleteFileCheckBox) {
		m_checkBox = new QCheckBox(tr("Delete the file"));
		grid->addWidget(m_checkBox, 1, 0, 2, 1);
	}

    grid->setSizeConstraint(QLayout::SetNoConstraint);
    this->setLayout(grid);

    if (!title.isEmpty() || !text.isEmpty()) {
        this->setWindowTitle(title);
        label->setText(text);
    }
    this->setModal(true);

#ifdef Q_WS_MAC
    QFont f = this->font();
    f.setBold(true);
    label->setFont(f);
#endif

	m_buttonBox->addButton(QDialogButtonBox::Yes);
	m_buttonBox->addButton(QDialogButtonBox::No);
	m_buttonBox->addButton(QDialogButtonBox::Cancel);
	m_buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Delete"));
	m_buttonBox->button(QDialogButtonBox::No)->setText(tr("Don't Delete"));

	iconLabel->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning));

	this->resize(300, 150);
}

void DeleteDialog::buttonClicked(QAbstractButton * button) {
	this->done(m_buttonBox->standardButton(button));
}

bool DeleteDialog::deleteFileChecked() {
	if (m_checkBox == NULL) return false;

	return m_checkBox->isChecked();
}

/////////////////////////////////////////////

QIcon * AsteriskIcon = NULL;

ProgramTab::ProgramTab(QString & filename, QWidget *parent) : QFrame(parent)
{
	m_tabWidget = NULL;
	while (parent != NULL) {
		QTabWidget * tabWidget = dynamic_cast<QTabWidget *>(parent);
		if (tabWidget) {
			m_tabWidget = tabWidget;
			break;
		}
		parent = parent->parentWidget();
    }

	if (AsteriskIcon == NULL) {
		AsteriskIcon = new QIcon(":/resources/images/icons/asterisk.png");
	}

    m_canCopy = false;
    m_canCut = false;
    m_canUndo = false;
    m_canRedo = false;
    m_language = "";
    m_port = "";
    m_filename = filename;

	m_updateEnabled = false;
	QGridLayout *editLayout = new QGridLayout(this);
	editLayout->setMargin(0);
	editLayout->setSpacing(0);

    m_programWindow = qobject_cast<ProgramWindow *>(window());

    // m_textEdit needs to be initialized before createFooter so
    // some signals get connected properly.
    m_textEdit = new QTextEdit;
    m_textEdit->setFontFamily("Droid Sans Mono");
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    QFontMetrics fm(m_textEdit->currentFont());
    m_textEdit->setTabStopWidth(fm.averageCharWidth() * 4);
    m_textEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_textEdit->setUndoRedoEnabled(true);
    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(m_textEdit, SIGNAL(undoAvailable(bool)), this, SLOT(enableUndo(bool)));
    connect(m_textEdit, SIGNAL(redoAvailable(bool)), this, SLOT(enableRedo(bool)));
    connect(m_textEdit, SIGNAL(copyAvailable(bool)), this, SLOT(enableCopy(bool)));
    connect(m_textEdit, SIGNAL(copyAvailable(bool)), this, SLOT(enableCut(bool))); // Reuse copy signal for cut
    m_highlighter = new Highlighter(m_textEdit);

	QFrame * footerFrame = createFooter();
	editLayout->addWidget(footerFrame,0,0);

	QSplitter * splitter = new QSplitter;
	splitter->setObjectName("splitter");
	splitter->setOrientation(Qt::Vertical);
    editLayout->addWidget(splitter, 1, 0);

	splitter->addWidget(m_textEdit);

	m_console = new QTextEdit();
	m_console->setLineWrapMode(QTextEdit::NoWrap);
	m_console->setObjectName("console");
	m_console->setReadOnly(true);

	splitter->addWidget(m_console);

	splitter->setStretchFactor(0, 8);
    splitter->setStretchFactor(1, 2);
}

ProgramTab::~ProgramTab()
{
}

/**
 * We override the showEvent for the tab to trigger a menu update
 */
void ProgramTab::showEvent(QShowEvent *event) {
    QFrame::showEvent(event);
    updateMenu();
}

QFrame * ProgramTab::createFooter() {
    QFrame * footerFrame = new QFrame();
    footerFrame->setObjectName("footer"); // Used for styling
	footerFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

    QLabel * languageLabel = new QLabel(tr("Language:"), this);

	m_languageComboBox = new QComboBox();
	m_languageComboBox->setEditable(false);
    m_languageComboBox->setEnabled(true);
    m_languageComboBox->addItems(m_programWindow->getAvailableLanguages());
    connect(m_languageComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(setLanguage(const QString &)));
	QSettings settings;
	QString currentLanguage = settings.value("programwindow/language", "").toString();
	if (currentLanguage.isEmpty()) {
		currentLanguage = m_languageComboBox->currentText();
	}
    setLanguage(currentLanguage);

	QPushButton * addButton = new QPushButton(tr("New"));
	//addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(addButton, SIGNAL(clicked()), m_programWindow, SLOT(addTab()));

    QPushButton * loadButton = new QPushButton(tr("Open..."));
	//loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadProgramFile()));

    m_saveButton = new QPushButton(tr("Save"));
	//m_saveButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()));

    QLabel * portLabel = new QLabel(tr("Port:"), this);

    m_portComboBox = new SerialPortComboBox();
    m_portComboBox->setEditable(false);
    m_portComboBox->setEnabled(true);
    m_portComboBox->addItems(m_programWindow->getSerialPorts());
    connect(m_portComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(setPort(const QString &)));
	connect(m_portComboBox, SIGNAL(aboutToShow()), this, SLOT(updateSerialPorts()), Qt::DirectConnection);

	QString currentPort = settings.value("programwindow/port", "").toString();
	if (currentPort.isEmpty()) {
		currentPort = m_portComboBox->currentText();
	}
    setPort(currentPort);

	m_programButton = new QPushButton(tr("Program"));
	m_programButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_programButton, SIGNAL(clicked()), this, SLOT(sendProgram()));
	m_programButton->setEnabled(false);

	QLabel * programmerLabel = new QLabel(tr("Programmer:"), this);

	m_programmerComboBox = new QComboBox();
	m_programmerComboBox->setEditable(false);
    m_programmerComboBox->setEnabled(true);
	updateProgrammers();
    connect(m_programmerComboBox, SIGNAL(activated(int)), this, SLOT(chooseProgrammer(int)));	
	QString currentProgrammer = ProgramWindow::LocateName;
	QString temp = settings.value("programwindow/programmer", "").toString();
	if (!temp.isEmpty()) {
		QFileInfo fileInfo(temp);
		if (fileInfo.exists()) {
			currentProgrammer = temp;
		}
	}
	chooseProgrammerAux(currentProgrammer);

	QHBoxLayout *footerLayout = new QHBoxLayout;

	footerLayout->setMargin(0);
	footerLayout->setSpacing(5);
    footerLayout->addWidget(addButton);
    footerLayout->addWidget(loadButton);
    footerLayout->addWidget(m_saveButton);

    footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Expanding,QSizePolicy::Minimum));

    footerLayout->addWidget(languageLabel);
    footerLayout->addWidget(m_languageComboBox);
    footerLayout->addWidget(portLabel);
    footerLayout->addWidget(m_portComboBox);
	footerLayout->addWidget(programmerLabel);
	footerLayout->addWidget(m_programmerComboBox);
	footerLayout->addWidget(m_programButton);

	footerFrame->setLayout(footerLayout);

	return footerFrame;
}

void ProgramTab::setLanguage(const QString & newLanguage) {
        DebugDialog::debug(QString("Setting language to %1").arg(newLanguage));
        m_language = newLanguage;
        m_languageComboBox->setCurrentIndex(m_languageComboBox->findText(newLanguage));
        m_highlighter->setSyntaxer(m_programWindow->getSyntaxerForLanguage(newLanguage));
		m_highlighter->rehighlight();
        updateMenu();
		QSettings settings;
		settings.setValue("programwindow/language", newLanguage);
}

void ProgramTab::setPort(const QString & newPort) {
        DebugDialog::debug(QString("Setting port to %1").arg(newPort));
        m_port = newPort;
        m_portComboBox->setCurrentIndex(m_portComboBox->findText(newPort));
        updateMenu();
		QSettings settings;
		settings.setValue("programwindow/port", newPort);
}

bool ProgramTab::loadProgramFile() {
	if (isModified() || !m_textEdit->toPlainText().isEmpty()) {
		m_programWindow->loadProgramFileNew();
		return false;
	}

	QString fileName = FolderUtils::getOpenFileName(
						this,
						tr("Select a program file to load"),
                        FolderUtils::openSaveFolder(),
                        m_highlighter->syntaxer()->extensionString()
		);

	if (fileName.isEmpty()) return false;

	return loadProgramFile(fileName, "");
}

bool ProgramTab::loadProgramFile(const QString & fileName, const QString & altFileName) {
	if (m_programWindow->alreadyHasProgram(fileName)) {
		return false;
	}

	QFile file(fileName);
	if (!file.open(QFile::ReadOnly)) {
		emit wantToLink(fileName, false);
		if (!altFileName.isEmpty()) {
			file.setFileName(altFileName);
			if (!file.open(QFile::ReadOnly)) {
				QFileInfo fileInfo(fileName);
				QString fn = FolderUtils::getOpenFileName(
										NULL,
										tr("Fritzing is unable to find '%1', please locate it").arg(fileInfo.fileName()),
										FolderUtils::openSaveFolder(),
										tr("Program (*.%1)").arg(fileInfo.suffix())
								);
				if (fn.isEmpty()) return false;

				file.setFileName(fn);
				if (!file.open(QFile::ReadOnly)) {
					return false;
				}
			}
		}
	}

	m_filename = file.fileName();
	QString text = file.readAll();
	m_textEdit->setText(text);
	setClean();
	QFileInfo fileInfo(m_filename);
	m_tabWidget->setTabText(m_tabWidget->currentIndex(), fileInfo.fileName());
	emit wantToLink(m_filename, true);
	return true;
}

void ProgramTab::textChanged() {
	QIcon tabIcon = m_tabWidget->tabIcon(m_tabWidget->currentIndex());
	bool modified = m_textEdit->document()->isModified();

	if (m_saveButton) {
		m_saveButton->setEnabled(modified);
		updateMenu();
	}

	if (tabIcon.isNull()) {
		if (modified) {
			m_tabWidget->setTabIcon(m_tabWidget->currentIndex(), *AsteriskIcon);
		}
	}
	else {
		if (!modified) {
			m_tabWidget->setTabIcon(m_tabWidget->currentIndex(), QIcon());
		}
	}
}

void ProgramTab::undo() {
	m_textEdit->undo();
}

void ProgramTab::enableUndo(bool enable) {
        m_canUndo = enable;
		m_saveButton->setEnabled(m_canUndo);
        updateMenu();
}

void ProgramTab::redo() {
    m_textEdit->redo();
}

void ProgramTab::enableRedo(bool enable) {
        m_canRedo = enable;
        updateMenu();
}

void ProgramTab::cut() {
        m_textEdit->cut();
}

void ProgramTab::enableCut(bool enable) {
        m_canCut = enable;
        updateMenu();
}

void ProgramTab::copy() {
        m_textEdit->copy();
}

void ProgramTab::enableCopy(bool enable) {
        m_canCopy = enable;
        updateMenu();
}

void ProgramTab::paste() {
        m_textEdit->paste();
}

void ProgramTab::selectAll() {
        m_textEdit->selectAll();
}

void ProgramTab::portProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	DebugDialog::debug(QString("process finished %1 %2").arg(exitCode).arg(exitStatus));

	// parse the text and update the combo box

	sender()->deleteLater();
}

void ProgramTab::portProcessReadyRead() {
    QStringList ports;

	QByteArray byteArray = qobject_cast<QProcess *>(sender())->readAllStandardOutput();
    QTextStream textStream(byteArray, QIODevice::ReadOnly);
    while (true) {
        QString line = textStream.readLine();
        if (line.isNull()) break;

        if (!line.contains("tty")) continue;
        if (!line.contains("serial", Qt::CaseInsensitive)) continue;

        QStringList candidates = line.split(" ");
        foreach (QString candidate, candidates) {
            if (candidate.contains("tty")) {
                ports.append(candidate);
                break;
            }
        }
    }
    m_portComboBox->addItems(ports);
}

void ProgramTab::deleteTab() {
	bool deleteFile = false;
	if (!m_textEdit->toPlainText().isEmpty()) {
		QString name = QFileInfo(m_filename).baseName();
		if (name.isEmpty()) {
			name = m_tabWidget->tabText(m_tabWidget->currentIndex());
		}

		DeleteDialog deleteDialog(tr("Delete \"%1\"?").arg(name),
								  tr("Are you sure you want to delete \"%1\"?").arg(name),
								  !FolderUtils::isEmptyFileName(m_filename, "Untitled"),
								  NULL, 0);
		int reply = deleteDialog.exec();

		/*

		QMessageBox messageBox(
				tr("Delete \"%1\"?").arg(name),
				tr("Are you sure you want to delete \"%1\"?").arg(name),
				QMessageBox::Warning,
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default,
				this->window(), Qt::Sheet);

		messageBox.setButtonText(QMessageBox::Yes, tr("Delete"));
		messageBox.setButtonText(QMessageBox::No, tr("Don't Delete"));
		messageBox.button(QMessageBox::No)->setShortcut(tr("Ctrl+D"));
		messageBox.setInformativeText(tr("This program will be unlinked from the sketch, but the file won't be deleted."));

		QMessageBox::StandardButton reply = (QMessageBox::StandardButton) messageBox.exec();

		*/

 		if (reply != QMessageBox::Yes) {
     		return;
		}

		deleteFile = deleteDialog.deleteFileChecked();
	}

	if (m_tabWidget) {
		emit wantToDelete(m_tabWidget->currentIndex(), deleteFile);
		this->deleteLater();
	}
}

bool ProgramTab::isModified() {
	return m_textEdit->document()->isModified();
}

const QString & ProgramTab::filename() {
	return m_filename;
}

void ProgramTab::setFilename(const QString & name) {
        m_filename = name;
        updateMenu();

        // Here we update the widget's tab label. We need
        // to check that it both has the PTabWidget as its
        // parent and it's already been added to the PTabWidget
        int currentIndex = m_tabWidget->indexOf(this);
        if (currentIndex >= 0) {
            m_tabWidget->setTabText(currentIndex, name.section("/",-1));
        }
        else {
            DebugDialog::debug("Negative index");
        }
}

const QStringList & ProgramTab::extensions() {
    if (m_highlighter == NULL) return ___emptyStringList___;

	Syntaxer * syntaxer = m_highlighter->syntaxer();
	if (syntaxer == NULL) return ___emptyStringList___;

	return syntaxer->extensions();
}

const QString & ProgramTab::extensionString() {
    if (m_highlighter == NULL) return ___emptyString___;

	Syntaxer * syntaxer = m_highlighter->syntaxer();
	if (syntaxer == NULL) return ___emptyString___;

	return syntaxer->extensionString();
}


void ProgramTab::setClean() {
	m_textEdit->document()->setModified(false);
	textChanged();
}

void ProgramTab::save() {
	emit wantToSave(m_tabWidget->currentIndex());
}

void ProgramTab::saveAs() {
	emit wantToSaveAs(m_tabWidget->currentIndex());
}

void ProgramTab::rename() {
        emit wantToRename(m_tabWidget->currentIndex());
}

void ProgramTab::print(QPrinter &printer) {
        m_textEdit->print(&printer);
}

void ProgramTab::setText(QString text) {
        m_textEdit->setPlainText(text);
}

QString ProgramTab::text() {
        return m_textEdit->toPlainText();
}

bool ProgramTab::readOnly() {
	// TODO: return true if it's a sample file
	return false;
}

bool ProgramTab::save(const QString & filename) {
	QFile file(filename);
	if (!file.open(QFile::WriteOnly)) {
		return false;
	}

	QByteArray b = m_textEdit->toPlainText().toLatin1();
	qint64 written = file.write(b);
	file.close();
	bool result = (b.length() == written);
	if (result) {
                setFilename(filename);
	}
	return result;
}

void ProgramTab::sendProgram() {
	if (m_programmerPath.isEmpty()) return;

	m_programButton->setEnabled(false);

	QString language = m_languageComboBox->currentText();
	if (language.compare("picaxe", Qt::CaseInsensitive) == 0) {
		QProcess * process = new QProcess(this);
		process->setProcessChannelMode(QProcess::MergedChannels);
		process->setReadChannel(QProcess::StandardOutput);

		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(programProcessFinished(int, QProcess::ExitStatus)));
		connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(programProcessReadyRead()));

		QStringList args;
		args.append(QString("-c%1").arg(m_portComboBox->currentText()));
		args.append(m_filename);
		m_console->setPlainText("");
		
        process->start(m_programmerPath, args);
	}

}

void ProgramTab::chooseProgrammer(int index) {
	QString realname = m_programmerComboBox->itemData(index).toString();
	if (realname == ProgramWindow::LocateName) {
		// if "Locate..." is chosen, always trigger
		chooseProgrammer(realname);
	}
	else {
		if (m_programmerPath != realname) {
			// only trigger if path actually changed
			chooseProgrammer(realname);
		}
	}
}

void ProgramTab::chooseProgrammer(const QString & programmer)
{
	QString filename = programmer;
	if (!QFileInfo(filename).exists()) {
		filename = FolderUtils::getOpenFileName(
							this,
							tr("Select a programmer (executable) for %1").arg(this->m_language),
							FolderUtils::openSaveFolder(),
                            "(Programmer *.exe)"
							);
	}
    if (filename.isEmpty()) return;

	chooseProgrammerAux(filename);
}

void ProgramTab::chooseProgrammerAux(const QString & programmer) 
{
	if (programmer == ProgramWindow::LocateName) {
		if (m_programButton) {
			m_programButton->setEnabled(false);
		}
		m_programmerPath.clear();
		updateProgrammerComboBox(programmer);
		updateMenu();
		return;
	}

	if (m_programmerComboBox->findData(programmer) < 0) {
		QFileInfo fileInfo(programmer);
		m_programmerComboBox->addItem(fileInfo.fileName(), programmer);
	}

    m_programmerPath = programmer;
	if (m_programButton) {
		m_programButton->setEnabled(true);
	}
	updateProgrammerComboBox(programmer);
    updateMenu();
	QSettings settings;
	settings.setValue("programwindow/programmer", programmer);
}

void ProgramTab::programProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	DebugDialog::debug(QString("program process finished %1 %2").arg(exitCode).arg(exitStatus));

	m_programButton->setEnabled(true);

	sender()->deleteLater();
}

void ProgramTab::programProcessReadyRead() {
    QStringList ports;

	QByteArray byteArray = qobject_cast<QProcess *>(sender())->readAllStandardOutput();
    m_console->append(byteArray);
}


/**
 * This function emits all the required signals to update the menu of its owning ProgramWindow.
 * It is currently called on the QTabWidget's currentChanged() signal.
 */
void ProgramTab::updateMenu() {
//        DebugDialog::debug(QString("Updating the menu for tab %1").arg(m_tabWidget->currentIndex()));
//        DebugDialog::debug(QString("Program: %1").arg(!m_programmerPath.isEmpty()));
//        DebugDialog::debug(QString("Undo: %1").arg(m_canUndo));
//        DebugDialog::debug(QString("Redo: %1").arg(m_canRedo));
//        DebugDialog::debug(QString("Cut: %1").arg(m_canCut));
//        DebugDialog::debug(QString("Copy: %1").arg(m_canCopy));
//        DebugDialog::debug(QString("Language: %1").arg(m_language));
//        DebugDialog::debug(QString("Port: %1").arg(m_port));

        // Emit a signal so that the ProgramWindow can update its own UI.
        emit programWindowUpdateRequest(!m_programmerPath.isEmpty(), m_canUndo, m_canRedo, m_canCut, m_canCopy, 
			m_language, m_port, m_programmerPath.isEmpty() ? ProgramWindow::LocateName : m_programmerPath, m_filename);
}

void ProgramTab::updateProgrammerComboBox(const QString & programmer) {
	// don't want activate signal triggered recursively
	disconnect(m_programmerComboBox, SIGNAL(activated(int)), this, SLOT(chooseProgrammer(int)));
	m_programmerComboBox->setCurrentIndex(m_programmerComboBox->findData(programmer));
	connect(m_programmerComboBox, SIGNAL(activated(int)), this, SLOT(chooseProgrammer(int)));
}

void ProgramTab::updateProgrammers() {

	QString currentProgrammer;
	int index = m_programmerComboBox->currentIndex();
	if (index >= 0) {
		currentProgrammer = m_programmerComboBox->itemData(index).toString();
	}

	while (m_programmerComboBox->count() > 0) {
		m_programmerComboBox->removeItem(0);
	}

	QHash<QString, QString> programmerNames = m_programWindow->getProgrammerNames();
	foreach (QString name, programmerNames.keys()) {
		m_programmerComboBox->addItem(name, programmerNames.value(name));
	}

	if (!currentProgrammer.isEmpty()) {
		updateProgrammerComboBox(currentProgrammer);
	}
}

void ProgramTab::updateSerialPorts() {
	QStringList ports = m_programWindow->getSerialPorts();

	QStringList newPorts;
	foreach (QString port, ports) {
		if (m_portComboBox->findText(port) < 0) {
			newPorts.append(port);
		}
	}
	QList<int> obsoletePorts;
	for (int i = 0; i < m_portComboBox->count(); i++) {
		QString port = m_portComboBox->itemText(i);
		if (!ports.contains(port)) {
			obsoletePorts.append(i);
		}
		if (i == m_portComboBox->currentIndex()) {
			// TODO: what?
		}
	}

	for (int i = obsoletePorts.count() - 1; i >= 0; i--) {
		m_portComboBox->removeItem(obsoletePorts.at(i));
	}
	foreach (QString port, newPorts) {
		m_portComboBox->addItem(port);
	}
}
