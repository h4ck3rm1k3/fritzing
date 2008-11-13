/*
 *  aboutbox.h
 *  Fritzing
 *
 *  Created by Dirk van Oosterbosch on 18-10-08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include <QWidget>
#include <QTime>

class QScrollArea;
class QTimer;

class AboutBox : public QWidget {
	Q_OBJECT

private:
	void resetScrollAnimation();
	AboutBox(QWidget *parent = 0);

	QScrollArea *m_scrollArea;
	int m_currentPosition;
	bool m_restartAtTop;
	QTime m_startTime;
	QTimer *m_autoScrollTimer;

public:
	static void hideAbout();
	static void showAbout();
	static void closeAbout(); // Maybe we don't need close as a public method (we only want to hide)

public slots:
	void scrollCredits();

protected:
	static AboutBox* singleton;

	void closeEvent ( QCloseEvent * event );
	void keyPressEvent ( QKeyEvent * event );
};

#endif
