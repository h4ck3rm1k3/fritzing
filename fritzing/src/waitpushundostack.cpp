/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

WaitPushUndoStack::WaitPushUndoStack(QObject * parent) :
	QUndoStack(parent)
{
}

void WaitPushUndoStack::waitPush(QUndoCommand * command, int delayMS) {
	QMutexLocker locker(&m_mutex);
	foreach (QTimer * timer, m_deadTimers) {
		delete timer;
	}
	m_deadTimers.clear();

	new CommandTimer(command, delayMS, this);
}

void WaitPushUndoStack::deleteTimer(QTimer * timer) {
	QMutexLocker locker(&m_mutex);
	m_deadTimers.append(timer);
}
