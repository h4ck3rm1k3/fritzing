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

#include "modelpartshared.h"
#include "../connectors/connectorshared.h"
#include "../debugdialog.h"
#include "../connectors/busshared.h"


#include <QHash>
#include <QMessageBox>
#include <math.h>

void copyPinAttributes(QDomElement & from, QDomElement & to)
{
	to.setAttribute("svgId", from.attribute("svgId"));
	QString terminalId = from.attribute("terminalId");
	if (!terminalId.isEmpty()) {
		to.setAttribute("terminalId", terminalId);
	}
	QString hybrid = from.attribute("hybrid");
	if (!hybrid.isEmpty()) {
		to.setAttribute("hybrid", hybrid);
	}
	QString legId = from.attribute("legId");
	if (!legId.isEmpty()) {
		to.setAttribute("legId", legId);
	}
}

///////////////////////////////////////////////

ViewImage::ViewImage(ViewIdentifierClass::ViewIdentifier vi) {
    flipped = sticky = layers = 0;
    canFlipVertical = canFlipHorizontal = false;
    viewIdentifier = vi;
}

///////////////////////////////////////////////

const QString & ModelPartSharedRoot::icon() {
	return m_icon;
}

void ModelPartSharedRoot::setIcon(const QString & filename) {
	m_icon = filename;
}

const QString & ModelPartSharedRoot::searchTerm() {
	return m_searchTerm;
}

void ModelPartSharedRoot::setSearchTerm(const QString & searchTerm) {
	m_searchTerm = searchTerm;
}

///////////////////////////////////////////////

const QString ModelPartShared::PartNumberPropertyName = "part number";

ModelPartShared::ModelPartShared() {

	commonInit();

	m_path = "";
}

ModelPartShared::ModelPartShared(QDomDocument * domDocument, const QString & path) {
	commonInit();

	m_path = path;

	if (domDocument) {
        setDomDocument(domDocument);
	}
}

bool ModelPartShared::setDomDocument(QDomDocument * domDocument) {
	QDomElement root = domDocument->documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "module") {
		return false;
	}

	loadTagText(root, "title", m_title);
	loadTagText(root, "label", m_label);
	loadTagText(root, "version", m_version);
	loadTagText(root, "author", m_author);
	loadTagText(root, "description", m_description);
	loadTagText(root, "url", m_url);
	loadTagText(root, "taxonomy", m_taxonomy);
	loadTagText(root, "date", m_date);
	QDomElement version = root.firstChildElement("version");
	if (!version.isNull()) {
		m_replacedby = version.attribute("replacedby");
	}

	populateTags(root, m_tags);
	populateProperties(root, m_properties, m_displayKeys);
	ensurePartNumberProperty();

	m_moduleID = root.attribute("moduleId", "");
    m_fritzingVersion = root.attribute("fritzingVersion", "");

	QDomElement views = root.firstChildElement("views");
	if (!views.isNull()) {
		QDomElement view = views.firstChildElement();
		while (!view.isNull()) {
			ViewIdentifierClass::ViewIdentifier viewIdentifier = ViewIdentifierClass::idFromXmlName(view.nodeName());
            ViewImage * viewImage = new ViewImage(viewIdentifier);
            m_viewImages.insert(viewIdentifier, viewImage);
            viewImage->canFlipHorizontal = view.attribute("fliphorizontal","").compare("true") == 0;
            viewImage->canFlipVertical = view.attribute("flipvertical","").compare("true") == 0;
			QDomElement layers = view.firstChildElement("layers");
			if (!layers.isNull()) {
                viewImage->image = layers.attribute("image", "");
				QDomElement layer = layers.firstChildElement("layer");
				while (!layer.isNull()) {
					ViewLayer::ViewLayerID viewLayerID = ViewLayer::viewLayerIDFromXmlString(layer.attribute("layerId"));
                    qulonglong sticky = (layer.attribute("sticky", "").compare("true") == 0) ? 1 : 0;
                    qulonglong one = 1;
                    viewImage->layers |= (one << viewLayerID); 
                    viewImage->sticky |= (sticky << viewLayerID); 
					layer = layer.nextSiblingElement("layer");
				}
			}

			view = view.nextSiblingElement();
		}
	}

    return true;
}

void ModelPartShared::commonInit() {
	m_moduleID = "";
    m_dbid = 0;
	m_flippedSMD = m_connectorsInitialized = m_ignoreTerminalPoints = m_needsCopper1 = false;
}

ModelPartShared::~ModelPartShared() {
	foreach (ConnectorShared * connectorShared, m_connectorSharedHash.values()) {
		delete connectorShared;
	}
	m_connectorSharedHash.clear();

	foreach (ViewImage * viewImage, m_viewImages.values()) {
		delete viewImage;
	}
	m_viewImages.clear();

	foreach (ConnectorShared * connectorShared, m_deletedList) {
		delete connectorShared;
	}
	m_deletedList.clear();

	foreach (BusShared * busShared, m_buses.values()) {
		delete busShared;
	}
	m_buses.clear();

}

void ModelPartShared::loadTagText(QDomElement parent, QString tagName, QString &field) {
	QDomElement tagElement = parent.firstChildElement(tagName);
	if (!tagElement.isNull()) {
		field = tagElement.text();
	}
}

void ModelPartShared::populateTags(QDomElement parent, QStringList &list) {
	QDomElement tags = parent.firstChildElement("tags");
	QDomElement tag = tags.firstChildElement("tag");
	while (!tag.isNull()) {
		list << tag.text();
		tag = tag.nextSiblingElement("tag");
	}
}

void ModelPartShared::populateProperties(QDomElement parent, QHash<QString,QString> &hash, QStringList & displayKeys) {
	QDomElement properties = parent.firstChildElement("properties");
	QDomElement prop = properties.firstChildElement("property");
	while (!prop.isNull()) {
		QString name = prop.attribute("name");
		QString value = prop.text();
		hash.insert(name.toLower().trimmed(),value);
		if (prop.attribute("showInLabel", "").compare("yes", Qt::CaseInsensitive) == 0) {
			displayKeys.append(name);
		}
		prop = prop.nextSiblingElement("property");
	}
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

const QString & ModelPartShared::url() {
	return m_url;
}

void ModelPartShared::setDescription(QString description) {
	m_description = description;
}

void ModelPartShared::setUrl(QString url) {
	m_url = url;
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

void ModelPartShared::setTag(const QString &tag) {
	m_tags.append(tag);
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

void ModelPartShared::setProperties(const QHash<QString,QString> &properties) {
	m_properties = properties;
	ensurePartNumberProperty();
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

const QList< QPointer<ConnectorShared> > ModelPartShared::connectorsShared() {
	return m_connectorSharedHash.values();
}

void ModelPartShared::setConnectorsShared(QList< QPointer<ConnectorShared> > connectors) {
	for (int i = 0; i < connectors.size(); i++) {
		ConnectorShared* cs = connectors[i];
		m_connectorSharedHash[cs->id()] = cs;
	}
}

void ModelPartShared::resetConnectorsInitialization() {
	m_connectorsInitialized = false;

	foreach (ConnectorShared * cs, m_connectorSharedHash.values()) {
		// due to craziness in the parts editor
		m_deletedList.append(cs);
	}
	m_connectorSharedHash.clear();
}

void ModelPartShared::setConnectorsInitialized(bool init) {
    m_connectorsInitialized = init;
}

void ModelPartShared::initConnectors() {
	if (m_connectorsInitialized)
		return;

	QFile file(m_path);
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
    doc.setContent(&file, &errorStr, &errorLine, &errorColumn);

	m_connectorsInitialized = true;
	QDomElement root = doc.documentElement();
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

ConnectorShared * ModelPartShared::getConnectorShared(const QString & id) {
	return m_connectorSharedHash.value(id);
}

bool ModelPartShared::ignoreTerminalPoints() {
	return m_ignoreTerminalPoints;
}

void ModelPartShared::copy(ModelPartShared* other) {
	setAuthor(other->author());
	setConnectorsShared(other->connectorsShared());
	setDate(other->date());
	setLabel(other->label());
	setDescription(other->description());
	setUrl(other->url());
	setFamily(other->family());
	setProperties(other->properties());
	setTags(other->tags());
	setTaxonomy(other->taxonomy());
	setTitle(other->title());
	setUri(other->uri());
	setVersion(other->version());

	foreach (ViewIdentifierClass::ViewIdentifier viewIdentifier, other->m_viewImages.keys()) {
        ViewImage * otherViewImage = other->m_viewImages.value(viewIdentifier);
        ViewImage * viewImage = new ViewImage(viewIdentifier);
        viewImage->layers = otherViewImage->layers;
        viewImage->sticky = otherViewImage->sticky;
        viewImage->canFlipHorizontal = otherViewImage->canFlipHorizontal;
        viewImage->canFlipVertical = otherViewImage->canFlipVertical;
        viewImage->image = otherViewImage->image;
        m_viewImages.insert(viewIdentifier, viewImage);
	}
}

void ModelPartShared::setProperty(const QString & key, const QString & value) {
	m_properties.insert(key, value);
}

const QString & ModelPartShared::replacedby() {
	return m_replacedby;
}

void ModelPartShared::setReplacedby(const QString & replacedby) {
	m_replacedby = replacedby;
}

void ModelPartShared::setFlippedSMD(bool f) {
	m_flippedSMD = f;
}

bool ModelPartShared::flippedSMD() {
	return m_flippedSMD;
}

bool ModelPartShared::needsCopper1() {
	return m_needsCopper1;
}

void ModelPartShared::connectorIDs(ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID, QStringList & connectorIDs, QStringList & terminalIDs, QStringList & legIDs) {
	foreach (ConnectorShared * connectorShared, m_connectorSharedHash.values()) {
		SvgIdLayer * svgIdLayer = connectorShared->fullPinInfo(viewIdentifier, viewLayerID);
		if (svgIdLayer == NULL) {
			continue;
		}
		else {
			connectorIDs.append(svgIdLayer->m_svgId);
			terminalIDs.append(svgIdLayer->m_terminalId);
			legIDs.append(svgIdLayer->m_legId);
		}
	}
}

void ModelPartShared::loadDocument() {

	//DebugDialog::debug("loading document " + m_moduleID);

	QFile file(m_path);
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	if (!doc.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("ModelPartShared load document failed: %1 line:%2 col:%3 on file '%4'").arg(errorStr).arg(errorLine).arg(errorColumn).arg(m_path));
		QMessageBox::critical(NULL, tr("Fritzing"), tr("Unable to parse '%1': %2: line %3 column %4.").arg(m_path).arg(errorStr).arg(errorLine).arg(errorColumn));
	}
	else {
		flipSMDAnd();
	}
}

void ModelPartShared::flipSMDAnd() {
    LayerList layerList = viewLayers(ViewIdentifierClass::PCBView);
    if (layerList.isEmpty()) return;
    if (!layerList.contains(ViewLayer::Copper0) && !layerList.contains(ViewLayer::Copper1)) return;
    if (this->path().startsWith(ResourcePath)) {
        // assume resources are set up exactly as intended
        // DebugDialog::debug(QString("skip flip %1").arg(path()));
        return;
    }

    if (layerList.contains(ViewLayer::Copper0)) {
        if (!layerList.contains(ViewLayer::Copper1)) {
            m_needsCopper1 = true;
            ViewImage * viewImage = m_viewImages.value(ViewIdentifierClass::PCBView);
            qulonglong one = 1;
            viewImage->layers |= (one << ViewLayer::Copper1);
            copyPins(ViewLayer::Copper0, ViewLayer::Copper1);            
        }
        return;
    }

	setFlippedSMD(true);
    // DebugDialog::debug("set flipped smd " + moduleID());

    qulonglong one = 1;
    ViewImage * viewImage = m_viewImages.value(ViewIdentifierClass::PCBView);

    if (!layerList.contains(ViewLayer::Copper0)) {
        viewImage->layers |= (one << ViewLayer::Copper0);
    }

    viewImage->flipped |= (one << ViewLayer::Copper0);

	if (layerList.contains(ViewLayer::Silkscreen1) &&  !layerList.contains(ViewLayer::Silkscreen0)) {
        viewImage->layers |= (one << ViewLayer::Silkscreen0);
        layerList << ViewLayer::Silkscreen0;
	}

    if (layerList.contains(ViewLayer::Silkscreen0)) {
	    viewImage->flipped |= (one << ViewLayer::Silkscreen0);
    }

    copyPins(ViewLayer::Copper1, ViewLayer::Copper0);   
}

bool ModelPartShared::hasViewFor(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier, NULL);
    if (viewImage == NULL) return false;

    return viewImage->layers != 0;
}

bool ModelPartShared::hasViewFor(ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier, NULL);
    if (viewImage == NULL) return false;

    qulonglong one = 1;
    return (viewImage->layers & (one << viewLayerID)) != 0;
}

QString ModelPartShared::hasBaseNameFor(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier, NULL);
    if (viewImage == NULL) return "";

    return viewImage->image;
}

const QStringList & ModelPartShared::displayKeys() {
	return m_displayKeys;
}

void ModelPartShared::setDisplayKeys(const QStringList & displayKeys) {
	m_displayKeys = displayKeys;
}

void ModelPartShared::ensurePartNumberProperty() {
	if (!m_properties.keys().contains(PartNumberPropertyName)) {
		m_properties.insert(PartNumberPropertyName, "");
		m_displayKeys.append(PartNumberPropertyName);
	}
}

void ModelPartShared::setDBID(qulonglong dbid) {
    m_dbid = dbid;
}

qulonglong ModelPartShared::dbid() {
    return m_dbid;
}

const QString & ModelPartShared::fritzingVersion() {
	return m_fritzingVersion;
}

void ModelPartShared::setFritzingVersion(const QString & fv) {
	m_fritzingVersion = fv;
}

void ModelPartShared::setViewImage(ViewImage * viewImage) {
    ViewImage * old = m_viewImages.value(viewImage->viewIdentifier);
    if (old) delete old;
    m_viewImages.insert(viewImage->viewIdentifier, viewImage);
}

const QList<ViewImage *> ModelPartShared::viewImages() {
    return m_viewImages.values();
}

QString ModelPartShared::imageFileName(ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return "";

    qulonglong one = 1;
    if ((viewImage->layers & (one << viewLayerID)) == 0) return "";

    return viewImage->image;
}

QString ModelPartShared::imageFileName(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return "";

    return viewImage->image;
}

void ModelPartShared::setImageFileName(ViewIdentifierClass::ViewIdentifier viewIdentifier, const QString & filename) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return;

    viewImage->image = filename;
}

bool ModelPartShared::hasViewIdentifier(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return false;

    return viewImage->layers != 0;
}

bool ModelPartShared::hasMultipleLayers(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return false;

    // http://eli.thegreenplace.net/2004/07/30/a-cool-algorithm-for-counting-ones-in-a-bitstring/

    qulonglong layers = viewImage->layers;
    return ((layers & (layers - 1)) != 0);
}

qulonglong layers(ViewImage * viewImage) {
    return viewImage->layers;
}

qulonglong flipped(ViewImage * viewImage) {
    return viewImage->flipped;
}

LayerList ModelPartShared::viewLayersFlipped(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    return viewLayersAux(viewIdentifier, flipped);
}

LayerList ModelPartShared::viewLayers(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    return viewLayersAux(viewIdentifier, layers);
}

LayerList ModelPartShared::viewLayersAux(ViewIdentifierClass::ViewIdentifier viewIdentifier, qulonglong (*accessor)(ViewImage *)) {

    static QHash<qulonglong, ViewLayer::ViewLayerID> ToLayerIDs;

    LayerList layerList;
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return layerList;

    // http://eli.thegreenplace.net/2004/07/30/a-cool-algorithm-for-counting-ones-in-a-bitstring/

    if (ToLayerIDs.isEmpty()) {
        qulonglong one = 1;
        for (int ix = 0; ix < ViewLayer::ViewLayerCount; ix++) {
            ToLayerIDs.insert(one, (ViewLayer::ViewLayerID) ix);
            one = one << 1;
        }
    }

    qulonglong layers = accessor(viewImage);
    while (layers) {
        qulonglong removeLeast = layers & (layers - 1);
        qulonglong diff = layers - removeLeast;
        layerList << ToLayerIDs.value(diff);
        layers = removeLeast;
    }

    return layerList;
}


bool ModelPartShared::canFlipHorizontal(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return false;

    return viewImage->canFlipHorizontal;
}

bool ModelPartShared::canFlipVertical(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return false;

    return viewImage->canFlipVertical;
}

bool ModelPartShared::isSticky(ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID) {
    ViewImage * viewImage = m_viewImages.value(viewIdentifier);
    if (viewImage == NULL) return false;

    qulonglong one = 1;
    return ((viewImage->sticky & (one << viewLayerID)) != 0);
}

void ModelPartShared::addConnector(ConnectorShared * connectorShared)
{
    m_connectorSharedHash.insert(connectorShared->id(), connectorShared);
}

void ModelPartShared::copyPins(ViewLayer::ViewLayerID from, ViewLayer::ViewLayerID to) {
    foreach (ConnectorShared * connectorShared, m_connectorSharedHash.values()) {
        SvgIdLayer * svgIdLayer = connectorShared->fullPinInfo(ViewIdentifierClass::PCBView, to);
        if (svgIdLayer) {
            // already there
            continue;
        }

        svgIdLayer = connectorShared->fullPinInfo(ViewIdentifierClass::PCBView, from);
        if (svgIdLayer == NULL) {
            DebugDialog::debug(QString("missing connector in %1").arg(moduleID()));
            continue;
        }

        SvgIdLayer * newSvgIdLayer = svgIdLayer->copyLayer();
        newSvgIdLayer->m_svgViewLayerID = to;
        connectorShared->insertPin(ViewIdentifierClass::PCBView, newSvgIdLayer);
    }
}

void ModelPartShared::insertBus(BusShared * busShared) {
    m_buses.insert(busShared->id(), busShared);
}
