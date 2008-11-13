/*
 * (c) Fachhochschule Potsdam
 */

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
