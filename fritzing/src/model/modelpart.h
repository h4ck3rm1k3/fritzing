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
#include <QPointer>

#include "modelpartshared.h"
#include "../connectors/connector.h"
#include "../connectors/bus.h"
#include "../utils/svgandpartfilepath.h"

class ModelPart : public QObject
{
	Q_OBJECT

public:
	enum ItemType {
		 Part,
		 Wire,
		 Breadboard,
		 Board,
		 ResizableBoard,
		 Note,
		 Symbol,
		 Jumper,
		 CopperFill,
		 Logo,
		 Ruler,
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
	void saveInstances(const QString & fileName, QXmlStreamWriter & streamWriter, bool startDocument);
	void saveAsPart(QXmlStreamWriter & streamWriter, bool startDocument);
	void addViewItem(class ItemBase *);
	void removeViewItem(class ItemBase *);
	class ItemBase * viewItem(QGraphicsScene * scene);
	void initConnectors(bool force=false);
	const QHash<QString, QPointer<Connector> > & connectors();
	long modelIndex();
	void setModelIndex(long index);
	void setModelIndexFromMultiplied(long multipliedIndex);
	void setInstanceDomElement(const QDomElement &);
	const QDomElement & instanceDomElement();
	Connector * getConnector(const QString & id);

	const QString & title();
	const QString & description();
	const QStringList & tags();
	const QHash<QString,QString> & properties() const;
	const QHash<QString, QPointer<Bus> > & buses();
	QDomDocument * domDocument();
	const QString & version();
	const QString & path();
	const QString & label();
	const QString & author();
	const QString & language();
	const QString & uri();
	const QDate & date();

	Bus * bus(const QString & busID);
	bool ignoreTerminalPoints();

	bool isCore();
	void setCore(bool core);

	bool isContrib();
	void setContrib(bool contrib);

	bool isAlien(); // from "outside" ?
	void setAlien(bool alien);

	bool hasViewIdentifier(ViewIdentifierClass::ViewIdentifier viewIdentifier);

	QList<ModelPart*> getAllParts();
	QList<ModelPart*> getAllNonCoreParts();
	QList<SvgAndPartFilePath> getAvailableViewFiles();
	bool hasViewID(long id);

	const QString & instanceTitle();
	const QString & instanceText();
	void setInstanceTitle(QString);
	void setInstanceText(QString);

	void setOrderedChildren(QList<QObject*> children);
	void setProp(const QString & name, const QVariant & value);
	QVariant prop(const QString & name) const;
	void setProp(const char * name, const QVariant & value);
	QVariant prop(const char * name) const;

	const QString & replacedby();
	bool isObsolete();

	void setFlippedSMD(bool);
	bool flippedSMD();
	bool hasViewFor(ViewIdentifierClass::ViewIdentifier);
	QString hasBaseNameFor(ViewIdentifierClass::ViewIdentifier);

public:
	static long nextIndex();
	static void updateIndex(long index);
	static const int indexMultiplier;
	static const QStringList & possibleFolders();

signals:
	void startSaveInstances(const QString & fileName, ModelPart *, QXmlStreamWriter &);

protected:
	void writeTag(QXmlStreamWriter & streamWriter, QString tagName, QString tagValue);
	void writeNestedTag(QXmlStreamWriter & streamWriter, QString tagName, const QStringList &values, QString childTag);
	void writeNestedTag(QXmlStreamWriter & streamWriter, QString tagName, const QHash<QString,QString> &values, QString childTag, QString attrName);

	void grabImagePath(QHash<ViewIdentifierClass::ViewIdentifier, SvgAndPartFilePath> &viewImages, QDomElement &viewsElems, ViewIdentifierClass::ViewIdentifier viewId);
	QString inWhichFolder(const QString &partspath, const QString &imagepath);
	void commonInit(ItemType type);

protected:
	QList< QPointer<class ItemBase> > m_viewItems;

	ItemType m_type;
	QPointer<ModelPartShared> m_modelPartShared;
	QHash<QString, QPointer<Connector> > m_connectorHash;
	QList< QPointer<Connector> > m_deletedConnectors;
	QHash<QString, QPointer<Bus> > m_busHash;
	long m_index;						// only used at save time to identify model parts in the xml
	QDomElement m_instanceDomElement;	// only used at load time (so far)

	bool m_core;
	bool m_contrib;
	bool m_alien;
	bool m_originalModelPartShared;

	QString m_instanceTitle;
	QString m_instanceText;

	QList<QObject*> m_orderedChildren;

protected:
	static QHash<ItemType, QString> itemTypeNames;
	static long m_nextIndex;
	static QStringList m_possibleFolders;
};

Q_DECLARE_METATYPE( ModelPart* );			// so we can stash them in a QVariant


#endif
