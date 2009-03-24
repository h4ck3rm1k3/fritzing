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

        unRotateChild(element, transform);
    }

    // remove transform
    element.removeAttribute("transform");
}

void SvgFlattener::unRotateChild(QDomElement & element,QMatrix transform){

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
