#ifndef EVENTEATER_H
#define EVENTEATER_H

#include <QObject>
#include <QEvent>
#include <QWidget>
#include <QList>

class EventEater : public QObject
{
Q_OBJECT

public:
	EventEater(QObject * parent = 0);

	void allowEventsIn(QWidget *);

 protected:
     bool eventFilter(QObject *obj, QEvent *event);

protected:
	QList<QWidget *> m_allowedWidgets;
};

#endif