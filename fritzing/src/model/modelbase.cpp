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

#include "modelbase.h"
#include "../debugdialog.h"
#include "../items/pinheader.h"

#include <QMessageBox>

ModelBase::ModelBase( bool makeRoot )
{
	m_referenceModel = NULL;
	m_root = (makeRoot) ? new ModelPart() : NULL;
}

ModelBase::~ModelBase() {
	if (m_root) {
		delete m_root;
	}
}


ModelPart * ModelBase::root() {
	return m_root;
}

ModelPart * ModelBase::retrieveModelPart(const QString & /* moduleID */)  {
	return NULL;
}

// loads a model from an fz file--assumes a reference model exists with all parts
bool ModelBase::load(const QString & fileName, ModelBase * refModel, QList<ModelPart *> & modelParts) {
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
    QDomDocument domDocument;

    if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"),
                                 QObject::tr("Parse error (1) at line %1, column %2:\n%3\n%4")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr)
								 .arg(fileName));
        return false;
    }

    QDomElement root = domDocument.documentElement();
   	if (root.isNull()) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file %1 is not a Fritzing file (2).").arg(fileName));
   		return false;
	}

    if (root.tagName() != "module") {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file %1 is not a Fritzing file (4).").arg(fileName));
        return false;
    }

    QDomElement title = root.firstChildElement("title");
	if (!title.isNull()) {
		this->root()->modelPartShared()->setTitle(title.text());
	}

    QDomElement views = root.firstChildElement("views");
	emit loadedViews(this, views);

	QDomElement instances = root.firstChildElement("instances");
	if (instances.isNull()) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file %1 is not a Fritzing file (3).").arg(fileName));
        return false;
	}

	// delete any aready-existing model parts
    for (int i = m_root->children().count() - 1; i >= 0; i--) {
    	QObject* child = m_root->children()[i];
    	child->setParent(NULL);
    	delete child;
   	}

	return loadInstances(domDocument, instances, modelParts);

}

ModelPart * ModelBase::fixObsoleteModuleID(QDomDocument & domDocument, QDomElement & instance, QString & moduleIDRef) {
	// TODO: less hard-coding
	QRegExp oldDip("generic_ic_dip_(\\d{1,2})_(\\d{3}mil)");
	if (oldDip.indexIn(moduleIDRef) == 0) {
		QString spacing = oldDip.cap(2);
		QString pins = oldDip.cap(1);
		moduleIDRef = QString("generic_ic_dip_%1_300mil").arg(pins);
		ModelPart * modelPart = m_referenceModel->retrieveModelPart(moduleIDRef);
		if (modelPart != NULL) {
			instance.setAttribute("moduleIdRef", moduleIDRef);
			QDomElement prop = domDocument.createElement("property");
			instance.appendChild(prop);
			prop.setAttribute("name", "spacing");
			prop.setAttribute("value", spacing);
			return modelPart;
		}
	}

	if (moduleIDRef.startsWith("generic_male")) {
		moduleIDRef.replace("male", "female");
		ModelPart * modelPart = m_referenceModel->retrieveModelPart(moduleIDRef);
		if (modelPart != NULL) {
			instance.setAttribute("moduleIdRef", moduleIDRef);
			QDomElement prop = domDocument.createElement("property");
			instance.appendChild(prop);
			prop.setAttribute("name", "form");
			prop.setAttribute("value", PinHeader::MaleFormString);
			return modelPart;
		}
	}

	if (moduleIDRef.startsWith("generic_rounded_female")) {
		moduleIDRef.replace("rounded_female", "female");
		ModelPart * modelPart = m_referenceModel->retrieveModelPart(moduleIDRef);
		if (modelPart != NULL) {
			instance.setAttribute("moduleIdRef", moduleIDRef);
			QDomElement prop = domDocument.createElement("property");
			instance.appendChild(prop);
			prop.setAttribute("name", "form");
			prop.setAttribute("value", PinHeader::FemaleRoundedFormString);
			return modelPart;
		}
	}

	return NULL;
}

bool ModelBase::loadInstances(QDomDocument & domDocument, QDomElement & instances, QList<ModelPart *> & modelParts)
{
   	QDomElement instance = instances.firstChildElement("instance");
   	ModelPart* modelPart = NULL;
   	while (!instance.isNull()) {
   		// for now assume all parts are in the palette
   		QString moduleIDRef = instance.attribute("moduleIdRef");
   		modelPart = m_referenceModel->retrieveModelPart(moduleIDRef);
   		if (modelPart == NULL) {
			DebugDialog::debug(QString("module id %1 not found in database").arg(moduleIDRef));
			modelPart = fixObsoleteModuleID(domDocument, instance, moduleIDRef);
			if (modelPart == NULL) {
   				instance = instance.nextSiblingElement("instance");
   				continue;
			}
   		}

   		modelPart = addModelPart(m_root, modelPart);
   		modelPart->setInstanceDomElement(instance);
		modelParts.append(modelPart);

   		// TODO Mariano: i think this is not the way
   		QString instanceTitle = instance.firstChildElement("title").text();
   		if(!instanceTitle.isNull() && !instanceTitle.isEmpty()) {
   			modelPart->setInstanceTitle(instanceTitle);
   		}

   		QString instanceText = instance.firstChildElement("text").text();
   		if(!instanceText.isNull() && !instanceText.isEmpty()) {
   			modelPart->setInstanceText(instanceText);
   		}

   		bool ok;
   		long index = instance.attribute("modelIndex").toLong(&ok);
   		if (ok) {
			// set the index so we can find the same model part later, as we continue loading
			modelPart->setModelIndex(index);
  		}
   		long oindex = instance.attribute("originalModelIndex").toLong(&ok);
   		if (ok) {
			// used for saving connections to parts in modules
			modelPart->setOriginalModelIndex(oindex);
			//DebugDialog::debug(QString("loadinstances original model index %1 %2").arg(oindex).arg((long) modelPart, 0, 16));
  		}
		else {
			modelPart->setOriginalModelIndex(index);
		}

		// note: this QDomNamedNodeMap loop is obsolete, but leaving it here so that old sketches don't get broken (jc, 22 Oct 2009)
		QDomNamedNodeMap map = instance.attributes();
		for (int m = 0; m < map.count(); m++) {
			QDomNode node = map.item(m);
			QString nodeName = node.nodeName();

			if (nodeName.isEmpty()) continue;
			if (nodeName.compare("moduleIdRef") == 0) continue;
			if (nodeName.compare("modelIndex") == 0) continue;
			if (nodeName.compare("originalModelIndex") == 0) continue;
			if (nodeName.compare("path") == 0) continue;

			modelPart->setProp(nodeName.toUtf8().constData(), node.nodeValue());
		}

		// "property" loop replaces previous QDomNamedNodeMap loop (jc, 22 Oct 2009)
		QDomElement prop = instance.firstChildElement("property");
		while(!prop.isNull()) {
			QString name = prop.attribute("name");
			if (!name.isEmpty()) {
				QString value = prop.attribute("value");
				if (!value.isEmpty()) {
					modelPart->setProp(name.toUtf8().constData(), value);
				}
			}

			prop = prop.nextSiblingElement("property");
		}

   		instance = instance.nextSiblingElement("instance");
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
	save(streamWriter, asPart);
	file1.close();
	QFile original(fileName);
	if(original.exists() && !original.remove()) {
		file1.remove();
		QMessageBox::warning(
			NULL,
			tr("File save failed!"),
			tr("Couldn't overwrite file '%1'.\nReason: %2 (errcode %3)")
				.arg(fileName)
				.arg(original.errorString())
				.arg(original.error())
			);
		return;
	}
	file1.rename(fileName);
}

void ModelBase::save(QXmlStreamWriter & streamWriter, bool asPart) {
    streamWriter.setAutoFormatting(true);
    if(asPart) {
    	m_root->saveAsPart(streamWriter, true);
    } else {
    	m_root->saveInstances(streamWriter, true);
    }
}

bool ModelBase::paste(ModelBase * refModel, QByteArray & data, QList<ModelPart *> & modelParts, QHash<QList<long> *, QString > * externalConnectors)
{
	m_referenceModel = refModel;

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(data, &errorStr, &errorLine, &errorColumn);
	if (!result) return false;

	QDomElement module = domDocument.documentElement();
	if (module.isNull()) {
		return false;
	}

	QDomElement instances = module.firstChildElement("instances");
   	if (instances.isNull()) {
   		return false;
	}

	// need to map modelIndexes from copied parts to new modelIndexes
	QHash<long, long> oldToNew;
	QDomElement instance = instances.firstChildElement("instance");
	while (!instance.isNull()) {
		long oldModelIndex = instance.attribute("modelIndex").toLong();
		oldToNew.insert(oldModelIndex, ModelPart::nextIndex());
		instance = instance.nextSiblingElement("instance");
	}
	renewModelIndexes(instances, "instance", oldToNew);
	if (externalConnectors) {
		QDomElement externals = module.firstChildElement("externals");
   		if (!externals.isNull()) {
			renewExternalIndexes(externals, "external", oldToNew, externalConnectors);
		}
	}

	//QFile file("test.xml");
	//file.open(QFile::WriteOnly);
	//file.write(domDocument.toByteArray());
	//file.close();

	return loadInstances(domDocument, instances, modelParts);
}

void ModelBase::renewModelIndexes(QDomElement & parentElement, const QString & childName, QHash<long, long> & oldToNew)
{
	QDomElement instance = parentElement.firstChildElement(childName);
	while (!instance.isNull()) {
		long oldModelIndex = instance.attribute("modelIndex").toLong();
		instance.setAttribute("modelIndex", QString::number(oldToNew.value(oldModelIndex)));
		instance.setAttribute("originalModelIndex", QString::number(oldModelIndex));
		QDomElement views = instance.firstChildElement("views");
		if (!views.isNull()) {
			QDomElement view = views.firstChildElement();
			while (!view.isNull()) {
				QDomElement connectors = view.firstChildElement("connectors");
				if (!connectors.isNull()) {
					QDomElement connector = connectors.firstChildElement("connector");
					while (!connector.isNull()) {
						QDomElement connects = connector.firstChildElement("connects");
						if (!connects.isNull()) {
							QDomElement connect = connects.firstChildElement("connect");
							while (!connect.isNull()) {
								bool ok;
								oldModelIndex = connect.attribute("modelIndex").toLong(&ok);
								if (ok) {
									connect.setAttribute("modelIndex", QString::number(oldToNew.value(oldModelIndex)));
								}
								else {
									// we're connected to something inside a module; fixup the first modelIndex
									QDomElement p = connect.firstChildElement("mp");
									if (!p.isNull()) {
										oldModelIndex = p.attribute("i").toLong();
										p.setAttribute("i", QString::number(oldToNew.value(oldModelIndex)));
									}
								}
								connect = connect.nextSiblingElement("connect");
							}
						}
						connector = connector.nextSiblingElement("connector");
					}
				}

				view = view.nextSiblingElement();
			}
		}

		instance = instance.nextSiblingElement(childName);
	}
}


void ModelBase::renewExternalIndexes(QDomElement & parentElement, const QString & childName, QHash<long, long> & oldToNew, QHash<QList<long> *, QString > * externalConnectors)
{
	QDomElement instance = parentElement.firstChildElement(childName);
	while (!instance.isNull()) {
		bool ok;
		QString connectorID = instance.attribute("connectorId");
		long oldModelIndex = instance.attribute("modelIndex").toLong(&ok);
		QList<long> * l = new QList<long>;
		if (ok) {
			instance.setAttribute("modelIndex", QString::number(oldToNew.value(oldModelIndex)));
			l->append(oldToNew.value(oldModelIndex));
		}
		else {
			// we're connected to something inside a module; fixup the first modelIndex
			QDomElement p = instance.firstChildElement("mp");
			if (!p.isNull()) {
				oldModelIndex = p.attribute("i").toLong();
				p.setAttribute("i", QString::number(oldToNew.value(oldModelIndex)));
				l->append(oldToNew.value(oldModelIndex));

				while (true) {
					p = p.firstChildElement("mp");
					if (p.isNull()) break;

					l->append(p.attribute("i").toLong());
				}
			}
		}
		externalConnectors->insert(l, connectorID);

		instance = instance.nextSiblingElement(childName);
	}
}

