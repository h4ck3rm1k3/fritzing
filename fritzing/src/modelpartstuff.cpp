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

#include "modelpartstuff.h"
#include "connectorstuff.h"
#include "debugdialog.h"
#include "busstuff.h"

#include <QHash>

ModelPartStuff::ModelPartStuff() {
	m_moduleID = "";
	m_domDocument = NULL;
	m_path = "";
	m_connectorsInitialized = false;
	m_ignoreTerminalPoints = false;
}

ModelPartStuff::ModelPartStuff(QDomDocument * domDocument, const QString & path) {
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

	loadText(root, "title", m_title);
	loadText(root, "label", m_label);
	loadText(root, "version", m_version);
	loadText(root, "author", m_author);
	loadText(root, "description", m_description);
	loadText(root, "taxonomy", m_taxonomy);
	loadText(root, "date", m_date);

	populateTagCollection(root, m_tags, "tags");
	populateTagCollection(root, m_properties, "properties", "name");

	m_moduleID = root.attribute("moduleId", "");
}

void ModelPartStuff::loadText(QDomElement parent, QString tagName, QString &field) {
	QDomElement tagElement = parent.firstChildElement(tagName);
	if (!tagElement.isNull()) {
		field = tagElement.text();
	}
}

void ModelPartStuff::populateTagCollection(QDomElement parent, QStringList &list, const QString &tagName) {
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

void ModelPartStuff::populateTagCollection(QDomElement parent, QMultiHash<QString,QString> &hash, const QString &tagName, const QString &attrName) {
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

void ModelPartStuff::setDomDocument(QDomDocument * domDocument) {
	m_domDocument = domDocument;
}

QDomDocument* ModelPartStuff::domDocument() {
	return m_domDocument;
}

const QString & ModelPartStuff::title() {
	return m_title;
}
void ModelPartStuff::setTitle(QString title) {
	m_title = title;
}

const QString & ModelPartStuff::label() {
	return m_label;
}
void ModelPartStuff::setLabel(QString label) {
	m_label = label;
}

const QString & ModelPartStuff::uri() {
	return m_uri;
}
void ModelPartStuff::setUri(QString uri) {
	m_uri = uri;
}

const QString & ModelPartStuff::version() {
	return m_version;
}
void ModelPartStuff::setVersion(QString version) {
	m_version = version;
}

const QString & ModelPartStuff::author() {
	return m_author;
}
void ModelPartStuff::setAuthor(QString author) {
	m_author = author;
}

const QString & ModelPartStuff::description() {
	return m_description;
}
void ModelPartStuff::setDescription(QString description) {
	m_description = description;
}

const QDate & ModelPartStuff::date() {
	return *new QDate(QDate::fromString(m_date,Qt::ISODate));
}
void ModelPartStuff::setDate(QDate date) {
	m_date = date.toString(Qt::ISODate);
}
const QString & ModelPartStuff::dateAsStr() {
	return m_date;
}
void ModelPartStuff::setDate(QString date) {
	m_date = date;
}

const QStringList & ModelPartStuff::tags() {
	return m_tags;
}
void ModelPartStuff::setTags(const QStringList &tags) {
	m_tags = tags;
}

QString ModelPartStuff::family() {
	return m_properties.value("family");
}
void ModelPartStuff::setFamily(const QString &family) {
	m_properties.insert("family",family);
}

QMultiHash<QString,QString> & ModelPartStuff::properties() {
	return m_properties;
}
void ModelPartStuff::setProperties(const QMultiHash<QString,QString> &properties) {
	m_properties = properties;
}

const QString & ModelPartStuff::path() {
	return m_path;
}
void ModelPartStuff::setPath(QString path) {
	m_path = path;
}

const QString & ModelPartStuff::taxonomy() {
	return m_taxonomy;
}
void ModelPartStuff::setTaxonomy(QString taxonomy) {
	m_taxonomy = taxonomy;
}

const QString & ModelPartStuff::moduleID() {
	return m_moduleID;
}
void ModelPartStuff::setModuleID(QString moduleID) {
	m_moduleID = moduleID;
}

const QList<ConnectorStuff *> ModelPartStuff::connectors() {
	return m_connectorStuffHash.values();
}
void ModelPartStuff::setConnectorsStuff(QList<ConnectorStuff *> connectors) {
	for (int i = 0; i < connectors.size(); i++) {
		ConnectorStuff* cs = connectors[i];
		m_connectorStuffHash[cs->id()] = cs;
	}
}

void ModelPartStuff::resetConnectorsInitialization() {
	m_connectorsInitialized = false;
	m_connectorStuffHash.clear();
}

void ModelPartStuff::initConnectors() {
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

	QDomElement connector = connectors.firstChildElement("connector");
	while (!connector.isNull()) {
		ConnectorStuff * connectorStuff = new ConnectorStuff(connector);
		m_connectorStuffHash.insert(connectorStuff->id(), connectorStuff);

		connector = connector.nextSiblingElement("connector");
	}

	QDomElement buses = root.firstChildElement("buses");
	if (!buses.isNull()) {
		QDomElement busElement = buses.firstChildElement("bus");
		while (!busElement.isNull()) {
			BusStuff * bus = new BusStuff(busElement, m_connectorStuffHash);
			m_buses.insert(bus->id(), bus);

			busElement = busElement.nextSiblingElement("bus");
		}
	}

	DebugDialog::debug(QString("model %1 has %2 connectors and %3 bus connectors").arg(this->title()).arg(m_connectorStuffHash.count()).arg(m_buses.count()) );


}

const QHash<QString, BusStuff *> & ModelPartStuff::buses() {
	return m_buses;
}

ConnectorStuff * ModelPartStuff::getConnectorStuff(const QString & id) {
	return m_connectorStuffHash.value(id);
}

BusStuff * ModelPartStuff::bus(const QString & busID) {
	return m_buses.value(busID);
}

bool ModelPartStuff::ignoreTerminalPoints() {
	return m_ignoreTerminalPoints;
}

void ModelPartStuff::copy(ModelPartStuff* other) {
	setAuthor(other->author());
	setConnectorsStuff(other->connectors());
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


