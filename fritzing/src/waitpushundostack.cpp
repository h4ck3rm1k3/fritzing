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

#include "waitpushundostack.h"
#include "utils/misc.h"
#include "utils/folderutils.h"
#include "commands.h"

#include <QCoreApplication>
#include <QTextStream>

CommandTimer::CommandTimer(QUndoCommand * command, int delayMS, WaitPushUndoStack * undoStack) : QTimer()
{
	m_command = command;
	m_undoStack = undoStack;
	setSingleShot(true);
	setInterval(delayMS);
	connect(this, SIGNAL(timeout()), this, SLOT(timedout()));
	start();
}

void CommandTimer::timedout() {
	m_undoStack->push(m_command);
	m_undoStack->deleteTimer(this);
}

/////////////////////////////////

WaitPushUndoStack::WaitPushUndoStack(QObject * parent) :
	QUndoStack(parent)
{
#ifndef QT_NO_DEBUG
    QString path = FolderUtils::getUserDataStorePath("");
    path += "/undostack.txt";

	m_file.setFileName(path);
	m_file.remove();
#endif
}

WaitPushUndoStack::~WaitPushUndoStack() {
	clearDeadTimers();
}

#ifndef QT_NO_DEBUG
void WaitPushUndoStack::push(QUndoCommand * cmd) 
{
	writeUndo(cmd, 0, NULL);

    if (m_temporary == cmd) {
        m_temporary->redo();
        return;
    }
	
	QUndoStack::push(cmd);
}
#endif

void WaitPushUndoStack::waitPush(QUndoCommand * command, int delayMS) {
	clearDeadTimers();
	new CommandTimer(command, delayMS, this);
}


void WaitPushUndoStack::waitPushTemporary(QUndoCommand * command, int delayMS) {
    m_temporary = command;
	waitPush(command, delayMS);
}

void WaitPushUndoStack::clearDeadTimers() {
	QMutexLocker locker(&m_mutex);
	foreach (QTimer * timer, m_deadTimers) {
		delete timer;
	}
	m_deadTimers.clear();
}


void WaitPushUndoStack::deleteTimer(QTimer * timer) {
	QMutexLocker locker(&m_mutex);
	m_deadTimers.append(timer);
}

void WaitPushUndoStack::resolveTemporary() {
    TemporaryCommand * tc = dynamic_cast<TemporaryCommand *>(m_temporary);
    m_temporary = NULL;
    tc->setEnabled(false);
    push(tc);
    tc->setEnabled(true);
}

void WaitPushUndoStack::deleteTemporary() {
    delete m_temporary;
    m_temporary = NULL;
}

#ifndef QT_NO_DEBUG
void WaitPushUndoStack::writeUndo(const QUndoCommand * cmd, int indent, const BaseCommand * parent) 
{
	const BaseCommand * bcmd = dynamic_cast<const BaseCommand *>(cmd);
	QString cmdString; 
	QString indexString;
	if (bcmd == NULL) {
		cmdString = cmd->text();
	}
	else {
		cmdString = bcmd->getDebugString();
		indexString = QString::number(bcmd->index()) + " ";
	}

   	if (m_file.open(QIODevice::Append | QIODevice::Text)) {
   		QTextStream out(&m_file);
		QString indentString(indent, QChar(' '));	
		if (parent) {
			indentString += QString("(%1) ").arg(parent->index());
		}
		indentString += indexString;
		out << indentString << cmdString << "\n";
		m_file.close();
	}

	for (int i = 0; i < cmd->childCount(); i++) {
		writeUndo(cmd->child(i), indent + 4, NULL);
	}

	if (bcmd) {
		for (int i = 0; i < bcmd->subCommandCount(); i++) {
			writeUndo(bcmd->subCommand(i), indent + 4, bcmd);
		}
	}
}
#endif

