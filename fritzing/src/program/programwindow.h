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


#ifndef PROGRAMWINDOW_H_
#define PROGRAMWINDOW_H_

#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QFrame>
#include <QTextEdit>
#include <QProcess>
#include <QTabWidget>
#include <QComboBox>

#include "../fritzingwindow.h"

class PTabWidget : public QTabWidget 
{
	Q_OBJECT
public:
	PTabWidget(QWidget * parent);
	QTabBar * tabBar();
};

class ProgramWindow : public FritzingWindow
{
Q_OBJECT

public:
	ProgramWindow(QWidget *parent=0);
	~ProgramWindow();

	void setup(const QStringList & programs, const QString & alternativePath);
	const QString defaultSaveFolder();

public:
	static void initText();

signals:
	void closed();
	void changeActivationSignal(bool activate, QWidget * originator);
	void linkToProgramFile(const QString & filename, bool addLink);

protected slots:
	void parentAboutToClose();
	class ProgramTab * addTab();
	void tabSaveAs(int);
	void tabSave(int);
	void tabBeforeClosing(int, bool & ok);
	void tabDelete(int index);
	void tabLinkTo(const QString & filename, bool);

protected:
	void closeEvent(QCloseEvent *event);
	bool eventFilter(QObject *object, QEvent *event);

	QFrame * createHeader();
	QFrame * createCenter();

	const QString untitledFileName();
	const QString fileExtension();

	void cleanUp();
	bool event(QEvent *);
	int &untitledFileCount();
	QStringList getSerialPorts();
	void setTitle();
	bool beforeClosing(bool showCancel=true); // returns true if close, false if cancel
	bool saveAsAux(const QString & fileName);
	bool prepSave(int index, class ProgramTab *, bool saveAsFlag);
	bool beforeClosingTab(int index, bool showCancel);

protected:
	QPointer<PTabWidget> m_tabWidget;
	QPointer<QPushButton> m_addButton;
	QPointer<class ProgramTab> m_savingProgramTab;
};

#endif /* ProgramWindow_H_ */
