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

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <QPointF>
#include <QDomElement>
#include <QSet>
#include <QPixmap>
#include <QMatrix>
#include <QTransform>
#include <QSvgRenderer>

class TextUtils
{

public:
	static QSet<QString> getRegexpCaptures(const QString &pattern, const QString &textToSearchIn);
	static QDomElement findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue);
	static qreal convertToInches(const QString & string, bool * ok, bool isIllustrator=false);
	static QString replaceTextElement(const QString & svg, const QString & label);
    static bool squashElement(QString & svg, const QString & elementName, const QString & attName, const QRegExp & matchContent);
    static bool squashNotElement(QString & svg, const QString & elementName, const QString & attName, const QRegExp & matchContent);
    static QString mergeSvg(const QString & svg1, const QString & svg2);
	static QString toHtmlImage(QPixmap *pixmap, const char* format = "PNG");
	static QString makeSVGHeader(qreal printerscale, qreal dpi, qreal width, qreal height);
	static bool isIllustratorFile(const QString &fileContent);
	static QString removeXMLEntities(QString svgContent);
	static bool cleanSodipodi(QString &bytes);
	static bool fixViewboxOrigin(QString &fileContent);
	static bool fixPixelDimensionsIn(QString &fileContent);
	static void flipSMDSvg(const QString & filename, QDomDocument & flipDoc, const QString & elementID, const QString & altElementID, qreal printerScale);
	static void setSVGTransform(QDomElement &, QMatrix &);
	static QString svgTransform(const QString & svg, QTransform & transform, bool translate);
	static bool getSvgSizes(QDomDocument & doc, qreal & sWidth, qreal & sHeight, qreal & vbWidth, qreal & vbHeight);

public:
	static const QRegExp FindWhitespace;
	static const QRegExp SodipodiDetector;
	static const QString SMDFlipSuffix;

protected:
	static bool moveViewboxToTopLeftCorner(QDomElement &elem);
	static bool pxToInches(QDomElement &elem, const QString &attrName, bool isIllustrator);
    static void squashNotElement(QDomElement & element, const QString & elementName, const QString & attName, const QRegExp & matchContent, bool & result);
	static void flipSMDElement(QDomDocument & domDocument, QSvgRenderer & renderer, QDomElement & element, const QString & elementID, QDomElement altElement, const QString & altElementID, qreal printerScale);

};

#endif
