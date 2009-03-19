/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

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
