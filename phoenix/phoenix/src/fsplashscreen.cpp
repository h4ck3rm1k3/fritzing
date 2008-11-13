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
