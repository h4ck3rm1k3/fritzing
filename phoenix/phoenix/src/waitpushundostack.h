/*
 * (c) Fachhochschule Potsdam
 */

#ifndef WAITPUSHUNDOSTACK_H
#define WAITPUSHUNDOSTACK_H

#include <QUndoStack>
#include <QTimer>
#include <QMutex>


class WaitPushUndoStack : public QUndoStack
{
public:
	WaitPushUndoStack(QObject * parent = 0);
	void waitPush(QUndoCommand *, int delayMS);
	void deleteTimer(QTimer *);

protected:
	QList<QTimer *> m_deadTimers;
	QMutex m_mutex;
};


class CommandTimer : public QTimer
{

Q_OBJECT

public:
	CommandTimer(QUndoCommand * command, int delayMS, WaitPushUndoStack * undoStack);

protected slots:
	void timedout();

protected:
	QUndoCommand * m_command;
	WaitPushUndoStack * m_undoStack;
};


#endif
