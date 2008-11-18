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
