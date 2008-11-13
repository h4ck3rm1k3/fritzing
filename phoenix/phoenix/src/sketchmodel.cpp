#include "sketchmodel.h"
#include "debugdialog.h"

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
	//DebugDialog::debug(QObject::tr("model count %1").arg(root()->children().size()));
}

