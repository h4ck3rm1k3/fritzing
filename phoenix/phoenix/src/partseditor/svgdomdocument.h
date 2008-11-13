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
	QDomElement createGroup(QString id);
	void setHeight(int, QString);
	void setWidth(int, QString);
	void setViewBox(int, int, int, int);
	void save(QString fileName);
	
	
};
#endif
