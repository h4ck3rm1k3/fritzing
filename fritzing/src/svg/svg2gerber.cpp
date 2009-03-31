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
#include "svgflattener.h"
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

    renderGerber();
}

QString SVG2gerber::getGerber(){
    return m_gerber_header + m_gerber_paths;
}

void SVG2gerber::renderGerber(){
    // initialize axes
    m_gerber_header = "%ASAXBY*%\n";

    // NOTE: this currently forces a 1 mil grid
    // format coordinates to drop leading zeros with 2,3 digits
    m_gerber_header += "%FSLAX23Y23*%\n";

    // set units to inches
    m_gerber_header += "%MOIN*%\n";

    // no offset
    m_gerber_header += "%OFA0B0*%\n";

    // scale factor 1x1
    m_gerber_header += "%SFA1.0B1.0*%\n";

    // define apertures and draw em
    allPaths2gerber();

    // label our layer
    m_gerber_header += "%LNFRITZING*%\n";

    //just to be safe: G90 (absolute coords) and G70 (inches)
    m_gerber_header += "G90*\nG70*\n";


    // now write the footer
    // comment to indicate end-of-sketch
    m_gerber_paths += "G04 End of Fritzing sketch*\n";

    // write gerber end-of-program
    m_gerber_paths += "M02*";
}

void SVG2gerber::normalizeSVG(){
    QDomElement root = m_SVGDom.documentElement();

    //  convert to paths
    convertShapes2paths(root);

    // dump paths SVG to tmp file for now
//    QFile dump("/tmp/paths_pre.svg");
//    if (!dump.open(QIODevice::WriteOnly | QIODevice::Text))
//        DebugDialog::debug("gerber svg dump: cannot open output file");
//
//    QTextStream out(&dump);
//    out << m_SVGDom.toString();

    //  get rid of transforms
    SvgFlattener flattener;
    flattener.flattenChildren(root);

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
            //path = rect2path(element);
            path = element;
        }
        else if(tag=="circle"){
            //path = circle2path(element);
            path = element;
        }
        else if(tag=="line"){
            path = element;
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

        copyStyles(element, path);

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

void SVG2gerber::copyStyles(QDomElement source, QDomElement dest){
    QList<QString> attrList;
    attrList << "stroke" << "fill" << "stroke-width" << "style";

    for (int i = 0; i < attrList.size(); ++i) {
        if (source.hasAttribute(attrList.at(i)))
            dest.setAttribute(attrList.at(i), source.attribute(attrList.at(i)));
    }
}

QMatrix SVG2gerber::parseTransform(QDomElement element){
    QMatrix transform = QMatrix();

    QString svgTransform = element.attribute("transform");

    return transform;
}

void SVG2gerber::allPaths2gerber() {
    QHash<QString, QString> apertureMap;
    int dcode_index = 10;

    // iterates through all circles, rects, lines and paths
    //  1. check if we already have an aperture
    //      if aperture does not exist, add it to the header
    //  2. switch to this aperture
    //  3. draw it at the correct path/location

    // circles
    QDomNodeList circleList = m_SVGDom.elementsByTagName("circle");

    DebugDialog::debug("circles to gerber: " + QString::number(circleList.length()));
    for(uint i = 0; i < circleList.length(); i++){
        QDomElement circle = circleList.item(i).toElement();
        QString aperture;

        QString cx = circle.attribute("cx");
        QString cy = circle.attribute("cy");
        qreal r = circle.attribute("r").toFloat();
        QString fill = circle.attribute("fill");
        qreal stroke_width = circle.attribute("stroke-width").toFloat();

        qreal diam = ((2*r) + stroke_width)/1000;
        qreal hole = ((2*r) - stroke_width)/1000;

        if(fill=="none")
            aperture = QString("C,%1X%2").arg(diam).arg(hole);
        else
            aperture = QString("C,%1").arg(diam);

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = "D" + QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            dcode_index++;
        }

        QString dcode = apertureMap[aperture];

        //switch to correct aperture
        m_gerber_paths += "G54" + dcode + "*\n";
        //flash
        m_gerber_paths += "X" + cx + "Y" + cy + "D03*\n";


    }

    // rects

    // lines

    // paths - NOTE: this assumes circular aperture

}

QDomElement SVG2gerber::rect2path(QDomElement rectElement){
    float x = rectElement.attribute("x").toFloat();
    float y = rectElement.attribute("y").toFloat();
    float width = rectElement.attribute("width").toFloat();
    float height = rectElement.attribute("height").toFloat();

    QString pathStr = "M " + QString::number(x) + "," + QString::number(y) + " ";
    pathStr += "L " + QString::number(x + width) + "," + QString::number(y);
    pathStr += "L " + QString::number(x + width) + "," + QString::number(y + height);
    pathStr += "L " + QString::number(x) + "," + QString::number(y + height);
    pathStr += "z";

    QDomElement path = m_SVGDom.createElement("path");
    path.setAttribute("d",pathStr);

    return path;
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
    QString pathStr = "M " + QString::number(cx) + "," + QString::number(cy + r) + " ";

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
    float x1 = lineElement.attribute("x1").toFloat();
    float y1 = lineElement.attribute("y1").toFloat();
    float x2 = lineElement.attribute("x2").toFloat();
    float y2 = lineElement.attribute("y2").toFloat();

    QString pathStr = "M " + QString::number(x1) + "," + QString::number(y1) + " ";
    pathStr += "L " + QString::number(x2) + "," + QString::number(y2) + " z";

    QDomElement path = m_SVGDom.createElement("path");
    path.setAttribute("d",pathStr);

    return path;
}

QDomElement SVG2gerber::poly2path(QDomElement polyElement){
    // TODO
    return polyElement;
}

QDomElement SVG2gerber::ellipse2path(QDomElement ellipseElement){
    // TODO
    return ellipseElement;
}

QString SVG2gerber::path2gerber(QDomElement pathElement){
    QString d;

    return d;
}
