


#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>
#include <QEvent>
#include <QTextEdit>
#include <QFile>

class DebugDialog : public QDialog
{
	Q_OBJECT

private:
	DebugDialog(QWidget *parent = 0);
	~DebugDialog();

public:
	static void debug(QString, QObject * ancestor = 0);
	static void hideDebug();
	static void showDebug();
	static void closeDebug();
	static bool visible();
	static bool connectToBroadcast(QObject * receiver, const char* slot); 

protected:
	bool event ( QEvent * e ); 
	void resizeEvent ( QResizeEvent * event );

protected:
	static DebugDialog* singleton;
	static QFile m_file;
	
	QTextEdit* m_textEdit;
	
signals:
	void debugBroadcast(const QString & message, QObject * ancestor);
};

#endif 
