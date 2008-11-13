/*
 * (c) Fachhochschule Potsdam
 */

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
