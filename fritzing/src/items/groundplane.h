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

#ifndef GROUNDPLANE_H
#define GROUNDPLANE_H

#include "paletteitem.h"

class GroundPlane : public PaletteItem
{

public:
	GroundPlane( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier,  const ViewGeometry & , long id, QMenu* itemMenu, bool doLabel); 

 	bool setUpImage(ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, ViewLayer::ViewLayerSpec, bool doConnectors);
	void saveParams();
	void getParams();
	QString retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	class ConnectorItem * connector0();
	bool hasCustomSVG();
	void setProp(const QString & prop, const QString & value);
	QString svg();
	bool hasPartLabel();

protected:
	void setSvg(const QString &);
	void setSvgAux(const QString &);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
	ConnectorItem * m_connector0;
	class FSvgRenderer * m_renderer;
};

#endif
