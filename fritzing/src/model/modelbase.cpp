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

#include "modelbase.h"
#include "../debugdialog.h"
#include "../items/partfactory.h"
#include "../items/moduleidnames.h"
#include "../version/version.h"

#include <QMessageBox>

ModelBase::ModelBase( bool makeRoot )
{
	m_reportMissingModules = true;
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

	emit loadedRoot(fileName, this, root);

    if (root.tagName() != "module") {
        QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file %1 is not a Fritzing file (4).").arg(fileName));
        return false;
    }

	bool checkForRats = true;
	QString fzVersion = root.attribute("fritzingVersion");
	if (fzVersion.length() > 0) {
		// with version 0.4.3 ratsnests in fz files are obsolete
		VersionThing versionThingRats;
		versionThingRats.majorVersion = 0;
		versionThingRats.minorVersion = 4;
		versionThingRats.minorSubVersion = 2;
		versionThingRats.releaseModifier = "";
		VersionThing versionThingFz;
		Version::toVersionThing(fzVersion,versionThingFz);
		checkForRats = !Version::greaterThan(versionThingRats, versionThingFz);
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

	emit loadingInstances(this, instances);
	bool result = loadInstances(domDocument, instances, modelParts, checkForRats);
	emit loadedInstances(this, instances);
	return result;
}

ModelPart * ModelBase::fixObsoleteModuleID(QDomDocument & domDocument, QDomElement & instance, QString & moduleIDRef) {
	return PartFactory::fixObsoleteModuleID(domDocument, instance, moduleIDRef, m_referenceModel);
}

bool ModelBase::loadInstances(QDomDocument & domDocument, QDomElement & instances, QList<ModelPart *> & modelParts, bool checkForRats)
{
	QHash<QString, QString> missingModules;
   	QDomElement instance = instances.firstChildElement("instance");
   	ModelPart* modelPart = NULL;
   	while (!instance.isNull()) {
		emit loadingInstance(this, instance);
		if (checkForRats && isRatsnest(instance)) {
			// ratsnests in sketches are now obsolete
			instance = instance.nextSiblingElement("instance");
			continue;
		}

   		// for now assume all parts are in the palette
   		QString moduleIDRef = instance.attribute("moduleIdRef");
		if (moduleIDRef.compare(ModuleIDNames::SpacerModuleIDName) == 0) {
			ModelPart * mp = new ModelPart(ModelPart::Space);
			mp->setInstanceText(instance.attribute("path"));
			mp->setParent(m_root);
			modelParts.append(mp);
			instance = instance.nextSiblingElement("instance");
			continue;
		}


   		modelPart = m_referenceModel->retrieveModelPart(moduleIDRef);
   		if (modelPart == NULL) {
			DebugDialog::debug(QString("module id %1 not found in database").arg(moduleIDRef));
			modelPart = fixObsoleteModuleID(domDocument, instance, moduleIDRef);
			if (modelPart == NULL) {
				if (genFZP(moduleIDRef, m_referenceModel)) {
					modelPart = m_referenceModel->retrieveModelPart(moduleIDRef);
				}
				if (modelPart == NULL) {
					missingModules.insert(moduleIDRef, instance.attribute("path"));
   					instance = instance.nextSiblingElement("instance");
   					continue;
				}
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

			modelPart->setProp(nodeName, node.nodeValue());
		}

		// "property" loop replaces previous QDomNamedNodeMap loop (jc, 22 Oct 2009)
		QDomElement prop = instance.firstChildElement("property");
		while(!prop.isNull()) {
			QString name = prop.attribute("name");
			if (!name.isEmpty()) {
				QString value = prop.attribute("value");
				if (!value.isEmpty()) {
					modelPart->setProp(name, value);
				}
			}

			prop = prop.nextSiblingElement("property");
		}

   		instance = instance.nextSiblingElement("instance");
  	}

	if (m_reportMissingModules && missingModules.count() > 0) {
		QString unableToFind = QString("<html><body><b>%1</b><br/><table style='border-spacing: 0px 12px;'>")
			.arg(tr("Unable to find the following %n part(s):", "", missingModules.count()));
		foreach (QString key, missingModules.keys()) {
			unableToFind += QString("<tr><td>'%1'</td><td><b>%2</b></td><td>'%3'</td></tr>")
				.arg(key).arg(tr("at")).arg(missingModules.value(key, ""));
		}
		unableToFind += "</table></body></html>";
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), unableToFind);
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
	throw "ModelBase::addPart should not be invoked";
	return NULL;
}

ModelPart * ModelBase::addPart(QString newPartPath, bool addToReference, bool updateIdAlreadyExists)
{
	Q_UNUSED(updateIdAlreadyExists);
	Q_UNUSED(newPartPath);
	Q_UNUSED(addToReference);
	throw "ModelBase::addPart should not be invoked";
	return NULL;
}

// TODO Mariano: this function should never get called. Make pure virtual
bool ModelBase::addPart(ModelPart * modelPart, bool update) {
	Q_UNUSED(modelPart);
	Q_UNUSED(update);
	throw "ModelBase::addPart should not be invoked";
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
	save(fileName, streamWriter, asPart);
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

void ModelBase::save(const QString & fileName, QXmlStreamWriter & streamWriter, bool asPart) {
    streamWriter.setAutoFormatting(true);
    if(asPart) {
    	m_root->saveAsPart(streamWriter, true);
    } else {
    	m_root->saveInstances(fileName, streamWriter, true);
    }
}

bool ModelBase::paste(ModelBase * refModel, QByteArray & data, QList<ModelPart *> & modelParts, QHash<QString, QRectF> & boundingRects, bool preserveIndex)
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

	QDomElement boundingRectsElement = module.firstChildElement("boundingRects");
	if (!boundingRectsElement.isNull()) {
		QDomElement boundingRect = boundingRectsElement.firstChildElement("boundingRect");
		while (!boundingRect.isNull()) {
			QString name = boundingRect.attribute("name");
			QString rect = boundingRect.attribute("rect");
			QRectF br;
			if (!rect.isEmpty()) {
				QStringList s = rect.split(" ");
				if (s.count() == 4) {
					QRectF r(s[0].toDouble(), s[1].toDouble(), s[2].toDouble(), s[3].toDouble());
					br = r;
				}
			}
			boundingRects.insert(name, br);
			boundingRect = boundingRect.nextSiblingElement("boundingRect");
		}
	}

	QDomElement instances = module.firstChildElement("instances");
   	if (instances.isNull()) {
   		return false;
	}

	if (!preserveIndex) {
		// need to map modelIndexes from copied parts to new modelIndexes
		QHash<long, long> oldToNew;
		QDomElement instance = instances.firstChildElement("instance");
		while (!instance.isNull()) {
			long oldModelIndex = instance.attribute("modelIndex").toLong();
			oldToNew.insert(oldModelIndex, ModelPart::nextIndex());
			instance = instance.nextSiblingElement("instance");
		}
		renewModelIndexes(instances, "instance", oldToNew);
	}

	//QFile file("test.xml");
	//file.open(QFile::WriteOnly);
	//file.write(domDocument.toByteArray());
	//file.close();

	return loadInstances(domDocument, instances, modelParts, false);
}

void ModelBase::renewModelIndexes(QDomElement & parentElement, const QString & childName, QHash<long, long> & oldToNew)
{
	QDomElement instance = parentElement.firstChildElement(childName);
	while (!instance.isNull()) {
		long oldModelIndex = instance.attribute("modelIndex").toLong();
		instance.setAttribute("modelIndex", QString::number(oldToNew.value(oldModelIndex)));
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
									long newModelIndex = oldToNew.value(oldModelIndex, -1);
									if (newModelIndex != -1) {
										connect.setAttribute("modelIndex", QString::number(newModelIndex));
									}
									else {
										//DebugDialog::debug(QString("keep old model index %1").arg(oldModelIndex));
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

bool ModelBase::isRatsnest(QDomElement & instance) {
	return PartFactory::isRatsnest(instance);
}

void ModelBase::setReportMissingModules(bool b) {
	m_reportMissingModules = b;
}

bool ModelBase::genFZP(const QString & moduleID, ModelBase * refModel) {
	QString path = PartFactory::getFzpFilename(moduleID);
	if (path.isEmpty()) return false;

	ModelPart* mp = refModel->addPart(path, true, true);
	return mp != NULL;
}


