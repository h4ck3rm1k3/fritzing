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

#ifdef Q_WS_WIN
#include "windows.h"
#endif

QHash<QString, QString> ProgramTab::m_languages;
QHash<QString, class Syntaxer *> ProgramTab::m_syntaxers;

ProgramTab::ProgramTab(QWidget *parent) : QFrame(parent)
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

	if (m_languages.count() == 0) {
		initLanguages();
	}

	m_updateEnabled = false;
	QGridLayout *editLayout = new QGridLayout(this);
	editLayout->setMargin(0);
	editLayout->setSpacing(0);

	QFrame * footerFrame = createFooter();
	editLayout->addWidget(footerFrame,0,0);

	m_textEdit = new QTextEdit;
	m_textEdit->setFontFamily("Droid Sans Mono");
	QFontMetrics fm(m_textEdit->currentFont());
	m_textEdit->setTabStopWidth(fm.averageCharWidth() * 4);
	m_textEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_textEdit->setUndoRedoEnabled(true);
	connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(m_textEdit, SIGNAL(undoAvailable(bool)), this, SLOT(textUndoAvailable(bool)));
	connect(m_textEdit, SIGNAL(redoAvailable(bool)), this, SLOT(textRedoAvailable(bool)));
	m_highlighter = new Highlighter(m_textEdit);

	editLayout->addWidget(m_textEdit, 1, 0);

	changeLanguage(m_languageComboBox->currentText());
}

ProgramTab::~ProgramTab()
{
}

void ProgramTab::initLanguages() {
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

QFrame * ProgramTab::createFooter() {
	QFrame * footerFrame = new QFrame();
	footerFrame->setObjectName("footer");
	footerFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

	m_languageComboBox = new QComboBox();
	m_languageComboBox->setEditable(false);
	m_languageComboBox->setEnabled(true);
	m_languageComboBox->addItems(m_languages.keys());
	connect(m_languageComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeLanguage(const QString &)));

	QPushButton * loadButton = new QPushButton(tr("Open..."));
	loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadProgramFile()));

	m_undoButton = new QPushButton(tr("undo"));
	m_undoButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_undoButton, SIGNAL(clicked()), this, SLOT(undoText()));
	m_undoButton->setEnabled(false);

	m_redoButton = new QPushButton(tr("redo"));
	m_undoButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_redoButton, SIGNAL(clicked()), this, SLOT(redoText()));
	m_redoButton->setEnabled(false);

	m_saveAsButton = new QPushButton(tr("save as"));
	m_saveAsButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_saveAsButton, SIGNAL(clicked()), this, SLOT(saveAs()));

	m_saveButton = new QPushButton(tr("save"));
	m_saveButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()));

	updateSaveButton();

    m_portComboBox = new QComboBox();
    m_portComboBox->setEditable(false);
    m_portComboBox->setEnabled(true);
    //connect(m_portComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeLanguage(const QString &)));
    m_portComboBox->addItems(getSerialPorts());

	QPushButton * programButton = new QPushButton(tr("Program"));
	programButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//connect(programButton, SIGNAL(clicked()), this, SLOT(program()));

	QHBoxLayout *footerLayout = new QHBoxLayout;

	footerLayout->setMargin(0);
	footerLayout->setSpacing(0);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_languageComboBox);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(loadButton);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_undoButton);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_redoButton);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
	footerLayout->addWidget(m_saveAsButton);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_saveButton);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
    footerLayout->addWidget(m_portComboBox);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(programButton);
	footerLayout->addSpacerItem(new QSpacerItem(5,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerFrame->setLayout(footerLayout);

	return footerFrame;
}



void ProgramTab::updateSaveButton() {
	if(m_saveButton) m_saveButton->setEnabled(m_updateEnabled);
}

void ProgramTab::changeLanguage(const QString & newLanguage) {
	Syntaxer * syntaxer = m_syntaxers.value(newLanguage, NULL);
	if (syntaxer == NULL) {
		syntaxer = new Syntaxer();
		if (syntaxer->loadSyntax(m_languages.value(newLanguage))) {
			m_syntaxers.insert(newLanguage, syntaxer);
		}
	}
	m_highlighter->setSyntaxer(syntaxer);
}

void ProgramTab::loadProgramFile() {
	QString fileName = FolderUtils::getOpenFileName(
							this,
							tr("Select a programming file to load"),
							FolderUtils::openSaveFolder(),
							m_highlighter->syntaxer()->extensions()
		);

	if (fileName.isEmpty()) return;

	QFile file(fileName);
	if (file.open(QFile::ReadOnly)) {
		QString text = file.readAll();
		m_textEdit->setUndoRedoEnabled(false);
		m_textEdit->setText(text);
		m_textEdit->setUndoRedoEnabled(true);
		m_filename = fileName;
		QFileInfo fileInfo(m_filename);
		if (m_tabWidget) {
			m_tabWidget->setTabText(m_tabWidget->currentIndex(), fileInfo.fileName());
		}
	}
}

void ProgramTab::textChanged() {
}

void ProgramTab::textUndoAvailable(bool b) {
	m_undoButton->setEnabled(b);
}

void ProgramTab::textRedoAvailable(bool b) {
	m_redoButton->setEnabled(b);
}

void ProgramTab::undoText() {
	m_textEdit->undo();
}

void ProgramTab::redoText() {
	m_textEdit->redo();
}

QStringList ProgramTab::getSerialPorts() {
	// TODO: make this call a plugin?
#ifdef Q_WS_WIN
	QStringList ports;
	for (int i = 1; i < 256; i++)
	{
		QString port = QString("COM%1").arg(i);
		QString sport = QString("\\\\.\\%1").arg(port);
		HANDLE hPort = ::CreateFileA(sport.toLatin1().constData(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (hPort == INVALID_HANDLE_VALUE) {
			DWORD dwError = GetLastError();
			if (dwError == ERROR_ACCESS_DENIED || dwError == ERROR_GEN_FAILURE || dwError == ERROR_SHARING_VIOLATION || dwError == ERROR_SEM_TIMEOUT) {
				ports.append(port);
			}
		}
		else {
			CloseHandle(hPort);
			ports.append(port);
		}
	}
	return ports;
#endif
#ifdef Q_WS_MAC

	CFDictionarySetValue(classesToMatch,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDRS232Type));

	return ___emptyStringList___;
#endif
#ifdef Q_WS_X11
	QProcess * process = new QProcess(this);
	process->setProcessChannelMode(QProcess::MergedChannels);
	process->setReadChannel(QProcess::StandardOutput);

	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(portProcessFinished(int, QProcess::ExitStatus)));
	connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(portProcessReadyRead()));

        process->start("dmesg");

	return ___emptyStringList___;
#endif

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
