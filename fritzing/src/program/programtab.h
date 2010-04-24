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


#ifndef PROGRAMTAB_H_
#define PROGRAMTAB_H_

#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QFrame>
#include <QTextEdit>
#include <QProcess>
#include <QTabWidget>
#include <QComboBox>
#include <QTabWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>

#include "programtab.h"

class ProgramTab : public QFrame 
{
	Q_OBJECT

public:
	ProgramTab(QWidget * parent);
	~ProgramTab();

	bool isModified();
	const QString & filename();
	void setFilename(const QString &);
	QString extension();
	bool readOnly();
	void setClean();
	bool save(const QString & filename);
	bool loadProgramFile(const QString & filename, const QString & altFilename);

protected slots:
	void changeLanguage(const QString &);
	void loadProgramFile();
	void textUndoAvailable(bool b);
	void textRedoAvailable(bool b);
	void textChanged();
	void redoText();
	void undoText();
	void portProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void portProcessReadyRead();
	void deleteTab();
	void save();
	void saveAs();
	void sendProgram();
	void chooseProgrammer();
	void programProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void programProcessReadyRead();

signals:
	void wantToSave(int);
	void wantToSaveAs(int);
	void wantBeforeClosing(int, bool & ok);
	void wantToDelete(int, bool deleteFile);
	void wantToLink(const QString & filename, bool);

protected:
	QFrame * createFooter();
	QStringList getSerialPorts();
	void initLanguages();

protected:
	static QHash<QString, QString> m_languages;
	static QHash<QString, class Syntaxer *> m_syntaxers;

protected:
	QPointer<QPushButton> m_saveAsButton;
	QPointer<QPushButton> m_saveButton;
	QPointer<QPushButton> m_cancelCloseButton;
	QPointer<QPushButton> m_undoButton;
	QPointer<QPushButton> m_redoButton;
	QPointer<QPushButton> m_programButton;
    QPointer<QComboBox> m_portComboBox;
	QPointer<QComboBox> m_languageComboBox;

	QPointer<QTextEdit> m_textEdit;
	QPointer<QTextEdit> m_console;
	QPointer<QTabWidget> m_tabWidget;

	bool m_updateEnabled;

	QPointer<class Highlighter> m_highlighter;
	QString m_filename;
	QString m_programmerPath;

};

class DeleteDialog : public QDialog {
	Q_OBJECT

public:
	DeleteDialog(const QString & title, const QString & text, bool deleteFileCheckBox, QWidget * parent = 0, Qt::WindowFlags f = 0);

	bool deleteFileChecked();

protected slots:
	void buttonClicked(QAbstractButton * button);

protected:
	QDialogButtonBox * m_buttonBox;
	QCheckBox * m_checkBox;

};

#endif /* PROGRAMTAB_H_ */
