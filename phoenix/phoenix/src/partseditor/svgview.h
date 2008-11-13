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
    QGraphicsView *m_graphicsView;
    QLabel *m_label;
    QToolButton *m_printButton;
    QToolButton *m_loadPCBXMLButton;
	QSvgRenderer *m_renderer;
	QSvgWidget *m_pcbWidget;
	PcbXML	*m_pcbXML;
    
    QGraphicsScene m_scene;
	QDomDocument *m_domDocument; // footprint xml file
	
	// graphics layers (svg groups really)
    
    qreal m_zoom;
    qreal m_rotation;

    void setupMatrix();
    void drawPCBXML(QDomElement * pcbDocument);

};
#endif
