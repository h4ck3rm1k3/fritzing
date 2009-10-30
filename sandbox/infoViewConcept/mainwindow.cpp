#include <QWebView>
#include "mainwindow.h"
#include "infoview/mywebpage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWebView *view = new QWebView(this);
    view->setPage(new MyWebPage(view));
    setCentralWidget(view);
}

MainWindow::~MainWindow()
{

}
