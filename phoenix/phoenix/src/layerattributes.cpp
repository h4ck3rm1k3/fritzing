/*
 *  layerattributes.cpp
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/20/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include "layerattributes.h"
#include "debugdialog.h"

LayerAttributes::LayerAttributes()
{
	m_sticky = false;
	m_multiLayer = false;
}

const QString & LayerAttributes::filename() {
	return m_filename;
}

void LayerAttributes::setFilename(const QString & filename) {
	m_filename = filename;
}

const QString & LayerAttributes::layerName() {
	return m_layerName;
}

bool LayerAttributes::multiLayer() {
	return m_multiLayer;
}

bool LayerAttributes::getSvgElementID(QDomDocument * doc, ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID) {
	int layerCount;
	QDomElement layer = getSvgElementLayer(doc, viewIdentifier, viewLayerID, layerCount);
	m_multiLayer = (layerCount > 1);

	if (layer.isNull()) return false;
	if (layerCount == 0) return false;

	m_layerName = layer.attribute("layerId");
	if (m_layerName.isNull()) return false;
	if (m_layerName.isEmpty()) return false;

	m_filename = layer.attribute("image");
	if (m_filename.isNull()) return false;
	if (m_filename.isEmpty()) return false;

	QString stickyVal = layer.attribute("sticky");
	m_sticky = stickyVal.compare("true", Qt::CaseInsensitive) == 0;

	return true;
}

QDomElement LayerAttributes::getSvgElementLayers(QDomDocument * doc, ItemBase::ViewIdentifier viewIdentifier )
{
   	if (doc == NULL) return ___emptyElement___;

   	QDomElement root = doc->documentElement();
   	if (root.isNull()) return ___emptyElement___;

	QDomElement views = root.firstChildElement("views");
	if (views.isNull()) return ___emptyElement___;

	QString name = ItemBase::viewIdentifierXmlName(viewIdentifier);
	if (name.isEmpty() || name.isNull()) return ___emptyElement___;

	QDomElement view = views.firstChildElement(name);
	if (view.isNull()) return ___emptyElement___;

	QDomElement layers = view.firstChildElement("layers");
	if (layers.isNull()) return ___emptyElement___;

	return layers;
}


QDomElement LayerAttributes::getSvgElementLayer(QDomDocument * doc, ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID, int & layerCount )
{
	QString layerName = ViewLayer::viewLayerXmlNameFromID(viewLayerID);
	if (layerName.isNull() || layerName.isEmpty()) return ___emptyElement___;

	layerCount = 0;

	QDomElement layers = getSvgElementLayers(doc, viewIdentifier);
	if (layers.isNull()) return ___emptyElement___;

	QDomElement retval = ___emptyElement___;
	QDomElement layer = layers.firstChildElement("layer");
	bool gotOne = false;
	while (!layer.isNull()) {
		layerCount++;
		if (gotOne && layerCount > 1) break;

		if (!gotOne && layer.attribute("layerId").compare(layerName, Qt::CaseInsensitive) == 0) {
			gotOne = true;
			retval = layer;
		}
		//DebugDialog::debug(QString("layer id %1, want: %2").arg(layer.attribute("layerId")).arg(layerName) );
		layer = layer.nextSiblingElement("layer");
	}

	return retval;

}


bool LayerAttributes::sticky() {
	return m_sticky;
}
