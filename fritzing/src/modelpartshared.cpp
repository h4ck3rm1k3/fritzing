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

#include "modelpartshared.h"
#include "connectors/connectorshared.h"
#include "debugdialog.h"
#include "connectors/busshared.h"

#include <QHash>

ModelPartShared::ModelPartShared() {
	m_moduleID = "";
	m_domDocument = NULL;
	m_path = "";
	m_connectorsInitialized = false;
	m_ignoreTerminalPoints = false;
}

ModelPartShared::ModelPartShared(QDomDocument * domDocument, const QString & path) {
	m_moduleID = "";
	m_domDocument = domDocument;
	m_path = path;
	m_connectorsInitialized = false;
	m_ignoreTerminalPoints = false;

	QDomElement root = domDocument->documentElement();
	if (root.isNull()) {
		return;
	}

	if (root.tagName() != "module") {
		return;
	}

	loadTagText(root, "title", m_title);
	loadTagText(root, "label", m_label);
	loadTagText(root, "version", m_version);
	loadTagText(root, "author", m_author);
	loadTagText(root, "description", m_description);
	loadTagText(root, "taxonomy", m_taxonomy);
	loadTagText(root, "date", m_date);

	populateTagCollection(root, m_tags, "tags");
	populateTagCollection(root, m_properties, "properties", "name");

	m_moduleID = root.attribute("moduleId", "");
}

ModelPartShared::~ModelPartShared() {
	foreach (ConnectorShared * connectorShared, m_connectorSharedHash.values()) {
		delete connectorShared;
	}
	m_connectorSharedHash.clear();

	foreach (ConnectorShared * connectorShared, m_deletedList) {
		delete connectorShared;
	}
	m_deletedList.clear();

	foreach (BusShared * busShared, m_buses.values()) {
		delete busShared;
	}
	m_buses.clear();

	if (m_domDocument) {
		delete m_domDocument;
		m_domDocument = NULL;
	}
}

void ModelPartShared::loadTagText(QDomElement parent, QString tagName, QString &field) {
	QDomElement tagElement = parent.firstChildElement(tagName);
	if (!tagElement.isNull()) {
		field = tagElement.text();
	}
}

void ModelPartShared::populateTagCollection(QDomElement parent, QStringList &list, const QString &tagName) {
	QDomElement bag = parent.firstChildElement(tagName);
	if (!bag.isNull()) {
		QDomNodeList childs = bag.childNodes();
		for(int i = 0; i < childs.size(); i++) {
			QDomNode child = childs.item(i);
			if(!child.isComment()) {
				list << child.toElement().text();
			}
		}
	}
}

void ModelPartShared::populateTagCollection(QDomElement parent, QHash<QString,QString> &hash, const QString &tagName, const QString &attrName) {
	QDomElement bag = parent.firstChildElement(tagName);
	if (!bag.isNull()) {
		QDomNodeList childs = bag.childNodes();
		for(int i = 0; i < childs.size(); i++) {
			QDomNode child = childs.item(i);
			if(!child.isComment()) {
				QString name = child.toElement().attribute(attrName);
				QString value = child.toElement().text();
				hash.insert(name.toLower().trimmed(),value);
			}
		}
	}
}

void ModelPartShared::setDomDocument(QDomDocument * domDocument) {
	if (m_domDocument) {
		delete m_domDocument;
	}
	m_domDocument = domDocument;
}

QDomDocument* ModelPartShared::domDocument() {
	return m_domDocument;
}

const QString & ModelPartShared::title() {
	return m_title;
}
void ModelPartShared::setTitle(QString title) {
	m_title = title;
}

const QString & ModelPartShared::label() {
	return m_label;
}
void ModelPartShared::setLabel(QString label) {
	m_label = label;
}

const QString & ModelPartShared::uri() {
	return m_uri;
}
void ModelPartShared::setUri(QString uri) {
	m_uri = uri;
}

const QString & ModelPartShared::version() {
	return m_version;
}
void ModelPartShared::setVersion(QString version) {
	m_version = version;
}

const QString & ModelPartShared::author() {
	return m_author;
}
void ModelPartShared::setAuthor(QString author) {
	m_author = author;
}

const QString & ModelPartShared::description() {
	return m_description;
}
void ModelPartShared::setDescription(QString description) {
	m_description = description;
}

const QDate & ModelPartShared::date() {
	// 	return *new QDate(QDate::fromString(m_date,Qt::ISODate));   // causes memory leak
	static QDate tempDate;
	tempDate = QDate::fromString(m_date,Qt::ISODate);
	return tempDate;
}

void ModelPartShared::setDate(QDate date) {
	m_date = date.toString(Qt::ISODate);
}
const QString & ModelPartShared::dateAsStr() {
	return m_date;
}
void ModelPartShared::setDate(QString date) {
	m_date = date;
}

const QStringList & ModelPartShared::tags() {
	return m_tags;
}
void ModelPartShared::setTags(const QStringList &tags) {
	m_tags = tags;
}

QString ModelPartShared::family() {
	return m_properties.value("family");
}
void ModelPartShared::setFamily(const QString &family) {
	m_properties.insert("family",family);
}

QHash<QString,QString> & ModelPartShared::properties() {
	return m_properties;
}
void ModelPartShared::setProperties(const QMultiHash<QString,QString> &properties) {
	m_properties = properties;
}

const QString & ModelPartShared::path() {
	return m_path;
}
void ModelPartShared::setPath(QString path) {
	m_path = path;
}

const QString & ModelPartShared::taxonomy() {
	return m_taxonomy;
}
void ModelPartShared::setTaxonomy(QString taxonomy) {
	m_taxonomy = taxonomy;
}

const QString & ModelPartShared::moduleID() {
	return m_moduleID;
}
void ModelPartShared::setModuleID(QString moduleID) {
	m_moduleID = moduleID;
}

const QList<ConnectorShared *> ModelPartShared::connectors() {
	return m_connectorSharedHash.values();
}
void ModelPartShared::setConnectorsShared(QList<ConnectorShared *> connectors) {
	for (int i = 0; i < connectors.size(); i++) {
		ConnectorShared* cs = connectors[i];
		m_connectorSharedHash[cs->id()] = cs;
	}
}

void ModelPartShared::resetConnectorsInitialization() {
	m_connectorsInitialized = false;

	foreach (ConnectorShared * cs, m_connectorSharedHash.values()) {
		m_deletedList.append(cs);
	}
	m_connectorSharedHash.clear();
}

void ModelPartShared::initConnectors() {
	if (m_domDocument == NULL)
		return;

	if (m_connectorsInitialized)
		return;

	m_connectorsInitialized = true;
	QDomElement root = m_domDocument->documentElement();
	if (root.isNull()) {
		return;
	}

	QDomElement connectors = root.firstChildElement("connectors");
	if (connectors.isNull())
		return;

	m_ignoreTerminalPoints = (connectors.attribute("ignoreTerminalPoints").compare("true", Qt::CaseInsensitive) == 0);

	//DebugDialog::debug(QString("part:%1 %2").arg(m_moduleID).arg(m_title));
	QDomElement connector = connectors.firstChildElement("connector");
	while (!connector.isNull()) {
		ConnectorShared * connectorShared = new ConnectorShared(connector);
		m_connectorSharedHash.insert(connectorShared->id(), connectorShared);

		connector = connector.nextSiblingElement("connector");
	}

	QDomElement buses = root.firstChildElement("buses");
	if (!buses.isNull()) {
		QDomElement busElement = buses.firstChildElement("bus");
		while (!busElement.isNull()) {
			BusShared * busShared = new BusShared(busElement, m_connectorSharedHash);
			m_buses.insert(busShared->id(), busShared);

			busElement = busElement.nextSiblingElement("bus");
		}
	}

	//DebugDialog::debug(QString("model %1 has %2 connectors and %3 bus connectors").arg(this->title()).arg(m_connectorSharedHash.count()).arg(m_buses.count()) );


}

const QHash<QString, BusShared *> & ModelPartShared::buses() {
	return m_buses;
}

ConnectorShared * ModelPartShared::getConnectorShared(const QString & id) {
	return m_connectorSharedHash.value(id);
}

BusShared * ModelPartShared::bus(const QString & busID) {
	return m_buses.value(busID);
}

bool ModelPartShared::ignoreTerminalPoints() {
	return m_ignoreTerminalPoints;
}

void ModelPartShared::copy(ModelPartShared* other) {
	setAuthor(other->author());
	setConnectorsShared(other->connectors());
	setDate(other->date());
	setLabel(other->label());
	setDescription(other->description());
	setFamily(other->family());
	setProperties(other->properties());
	setTags(other->tags());
	setTaxonomy(other->taxonomy());
	setTitle(other->title());
	setUri(other->uri());
	setVersion(other->version());
}

void ModelPartShared::setProperty(const QString & key, const QString & value) {
	m_properties.insert(key, value);
}
