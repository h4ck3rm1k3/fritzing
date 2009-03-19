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

#include "svg2gerber.h"
#include "../debugdialog.h"
#include <QTextStream>
#include <math.h>

//TODO: this assumes one layer right now (copper0)

SVG2gerber::SVG2gerber(QString svgStr)
{
    m_SVGDom = QDomDocument("svg");
    m_SVGDom.setContent(svgStr);

#ifndef QT_NO_DEBUG
    // dump paths SVG to tmp file for now
    QFile dump("/tmp/paths_in.svg");
    if (!dump.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber svg dump: cannot open output file");

    QTextStream out(&dump);
    out << m_SVGDom.toString();
#endif

    normalizeSVG();

#ifndef QT_NO_DEBUG
    // dump paths SVG to tmp file for now
    QFile dump2("/tmp/paths_normal.svg");
    if (!dump2.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber svg dump: cannot open output file");

    QTextStream out2(&dump2);
    out2 << m_SVGDom.toString();
#endif

    allPaths2gerber();
}

QString SVG2gerber::getGerber(){
    return m_gerber;
}

void SVG2gerber::normalizeSVG(){
    QDomElement root = m_SVGDom.documentElement();

    //  convert to paths
    convertShapes2paths(root);

    //  get rid of transforms
    flattenSVG(root);

}

void SVG2gerber::convertShapes2paths(QDomNode node){
    // I'm a leaf node.  make me a path
    if(!node.hasChildNodes()) {
        QString tag = node.nodeName().toLower();
        QDomElement element = node.toElement();
        QDomElement path;

        DebugDialog::debug("converting child to path: " + tag);

        if(tag=="polygon"){
            path = poly2path(element);
        }
        else if(tag=="rect"){
            path = rect2path(element);
        }
        else if(tag=="circle"){
            path = circle2path(element);
        }
        else if(tag=="line"){
            path = line2path(element);
        }
        else if(tag=="ellipse"){
            path = ellipse2path(element);
        }
        else if(tag=="path"){
            path = element;
        }
        else {
            DebugDialog::debug("svg2gerber ignoring SVG element: " + tag);
        }

        // add the path and delete the primitive element (is this ok for paths?)
        QDomNode parent = node.parentNode();
        parent.replaceChild(path, node);

        return;
    }

    // recurse the children
    QDomNodeList tagList = node.childNodes();

    DebugDialog::debug("child nodes: " + QString::number(tagList.length()));
    for(uint i = 0; i < tagList.length(); i++){
        convertShapes2paths(tagList.item(i));
    }
}

// note that this only works for paths!  convert to paths first.
void SVG2gerber::flattenSVG(QDomNode node){
    QDomElement element = node.toElement();
    QMatrix transform;

    // I'm a leaf node. flatten me
    if(!node.hasChildNodes()) {

    }

    // recurse the children
    QDomNodeList tagList = node.childNodes();

    for(uint i = 0; i < tagList.length(); i++){
        flattenSVG(tagList.item(i));

        // now apply my transform to the children

    }

    // if I'm a <g>, apply my transform to children, pull them up then delete me
}

// extract the SVG transform
QMatrix SVG2gerber::parseTransform(QDomElement element){
    QMatrix transform = QMatrix();

    return transform;
}

void SVG2gerber::allPaths2gerber() {

}

QDomElement SVG2gerber::rect2path(QDomElement rectElement){
    // 4 x line2path()
    return rectElement;
}

QDomElement SVG2gerber::circle2path(QDomElement circleElement){
    // 4 cubic bezier arcs
    float cx = circleElement.attribute("cx").toFloat();
    float cy = circleElement.attribute("cy").toFloat();
    float r = circleElement.attribute("r").toFloat();

    // approximate midpoint
    float k = r * (sqrt(2.0f) -1) * 4/3;

//    d="m 0,1                      // translate(radius) from center
//    C 0.552,1   1,0.552   1,0    // 1st quarter
//      1,-0.552  0.552,-1  0,-1   // 2nd
//      -0.552,-1 -1,-0.552 -1,0   // 3rd
//      -1,0.552  -0.552,1  0,1z"  // 4th

    //translate radius from center
    QString pathStr = "m " + QString::number(cx) + "," + QString::number(cy + r) + " ";

    //1st quarter
    pathStr += "C " + QString::number(cx + k) + "," + QString::number(cy + r) + " ";
    pathStr += QString::number(cx + r) + "," + QString::number(cy + k) + " ";
    pathStr += QString::number(cx + r) + "," + QString::number(cy) + " ";

    //2nd quarter
    pathStr += QString::number(cx + r) + "," + QString::number(cy - k) + " ";
    pathStr += QString::number(cx + k) + "," + QString::number(cy - r) + " ";
    pathStr += QString::number(cx) + "," + QString::number(cy - r) + " ";

    //3rd quarter
    pathStr += QString::number(cx - k) + "," + QString::number(cy - r) + " ";
    pathStr += QString::number(cx - r) + "," + QString::number(cy - k) + " ";
    pathStr += QString::number(cx - r) + "," + QString::number(cy) + " ";

    //4th quarter
    pathStr += QString::number(cx - r) + "," + QString::number(cy + k) + " ";
    pathStr += QString::number(cx - k) + "," + QString::number(cy + r) + " ";
    pathStr += QString::number(cx) + "," + QString::number(cy + r) + "z";


    QDomElement path = m_SVGDom.createElement("path");
    path.setAttribute("d",pathStr);

    return path;
}

QDomElement SVG2gerber::line2path(QDomElement lineElement){
    return lineElement;
}

QDomElement SVG2gerber::poly2path(QDomElement polyElement){
    // this is just a bunch of calls to line2path()
    return polyElement;
}

QDomElement SVG2gerber::ellipse2path(QDomElement ellipseElement){
    return ellipseElement;
}

QString SVG2gerber::path2gerber(QDomElement pathElement){
    return QString("NOT IMPLEMENTED YET");
}
