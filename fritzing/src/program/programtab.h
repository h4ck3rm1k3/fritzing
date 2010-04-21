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

#include "programtab.h"

class ProgramTab : public QFrame 
{
	Q_OBJECT

public:
	ProgramTab(QWidget * parent);
	~ProgramTab();

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

protected:
	bool saveAs();
	bool saveAsAux(const QString & fileName);
	QFrame * createFooter();
	void updateSaveButton();
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
    QPointer<QComboBox> m_portComboBox;
	QPointer<QComboBox> m_languageComboBox;

	QPointer<QTextEdit> m_textEdit;
	QPointer<QTabWidget> m_tabWidget;

	bool m_updateEnabled;

	QPointer<class Highlighter> m_highlighter;
	QString m_filename;

};

#endif /* PROGRAMMAINWINDOW_H_ */
