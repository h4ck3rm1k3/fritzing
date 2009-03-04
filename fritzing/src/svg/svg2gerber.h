#include <QString>
#include <QDomElement>
#include <QObject>
#include <QMatrix>

#ifndef SVG2GERBER_H
#define SVG2GERBER_H

class SVG2gerber
{
public:
    SVG2gerber(QString);
    QString getGerber();

protected:
    QDomDocument m_SVGDom;
    QString m_gerber;

    void normalizeSVG();
    void allPaths2gerber();
    void convertShapes2paths(QDomNode);
    void flattenSVG(QDomNode);
    QMatrix parseTransform(QDomElement);

    QDomElement rect2path(QDomElement);
    QDomElement circle2path(QDomElement);
    QDomElement line2path(QDomElement);
    QDomElement poly2path(QDomElement);
    QDomElement ellipse2path(QDomElement);

    QString path2gerber(QDomElement);

};

#endif // SVG2GERBER_H
