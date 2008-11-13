#ifndef SVGVIEW_H
#define SVGVIEW_H
//
#include <QFrame>
#include <qmath.h>
#include <QDomDocument>
#include <QSvgRenderer>
#include <QSvgWidget>
#include <QGraphicsScene>

#include "pcbxml.h"

QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QToolButton)

class SVGView : public QFrame
{
Q_OBJECT
public:
	SVGView(const QString &name, QWidget *parent = 0);
	QGraphicsView *view() const;

private slots:
    void print();
	void importPCBXML();
    void zoomIn();
    void zoomOut();
    void rotateLeft();
    void rotateRight();

private:
    QGraphicsView *graphicsView;
    QLabel *label;
    QToolButton *printButton;
    QToolButton *loadPCBXMLButton;
	QSvgRenderer *renderer;
	QSvgWidget *pcbWidget;
	PcbXML	*pcbXML;
    
    QGraphicsScene scene;
	QDomDocument *domDocument; // footprint xml file
	
	// graphics layers (svg groups really)
    
    qreal zoom;
    qreal rotation;

    void setupMatrix();
    void drawPCBXML(QDomElement * pcbDocument);

};
#endif
