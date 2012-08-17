/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef MODELPARTSHARED_H
#define MODELPARTSHARED_H

#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QStringList>
#include <QHash>
#include <QDate>
#include <QPointer>

#include "../viewidentifierclass.h"
#include "../viewlayer.h"

struct ViewImage {
    ViewIdentifierClass::ViewIdentifier viewIdentifier;
    qulonglong layers;
    qulonglong sticky;
    qulonglong flipped;
    QString image;
    bool canFlipHorizontal;
    bool canFlipVertical;

    ViewImage(ViewIdentifierClass::ViewIdentifier);
};

class ModelPartShared : public QObject
{
Q_OBJECT
public:
	ModelPartShared();
	ModelPartShared(QDomDocument *, const QString & path);
	~ModelPartShared();

	bool partlyLoaded();
	void setPartlyLoaded(bool);

	void setDomDocument(QDomDocument *);
	//QDomDocument * domDocument();

	void copy(ModelPartShared* other);

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
	const QString & url();
	void setUrl(QString url);
	const QString & title();
	void setTitle(QString title);
	const QString & label();
	void setLabel(QString label);
	const QDate & date();
	void setDate(QDate date);
	const QString & dateAsStr();
	void setDate(QString date);
    void setDBID(qulonglong);
    qulonglong dbid();
    const QString & fritzingVersion();
    void setFritzingVersion(const QString & fv);

    const QList<ViewImage *> viewImages();
    QString imageFileName(ViewIdentifierClass::ViewIdentifier, ViewLayer::ViewLayerID);
    void setImageFileName(ViewIdentifierClass::ViewIdentifier, const QString & filename);
    QString imageFileName(ViewIdentifierClass::ViewIdentifier);
    bool isSticky(ViewIdentifierClass::ViewIdentifier, ViewLayer::ViewLayerID);
    bool hasMultipleLayers(ViewIdentifierClass::ViewIdentifier);
    bool canFlipHorizontal(ViewIdentifierClass::ViewIdentifier);
    bool canFlipVertical(ViewIdentifierClass::ViewIdentifier);
    bool hasViewIdentifier(ViewIdentifierClass::ViewIdentifier viewIdentifier);
    LayerList viewLayers(ViewIdentifierClass::ViewIdentifier viewIdentifier);
    LayerList viewLayersFlipped(ViewIdentifierClass::ViewIdentifier viewIdentifier);

	const QString & path();
	void setPath(QString path);
	const QString & taxonomy();
	void setTaxonomy(QString taxonomy);

	const QList< QPointer<class ConnectorShared> > connectorsShared();
	void setConnectorsShared(QList< QPointer<class ConnectorShared> > connectors);
	void connectorIDs(ViewIdentifierClass::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerID, QStringList & connectorIDs, QStringList & terminalIDs, QStringList & legIDs);

	const QStringList &tags();
	void setTags(const QStringList &tags);
	void setTag(const QString &tag);

	QString family();
	void setFamily(const QString &family);

	QHash<QString,QString> & properties();
	void setProperties(const QHash<QString,QString> &properties);
	void setDisplayKeys(const QStringList &displayKeys);
	const QStringList & displayKeys();


	void initConnectors();
	void resetConnectorsInitialization();
    void setConnectorsInitialized(bool); 
	ConnectorShared * getConnectorShared(const QString & id);
	bool ignoreTerminalPoints();

	void setProperty(const QString & key, const QString & value);
	const QString & replacedby();
	void setReplacedby(const QString & replacedby);

	void flipSMDAnd();
	void setFlippedSMD(bool);
	bool flippedSMD();	
	bool needsCopper1();
	bool hasViewFor(ViewIdentifierClass::ViewIdentifier);
	bool hasViewFor(ViewIdentifierClass::ViewIdentifier, ViewLayer::ViewLayerID);
	QString hasBaseNameFor(ViewIdentifierClass::ViewIdentifier);
    void setViewImage(ViewImage *);
    void addConnector(ConnectorShared *);
    void insertBus(class BusShared *);

protected:
	void loadTagText(QDomElement parent, QString tagName, QString &field);
	// used to populate de StringList that contains both the <tags> and the <properties> values
	void populateTags(QDomElement parent, QStringList &list);
	void populateProperties(QDomElement parent, QHash<QString,QString> &hash, QStringList & displayKeys);
	void commonInit();
	void loadDocument();
	void ensurePartNumberProperty();
    void copyPins(ViewLayer::ViewLayerID from, ViewLayer::ViewLayerID to);
    LayerList viewLayersAux(ViewIdentifierClass::ViewIdentifier viewIdentifier, qulonglong (*accessor)(ViewImage *));

public:
	static const QString PartNumberPropertyName;

protected:

	//QDomDocument* m_domDocument;

	QString m_uri;
	QString m_moduleID;
    QString m_fritzingVersion;
	QString m_version;
	QString m_author;
	QString m_title;
	QString m_label;
	QString m_description;
	QString m_url;
	QString m_date;
	QString m_replacedby;

	QString m_path;
	QString m_taxonomy;

	QStringList m_tags;
	QStringList m_displayKeys;
	QHash<QString,QString> m_properties;

	QHash<QString, QPointer<class ConnectorShared> > m_connectorSharedHash;
	QHash<QString, class BusShared *> m_buses;
	QList<class ConnectorShared *> m_deletedList;
    QHash<ViewIdentifierClass::ViewIdentifier, ViewImage *> m_viewImages;

	bool m_connectorsInitialized;
	bool m_ignoreTerminalPoints;

	bool m_flippedSMD;
	bool m_partlyLoaded;
	bool m_needsCopper1;				// for converting pre-two-layer parts
    qulonglong m_dbid;

};

class ModelPartSharedRoot : public ModelPartShared
{
	Q_OBJECT
public:
	const QString & icon();
	void setIcon(const QString & filename);
	const QString & searchTerm();
	void setSearchTerm(const QString & searchTerm);

protected:
	QString m_icon;
	QString m_searchTerm;

};


#endif
