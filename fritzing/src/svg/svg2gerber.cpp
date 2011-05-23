/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include <qmath.h>

//TODO: currently only supports one board per sketch (i.e. multiple board outlines will mess you up)

SVG2gerber::SVG2gerber()
{
}

int SVG2gerber::convert(const QString & svgStr, bool doubleSided, const QString & mainLayerName, const QString & maskLayerName, bool forOutline)
{
    m_SVGDom = QDomDocument("svg");
    QString errorStr;
    int errorLine;
    int errorColumn;
    bool result = m_SVGDom.setContent(svgStr, &errorStr, &errorLine, &errorColumn);
    if (!result) {
        DebugDialog::debug(QString("gerber svg failed %1 %2 %3 %4").arg(svgStr).arg(errorStr).arg(errorLine).arg(errorColumn));
    }

#ifndef QT_NO_DEBUG
    QString temp;
    // dump paths SVG to tmp file for now
    QFile dump(QDir::temp().absoluteFilePath("paths_in" + mainLayerName + ".svg"));
    if (!dump.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber svg dump: cannot open output file");
    }
    else {
        QTextStream out(&dump);
		out.setCodec("UTF-8");
        out << m_SVGDom.toString();
    }
    temp = m_SVGDom.toString();
#endif

    normalizeSVG();

#ifndef QT_NO_DEBUG
    // dump paths SVG to tmp file for now
    QFile dump2(QDir::temp().absoluteFilePath("paths_normal" + mainLayerName + ".svg"));
    if (!dump2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber svg dump: cannot open output file");
    }
    else {
        QTextStream out2(&dump2);
		out2.setCodec("UTF-8");
        out2 << m_SVGDom.toString();
    }
    temp = m_SVGDom.toString();
#endif

    return renderGerber(doubleSided, mainLayerName, maskLayerName, forOutline);
}

QString SVG2gerber::getGerber(){
    return m_gerber_header + m_gerber_paths;
}

QString SVG2gerber::getSolderMask(){
    return m_soldermask_header + m_soldermask_paths;
}


QString SVG2gerber::getNCDrill(){
    return m_drill_header + m_drill_paths + m_drill_slots + m_drill_footer;
}

int SVG2gerber::renderGerber(bool doubleSided, const QString & mainLayerName, const QString & maskLayerName, bool forOutline){
    // human readable description comments
    m_gerber_header = "G04 MADE WITH FRITZING*\n";
    m_gerber_header += "G04 WWW.FRITZING.ORG*\n";
	m_gerber_header += QString("G04 %1 SIDED*\n").arg(doubleSided ? "DOUBLE" : "SINGLE");
	m_gerber_header += QString("G04 HOLES%1PLATED*\n").arg(doubleSided ? " " : " NOT ");
    m_gerber_header += "G04 CONTOUR ON CENTER OF CONTOUR VECTOR*\n";

    // initialize axes
    m_gerber_header += "%ASAXBY*%\n";

    // NOTE: this currently forces a 1 mil grid
    // format coordinates to drop leading zeros with 2,3 digits
    m_gerber_header += "%FSLAX23Y23*%\n";

    // set units to inches
    m_gerber_header += "%MOIN*%\n";

    // no offset
    m_gerber_header += "%OFA0B0*%\n";

    // scale factor 1x1
    m_gerber_header += "%SFA1.0B1.0*%\n";

    // clone it for the mask and contour headers
    m_soldermask_header = m_gerber_header;

    // set inverse polarity: Loch says don't do this
    //m_soldermask_header += "%IPNEG*%\n";

    // setup drill file header
    m_drill_header = "M48\n";
    // set to english (inches) units, with trailing zeros
    m_drill_header += "M72,TZ\n";

	// drill file unload tool and end of program
    m_drill_footer = "T00\n";
    m_drill_footer += "M30\n";


    // define apertures and draw em
    int invalidCount = allPaths2gerber(forOutline);

    // label our layers
    m_gerber_header += QString("%LN%1*%\n").arg(mainLayerName.toUpper());
    m_soldermask_header += QString("%LN%1*%\n").arg(maskLayerName.toUpper());

    // rewind drill to start position
    m_drill_header += "%\n";

    //just to be safe: G90 (absolute coords) and G70 (inches)
    m_gerber_header += "G90*\nG70*\n";
    m_soldermask_header += "G90*\nG70*\n";

    // now write the footer
    // comment to indicate end-of-sketch
    m_gerber_paths += QString("G04 End of %1*\n").arg(mainLayerName);
	m_soldermask_paths += QString("G04 End of %1*\n").arg(maskLayerName);

    // write gerber end-of-program
    m_gerber_paths += "M02*";
    m_soldermask_paths += "M02*";

	return invalidCount;
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
            path = element;
        }
        else if(tag=="rect"){
            path = element;
        }
        else if(tag=="circle"){
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
    QStringList attrList;
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

int SVG2gerber::allPaths2gerber(bool forOutline) {
	int invalidPathsCount = 0;
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

    QDomNodeList circleList = m_SVGDom.elementsByTagName("circle");
    DebugDialog::debug("circles to gerber: " + QString::number(circleList.length()));

    QDomNodeList rectList = m_SVGDom.elementsByTagName("rect");
    DebugDialog::debug("rects to gerber: " + QString::number(rectList.length()));

    QDomNodeList polyList = m_SVGDom.elementsByTagName("polygon");
    DebugDialog::debug("polygons to gerber: " + QString::number(polyList.length()));

	QDomNodeList lineList = m_SVGDom.elementsByTagName("line");
    DebugDialog::debug("lines to gerber: " + QString::number(lineList.length()));

	QDomNodeList pathList = m_SVGDom.elementsByTagName("path");
    DebugDialog::debug("paths to gerber: " + QString::number(pathList.length()));

	int totalCount = circleList.count() + rectList.count() + polyList.count() + lineList.count() + pathList.count();

    // if this is the board outline, use it as the contour
    if (forOutline) {
        DebugDialog::debug("drawing board outline");

        // switch aperture to the only one used for contour: note this is the last one on the list: the aperture is added at the end of this function
        m_gerber_paths += "G54D10*\n";
    }

	// circles
    for(uint i = 0; i < circleList.length(); i++){
        QDomElement circle = circleList.item(i).toElement();
		if (circle.attribute("drill").compare("0") == 0) {
			continue;
		}

		if (forOutline && totalCount > 1) {
			if (circle.attribute("id").compare("boardoutline", Qt::CaseInsensitive) != 0) continue;
		}

        QString aperture;
        QString mask_aperture;
        QString drill_aperture;

        qreal centerx = circle.attribute("cx").toDouble();
        qreal centery = circle.attribute("cy").toDouble();
        QString cx = QString::number(qRound(centerx));
        QString cy = QString::number(qRound(centery));
        QString drill_cx = QString::number(centerx / 1000, 'f');				// drill file seems to be in inches
        QString drill_cy = QString::number(centery / 1000, 'f');				// drill file seems to be in inches
        qreal r = circle.attribute("r").toDouble();
        QString fill = circle.attribute("fill");
        qreal stroke_width = circle.attribute("stroke-width").toDouble();

        qreal diam = ((2*r) + stroke_width)/1000;
        qreal hole = ((2*r) - stroke_width)/1000;
        //NOTE: assuming 3 mil soldermask clearance
        qreal mask_diam = diam + 0.006;

        if(fill=="none"){
			aperture = QString("C,%1X%2").arg(diam, 0, 'f').arg(hole);
            drill_aperture = QString("C%1").arg(hole, 0, 'f');
        }
        else
            aperture = QString("C,%1").arg(diam, 0, 'f');

        mask_aperture = QString("C,%1").arg(mask_diam, 0, 'f');

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            m_soldermask_header += "%ADD" + QString::number(dcode_index) + mask_aperture + "*%\n";
            if(drill_aperture != "")
                m_drill_header += "T" + QString::number(dcode_index) + drill_aperture + "\n";
            dcode_index++;
        }

		if (!forOutline) {
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
			if(drill_aperture != "") {
				m_drill_paths += "X" + drill_cx + "Y" + drill_cy + "\n";
			}
		}
		else {
			// create circle outline 
			m_gerber_paths += QString("G01X%1Y%2D02*\n"
									  "G75*\n"
									  "G03X%1Y%2I%3J0D01*\n")
					.arg(QString::number(qRound(centerx + r)))
					.arg(QString::number(qRound(centery)))
					.arg(QString::number(qRound(-r)));
			m_gerber_paths += "G01*\n";
		}
    }

    // rects
    for(uint j = 0; j < rectList.length(); j++){
        QDomElement rect = rectList.item(j).toElement();
		if (forOutline && totalCount > 1) {
			if (rect.attribute("id").compare("boardoutline", Qt::CaseInsensitive) != 0) continue;
		}

        QString aperture;
        QString mask_aperture;

        qreal width = rect.attribute("width").toDouble();
        qreal height = rect.attribute("height").toDouble();
        qreal x = rect.attribute("x").toDouble();
        qreal y = rect.attribute("y").toDouble();
        qreal centerx = x + (width/2);
        qreal centery = y + (height/2);
        QString cx = QString::number(qRound(centerx));
        QString cy = QString::number(qRound(centery));
        QString fill = rect.attribute("fill");
        qreal stroke_width = rect.attribute("stroke-width").toDouble();

        qreal totalx = (width + stroke_width)/1000;
        qreal totaly = (height + stroke_width)/1000;
        qreal holex = (width - stroke_width)/1000;
        qreal holey = (height - stroke_width)/1000;

        //NOTE: assumes 3 mil mask clearance
        qreal mask_totalx = totalx + 0.006;
        qreal mask_totaly = totaly + 0.006;

        if(fill=="none")
            aperture = QString("R,%1X%2X%3X%4").arg(totalx, 0, 'f').arg(totaly, 0, 'f').arg(holex, 0, 'f').arg(holey, 0, 'f');
        else
            aperture = QString("R,%1X%2").arg(totalx, 0, 'f').arg(totaly, 0, 'f');

        mask_aperture = QString("R,%1X%2").arg(mask_totalx, 0, 'f').arg(mask_totaly, 0, 'f');

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = "D" + QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            m_soldermask_header += "%ADD" + QString::number(dcode_index) + mask_aperture + "*%\n";
            dcode_index++;
        }

		if (!forOutline) {
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
        else {
            // draw 4 lines
            m_gerber_paths += "X" + QString::number(qRound(x)) + "Y" + QString::number(qRound(y)) + "D02*\n";
            m_gerber_paths += "X" + QString::number(qRound(x+width)) + "Y" + QString::number(qRound(y)) + "D01*\n";
            m_gerber_paths += "X" + QString::number(qRound(x+width)) + "Y" + QString::number(qRound(y+height)) + "D01*\n";
            m_gerber_paths += "X" + QString::number(qRound(x)) + "Y" + QString::number(qRound(y+height)) + "D01*\n";
            m_gerber_paths += "X" + QString::number(qRound(x)) + "Y" + QString::number(qRound(y)) + "D01*\n";
            m_gerber_paths += "D02*\n";
        }
    }

	// polys - NOTE: assumes comma- or space- separated formatting
    for(uint p = 0; p < polyList.length(); p++){
        QDomElement polygon = polyList.item(p).toElement();
		if (forOutline && totalCount > 1) {
			if (polygon.attribute("id").compare("boardoutline", Qt::CaseInsensitive) != 0) continue;
		}

        QString points = polygon.attribute("points");
		QStringList pointList = points.split(QRegExp("\\s+|,"), QString::SkipEmptyParts);

		QString aperture;
		QString mask_aperture;

        // set poly fill if this is actually a filled in shape
        if(!forOutline && polygon.hasAttribute("fill") && !(polygon.hasAttribute("stroke"))){
            // start poly fill
            m_gerber_paths += "G36*\n";
			aperture = QString("C,%1").arg(1/1000.0, 0, 'f');
			mask_aperture = QString("C,%1").arg((1/1000.0) + 0.006, 0, 'f');
        }
		else {
			qreal stroke_width = polygon.attribute("stroke-width").toDouble();

			aperture = QString("C,%1").arg(stroke_width/1000, 0, 'f');
			mask_aperture = QString("C,%1").arg((stroke_width/1000) + 0.006, 0, 'f');

		}

		// add aperture to defs if we don't have it yet
		if(!apertureMap.contains(aperture)){
			apertureMap[aperture] = "D" + QString::number(dcode_index);
			m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
			m_soldermask_header += "%ADD" + QString::number(dcode_index) + mask_aperture + "*%\n";
			dcode_index++;
		}

		if (!forOutline) {
			QString dcode = apertureMap[aperture];
			if(current_dcode != dcode){
				//switch to correct aperture
				m_gerber_paths += "G54" + dcode + "*\n";
				m_soldermask_paths += "G54" + dcode + "*\n";
				current_dcode = dcode;
			}
		}

        int startx = qRound(pointList.at(0).toDouble());
        int starty = qRound(pointList.at(1).toDouble());
        // move to start - light off
        m_gerber_paths += "X" + QString::number(startx) + "Y" + QString::number(starty) + "D02*\n";
        m_soldermask_paths += "X" + QString::number(startx) + "Y" + QString::number(starty) + "D02*\n";

        // iterate through all other points - light on
        for(int pt = 2; pt < pointList.length(); pt +=2){
            int ptx = qRound(pointList.at(pt).toDouble());
            int pty = qRound(pointList.at(pt+1).toDouble());
            m_gerber_paths += "X" + QString::number(ptx) + "Y" + QString::number(pty) + "D01*\n";
            m_soldermask_paths += "X" + QString::number(ptx) + "Y" + QString::number(pty) + "D01*\n";
        }

        // move back to start point
        m_gerber_paths += "X" + QString::number(startx) + "Y" + QString::number(starty) + "D01*\n";
        m_soldermask_paths += "X" + QString::number(startx) + "Y" + QString::number(starty) + "D01*\n";

        // stop poly fill if this is actually a filled in shape
        if(!forOutline && polygon.hasAttribute("fill") && !(polygon.hasAttribute("stroke"))){
            // stop poly fill
            m_gerber_paths += "G37*\n";
        }

        // light off
        m_gerber_paths += "D02*\n";
    }

    // lines - NOTE: this assumes a circular aperture
    for(uint k = 0; k < lineList.length(); k++){
        QDomElement line = lineList.item(k).toElement();
		if (forOutline && totalCount > 1) {
			if (line.attribute("id").compare("boardoutline", Qt::CaseInsensitive) != 0) continue;
		}

		QString aperture;

        int x1 = qRound(line.attribute("x1").toDouble());
        int y1 = qRound(line.attribute("y1").toDouble());
        int x2 = qRound(line.attribute("x2").toDouble());
        int y2 = qRound(line.attribute("y2").toDouble());
        qreal stroke_width = line.attribute("stroke-width").toDouble();
		if (stroke_width == 0) continue;

        aperture = QString("C,%1").arg(stroke_width/1000, 0, 'f');

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

		if (!forOutline) {
			QString dcode = apertureMap[aperture];
			if(current_dcode != dcode){
				//switch to correct aperture
				m_gerber_paths += "G54" + dcode + "*\n";
				current_dcode = dcode;
			}
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
    for(uint n = 0; n < pathList.length(); n++){
        QDomElement path = pathList.item(n).toElement();
		if (forOutline && totalCount > 1) {
			if (path.attribute("id").compare("boardoutline", Qt::CaseInsensitive) != 0) continue;
		}

		QString data = path.attribute("d").trimmed();
        QString aperture;

        const char * slot = SLOT(path2gerbCommandSlot(QChar, bool, QList<double> &, void *));

        PathUserData pathUserData;
        pathUserData.x = 0;
        pathUserData.y = 0;
        pathUserData.string = "";

        SvgFlattener flattener;
        flattener.parsePath(data, slot, pathUserData, this, true);

		handleOblongPath(path, dcode_index);

        // only add paths if they contained gerber-izable path commands (NO CURVES!)
        // TODO: display some informative error for the user
        if(pathUserData.string.contains("INVALID")) {
			invalidPathsCount++;
            continue;
		}

        // set poly fill if this is actually a filled in shape
        if(!forOutline && path.hasAttribute("fill") && !(path.hasAttribute("stroke"))){
            // start poly fill
            m_gerber_paths += "G36*\n";
        }

		qreal stroke_width = path.attribute("stroke-width").toDouble();

        aperture = QString("C,%1").arg(stroke_width/1000, 0, 'f');

        // add aperture to defs if we don't have it yet
        if(!apertureMap.contains(aperture)){
            apertureMap[aperture] = "D" + QString::number(dcode_index);
            m_gerber_header += "%ADD" + QString::number(dcode_index) + aperture + "*%\n";
            dcode_index++;
        }

        m_gerber_paths += pathUserData.string;

        DebugDialog::debug("path id: " + path.attribute("id"));

        // stop poly fill if this is actually a filled in shape
        if(!forOutline && path.hasAttribute("fill") && !(path.hasAttribute("stroke"))){
            // stop poly fill
            m_gerber_paths += "G37*\n";
        }
    }


    if (forOutline) {
        // add circular aperture with 0 width
        m_gerber_header += "%ADD10C,0.008*%\n";
    }

	return invalidPathsCount;
}

void SVG2gerber::handleOblongPath(QDomElement & path, int & dcode_index) {
		// look for oblong paths
	QDomElement parent = path.parentNode().toElement();

	if (!parent.attribute("id").compare("oblong") == 0) return;

	QDomElement nextPath = path.nextSiblingElement("path");
	if (nextPath.isNull()) return;

	QDomElement nextLine = nextPath.nextSiblingElement("line");
	if (nextLine.isNull()) return;
				
	qreal diameter = parent.attribute("stroke-width").toDouble();
	qreal cx1 = nextLine.attribute("x1").toDouble();
	qreal cy1 = nextLine.attribute("y1").toDouble();
	qreal cx2 = nextLine.attribute("x2").toDouble();
	qreal cy2 = nextLine.attribute("y2").toDouble();

	QString drill_aperture = QString("C%1").arg(diameter / 1000, 0, 'f') + "\n";
	if (!m_drill_header.contains(drill_aperture)) {
		m_drill_header += "T" + QString::number(dcode_index++) + drill_aperture;
	}
	int ix = m_drill_header.indexOf(drill_aperture);
	int it = m_drill_header.lastIndexOf("T", ix);
	m_drill_slots += QString("%1\nX%2Y%3G85X%4Y%5\nG05\n")
		.arg(m_drill_header.mid(it, ix - it), 0, 'f')
		.arg(cx1 / 1000, 0, 'f')
		.arg(cy1 / 1000, 0, 'f')
		.arg(cx2 / 1000, 0, 'f')
		.arg(cy2 / 1000, 0, 'f');
}

QDomElement SVG2gerber::ellipse2path(QDomElement ellipseElement){
    // TODO
    return ellipseElement;
}

QString SVG2gerber::path2gerber(QDomElement pathElement){
    Q_UNUSED(pathElement);
    QString d;

    return d;
}

void SVG2gerber::path2gerbCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {
    QString gerb;
    int x, y;

    PathUserData * pathUserData = (PathUserData *) userData;

    switch(command.toAscii()) {
				case 'a':
				case 'A':
					// TODO: implement elliptical arc 
					pathUserData->string.append("INVALID");
					break;
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
					DebugDialog::debug("'v' and 'V' are now removed by preprocessing; shouldn't be here");
					/*
                    y = args[0];
                    if (relative) {
                        y += pathUserData->y;
                    }
                    gerb = "Y" + QString::number(y) + "D01*\n";
                    pathUserData->y = y;
                    pathUserData->string.append(gerb);
					*/
                    break;
                case 'h':
                case 'H':
					DebugDialog::debug("'h' and 'H' are now removed by preprocessing; shouldn't be here");
					/*
                    x = args[0];
                    if (relative) {
                        x += pathUserData->x;
                    }
                    gerb = "X" + QString::number(x) + "D01*\n";
                    pathUserData->x = x;
                    pathUserData->string.append(gerb);
					*/
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
                    pathUserData->string.append(gerb);
                    break;
                default:
                    pathUserData->string.append("INVALID");
                    break;
    }
}
