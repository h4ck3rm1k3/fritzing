#ifndef FSPLASHSCREEN_H
#define FSPLASHSCREEN_H

#include <QSplashScreen>
#include <QPainter>

class FSplashScreen : public QSplashScreen {
public:
	FSplashScreen(const QPixmap & pixmap = QPixmap(), Qt::WindowFlags f = 0);
	void setTextPosition(int x, int y);

protected:
	void drawContents ( QPainter * painter );

public slots:
	void showMessage(const QString &message, int alignment = Qt::AlignLeft, const QColor &color = Qt::black);

protected:
    QPixmap m_pixmap;
    QString m_currStatus;
    QColor m_currColor;
    int m_currAlign;
	int m_x;
	int m_y;


};

#endif
