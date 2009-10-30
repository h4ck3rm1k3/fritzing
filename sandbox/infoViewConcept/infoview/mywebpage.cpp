#include "mywebpage.h"

#include <QWebFrame>
#include <QtDebug>

#include "mysubwebview.h"

MyWebPage::MyWebPage(QWidget *parent) :QWebPage(parent)
{
    m_parent = parent;
    QString html =
        "<html><body style='background-color: red;'>"
        "red = TEXT BEFORE THE PLUGIN"
        "<object type='application/x-qt-plugin' classid='MYCLASS'></object>"
        "red = TEXT INSIDE THE PLUGIN"
        "</body></html>";
    settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    mainFrame()->setHtml(html);
}

QObject * MyWebPage::createPlugin(
        const QString &classid, const QUrl &url,
        const QStringList &paramNames, const QStringList &paramValues)
{
       if(classid=="MYCLASS") {
           qDebug() << "loading plugin";
           return new MySubWebView(m_parent);
       } else {
           return QWebPage::createPlugin(classid, url, paramNames, paramValues);
       }
}

