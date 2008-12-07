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

#ifndef FSPLASHSCREEN_H
#define FSPLASHSCREEN_H

#include <QSplashScreen>
#include <QPainter>

struct MessageThing {
	QString message;
	QRect rect;
	int alignment;
	QColor color;
};

class FSplashScreen : public QSplashScreen {
public:
	FSplashScreen(const QPixmap & pixmap = QPixmap(), Qt::WindowFlags f = 0);
	~FSplashScreen();

protected:
	void drawContents ( QPainter * painter );

public slots:
	void showMessage(const QString &message, QRect rect, int alignment = Qt::AlignLeft, const QColor &color = Qt::black);

protected:
    QPixmap m_pixmap;
	QList<MessageThing *> m_messages;

};

#endif
