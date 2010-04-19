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


#ifndef PROGRAMMAINWINDOW_H_
#define PROGRAMMAINWINDOW_H_

#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QFrame>
#include <QTextEdit>

#include "../fritzingwindow.h"

class ProgramMainWindow : public FritzingWindow
{
Q_OBJECT

public:
	ProgramMainWindow(QWidget *parent=0);
	~ProgramMainWindow();

	void setup();
	const QDir& tempDir();

public:
	static void initText();

signals:
	void closed();
	void changeActivationSignal(bool activate, QWidget * originator);
	void saveButtonClicked();

public slots:
	bool save();

protected slots:
	bool saveAs();
	void changeLanguage(const QString &);
	void parentAboutToClose();
	void loadProgramFile();

protected:
	bool saveAsAux(const QString & fileName);
	const QDir& createTempFolderIfNecessary();
	void closeEvent(QCloseEvent *event);
	bool eventFilter(QObject *object, QEvent *event);

	void createHeader();
	void createCenter();
	void createFooter();

	const QString untitledFileName();
	const QString fileExtension();
	const QString defaultSaveFolder();

	void updateSaveButton();
	void updateButtons();
	const QString fritzingTitle();

	void cleanUp();
	bool event(QEvent *);
	int &untitledFileCount();

protected:

	QPointer<QPushButton> m_saveAsNewPartButton;
	QPointer<QPushButton> m_saveButton;
	QPointer<QPushButton> m_cancelCloseButton;

	QPointer<QTextEdit> m_textEdit;

	QPointer<QFrame> m_mainFrame;
    QPointer<QFrame> m_headerFrame;
    QPointer<QFrame> m_centerFrame;
    QPointer<QFrame> m_footerFrame;

	bool m_updateEnabled;

	QPointer<class Highlighter> m_highlighter;


protected:
	static QHash<QString, QString> m_languages;
	static QHash<QString, class Syntaxer *> m_syntaxers;
};

#endif /* PROGRAMMAINWINDOW_H_ */
