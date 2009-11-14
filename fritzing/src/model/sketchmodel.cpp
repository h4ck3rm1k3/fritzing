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

#include "sketchmodel.h"
#include "../debugdialog.h"

#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QXmlStreamWriter>

SketchModel::SketchModel(bool makeRoot) : ModelBase(makeRoot)
{
}

SketchModel::SketchModel(ModelPart * root) : ModelBase(false) {
	m_root = root;
}

void SketchModel::removeModelPart(ModelPart * modelPart) {
	modelPart->setParent(NULL);
	//DebugDialog::debug(QString("model count %1").arg(root()->children().size()));
}

ModelPart * SketchModel::findModelPart(const QString & moduleID, long id) {
	if (m_root == NULL) return NULL;

	return findModelPartAux(m_root, moduleID, id);
}

ModelPart * SketchModel::findModelPartFromOriginal(ModelPart * parent, long originalModelIndex) {
	if (parent->originalModelIndex() == originalModelIndex) return parent;

	foreach (QObject * child, parent->children()) {
		ModelPart * mp = dynamic_cast<ModelPart *>(child);
		if (mp == NULL) continue;

		mp = findModelPartFromOriginal(mp, originalModelIndex);
		if (mp != NULL) {
			return mp;
		}
	}

	return NULL;
}


ModelPart * SketchModel::findModelPartAux(ModelPart * modelPart, const QString & moduleID, long id) 
{
	if (modelPart->moduleID().compare(moduleID) == 0) {
		if (modelPart->hasViewID(id)) {
			return modelPart;
		}
		if (modelPart->modelIndex() * ModelPart::indexMultiplier == id) {
			return modelPart;
		}
	}

	foreach (QObject * child, modelPart->children()) {
		ModelPart * mp = dynamic_cast<ModelPart *>(child);
		if (mp == NULL) continue;

		mp = findModelPartAux(mp, moduleID, id);
		if (mp != NULL) {
			return mp;
		}
	}

	return NULL;
}

bool SketchModel::paste(ModelBase * refModel, const QString & filename, QList<ModelPart *> & modelParts, QHash<QList<long> *, QString > * externalConnectors) 
{
	QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return false;
	}

	QByteArray itemData = file.readAll();
	file.close();

	return ModelBase::paste(refModel, itemData, modelParts, externalConnectors);
}

void SketchModel::walk(ModelPart * modelPart, int indent) 
{
	DebugDialog::debug(QString("%1 %2 mi:%3 omi:%4 %5 %6")
		.arg(indent)
		.arg(QString(indent * 4, ' '))
		.arg(modelPart->modelIndex())
		.arg(modelPart->originalModelIndex())
		.arg(modelPart->moduleID())
		.arg(modelPart->title()) );

	foreach (QObject * child, modelPart->children()) {
		walk(dynamic_cast<ModelPart *>(child), indent + 1);
	}
}

ModelPartTiny * SketchModel::makeTiny(ModelPart * modelPart) {
	ModelPartTiny * modelPartTiny = new ModelPartTiny();
	modelPartTiny->m_index = modelPart->modelIndex();
	modelPartTiny->m_originalIndex = modelPart->originalModelIndex();
	foreach (QObject * child, modelPart->children()) {
		ModelPart * mp = dynamic_cast<ModelPart *>(child);
		if (mp == NULL) continue;

		ModelPartTiny * mpt = makeTiny(mp);
		modelPartTiny->m_children.append(mpt);
	}

	return modelPartTiny;
}
