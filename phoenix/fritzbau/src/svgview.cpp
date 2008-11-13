#include "svgview.h"
#include <qmath.h>
#include <QtGui>
//
SVGView::SVGView(const QString &name, QWidget *parent) 
	: QFrame(parent)
{
	setFrameStyle(Sunken | StyledPanel);
    graphicsView = new QGraphicsView;
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);
    
    domDocument = new QDomDocument;

	zoom = 1;
	rotation = 0;

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);

    QToolButton *rotateLeftIcon = new QToolButton;
    rotateLeftIcon->setIcon(QPixmap(":/rotateleft.png"));
    rotateLeftIcon->setIconSize(iconSize);
    QToolButton *rotateRightIcon = new QToolButton;
    rotateRightIcon->setIcon(QPixmap(":/rotateright.png"));
    rotateRightIcon->setIconSize(iconSize);

    // Label layout
    QHBoxLayout *labelLayout = new QHBoxLayout;
    label = new QLabel(name);

	//TODO: put proper icons in here
    printButton = new QToolButton;
    printButton->setIcon(QIcon(QPixmap(":/images/document-print.png")));
    printButton->setText(tr("Print"));
    printButton->setToolTip(tr("Print"));

	loadPCBXMLButton = new QToolButton;
    loadPCBXMLButton->setIcon(QIcon(QPixmap(":/images/applications-accessories.png")));
	loadPCBXMLButton->setText(tr("Import XML"));
	loadPCBXMLButton->setToolTip(tr("Import XML"));

    labelLayout->addWidget(label);
    labelLayout->addStretch();
    labelLayout->addWidget(printButton);
    labelLayout->addWidget(loadPCBXMLButton);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addLayout(labelLayout, 0, 0);
    topLayout->addWidget(graphicsView, 1, 0);
    setLayout(topLayout);

    connect(rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(printButton, SIGNAL(clicked()), this, SLOT(print()));
    connect(loadPCBXMLButton, SIGNAL(clicked()), this, SLOT(importPCBXML()));

    setupMatrix();
}
//

void SVGView::importPCBXML(){
	QString path = QFileDialog::getOpenFileName(this,
         tr("Select Footprint XML File"), "",
         tr("Fritzing Footprint XML Files (*.fzp);;All Files (*)"));
	QFile file(path);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
    	QMessageBox::warning(NULL, QObject::tr("FritzBau"),
                     QObject::tr("Cannot read file %1:\n%2.")
                     .arg(path)
                     .arg(file.errorString()));
    }
    
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument->setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
        QMessageBox::critical(NULL, QObject::tr("FritzBau"),
                                 QObject::tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return;
    }
    
    QDomElement root = domDocument->documentElement();
   	if (root.isNull()) {
        QMessageBox::critical(NULL, QObject::tr("FritzBau"), QObject::tr("The file is not a Fritzing file (2)."));
   		return;
	}
	
    if (root.tagName().toLower() != "element") {
        QMessageBox::critical(NULL, QObject::tr("FritzBau"), QObject::tr("The file is not a Fritzing Footprint XML file."));
        return;
    }
    
    drawPCBXML(&root);
}

void SVGView::drawPCBXML(QDomElement * rootElement) {
	pcbXML = new PcbXML(rootElement);
	pcbWidget = new QSvgWidget(pcbXML->getSvgFile());
	scene.addWidget(pcbWidget);

	graphicsView->setScene(&scene);
	graphicsView->show();

    //connect(pcbWidget, SIGNAL(repaintNeeded()), this, SLOT(pcbWidget->update()));
}

void SVGView::setupMatrix(){
	// TODO: add support for scaling and rotation
    QMatrix matrix;
    matrix.scale(qreal(1), qreal(1));
    matrix.rotate(rotation);

    graphicsView->setMatrix(matrix);
}

QGraphicsView *SVGView::view() const
{
    return graphicsView;
}

void SVGView::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        graphicsView->render(&painter);
    }
#endif
}

void SVGView::zoomIn()
{
	zoom++;
    //zoomSlider->setValue(zoomSlider->value() + 1);
}

void SVGView::zoomOut()
{
	zoom--;
    //zoomSlider->setValue(zoomSlider->value() - 1);
}

void SVGView::rotateRight(){
	
}

void SVGView::rotateLeft(){
	
}
