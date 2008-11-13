#ifndef SVGDOMDOCUMENT_H
#define SVGDOMDOCUMENT_H
//
#include <QtXml>
#include <QtGui>
//
class SVGDomDocument : public QDomDocument
{
public:
	SVGDomDocument();
	QDomElement * createGroup(QString id);
	void addToGroup(QString id, QDomElement * element);
	QDomElement * createCircle();
	QDomElement * createRect();
	QDomElement * createLine();
	QDomElement * createText();
	QDomElement * createArc();
	QDomElement * createPath();
	void save(QString fileName);
	
	
};
#endif
