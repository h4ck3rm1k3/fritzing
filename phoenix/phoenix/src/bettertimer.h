#ifndef BETTERTIMER_H
#define BETTERTIMER_H

#include <QTimer>

class BetterTimer : public QTimer
{
Q_OBJECT
public:
	BetterTimer(QObject * parent);

protected slots:
	void self_timeout();

signals:
	void betterTimeout(BetterTimer *);
};

#endif
