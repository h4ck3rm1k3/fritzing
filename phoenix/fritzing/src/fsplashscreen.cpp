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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#include "fsplashscreen.h"

#include <QTextDocument>
#include <QTextCursor>

FSplashScreen::FSplashScreen(const QPixmap & pixmap, Qt::WindowFlags f ) : QSplashScreen(pixmap, f)
{
	m_x = m_y = 0;
}

void FSplashScreen::setTextPosition(int x, int y) {
	m_x = x;
	m_y = y;
}

void FSplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    m_currStatus = message;
    m_currAlign = alignment;
    m_currColor = color;
    emit messageChanged(m_currStatus);
    repaint();
}


void FSplashScreen::drawContents ( QPainter * painter )
{
	// copied from QSplashScreen::drawContents

    QRect r = rect();
    r.setRect(r.x() + m_x, r.y() + m_y, r.width() - m_x, r.height() - m_y);
    if (Qt::mightBeRichText(m_currStatus)) {
        QTextDocument doc;
#ifdef QT_NO_TEXTHTMLPARSER
        doc.setPlainText(m_currStatus);
#else
        doc.setHtml(m_currStatus);
#endif
        doc.setTextWidth(r.width());
        QTextCursor cursor(&doc);
        cursor.select(QTextCursor::Document);
        QTextBlockFormat fmt;
        fmt.setAlignment(Qt::Alignment(m_currAlign));
        cursor.mergeBlockFormat(fmt);
        painter->save();
        painter->translate(r.topLeft());
        doc.drawContents(painter);
        painter->restore();
    } else {
        painter->drawText(r, m_currAlign, m_currStatus);
    }
}
