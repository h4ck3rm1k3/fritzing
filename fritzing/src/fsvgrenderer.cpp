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

#include "fsvgrenderer.h"
#include "debugdialog.h"
#include "svg/svgflattener.h"

#include <QPainter>

QHash<QString, RendererHash *> FSvgRenderer::m_moduleIDRendererHash;
QHash<QString, RendererHash * > FSvgRenderer::m_filenameRendererHash;
qreal FSvgRenderer::m_printerScale = 1;

FSvgRenderer::FSvgRenderer(QObject * parent) : QSvgRenderer(parent)
{
	m_defaultSizeF = QSizeF(0,0);
}

void FSvgRenderer::cleanup() {
	foreach (RendererHash * rendererHash, m_filenameRendererHash.values()) {
		foreach (FSvgRenderer * renderer, rendererHash->values()) {
			delete renderer;
		}
		delete rendererHash;
	}
	m_filenameRendererHash.clear();
	foreach (RendererHash * rendererHash, m_moduleIDRendererHash.values()) {
		delete rendererHash;
	}
	m_moduleIDRendererHash.clear();

}

bool FSvgRenderer::load ( const QString & filename, bool readConnectors ) {
	if (!QFileInfo(filename).exists() || !QFileInfo(filename).isFile()) {
		return false;
	}

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return false;
	}

	QXmlStreamReader xml(&file);
	parseForWidthAndHeight(xml);

	if (readConnectors) {
		file.seek(0);
		m_svgXml = file.readAll();
	}
	file.close();

	bool result = QSvgRenderer::load(filename);
	if (result) {
		m_filename = filename;
	}
	return result;
}

bool FSvgRenderer::load ( const QByteArray & contents, const QString & filename, bool readConnectors) {
	QXmlStreamReader xml(contents);
	parseForWidthAndHeight(xml);
	if (readConnectors) {
		m_svgXml =  contents;
	}

	bool result = QSvgRenderer::load(contents);
	if (result) {
		m_filename = filename;
	}
	return result;
}

bool FSvgRenderer::fastLoad(const QByteArray & contents) {
	return QSvgRenderer::load(contents);
}

const QString & FSvgRenderer::filename() {
	return m_filename;
}

FSvgRenderer * FSvgRenderer::getByFilename(const QString & filename, ViewLayer::ViewLayerID viewLayerID) {
	RendererHash * rendererHash = m_filenameRendererHash.value(filename);
	if (rendererHash == NULL) return NULL;

	return rendererHash->value(viewLayerID, NULL);
}

FSvgRenderer * FSvgRenderer::getByModuleID(const QString & moduleID, ViewLayer::ViewLayerID viewLayerID) {
	RendererHash * rendererHash = m_moduleIDRendererHash.value(moduleID);
	if (rendererHash == NULL) return NULL;

	return rendererHash->value(viewLayerID, NULL);
}

QPixmap * FSvgRenderer::getPixmap(const QString & moduleID, ViewLayer::ViewLayerID viewLayerId, QSize size) {
	// TODO: cache pixmap by size?

	QPixmap *pixmap = NULL;
	FSvgRenderer * renderer = getByModuleID(moduleID, viewLayerId);
	if (renderer) {
		pixmap = new QPixmap(size);
		pixmap->fill(Qt::transparent);
		QPainter painter(pixmap);
		// preserve aspect ratio
		QSizeF def = renderer->defaultSizeF();
		qreal newW = size.width();
		qreal newH = newW * def.height() / def.width();
		if (newH > size.height()) {
			newH = size.height();
			newW = newH * def.width() / def.height();
		}
		QRectF bounds((size.width() - newW) / 2.0, (size.height() - newH) / 2.0, newW, newH);
		renderer->render(&painter, bounds);
		painter.end();
	}
	return pixmap;
}

void FSvgRenderer::set(const QString & moduleID, ViewLayer::ViewLayerID viewLayerID, FSvgRenderer * renderer) {
	RendererHash * rendererHash = m_filenameRendererHash.value(renderer->filename());
	if (rendererHash == NULL) {
		rendererHash = new RendererHash();
		m_filenameRendererHash.insert(renderer->filename(), rendererHash);
	}
	rendererHash->insert(viewLayerID, renderer);
	rendererHash = m_moduleIDRendererHash.value(moduleID);
	if (rendererHash == NULL) {
		rendererHash = new RendererHash();
		m_moduleIDRendererHash.insert(moduleID, rendererHash);
	}
	rendererHash->insert(viewLayerID, renderer);
}

void FSvgRenderer::parseForWidthAndHeight(QXmlStreamReader & xml)
{
    xml.setNamespaceProcessing(false);

	while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
			if (xml.name().toString().compare("svg") == 0) {
				QString ws = xml.attributes().value("width").toString();
				QString hs = xml.attributes().value("height").toString();
				bool ok;
				qreal w = convertToInches(ws, &ok);
				if (!ok) return;

				qreal h = convertToInches(hs, &ok);
				if (!ok) return;

				m_defaultSizeF = QSizeF(w * m_printerScale, h * m_printerScale);

			}
			return;
		default:
			break;
		}
	}
}

QSizeF FSvgRenderer::defaultSizeF() {
	if (m_defaultSizeF.width() == 0 && m_defaultSizeF.height() == 0) {
		return defaultSize();
	}

	return m_defaultSizeF;
}

void FSvgRenderer::calcPrinterScale() {

	// note: I think that printerScale is probably just 90 dpi, since the calculation
	// result is 89.8407 across all three platforms

	m_printerScale = 90.0;
	return;

/*
	m_printerScale = 1;
	ViewGeometry viewGeometry;
	ItemBase * itemBase = m_breadboardGraphicsView->addItem(ItemBase::rulerModuleIDName, BaseCommand::SingleView, viewGeometry, ItemBase::getNextID());
	if (itemBase == NULL) return;

	QSize size = itemBase->size();
	QString filename = dynamic_cast<PaletteItemBase *>(itemBase)->filename();
	m_breadboardGraphicsView->deleteItem(itemBase, true, false, false);

	qreal width = getSvgWidthInInches(filename);
	if (width <= 0) return;

	m_printerScale = size.width() / width;
	DebugDialog::debug(QString("printerscale %1").arg(m_printerScale));
*/

}

qreal FSvgRenderer::printerScale() {
	return m_printerScale;
}

bool FSvgRenderer::getSvgConnectorInfo(ViewLayer::ViewLayerID viewLayerID, const QString & connectorName, QRectF & bounds, qreal & radius, qreal & strokeWidth) {
	Q_UNUSED(viewLayerID);
	
	if (m_svgXml.size() == 0 && m_svgDomDocument.isNull()) {
		return false;
	}

	if (m_svgXml.size() > 0) {
		QString errorStr;
		int errorLine;
		int errorColumn;
		bool result = m_svgDomDocument.setContent(m_svgXml, &errorStr, &errorLine, &errorColumn);
		m_svgXml.clear();
		if (!result) {
			return false;
		}

		SvgFlattener flattener;
		flattener.flattenChildren(m_svgDomDocument.documentElement());

		SvgFileSplitter::fixStyleAttributeRecurse(m_svgDomDocument.documentElement());
	}


	QDomElement element;
	if (m_cachedElement.isNull()) {
		element = findElementWithAttribute(m_svgDomDocument.documentElement(), "id", connectorName);
	}
	else {
		element = findElementWithAttribute(m_cachedElement.parentNode().toElement(), "id", connectorName);
	}
	if (element.isNull()) return false;


	if (element.nodeName().compare("circle") != 0) return false;

	// right now we only handle circles

	bool ok;
	qreal cx = element.attribute("cx").toDouble(&ok);
	if (!ok) return false;
	qreal cy = element.attribute("cy").toDouble(&ok);
	if (!ok) return false;
	qreal r = element.attribute("r").toDouble(&ok);
	if (!ok) return false;
	qreal sw = element.attribute("stroke-width").toDouble(&ok);			// for now, assumes this isn't in the style attribute
	if (!ok) return false;

	m_cachedElement = element;

	bounds.setRect(cx - r - (sw / 2.0), cy - r - (sw / 2.0), (r * 2) + sw, (r * 2) + sw);
	radius = r;
	strokeWidth = sw;
	return true;

}
