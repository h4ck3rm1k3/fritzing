#include "mainwindowimpl.h"

#include <QtGui>
#include "svgview.h"
//
MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f) 
	: QMainWindow(parent, f)
{
	setupUi(this);
	setupGraphics();
	setWindowTitle(tr("FritzBau"));
	
	hSplitter = new QSplitter;
	
	SVGView *svgview = new SVGView("part view");
    svgview->view()->setScene(scene);
	
    setCentralWidget(svgview);
    statusBar()->showMessage(tr(" "), 2000);
}

void MainWindowImpl::setupGraphics(){
	return;
}
