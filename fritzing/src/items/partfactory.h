/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef PARTFACTORY_H
#define PARTFACTORY_H

#include <QMenu>
#include <QDomDocument>
#include <QDomElement>
#include "../viewidentifierclass.h"
#include "../viewlayer.h"
#include "paletteitem.h"


class PartFactory
{
public:
	static class ItemBase * createPart(class ModelPart *, ViewLayer::ViewLayerSpec, ViewIdentifierClass::ViewIdentifier, const class ViewGeometry & viewGeometry, long id, QMenu * itemMenu, QMenu * wireMenu, bool doLabel);
	static QString getSvgFilename(class ModelPart *, const QString & filename);
	static QString getFzpFilename(const QString & moduleID);
	static void initFolder();
	static void cleanup();
	static class ModelPart * fixObsoleteModuleID(QDomDocument & domDocument, QDomElement & instance, QString & moduleIDRef, class ModelBase * referenceModel);
	static QString folderPath();
	static QString fzpPath();
	static QString partPath();
    static bool svgFileExists(const QString & expectedFileName, QString & path);
    static bool fzpFileExists(const QString & moduleID, QString & path);
    static QString makeSipOrDipOr(const QStringList & labels, bool hasLayout, bool sip);


protected:
	static QString getFzpFilenameAux(const QString & moduleID, GenFzp);
	static QString getSvgFilenameAux(const QString & expectedFileName, const ModelPart *, GenSvg);
	static class ItemBase * createPartAux(class ModelPart *, ViewIdentifierClass::ViewIdentifier, const class ViewGeometry & viewGeometry, long id, QMenu * itemMenu, QMenu * wireMenu, bool doLabel);
};

#endif
