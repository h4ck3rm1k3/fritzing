/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "modelpart.h"
#include "debugdialog.h"
#include "connectorstuff.h"
#include "busstuff.h"
#include "bus.h"
#include "version.h"

#include <QDomElement>

QHash<ModelPart::ItemType, QString> ModelPart::itemTypeNames;
long ModelPart::nextIndex = 0;


ModelPart::ModelPart(ItemType type)
	: QObject()
{
	m_type = type;
	m_modelPartStuff = NULL;
	m_partInstanceStuff = NULL;
	m_index = nextIndex++;
	m_core = false;
	m_alien = false;
}

ModelPart::ModelPart(QDomDocument * domDocument, const QString & path, ItemType type)
	: QObject()
{
	m_type = type;
	m_modelPartStuff = new ModelPartStuff(domDocument, path);
	m_partInstanceStuff = new PartInstanceStuff(domDocument, path);
	m_core = false;
	m_alien = false;

	//TODO Mariano: enough for now
	QDomElement viewsElems = domDocument->documentElement().firstChildElement("views");
	if(!viewsElems.isNull()) {
		m_valid = !viewsElems.firstChildElement(ItemBase::viewIdentifierXmlName(ItemBase::IconView)).isNull();
	} else {
		m_valid = false;
	}
}

ModelPart::~ModelPart() {
	foreach (ItemBase * itemBase, m_viewItems) {
		itemBase->clearModelPart();
	}
}

const QString & ModelPart::moduleID() {
	if (m_modelPartStuff != NULL) return m_modelPartStuff->moduleID();

	return ___emptyString___;
}


const QString & ModelPart::itemTypeName(ModelPart::ItemType itemType) {
	return itemTypeNames[itemType];
}

const QString & ModelPart::itemTypeName(int itemType) {
	return itemTypeNames[(ModelPart::ItemType) itemType];
}

void ModelPart::initNames() {
	if (itemTypeNames.count() == 0) {
		itemTypeNames.insert(ModelPart::Part, QObject::tr("part"));
		itemTypeNames.insert(ModelPart::Wire, QObject::tr("wire"));
		itemTypeNames.insert(ModelPart::Breadboard, QObject::tr("breadboard"));
		itemTypeNames.insert(ModelPart::Board, QObject::tr("board"));
		itemTypeNames.insert(ModelPart::Module, QObject::tr("module"));
	}
}

void ModelPart::setItemType(ItemType t) {
	m_type = t;
}


void ModelPart::copy(ModelPart * modelPart) {
	m_type = modelPart->itemType();
	m_modelPartStuff = modelPart->modelPartStuff();
	m_core = modelPart->isCore();
}

void ModelPart::copyNew(ModelPart * modelPart) {
	copy(modelPart);
}

void ModelPart::copyStuff(ModelPart * modelPart) {
	modelPartStuff()->copy(modelPart->modelPartStuff());
}

ModelPartStuff * ModelPart::modelPartStuff() {
	if(!m_modelPartStuff) {
		m_modelPartStuff = new ModelPartStuff();
	}
	return m_modelPartStuff;
}
void ModelPart::setModelPartStuff(ModelPartStuff * modelPartStuff) {
	m_modelPartStuff = modelPartStuff;
}

PartInstanceStuff * ModelPart::partInstanceStuff() {
	if(!m_partInstanceStuff) {
		m_partInstanceStuff = new PartInstanceStuff();
	}
	return m_partInstanceStuff;
}
void ModelPart::setPartInstanceStuff(PartInstanceStuff * partInstanceStuff) {
	m_partInstanceStuff = partInstanceStuff;
}

void ModelPart::addViewItem(ItemBase * item) {
	m_viewItems.append(item);
}

void ModelPart::removeViewItem(ItemBase * item) {
	m_viewItems.removeOne(item);
}

ItemBase * ModelPart::viewItem(QGraphicsScene * scene) {
	foreach (ItemBase * itemBase, m_viewItems) {
		if (itemBase->scene() == scene) return itemBase;
	}

	return NULL;
}

void ModelPart::saveInstances(QXmlStreamWriter & streamWriter, bool startDocument) {
	if (startDocument) {
		streamWriter.writeStartDocument();
    	streamWriter.writeStartElement("module");
		streamWriter.writeAttribute("fritzingVersion", Version::versionString());
		QString title = this->modelPartStuff()->title();
		if(!title.isNull() && !title.isEmpty()) {
			streamWriter.writeTextElement("title",title);
		}
		streamWriter.writeStartElement("instances");
	}

	if (m_viewItems.size() > 0) {
		streamWriter.writeStartElement("instance");
		if (m_modelPartStuff != NULL) {
			const QString & moduleIdRef = m_modelPartStuff->moduleID();
			streamWriter.writeAttribute("moduleIdRef", moduleIdRef);
			streamWriter.writeAttribute("modelIndex", QString::number(m_index));
			streamWriter.writeAttribute("path", m_modelPartStuff->path());
		}
		if (m_partInstanceStuff != NULL) {
			QString title = m_partInstanceStuff->title();
			if(!title.isNull() && !title.isEmpty()) {
				writeTag(streamWriter,"title",m_partInstanceStuff->title());
			}
		}

		// tell the views to write themselves out
		streamWriter.writeStartElement("views");
		foreach (ItemBase * itemBase, m_viewItems) {
			itemBase->saveInstance(streamWriter);
		}
		streamWriter.writeEndElement();		// views
		streamWriter.writeEndElement();		//instance
	}

	QList<QObject *>::const_iterator i;
    for (i = children().constBegin(); i != children().constEnd(); ++i) {
		ModelPart* mp = qobject_cast<ModelPart *>(*i);
		if (mp == NULL) continue;

		mp->saveInstances(streamWriter, false);
	}

	if (startDocument) {
		streamWriter.writeEndElement();	  //  instances
		streamWriter.writeEndElement();   //  module
		streamWriter.writeEndDocument();
	}
}

void ModelPart::writeTag(QXmlStreamWriter & streamWriter, QString tagName, QString tagValue) {
	if(!tagValue.isEmpty()) {
		streamWriter.writeTextElement(tagName,tagValue);
	}
}

void ModelPart::writeNestedTag(QXmlStreamWriter & streamWriter, QString tagName, const QStringList &values, QString childTag) {
	if(values.count() > 0) {
		streamWriter.writeStartElement(tagName);
		for(int i=0; i<values.count(); i++) {
			writeTag(streamWriter, childTag, values[i]);
		}
		streamWriter.writeEndElement();
	}
}

void ModelPart::writeNestedTag(QXmlStreamWriter & streamWriter, QString tagName, const QHash<QString,QString> &values, QString childTag, QString attrName) {
	streamWriter.writeStartElement(tagName);
	for(int i=0; i<values.keys().count(); i++) {
		streamWriter.writeStartElement(childTag);
		QString key = values.keys()[i];
		streamWriter.writeAttribute(attrName,key);
		streamWriter.writeCharacters(values[key]);
		streamWriter.writeEndElement();
	}
	streamWriter.writeEndElement();
}

void ModelPart::saveAsPart(QXmlStreamWriter & streamWriter, bool startDocument) {
	if (startDocument) {
		streamWriter.writeStartDocument();
    	streamWriter.writeStartElement("module");
		streamWriter.writeAttribute("fritzingVersion", Version::versionString());
		streamWriter.writeAttribute("moduleId", m_modelPartStuff->moduleID());
    	writeTag(streamWriter,"version",m_modelPartStuff->version());
    	writeTag(streamWriter,"author",m_modelPartStuff->author());
    	writeTag(streamWriter,"title",m_modelPartStuff->title());
    	writeTag(streamWriter,"label",m_modelPartStuff->label());
    	writeTag(streamWriter,"date",m_modelPartStuff->dateAsStr());

    	writeNestedTag(streamWriter,"tags",m_modelPartStuff->tags(),"tag");
    	writeNestedTag(streamWriter,"properties",m_modelPartStuff->properties(),"property","name");

    	writeTag(streamWriter,"taxonomy",m_modelPartStuff->taxonomy());
    	writeTag(streamWriter,"description",m_modelPartStuff->description());
	}

	if (m_viewItems.size() > 0) {
		if (startDocument) {
			streamWriter.writeStartElement("views");
		}
		for (int i = 0; i < m_viewItems.size(); i++) {
				ItemBase * item = m_viewItems[i];
				item->writeXml(streamWriter);
		}

		if(startDocument) {
			streamWriter.writeEndElement();
		}

		streamWriter.writeStartElement("connectors");
		const QList<ConnectorStuff *> connectors = m_modelPartStuff->connectors();
		for (int i = 0; i < connectors.count(); i++) {
			Connector * connector = new Connector(connectors[i], this);
			connector->saveAsPart(streamWriter);
			delete connector;
		}
		streamWriter.writeEndElement();
	}

	QList<QObject *>::const_iterator i;
    for (i = children().constBegin(); i != children().constEnd(); ++i) {
		ModelPart * mp = qobject_cast<ModelPart *>(*i);
		if (mp == NULL) continue;

		mp->saveAsPart(streamWriter, false);
	}

	if (startDocument) {
		streamWriter.writeEndElement();
		streamWriter.writeEndElement();
		streamWriter.writeEndDocument();
	}
}

void ModelPart::initConnectors(bool force) {
	if(m_modelPartStuff == NULL) return;
	if(force) {
		m_connectorHash.clear();
		m_modelPartStuff->resetConnectorsInitialization();
	}
	if(m_connectorHash.count() > 0) return;		// already done

	m_modelPartStuff->initConnectors();
	foreach (ConnectorStuff * connectorStuff, m_modelPartStuff->connectors()) {
		Connector * connector = new Connector(connectorStuff, this);
		m_connectorHash.insert(connectorStuff->id(), connector);
		BusStuff * busStuff = connectorStuff->bus();
		if (busStuff != NULL) {
			Bus * bus = m_busHash.value(busStuff->id());
			if (bus == NULL) {
				bus = new Bus(busStuff, this);
				m_busHash.insert(busStuff->id(), bus);
			}
			connector->setBus(bus);
			bus->addConnector(connector);
		}
	}
}

const QHash<QString, Connector *> & ModelPart::connectors() {
	return m_connectorHash;
}

long ModelPart::modelIndex() {
	return m_index;
}

void ModelPart::setModelIndex(long index) {
	m_index = index;
	if (index >= nextIndex) {
		nextIndex = index + 1;
	}
}

void ModelPart::initConnections(QHash<long, ModelPart *> & partHash) {
	if (m_instanceDomElement.isNull()) return;

	QDomElement connectors = m_instanceDomElement.firstChildElement("connectors");
	if (connectors.isNull()) return;

	QDomElement connectorElement = connectors.firstChildElement("connector");
	while (!connectorElement.isNull()) {
		Connector * connector = m_connectorHash.value(connectorElement.attribute("connectorId"));
		if (connector != NULL){
			QDomElement connectElement = connectorElement.firstChildElement("connect");
			while (!connectElement.isNull()) {
				QString connectorID = connectElement.attribute("connectorId");
				bool ok;
				int modelIndex = connectElement.attribute("modelIndex").toLong(&ok);
				if (ok){
					ModelPart * modelPart = partHash.value(modelIndex);
					if (modelPart != NULL) {
						Connector * otherConnector = modelPart->connectors().value(connectorID);
						if (otherConnector != NULL) {
							connector->connectTo(otherConnector);
						}
						else {
							QString busID = connectElement.attribute("busId");
							Bus * theBus = modelPart->bus(busID);
							if (theBus != NULL) {
								Connector * busConnector = theBus->busConnector();
								if (busConnector != NULL) {
									connector->connectTo(busConnector);
								}
							}
						}
					}
				}
				connectElement = connectElement.nextSiblingElement("connect");
			}
		}
		connectorElement = connectorElement.nextSiblingElement("connector");
	}

}

void ModelPart::setInstanceDomElement(const QDomElement & domElement) {
	m_instanceDomElement = domElement;
}

const QDomElement & ModelPart::instanceDomElement() {
	return m_instanceDomElement;
}

const QString & ModelPart::title() {
	if (m_modelPartStuff != NULL) return m_modelPartStuff->title();

	return ___emptyString___;
}

const QStringList & ModelPart::tags() {
	if (m_modelPartStuff != NULL) return m_modelPartStuff->tags();

	return ___emptyStringList___;
}

const QHash<QString,QString> & ModelPart::properties() {
	if (m_modelPartStuff != NULL) return m_modelPartStuff->properties();

	return ___emptyStringHash___;
}

Connector * ModelPart::getConnector(const QString & id) {
	return m_connectorHash.value(id);
}

const QHash<QString, Bus *> & ModelPart::buses() {
	return  m_busHash;
}

Bus * ModelPart::bus(const QString & busID) {
	return m_busHash.value(busID);
}

bool ModelPart::ignoreTerminalPoints() {
	if (m_modelPartStuff != NULL) return m_modelPartStuff->ignoreTerminalPoints();

	return true;
}

bool ModelPart::isCore() {
	return m_core;
}

void ModelPart::setCore(bool core) {
	m_core = core;
}

bool ModelPart::isAlien() {
	return m_alien;
}

void ModelPart::setAlien(bool alien) {
	m_alien = alien;
}

bool ModelPart::isValid() {
	return m_valid;
}

QList<ModelPart*> ModelPart::getAllNonCoreParts() {
	QList<ModelPart*> retval;
	QList<QObject *>::const_iterator i;
	for (i = children().constBegin(); i != children().constEnd(); ++i) {
		ModelPart* mp = qobject_cast<ModelPart *>(*i);
		if (mp == NULL) continue;

		if(!mp->isCore()) {
			retval << mp;
		}
	}

	return retval;
}

QList<SvgAndPartFilePath> ModelPart::getAvailableViewFiles() {
	QDomElement viewsElems = modelPartStuff()->domDocument()->documentElement().firstChildElement("views");
	QHash<ItemBase::ViewIdentifier, SvgAndPartFilePath> viewImages;

	grabImagePath(viewImages, viewsElems, ItemBase::IconView);
	grabImagePath(viewImages, viewsElems, ItemBase::BreadboardView);
	grabImagePath(viewImages, viewsElems, ItemBase::SchematicView);
	grabImagePath(viewImages, viewsElems, ItemBase::PCBView);

	return viewImages.values();
}

void ModelPart::grabImagePath(QHash<ItemBase::ViewIdentifier, SvgAndPartFilePath> &viewImages, QDomElement &viewsElems, ItemBase::ViewIdentifier viewId) {
	QDomElement viewElem = viewsElems.firstChildElement(ItemBase::viewIdentifierXmlName(viewId));
	if(!viewElem.isNull()) {
		QString partspath = getApplicationSubFolderPath("parts")+"/svg";
		QDomElement layerElem = viewElem.firstChildElement("layers");
		if (!layerElem.isNull()) {
			QString imagepath = layerElem.attribute("image");
			QString folderinparts = inWhichFolder(partspath, imagepath);
			if(folderinparts != ___emptyString___) {
				SvgAndPartFilePath st(partspath,folderinparts,imagepath);
				viewImages[viewId] = st;
			}
		}
	}
}

QString ModelPart::inWhichFolder(const QString &partspath, const QString &imagepath) {
	QStringList possibleFolders;
	possibleFolders << "core" << "contrib" << "user";
	for(int i=0; i < possibleFolders.size(); i++) {
		if (QFileInfo( partspath+"/"+possibleFolders[i]+"/"+imagepath ).exists()) {
			return possibleFolders[i];
		}
	}
	return ___emptyString___;
}
