#ifndef MYWEBPAGE_H
#define MYWEBPAGE_H

#include <QWebPage>

class MyWebPage : public QWebPage
{
public:
    MyWebPage(QWidget *parent=0);

protected:
    QObject * createPlugin ( const QString & classid, const QUrl & url, const QStringList & paramNames, const QStringList & paramValues );

    QWidget *m_parent;
};

#endif // MYWEBPAGE_H
