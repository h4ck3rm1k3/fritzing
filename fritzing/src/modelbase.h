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

#ifndef MODELBASE_H
#define MODELBASE_H

#include <QObject>
#include "modelpart.h"

class ModelBase : public QObject
{
Q_OBJECT

public:
	ModelBase(bool makeRoot);
	virtual ~ModelBase();

	ModelPart * root();
	virtual ModelPart* retrieveModelPart(const QString & moduleID);
	virtual ModelPart * addModelPart(ModelPart * parent, ModelPart * copyChild);
	virtual bool load(const QString & fileName, ModelBase* refModel, QList<ModelPart *> & modelParts);
	void save(const QString & fileName, bool asPart=false);
	void save(class QXmlStreamWriter &, bool asPart);
	virtual ModelPart * addPart(QString newPartPath, bool addToReference);
	virtual bool addPart(ModelPart * modelPart, bool update);
	virtual bool paste(ModelBase * refModel, QByteArray & data, QList<ModelPart *> & modelParts);

protected:
	void renewModelIndexes(QDomElement & root, QHash<long, long> & oldToNew);
	bool loadInstances(QDomElement & root, QList<ModelPart *> & modelParts);

protected:
	ModelPart * m_root;
	ModelBase * m_referenceModel;

};

#endif
