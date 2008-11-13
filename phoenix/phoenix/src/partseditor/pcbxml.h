#ifndef PCBXML_H
#define PCBXML_H
//
#include <QDomElement>
#include <QSvgGenerator>
#include "svgdomdocument.h"
//
class PcbXML  
{

public:
	PcbXML(const QDomElement & pcbDocument);
	
	QString getSvgFile();
	
private:
	SVGDomDocument * m_svg;
	QString m_svgFile;
	QDomElement m_svgroot;
	QDomElement m_silkscreen;
	QDomElement m_copper;
	QDomElement m_keepout;
	QDomElement m_mask;
	QDomElement m_outline;
	int m_markx;
	int m_marky;
	int m_minx;
	int m_miny;
	int m_maxx;
	int m_maxy;
	int m_pinCount;
	int m_padCount;
	QString m_units; // length units for the root element coordinates
	
	void drawNode(QDomNode node);
	void drawPin(QDomNode node);
	void drawPad(QDomNode node);
	void drawElementLine(QDomNode node);
	void drawElementArc(QDomNode node);
	void drawMark(QDomNode node);
	void minMax(int, int, int);
	void shiftCoordinates();
};
#endif
