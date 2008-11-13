#ifndef CONSOLE_H
#define CONSOLE_H

#include <QTextEdit>

class Console : public QTextEdit
{
Q_OBJECT
public:
	Console(QWidget * parent = 0);
	
public slots:
	void receiveDebugBroadcast(const QString & message, QObject * ancestor);
};

#endif
