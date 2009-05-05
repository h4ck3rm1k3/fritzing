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

#ifdef Q_WS_WIN
#define round(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))
#endif

//TODO: this assumes one layer right now (copper0)

SVG2gerber::SVG2gerber(QString svgStr, QString debugStr)
{
    m_SVGDom = QDomDocument("svg");
    m_SVGDom.setContent(svgStr);

#ifndef QT_NO_DEBUG
    // dump paths SVG to tmp file for now
    QFile dump("/tmp/paths_in" + debugStr + ".svg");
    if (!dump.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber svg dump: cannot open output file");

    QTextStream out(&dump);
    out << m_SVGDom.toString();
#endif

    normalizeSVG();

#ifndef QT_NO_DEBUG
    // dump paths SVG to tmp file for now
    QFile dump2("/tmp/paths_normal" + debugStr + ".svg");
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

QString SVG2gerber::getSolderMask(){
    return m_soldermask_header + m_soldermask_paths;
}

QString SVG2gerber::getNCDrill(){
    return m_drill_header + m_drill_paths;
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

    // clone it for the mask header
    m_soldermask_header = m_gerber_header;

    // set inverse polarity
    m_soldermask_header += "%IPNEG*%\n";

    // setup drill file header
    m_drill_header = "M48\n";
    // set to english (inches) units, with trailing zeros
    m_drill_header += "M72,TZ\n";

    // define apertures and draw em
    allPaths2gerber();

    // label our layers
    m_gerber_header += "%LNFRITZING*%\n";
    m_soldermask_header += "%LNMASK*%\n";

    // rewind drill to start position
    m_drill_header += "%\n";

    //just to be safe: G90 (absolute coords) and G70 (inches)
    m_gerber_header += "G90*\nG70*\n";
    m_soldermask_header += "G90*\nG70*\n";

    // now write the footer
    // comment to indicate end-of-sketch
    m_gerber_paths += "G04 End of Fritzing sketch*\n";
    m_soldermask_paths += "G04 End of Fritzing solder mask*\n";

    // write gerber end-of-program
    m_gerber_paths += "M02*";
    m_soldermask_paths += "M02*";

    // drill file unload tool and end of program
    m_drill_paths += "T00\n";
    m_drill_paths += "M30\n";
}

void SVG2gerber::normalizeSVG(){
    QDomElement root = m_SVGDom.documentElement();

    //  convert to paths
    convertShapes2paths(root);

    //  get rid of transforms
    SvgFlattener flattener;
    flattener.flattenChildren(root);

}

void SVG2gerber::convertShapes2paths(QDomNode node){
    // I'm a leaf node.  make me a path
    //TODO: this should strip svg: namspaces
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
        else if((tag=="path") || (tag=="svg:path")){
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
    QString current_dcode;
    int dcode_index = 10;
    bool light_on = false;
    int currentx = -1;
    int currenty = -1;

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
        QString mask_aperture;
        QString drill_aperture;

        qreal centerx = circle.attribute("cx").toFloat();
        qreal centery = circle.attribute("cy").toFloat();
        QString cx = QString::number(round(centerx));
        QString cy = QString::number(round(centery));
        QString drill_cx = QString::number(round(centerx)*10);
        QString drill_cy = QString::number(round(centery)*10);
        qreal r = circle.attribute("r").toFloat();
        QString fill = circle.attribute("fill");
        qreal stroke_width = circle.attribute("stroke-width").toFloat();

        qreal diam = ((2*r) + stroke_width)/1000;
        qreal hole = ((2*r) - stroke_width)/1000;
        //NOTE: assuming 3 mil soldermask clearance
        qreal mask_diam = diam + 0.006;

        if(fill=="none"){
            aperture = QString("C,%1X%2").arg(diam).arg(hole);
            drill_aperture = QString("C%1").arg(hole);
        }
        else
            aperture = QString("C,%1").arg(diam);

        mask_aperture = QString("C,%1").arg(mask_diam);

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            m_soldermask_header += "%ADD" + QString::number(dcode_index) + mask_aperture + "*%\n";
            if(drill_aperture != "")
                m_drill_header += "T" + QString::number(dcode_index) + drill_aperture + "\n";
            dcode_index++;
        }

        QString dcode = apertureMap[aperture];
        if(current_dcode != dcode){
            //switch to correct aperture
            m_gerber_paths += "G54D" + dcode + "*\n";
            m_soldermask_paths += "G54D" + dcode + "*\n";
            if(drill_aperture != "")
                m_drill_paths += "T" + dcode + "\n";
            current_dcode = dcode;
        }
        //flash
        m_gerber_paths += "X" + cx + "Y" + cy + "D03*\n";
        m_soldermask_paths += "X" + cx + "Y" + cy + "D03*\n";
        if(drill_aperture != "")
            m_drill_paths += "X" + drill_cx + "Y" + drill_cy + "\n";
    }

    // rects
    QDomNodeList rectList = m_SVGDom.elementsByTagName("rect");

    DebugDialog::debug("rects to gerber: " + QString::number(rectList.length()));
    for(uint j = 0; j < rectList.length(); j++){
        QDomElement rect = rectList.item(j).toElement();
        QString aperture;
        QString mask_aperture;

        qreal width = rect.attribute("width").toFloat();
        qreal height = rect.attribute("height").toFloat();
        qreal centerx = rect.attribute("x").toFloat() + (width/2);
        qreal centery = rect.attribute("y").toFloat() + (height/2);
        QString cx = QString::number(round(centerx));
        QString cy = QString::number(round(centery));
        QString fill = rect.attribute("fill");
        qreal stroke_width = rect.attribute("stroke-width").toFloat();

        qreal totalx = (width + stroke_width)/1000;
        qreal totaly = (height + stroke_width)/1000;
        qreal holex = (width - stroke_width)/1000;
        qreal holey = (height - stroke_width)/1000;

        //NOTE: assumes 3 mil mask clearance
        qreal mask_totalx = totalx + 0.006;
        qreal mask_totaly = totaly + 0.006;

        if(fill=="none")
            aperture = QString("R,%1X%2X%3X%4").arg(totalx).arg(totaly).arg(holex).arg(holey);
        else
            aperture = QString("R,%1X%2").arg(totalx).arg(totaly);

        mask_aperture = QString("R,%1X%2").arg(mask_totalx).arg(mask_totaly);

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = "D" + QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            m_soldermask_header += "%ADD" + QString::number(dcode_index) + mask_aperture + "*%\n";
            dcode_index++;
        }

        QString dcode = apertureMap[aperture];
        if(current_dcode != dcode){
            //switch to correct aperture
            m_gerber_paths += "G54" + dcode + "*\n";
            m_soldermask_paths += "G54" + dcode + "*\n";
            current_dcode = dcode;
        }
        //flash
        m_gerber_paths += "X" + cx + "Y" + cy + "D03*\n";
        m_soldermask_paths += "X" + cx + "Y" + cy + "D03*\n";
    }

    // lines - NOTE: this assumes a circular aperture
    QDomNodeList lineList = m_SVGDom.elementsByTagName("line");

    DebugDialog::debug("lines to gerber: " + QString::number(lineList.length()));
    for(uint k = 0; k < lineList.length(); k++){
        QDomElement line = lineList.item(k).toElement();
        QString aperture;

        int x1 = round(line.attribute("x1").toFloat());
        int y1 = round(line.attribute("y1").toFloat());
        int x2 = round(line.attribute("x2").toFloat());
        int y2 = round(line.attribute("y2").toFloat());
        qreal stroke_width = line.attribute("stroke-width").toFloat();

        aperture = QString("C,%1").arg(stroke_width/1000);

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = "D" + QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            dcode_index++;
        }

        // turn off light if we are not continuing along a path
        if ((y1 != currenty) || (x1 != currentx)) {
            if (light_on) {
                m_gerber_paths += "D02*\n";
                light_on = false;
            }
        }

        QString dcode = apertureMap[aperture];
        if(current_dcode != dcode){
            //switch to correct aperture
            m_gerber_paths += "G54" + dcode + "*\n";
            current_dcode = dcode;
        }
        //go to start - light off
        m_gerber_paths += "X" + QString::number(x1) + "Y" + QString::number(y1) + "D02*\n";
        //go to end point - light on
        m_gerber_paths += "X" + QString::number(x2) + "Y" + QString::number(y2) + "D01*\n";
        light_on = true;
        currentx = x2;
        currenty = y2;
    }

    // paths - NOTE: this assumes circular aperture and does not fill!
    QDomNodeList pathList = m_SVGDom.elementsByTagName("path");

    DebugDialog::debug("paths to gerber: " + QString::number(pathList.length()));
    for(uint n = 0; n < pathList.length(); n++){
        QDomElement path = pathList.item(n).toElement();
        QString data = path.attribute("d");
        QString aperture;

        const char * slot = SLOT(path2gerbCommandSlot(QChar, bool, QList<double> &, void *));

        PathUserData pathUserData;
        pathUserData.x = 0;
        pathUserData.y = 0;
        pathUserData.string = "";

        SvgFlattener flattener;
        flattener.parsePath(data, slot, pathUserData, this);

        // only add paths if they contained gerber-izable path commands (NO CURVES!)
        if(pathUserData.string.contains("INVALID"))
            continue;

        qreal stroke_width = path.attribute("stroke-width").toFloat();

        aperture = QString("C,%1").arg(stroke_width/1000);

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = "D" + QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            dcode_index++;
        }

        m_gerber_paths += pathUserData.string;
    }
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

void SVG2gerber::path2gerbCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {
    QString gerb;
    int x, y;

    PathUserData * pathUserData = (PathUserData *) userData;

    switch(command.toAscii()) {
                case 'm':
                case 'M':
                    x = args[0];
                    y = args[1];
                    if (relative) {
                        x += pathUserData->x;
                        y += pathUserData->y;
                    }
                    gerb = "X" + QString::number(x) + "Y" + QString::number(y) + "D02*\n";
                    pathUserData->x = x;
                    pathUserData->y = y;
                    m_pathstart_x = x;
                    m_pathstart_y = y;
                    pathUserData->string.append(gerb);
                    break;
                case 'v':
                case 'V':
                    y = args[0];
                    if (relative) {
                        y += pathUserData->y;
                    }
                    gerb = "Y" + QString::number(y) + "D01*\n";
                    pathUserData->y = y;
                    pathUserData->string.append(gerb);
                    break;
                case 'h':
                case 'H':
                    x = args[0];
                    if (relative) {
                        x += pathUserData->x;
                    }
                    gerb = "X" + QString::number(x) + "D01*\n";
                    pathUserData->x = x;
                    pathUserData->string.append(gerb);
                    break;
                case 'l':
                case 'L':
                    x = args[0];
                    y = args[1];
                    if (relative) {
                        x += pathUserData->x;
                        y += pathUserData->y;
                    }
                    gerb = "X" + QString::number(x) + "Y" + QString::number(y) + "D01*\n";
                    pathUserData->x = x;
                    pathUserData->y = y;
                    pathUserData->string.append(gerb);
                    break;
                case 'z':
                case 'Z':
                    gerb = "X" + QString::number(m_pathstart_x) + "Y" + QString::number(m_pathstart_y) + "D01*\n";
                    gerb += "D02*\n";
                    pathUserData->x = x;
                    pathUserData->y = y;
                    pathUserData->string.append(gerb);
                    break;
                default:
                    pathUserData->string.append("INVALID");
                    break;
    }
}
