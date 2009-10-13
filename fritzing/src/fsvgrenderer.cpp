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

You should have received a copy of the GNU General Public Licen/#demose
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

#include "fsvgrenderer.h"
#include "debugdialog.h"
#include "svg/svgfilesplitter.h"
#include "svg/svgflattener.h"
#include "utils/textutils.h"

#include <QRegExp>
#include <QTextStream>
#include <QPainter>
#include <QCoreApplication>
#include <QGraphicsSvgItem>

QHash<QString, RendererHash *> FSvgRenderer::m_moduleIDRendererHash;
QHash<QString, RendererHash * > FSvgRenderer::m_filenameRendererHash;
qreal FSvgRenderer::m_printerScale = 90.0;

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

	QByteArray contents = file.readAll();
	file.close();

	if (contents.length() <= 0) return false;

	return loadAux(contents, filename, readConnectors);

	/*

	QXmlStreamReader xml(contents);
	determineDefaultSize(xml);

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
	*/

}

bool FSvgRenderer::load ( const QByteArray & contents, const QString & filename, bool readConnectors) {
	return loadAux(contents, filename, readConnectors);
}


bool FSvgRenderer::loadAux ( const QByteArray & contents, const QString & filename, bool readConnectors) {
	QByteArray cleanContents = contents; // if the part has been created through the parts editor, it's clean

	/*
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	doc.setContent(cleanContents, &errorStr, &errorLine, &errorColumn);
	QDomElement root = doc.documentElement();
	SvgFileSplitter::fixStyleAttributeRecurse(root);
	cleanContents = doc.toByteArray();

	//QFile file("all.txt");
	//if (file.open(QIODevice::Append)) {
		//QTextStream t(&file);
		//t << cleanContents;
		//file.close();
	//}

	*/

	//DebugDialog::debug(cleanContents.data());

	QXmlStreamReader xml(cleanContents);
	determineDefaultSize(xml);
	if (readConnectors) {
		m_svgXml =  cleanContents;
	}

	bool result = QSvgRenderer::load(cleanContents);
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

void FSvgRenderer::determineDefaultSize(QXmlStreamReader & xml)
{
	QSizeF size = parseForWidthAndHeight(xml);
	m_defaultSizeF = QSizeF(size.width() * m_printerScale, size.height() * m_printerScale);
}

QSizeF FSvgRenderer::parseForWidthAndHeight(QXmlStreamReader & xml)
{
    xml.setNamespaceProcessing(false);

	QSizeF size(0,0);

	while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
			if (xml.name().toString().compare("svg") == 0) {
				QString ws = xml.attributes().value("width").toString();
				QString hs = xml.attributes().value("height").toString();
				bool ok;
				qreal w = TextUtils::convertToInches(ws, &ok);
				if (!ok) return size;

				qreal h = TextUtils::convertToInches(hs, &ok);
				if (!ok) return size;

				size.setWidth(w);
				size.setHeight(h);
				return size;
			}
			return size;
		default:
			break;
		}
	}

	return size;
}



QSizeF FSvgRenderer::defaultSizeF() {
	if (m_defaultSizeF.width() == 0 && m_defaultSizeF.height() == 0) {
		return defaultSize();
	}

	return m_defaultSizeF;
}

void FSvgRenderer::calcPrinterScale() {

	// note: I think that printerScale is probably just 90 dpi, since the calculation
	// result is 89.8407 for the breadboard svg across all three platforms 
	// note: calculation result depends on the svg used; if the svg size is a float, the scale will vary a little
	// using an svg file with exactly a 1-inch width (like 'wire.svg') gives exactly a 90.0 printerscale value.

	m_printerScale = 90.0;

	/*

	QString filename(":/resources/parts/svg/core/breadboard/wire.svg");
	
	QGraphicsSvgItem item(filename);
	QRectF b = item.boundingRect();
	QFile file(filename);
	file.open(QFile::ReadOnly);
	QXmlStreamReader xml(&file);
	QSizeF size = parseForWidthAndHeight(xml);
	if (size.width() <= 0) return;

	qreal pscale = b.width() / size.width();
	DebugDialog::debug(QString("printerscale %1").arg(pscale));

	*/
}

qreal FSvgRenderer::printerScale() {
	return m_printerScale;
}

bool FSvgRenderer::getSvgCircleConnectorInfo(ViewLayer::ViewLayerID viewLayerID, const QString & connectorName, QRectF & bounds, qreal & radius, qreal & strokeWidth, QMatrix & matrix, const QString & terminalName, QMatrix & terminalMatrix)
{
	Q_UNUSED(viewLayerID);

	radius = strokeWidth = 0;
	
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
	}

	QDomElement element, terminalElement;
	if (!m_cachedElement.isNull()) {
		element = TextUtils::findElementWithAttribute(m_cachedElement.parentNode().toElement(), "id", connectorName);
		if (!terminalName.isEmpty()) {
			terminalElement = TextUtils::findElementWithAttribute(m_cachedElement.parentNode().toElement(), "id", terminalName);
		}
	}
	if (element.isNull()) {
		element = TextUtils::findElementWithAttribute(m_svgDomDocument.documentElement(), "id", connectorName);
		if (!terminalName.isEmpty() && terminalElement.isNull()) {
			terminalElement = TextUtils::findElementWithAttribute(element.parentNode().toElement(), "id", terminalName);
		}
	}
	if (!terminalName.isEmpty() && terminalElement.isNull()) {
		terminalElement = TextUtils::findElementWithAttribute(m_svgDomDocument.documentElement(), "id", terminalName);
	}

	if (!terminalElement.isNull()) {
		terminalMatrix = SvgFlattener::elementToMatrix(terminalElement);
	}

	if (element.isNull()) return false;

	matrix = SvgFlattener::elementToMatrix(element);


	if (element.nodeName().compare("circle") != 0) return false;

	// right now we only handle circles

	bool ok;
	qreal cx = element.attribute("cx").toDouble(&ok);
	if (!ok) return false;
	qreal cy = element.attribute("cy").toDouble(&ok);
	if (!ok) return false;
	qreal r = element.attribute("r").toDouble(&ok);
	if (!ok) return false;
	qreal sw = element.attribute("stroke-width").toDouble(&ok);	
	if (!ok) {
        QString sw("stroke-width");
        QString s = element.attribute("style");
        SvgFileSplitter::fixStyleAttribute(element, s, sw);
		sw = element.attribute("stroke-width").toDouble(&ok);
		if (!ok) {
			return false;
		}
	}

	m_cachedElement = element;

	bounds.setRect(cx - r - (sw / 2.0), cy - r - (sw / 2.0), (r * 2) + sw, (r * 2) + sw);
	radius = r;
	strokeWidth = sw;
	return true;
}

void FSvgRenderer::removeFromHash(const QString &moduleId, const QString filename) {
	DebugDialog::debug(QString("length before %1").arg(m_moduleIDRendererHash.size()));
	m_moduleIDRendererHash.remove(moduleId);
	DebugDialog::debug(QString("length after %1").arg(m_moduleIDRendererHash.size()));
	m_filenameRendererHash.remove(filename);
}

