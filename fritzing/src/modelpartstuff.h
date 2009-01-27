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

#ifndef MODELPARTSTUFF_H
#define MODELPARTSTUFF_H

#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QStringList>
#include <QHash>
#include <QDate>


class ModelPartStuff
{

public:
	ModelPartStuff();
	ModelPartStuff(QDomDocument *, const QString & path);
	void setDomDocument(QDomDocument *);
	QDomDocument * domDocument();

	void copy(ModelPartStuff* other);

	const QString & uri();
	void setUri(QString uri);
	const QString & moduleID();
	void setModuleID(QString moduleID);
	const QString & version();
	void setVersion(QString version);
	const QString & author();
	void setAuthor(QString author);
	const QString & description();
	void setDescription(QString description);
	const QString & title();
	void setTitle(QString title);
	const QString & label();
	void setLabel(QString label);
	const QDate & date();
	void setDate(QDate date);
	const QString & dateAsStr();
	void setDate(QString date);

	const QString & path();
	void setPath(QString path);
	const QString & taxonomy();
	void setTaxonomy(QString taxonomy);

	const QList<class ConnectorStuff *> connectors();
	void setConnectorsStuff(QList<class ConnectorStuff *> connectors);

	const QStringList &tags();
	void setTags(const QStringList &tags);

	QString family();
	void setFamily(const QString &family);

	QMultiHash<QString,QString> & properties();
	void setProperties(const QMultiHash<QString,QString> &properties);

	void initConnectors();
	void resetConnectorsInitialization();
	const QHash<QString, class BusStuff *> & buses();
	ConnectorStuff * getConnectorStuff(const QString & id);
	BusStuff * bus(const QString & busID);
	bool ignoreTerminalPoints();

protected:
	void loadTagText(QDomElement parent, QString tagName, QString &field);
	// used to populate de StringList that contains both the <tags> and the <properties> values
	void populateTagCollection(QDomElement parent, QStringList &list, const QString &tagName);
	void populateTagCollection(QDomElement parent, QMultiHash<QString,QString> &hash, const QString &tagName, const QString &attrName);

	QDomDocument* m_domDocument;

	QString m_uri;
	QString m_moduleID;
	QString m_version;
	QString m_author;
	QString m_title;
	QString m_label;
	QString m_description;
	QString m_date;

	QString m_path;
	QString m_taxonomy;

	QStringList m_tags;
	QMultiHash<QString,QString> m_properties;

	QHash<QString, class ConnectorStuff *> m_connectorStuffHash;
	QHash<QString, class BusStuff *> m_buses;

	bool m_connectorsInitialized;
	bool m_ignoreTerminalPoints;
};


#endif
