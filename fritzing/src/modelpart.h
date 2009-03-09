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


#ifndef MODELPART_H
#define MODELPART_H

#include <QString>
#include <QPointF>
#include <QSize>
#include <QGraphicsItem>
#include <QDomDocument>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QHash>
#include <QList>

#include "itembase.h"
#include "modelpartshared.h"
#include "connector.h"
#include "svgandpartfilepath.h"

class ModelPart : public QObject
{
	Q_OBJECT

public:
	enum ItemType {
		 Part,
		 Wire,
		 Breadboard,
		 Board,
		 Module,
		 Note,
		 Unknown
	};

public:
	ModelPart(QDomDocument *, const QString& path, ItemType type);
	ModelPart(ItemType type = ModelPart::Unknown);
	~ModelPart();

	ItemType itemType() const { return m_type; };
	void setItemType(ItemType);
	const QString & moduleID();
	void copy(ModelPart *);
	void copyNew(ModelPart *);
	void copyStuff(ModelPart * modelPart);
	ModelPartShared * modelPartShared();
	void setModelPartShared(ModelPartShared *modelPartShared);
	void saveInstances(QXmlStreamWriter & streamWriter, bool startDocument);
	void saveAsPart(QXmlStreamWriter & streamWriter, bool startDocument);
	void addViewItem(ItemBase *);
	void removeViewItem(ItemBase *);
	ItemBase * viewItem(QGraphicsScene * scene);
	void initConnectors(bool force=false);
	const QHash<QString, Connector *> & connectors();
	long modelIndex();
	void setModelIndex(long index);
	void setInstanceDomElement(const QDomElement &);
	const QDomElement & instanceDomElement();
	Connector * getConnector(const QString & id);

	const QString & title();
	const QStringList & tags();
	const QHash<QString,QString> & properties() const;
	const QHash<QString, class Bus *> & buses();

	class Bus * bus(const QString & busID);
	bool ignoreTerminalPoints();

	bool isCore();
	void setCore(bool core);

	bool isAlien(); // from "outside" ?
	void setAlien(bool alien);

	bool isValid();

	QList<ModelPart*> getAllNonCoreParts();
	QList<SvgAndPartFilePath> getAvailableViewFiles();
	bool hasViewID(long id);

	const QString & instanceTitle();
	const QString & instanceText();
	void setInstanceTitle(QString);
	void setInstanceText(QString);

public:
	static const QString & itemTypeName(ModelPart::ItemType);
	static const QString & itemTypeName(int);
	static void initNames();
	static long nextIndex();


protected:
	void writeTag(QXmlStreamWriter & streamWriter, QString tagName, QString tagValue);
	void writeNestedTag(QXmlStreamWriter & streamWriter, QString tagName, const QStringList &values, QString childTag);
	void writeNestedTag(QXmlStreamWriter & streamWriter, QString tagName, const QHash<QString,QString> &values, QString childTag, QString attrName);

	void grabImagePath(QHash<ItemBase::ViewIdentifier, SvgAndPartFilePath> &viewImages, QDomElement &viewsElems, ItemBase::ViewIdentifier viewId);
	QString inWhichFolder(const QString &partspath, const QString &imagepath);

	QList<ItemBase *> m_viewItems;

	ItemType m_type;
	ModelPartShared *m_modelPartShared;
	QHash<QString, Connector *> m_connectorHash;
	QHash<QString, class Bus *> m_busHash;
	long m_index;						// only used at save time to identify model parts in the xml
	QDomElement m_instanceDomElement;	// only used at load time (so far)

	bool m_core;
	bool m_valid;
	bool m_alien;
	bool m_originalModelPartShared;

	QString m_instanceTitle;
	QString m_instanceText;

	static QHash<ItemType, QString> itemTypeNames;
	static long m_nextIndex;


};

Q_DECLARE_METATYPE( ModelPart * );			// so we can stash them in a QVariant

#endif
