#ifndef PCBXML_H
#define PCBXML_H
//
#import <QDomElement>
#import <QSvgGenerator>
#include "svgdomdocument.h"
//
class PcbXML  
{

public:
	PcbXML(QDomElement * pcbDocument);
	
	QString getSvgFile();
	
private:
	SVGDomDocument * svg;
	QString svgFile;
	
	void drawNode(QDomNode node);
	void drawPin(QDomNode node);
};
#endif
