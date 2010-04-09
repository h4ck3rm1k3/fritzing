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

#ifndef FSVGRENDERER_H
#define FSVGRENDERER_H

#include <QHash>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QMatrix>
#include <QStringList>

#include "viewlayer.h"

struct ConnectorInfo {
	qreal radius;
	qreal strokeWidth;
	QMatrix matrix;
	QRectF bounds;
	QMatrix terminalMatrix;
	bool gotCircle;
};

typedef QHash<ViewLayer::ViewLayerID, class FSvgRenderer *> RendererHash;

class FSvgRenderer : public QSvgRenderer
{
public:
	FSvgRenderer(QObject * parent = 0);
	~FSvgRenderer();

	bool loadSvg(const QString & filename, const QStringList & connectorIDs, const QStringList & terminalIDs, const QString & setColor, const QString & colorElementID);
	bool loadSvg(const QString & filename);
	bool loadSvg( const QByteArray & contents, const QString & filename, const QStringList & connectorNames, const QStringList & terminalNames, const QString & setColor, const QString & colorElementID);     // for SvgSplitter loads
	bool loadSvg( const QByteArray & contents, const QString & filename);						// for SvgSplitter loads
	bool fastLoad(const QByteArray & contents);								
	const QString & filename();
	QSizeF defaultSizeF();
	void initConnectorInfo(QDomDocument &, const QStringList & connectorIDs, const QStringList & terminalIDs);
	ConnectorInfo * getConnectorInfo(const QString & connectorID);

public:
	static void set(const QString & moduleID, ViewLayer::ViewLayerID, FSvgRenderer *);
	static FSvgRenderer * getByModuleID(const QString & moduleID, ViewLayer::ViewLayerID);
	static FSvgRenderer * getByFilename(const QString & filename, ViewLayer::ViewLayerID);
	static QPixmap * getPixmap(const QString & moduleID, ViewLayer::ViewLayerID viewLayerID, QSize size);
	static void calcPrinterScale();
	static qreal printerScale();
	static void cleanup();
	static QSizeF parseForWidthAndHeight(QXmlStreamReader &);
	static void removeFromHash(const QString &moduleId, const QString filename);

protected:
	void determineDefaultSize(QXmlStreamReader &);
	QByteArray cleanXml(const QByteArray & contents, const QString & filename);
	bool loadAux (const QByteArray & contents, const QString & filename, const QStringList & connectorNames, const QStringList & terminalNames, const QString & setColor, const QString & colorElementID);

protected:
	QString m_filename;
	QSizeF m_defaultSizeF;
	QHash<QString, ConnectorInfo *> m_connectorInfoHash;

protected:
	static qreal m_printerScale;
	static QHash<QString, RendererHash * > m_filenameRendererHash;
	static QHash<QString, RendererHash * > m_moduleIDRendererHash;
	static QSet<RendererHash *> m_deleted;
};


#endif
