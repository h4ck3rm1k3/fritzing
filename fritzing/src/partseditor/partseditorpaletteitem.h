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



#ifndef PARTSEDITORPALETTEITEM_H_
#define PARTSEDITORPALETTEITEM_H_

#include "../paletteitem.h"
#include "../misc.h"
#include "../svgandpartfilepath.h"

class PartsEditorPaletteItem : public PaletteItem {
	Q_OBJECT
	public:
		PartsEditorPaletteItem(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, StringPair *path, QString layer);
		PartsEditorPaletteItem(ModelPart * modelPart, QDomDocument *svgFile, ItemBase::ViewIdentifier viewIdentifier, StringPair *path, QString layer);
		PartsEditorPaletteItem(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier);
		virtual void writeXml(QXmlStreamWriter &);
		virtual void writeXmlLocation(QXmlStreamWriter & streamWriter);
		const QList<Connector *> &connectors();
		StringPair* svgFilePath();
		void setSvgFilePath(StringPair *sp);

		QDomDocument *svgDom();
		QString flatSvgFilePath();

		void removeFromModel(); // To allow info items to be shown, but not to be persisted
		void setConnector(const QString &id, Connector *conn);

	public slots:
		void highlightConnectors(const QString &connId);

	protected:
		void createSvgFile(QString path);
		bool setUpImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);
		virtual ConnectorItem* newConnectorItem(Connector *connector);

		QDomDocument *m_svgDom;
		QString m_originalSvgPath;

		SvgAndPartFilePath *m_svgStrings;
		QList<Connector *> *m_connectors;
};

#endif /* PARTSEDITORPALETTEITEM_H_ */
