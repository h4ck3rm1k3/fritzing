/*
 *  layerattributes.h
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/20/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#ifndef LAYERATTRIBUTES_H
#define LAYERATTRIBUTES_H

#include <QString>
#include <QDomElement>

#include "itembase.h"

class LayerAttributes {
	
public:
	LayerAttributes();
	
	const QString & filename();
	void setFilename(const QString &);
	const QString & layerName();
	bool sticky();
	bool multiLayer();
	bool getSvgElementID(QDomDocument * , ItemBase::ViewIdentifier, ViewLayer::ViewLayerID );
	static QDomElement getSvgElementLayers(QDomDocument * doc, ItemBase::ViewIdentifier viewIdentifier );

protected:
	static QDomElement getSvgElementLayer(QDomDocument *, ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, int & layerCount );

protected:
	QString m_filename;
	QString m_layerName;
	bool m_multiLayer;
	bool m_sticky;
};

#endif
