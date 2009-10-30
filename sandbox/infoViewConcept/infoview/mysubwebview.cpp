#include "mysubwebview.h"

MySubWebView::MySubWebView(QWidget *parent) :QWebView(parent)
{
    setHtml("<html><body style='background-color: blue;'>blue = INSIDE PLUGIN</body></html>");
}
