#include "modelbase.h"
#include "debugdialog.h"

#include <QMessageBox>

ModelBase::ModelBase( bool makeRoot )
{
	m_referenceModel = NULL;
	m_root = (makeRoot) ? new ModelPart() : NULL;
}

ModelBase::~ModelBase() {
	// seems to get rid of a bunch of compiler warnings
}


ModelPart * ModelBase::root() {
	return m_root;
}

ModelPart * ModelBase::retrieveModelPart(const QString & /* moduleID */)  {
	return NULL;
}

// loads a model from an fz file--assumes a reference model exists with all parts
bool ModelBase::load(const QString & fileName, ModelBase * refModel, bool doConnections) {
	m_referenceModel = refModel;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(NULL, QObject::tr("Fritzing"),
                             QObject::tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument* domDocument = new QDomDocument();

    if (!domDocument->setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"),
                                 QObject::tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return false;
    }

    QDomElement root = domDocument->documentElement();
   	if (root.isNull()) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file is not a Fritzing file (2)."));
   		return false;
	}

    if (root.tagName() != "module") {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file is not a Fritzing file."));
        return false;
    }

    QDomElement title = root.firstChildElement("title");
	if (!title.isNull()) {
		this->root()->modelPartStuff()->setTitle(title.text());
	}

	QDomElement instances = root.firstChildElement("instances");
	if (instances.isNull()) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file is not a Fritzing file (3)."));
        return false;
	}

	// delete any aready-existing model parts
    for (int i = m_root->children().count() - 1; i >= 0; i--) {
    	QObject* child = m_root->children()[i];
    	child->setParent(NULL);
    	delete child;
   	}

   	QHash<long, ModelPart *> partHash;
   	QDomElement instance = instances.firstChildElement("instance");
   	ModelPart* modelPart = NULL;
   	while (!instance.isNull()) {
   		// for now assume all parts are in the palette
   		QString moduleIDRef = instance.attribute("moduleIdRef");
   		modelPart = refModel->retrieveModelPart(moduleIDRef);
   		if (modelPart == NULL) {
   			instance = instance.nextSiblingElement("instance");
   			continue;
   		}

   		modelPart = addModelPart(m_root, modelPart);
   		modelPart->setInstanceDomElement(instance);

   		// TODO Mariano: i think this is not the way
   		QString instanceTitle = instance.firstChildElement("title").text();
   		if(!instanceTitle.isNull() && !instanceTitle.isEmpty()) {
   			if(!modelPart->partInstanceStuff()) modelPart->setPartInstanceStuff(new PartInstanceStuff());
   			modelPart->partInstanceStuff()->setTitle(instanceTitle);
   		}

   		bool ok;
   		long index = instance.attribute("modelIndex").toLong(&ok);
   		if (ok) {
   			partHash.insert(index, modelPart);

			// set the index so we can find the same model part later, as we continue loading
			modelPart->setModelIndex(index);
  		}

   		instance = instance.nextSiblingElement("instance");
  	}

  	if (doConnections) {
		for (int i = 0; i < m_root->children().count(); i++) {
			ModelPart * modelPart = dynamic_cast<ModelPart *>(m_root->children()[i]);
			if (modelPart == NULL) continue;

			modelPart->initConnections(partHash);
		}
	}

	return true;
}

ModelPart * ModelBase::addModelPart(ModelPart * parent, ModelPart * copyChild) {
	ModelPart * modelPart = new ModelPart();
	modelPart->copyNew(copyChild);
	modelPart->setParent(parent);
	modelPart->initConnectors();
	return modelPart;
}

ModelPart * ModelBase::addPart(QString newPartPath, bool addToReference) {
	Q_UNUSED(newPartPath);
	Q_UNUSED(addToReference);
	Q_ASSERT(false);
	return NULL;
}

// TODO Mariano: this function should never get called. Make pure virtual
bool ModelBase::addPart(ModelPart * modelPart, bool update) {
        Q_UNUSED(modelPart);
        Q_UNUSED(update);
        Q_ASSERT(false);
	return false;
}


void ModelBase::save(const QString & fileName, bool asPart) {
	QDir dir(fileName);
	dir.cdUp();

	QString temp = dir.absolutePath() + "/" + "part_temp.xml";
    QFile file1(temp);
    if (!file1.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(NULL, QObject::tr("Fritzing"),
                             QObject::tr("Cannot write file temp file:\n%1.")
                              .arg(file1.errorString()));
        return;
    }

    QXmlStreamWriter streamWriter(&file1);
    streamWriter.setAutoFormatting(true);
    qint64 partsInsertPosition = 0;

    if(asPart) {
    	m_root->saveAsPart(streamWriter, true, partsInsertPosition);
    } else {
    	m_root->saveInstances(streamWriter, true, partsInsertPosition);
    }

	file1.close();
	QFile original(fileName);
	original.remove();
	file1.rename(fileName);
}
