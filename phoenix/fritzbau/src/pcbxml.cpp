#include "pcbxml.h"
#include <QSize>
#include <QPainter>
#include <QtDebug>
//
PcbXML::PcbXML( QDomElement * pcbDocument ) 
{
	svg = new SVGDomDocument();
    
    //painter = new QPainter(generator);
    //painter->fillRect(QRectF(0, 0, 30, 30), QColor(255,0,0,255));    // draw a red rectangle  
	QDomNodeList tagList = pcbDocument->childNodes();
	
	//TODO: eventually may need to support recursing into the tree here
	for(uint i = 0; i < tagList.length(); i++){
		drawNode(tagList.item(i));
	}

	svgFile = "/tmp/test.svg";
	svg->save(svgFile);
}

QString PcbXML::getSvgFile(){
	return svgFile;
}

void PcbXML::drawNode(QDomNode node){
	qDebug("drawing node:");

	QString tag = node.nodeName().toLower();

	if(tag=="pin"){
		qDebug("\tPin");
		drawPin(node);
	}
	else if(tag=="pad"){
		qDebug("\tPad");
	}
	else if(tag=="elementline"){
		qDebug("\tElementLine");
	}
	else if(tag=="elementarc"){
		qDebug("\tElementArc");
	}
	else if(tag=="mark"){
		qDebug("\tMark");
	}
	else {
		qDebug("cannot draw - unrecognized tag");
	}
}

void PcbXML::drawPin(QDomNode node){
	QDomElement element = node.toElement();
	
	int ax = element.attribute("aX").toInt();	
	int ay = element.attribute("aY").toInt();
	int radius = element.attribute("Drill").toInt()/2;	
	int thickness = element.attribute("Thickness").toInt();
	
}
