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

#include "svgflattener.h"
#include "../debugdialog.h"
#include <QMatrix>
#include <QRegExp>

SvgFlattener::SvgFlattener()
{
}

void SvgFlattener::flattenChildren(QDomElement &element){
    // I'm a leaf node. (NOTE: assumes no transforms)
    if(!element.hasChildNodes()) {
        return;
    }

    // recurse the children
    QDomNodeList childList = element.childNodes();

    for(uint i = 0; i < childList.length(); i++){
        QDomElement child = childList.item(i).toElement();
        flattenChildren(child);
    }

    //do translate
    if(hasTranslate(element)){
        QList<qreal> params = getTransformFloats(element);
        if(params.size() > 1)
            shiftChild(element, params.at(0), params.at(1));
        else
            shiftChild(element, params.at(0), 0);
        DebugDialog::debug(QString("translating %1 %2").arg(params.at(0)).arg(params.at(1)));
    }
    //do rotate
    if(hasRotate(element)){
        QList<qreal> params = getTransformFloats(element);
        QMatrix transform = QMatrix(params.at(0), params.at(1), params.at(2), params.at(3), params.at(4), params.at(5));

        DebugDialog::debug(QString("rotating %1 %2 %3 %4 %5 %6").arg(params.at(0)).arg(params.at(1)).arg(params.at(2)).arg(params.at(3)).arg(params.at(4)).arg(params.at(5)));
        unRotateChild(element, transform);
    }

    // remove transform
    element.removeAttribute("transform");
}

void SvgFlattener::unRotateChild(QDomElement & element,QMatrix transform){
    // I'm a leaf node.
    QString tag = element.nodeName().toLower();


    if(!element.hasChildNodes()) {
        if(tag == "path"){
            QString data = element.attribute("d");
            if (!data.isEmpty()) {
                const char * slot = SLOT(rotateCommandSlot(QChar, bool, QList<double> &, void *));
                PathUserData pathUserData;
                pathUserData.transform = transform;
                if (parsePath(data, slot, pathUserData)) {
                    element.setAttribute("d", pathUserData.string);
                }
            }
        }
        else if(tag == "rect"){
            // NOTE: this only works for 90/180/270 deg rotations
            float x = element.attribute("x").toFloat();
            float y = element.attribute("y").toFloat();
            float width = element.attribute("width").toFloat();
            float height = element.attribute("height").toFloat();
            float cx = x + (width/2);
            float cy = y + (height/2);
            QPointF point = transform.map(QPointF(cx,cy));
            element.setAttribute("x", point.x()-(width/2));
            element.setAttribute("y", point.y()-(height/2));
        }
        else if(tag == "circle"){
            float cx = element.attribute("cx").toFloat();
            float cy = element.attribute("cy").toFloat();
            QPointF point = transform.map(QPointF(cx,cy));
            element.setAttribute("cx", point.x());
            element.setAttribute("cy", point.y());
        }
        else if(tag == "line") {
            float x1 = element.attribute("x1").toFloat();
            float y1 = element.attribute("y1").toFloat();
            QPointF p1 = transform.map(QPointF(x1,y1));
            element.setAttribute("x1", p1.x());
            element.setAttribute("y1", p1.y());

            float x2 = element.attribute("x2").toFloat();
            float y2 = element.attribute("y2").toFloat();
            QPointF p2 = transform.map(QPointF(x2,y2));
            element.setAttribute("x2", p2.x());
            element.setAttribute("y2", p2.y());
        }
        else
            DebugDialog::debug("Warning! Can't rotate element: " + tag);
        return;
    }

    // recurse the children
    QDomNodeList childList = element.childNodes();

    for(uint i = 0; i < childList.length(); i++){
        QDomElement child = childList.item(i).toElement();
        unRotateChild(child, transform);
    }

}

bool SvgFlattener::hasTranslate(QDomElement & element){
    bool rtn = false;

    if(element.hasAttribute("transform")){
        rtn = element.attribute("transform").startsWith("translate");
    }

    return rtn;
}

bool SvgFlattener::hasRotate(QDomElement & element){
    bool rtn = false;

    // ACHTUNG! this assumes that all rotates are done using a matrix() not rotate()
    if(element.hasAttribute("transform")){
        rtn = element.attribute("transform").startsWith("matrix");
    }

    return rtn;
}

QList<qreal> SvgFlattener::getTransformFloats(QDomElement & element){
    QRegExp rx("([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)");
    QString transform = element.attribute("transform");
    QList<qreal> list;
    int pos = 0;

    while ((pos = rx.indexIn(transform, pos)) != -1) {
        list << rx.cap(1).toFloat();
        pos += rx.matchedLength();
        //Debug
    }

#ifndef QT_NO_DEBUG
    QString dbg = "got transform params: \n";
    dbg += element.attribute("transform") + "\n";
    for(int i=0; i < list.size(); i++){
        dbg += QString::number(list.at(i)) + " ";
    }
    DebugDialog::debug(dbg);
#endif

    return list;
}

void SvgFlattener::rotateCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {

        Q_UNUSED(relative);			// just normalizing here, so relative is not used

        PathUserData * pathUserData = (PathUserData *) userData;

        pathUserData->string.append(command);
        qreal x;
        qreal y;


        // probably will fuck things up with relative coordinates
        for (int i = 0; i < args.count(); i=i+2) {
            x = args[i];
            y = args[i+1];
            QPointF point = pathUserData->transform.map(QPointF(x,y));
            pathUserData->string.append(QString::number(point.x()));
            pathUserData->string.append(',');
            pathUserData->string.append(QString::number(point.y()));
        }


//        switch(command.toAscii()) {
//                case 'v':
//                case 'V':
//                        d = args[0];
//                        if (!relative) {
//                                 d += pathUserData->y;
//                        }
//                        pathUserData->string.append(QString::number(d));
//                        break;
//                case 'h':
//                case 'H':
//                        d = args[0];
//                        if (!relative) {
//                                 d += pathUserData->x;
//                        }
//                        pathUserData->string.append(QString::number(d));
//                        break;
//                default:
//                        for (int i = 0; i < args.count(); i++) {
//                                d = args[i];
//                                if (i % 2 == 0) {
//                                        if (!relative) {
//                                                d += pathUserData->x;
//                                        }
//                                }
//                                else {
//                                        if (!relative) {
//                                                d += pathUserData->y;
//                                        }
//                                }
//                                pathUserData->string.append(QString::number(d));
//                                if (i < args.count() - 1) {
//                                        pathUserData->string.append(',');
//                                }
//                        }
//                        break;
//        }
}
