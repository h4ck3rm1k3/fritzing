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
	static void findElementsWithAttribute(QDomElement & element, const QString & att, QList<QDomElement> & elements);
	static qreal convertToInches(const QString & string, bool * ok, bool isIllustrator=false);
	static qreal convertToInches(const QString & string);
	static QString convertToPowerPrefix(qreal);
	static qreal convertFromPowerPrefix(const QString & val, const QString & symbol);
	static qreal convertFromPowerPrefixU(QString & val, const QString & symbol);

	static QString replaceTextElement(const QString & svg, const QString & label);
    static bool squashElement(QDomDocument &, const QString & elementName, const QString & attName, const QRegExp & matchContent);
    static QString mergeSvg(const QString & svg1, const QString & svg2, const QString & id);
	static QString mergeSvgFinish(QDomDocument & doc);
	static bool mergeSvg(QDomDocument & doc1, const QString & svg, const QString & id);
	static QString toHtmlImage(QPixmap *pixmap, const char* format = "PNG");
	static QString makeSVGHeader(qreal printerscale, qreal dpi, qreal width, qreal height);
	static bool isIllustratorFile(const QString &fileContent);
	static QString removeXMLEntities(QString svgContent);
	static bool cleanSodipodi(QString &bytes);
	static bool fixViewboxOrigin(QString &fileContent);
	static bool fixPixelDimensionsIn(QString &fileContent);
	static bool addCopper1(const QString & filename, QDomDocument & doc, const QString & srcAtt, const QString & destAtt);
	static void setSVGTransform(QDomElement &, QMatrix &);
	static QString svgMatrix(QMatrix &);
	static QString svgTransform(const QString & svg, QTransform & transform, bool translate, const QString extra);
	static bool getSvgSizes(QDomDocument & doc, qreal & sWidth, qreal & sHeight, qreal & vbWidth, qreal & vbHeight);
	static bool findText(QDomNode & node, QString & text);
	static QString stripNonValidXMLCharacters(const QString & str); 
	static QString escapeAnd(const QString &);
	static QMatrix elementToMatrix(QDomElement & element);
	static QMatrix transformStringToMatrix(const QString & transform);
    static QList<qreal> getTransformFloats(QDomElement & element);
	static QList<qreal> getTransformFloats(const QString & transform);

public:
	static const QRegExp FindWhitespace;
	static const QRegExp SodipodiDetector;
	static const QString SMDFlipSuffix;
	static const QString MicroSymbol;
	static const QString PowerPrefixesString;
	static const QString CreatedWithFritzingString;
	static const QString CreatedWithFritzingXmlComment;
	static void collectLeaves(QDomElement & element, int & index, QVector<QDomElement> & leaves);
	static const QRegExp floatingPointMatcher;
	static const QString RegexFloatDetector;

protected:
	static bool moveViewboxToTopLeftCorner(QDomElement &elem);
	static bool pxToInches(QDomElement &elem, const QString &attrName, bool isIllustrator);
    static void squashNotElement(QDomElement & element, const QString & elementName, const QString & attName, const QRegExp & matchContent, bool & result);
	static void initPowerPrefixes();

};

#endif
