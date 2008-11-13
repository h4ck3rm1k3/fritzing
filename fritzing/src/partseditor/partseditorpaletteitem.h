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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/



#ifndef PARTSEDITORPALETTEITEM_H_
#define PARTSEDITORPALETTEITEM_H_

#include "../paletteitem.h"
#include "../misc.h"

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

		void removeFromModel(); // To allow info items to be shown, but not to be persisted
		void setConnector(const QString &id, Connector *conn);

	public slots:
		void highlightConnectors(const QString &connId);

	protected:
		void createSvgFile(QString path);
		bool setUpImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);
		virtual ConnectorItem* newConnectorItem(Connector *connector);

		QDomDocument *m_svgfile;

		/* TODO Mariano: Make this a QString + new class extend StringPair to share the
		 * same reference from the paletteItem and the ViewImage, and to be able to get the file name
		 * without knowing how are this two properties populated
		 */
		StringTriple *m_svgStrings;
		QList<Connector *> *m_connectors;
};

#endif /* PARTSEDITORPALETTEITEM_H_ */
