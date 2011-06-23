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

#include <QTextDocument>
#include "textutils.h"
#include "misc.h"
//#include "../debugdialog.h"

#include <QRegExp>
#include <QBuffer>
#include <QFile>

const QString TextUtils::CreatedWithFritzingString("Created with Fritzing (http://www.fritzing.org/)");
const QString TextUtils::CreatedWithFritzingXmlComment("<!-- " + CreatedWithFritzingString + " -->\n");

const QRegExp TextUtils::FindWhitespace("[\\s]+");
const QRegExp TextUtils::SodipodiDetector("((inkscape)|(sodipodi)):[^=\\s]+=\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"");
const QString TextUtils::SMDFlipSuffix("___");

const QString TextUtils::RegexFloatDetector = "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";
const QRegExp TextUtils::floatingPointMatcher(RegexFloatDetector);		

const QRegExp HexExpr("&#x[0-9a-fA-F];");   // &#x9; &#xa; &#xd;

static const ushort MicroSymbolCode = 181;
const QString TextUtils::MicroSymbol = QString::fromUtf16(&MicroSymbolCode, 1);

const QString TextUtils::AdobeIllustratorIdentifier = "Generator: Adobe Illustrator";

QList<QString> PowerPrefixes;
QList<qreal> PowerPrefixValues;
const QString TextUtils::PowerPrefixesString = QString("pnmkMGTu\\x%1").arg(MicroSymbolCode, 4, 16, QChar('0'));

void TextUtils::findElementsWithAttribute(QDomElement & element, const QString & att, QList<QDomElement> & elements) {
	if (!element.attribute(att).isEmpty()) {
		elements.append(element);
	}

	QDomElement child = element.firstChildElement();
	while (!child.isNull()) {
		findElementsWithAttribute(child, att, elements);
		child = child.nextSiblingElement();
	}
}

QDomElement TextUtils::findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue) {
	if (element.hasAttribute(attributeName)) {
		if (element.attribute(attributeName).compare(attributeValue) == 0) return element;
	}

     for(QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
     {
		 QDomElement result = findElementWithAttribute(e, attributeName, attributeValue);
		 if (!result.isNull()) return result;
     }

	return ___emptyElement___;
}


QSet<QString> TextUtils::getRegexpCaptures(const QString &pattern, const QString &textToSearchIn) {
	QRegExp re(pattern);
	QSet<QString> captures;
	int pos = 0;

	while ((pos = re.indexIn(textToSearchIn, pos)) != -1) {
		captures << re.cap(1);
		pos += re.matchedLength();
	}

	return captures;
}

qreal TextUtils::convertToInches(const QString & s, bool * ok, bool isIllustrator) {
	QString string = s;
	qreal divisor = 1.0;
	if (string.endsWith("cm", Qt::CaseInsensitive)) {
		divisor = 2.54;
		string.chop(2);
	}
	else if (string.endsWith("mm", Qt::CaseInsensitive)) {
		divisor = 25.4;
		string.chop(2);
	}
	else if (string.endsWith("in", Qt::CaseInsensitive)) {
		divisor = 1.0;
		string.chop(2);
	}
	else if (string.endsWith("px", Qt::CaseInsensitive)) {
		divisor = isIllustrator? 72.0: 90.0;
		string.chop(2);
	}
	else if (string.endsWith("mil", Qt::CaseInsensitive)) {
		divisor = 1000.0;
		string.chop(3);
	}
	else {
		divisor = 90.0;			// default to Qt's standard internal units if all else fails
	}

	bool fine;
	qreal result = string.toDouble(&fine);
	if (!fine) {
		if (ok) *ok = false;
		return 0;
	}

	if (ok) *ok = true;
	return result / divisor;
}

bool TextUtils::squashElement(QDomDocument & doc, const QString & elementName, const QString &attName, const QRegExp &matchContent) {
    bool result = false;
    QDomElement root = doc.documentElement();
    QDomNodeList domNodeList = root.elementsByTagName(elementName);
    for (int i = 0; i < domNodeList.count(); i++) {
        QDomElement node = domNodeList.item(i).toElement();
        if (node.isNull()) continue;

        if (!attName.isEmpty()) {
            QString att = node.attribute(attName);
            if (att.isEmpty()) continue;

            if (!matchContent.isEmpty()) {
                if (matchContent.indexIn(att) < 0) continue;
            }
        }

        node.setTagName("g");
        result = true;
    }

    return result;
}

QString TextUtils::replaceTextElement(const QString & svg, const QString & label) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	if (!doc.setContent(svg, &errorStr, &errorLine, &errorColumn)) return svg;

	QDomElement root = doc.documentElement();
	QDomNodeList domNodeList = root.elementsByTagName("text");
	for (int i = 0; i < domNodeList.count(); i++) {
		QDomElement node = domNodeList.item(i).toElement();
		if (node.isNull()) continue;

		if (node.attribute("id").compare("label") != 0) continue;

		QDomNodeList childList = node.childNodes();
		for (int j = 0; j < childList.count(); j++) {
			QDomNode child = childList.item(i);
			if (child.isText()) {
				child.setNodeValue(label);
				return doc.toString();
			}
		}
	}
		
	return svg;
}

bool TextUtils::mergeSvg(QDomDocument & doc1, const QString & svg, const QString & id) 
{
	QString errorStr;
	int errorLine;
	int errorColumn;
	if (doc1.isNull()) {
		return doc1.setContent(svg, &errorStr, &errorLine, &errorColumn);
	}

	QDomDocument doc2;
	if (!doc2.setContent(svg, &errorStr, &errorLine, &errorColumn)) return false;

	QDomElement root1 = doc1.documentElement();
	if (root1.tagName() != "svg") return false;

	QDomElement root2 = doc2.documentElement();
	if (root2.tagName() != "svg") return false;

	QDomElement id1;
	if (!id.isEmpty()) {
		id1 = findElementWithAttribute(root1, "id", id);
	}
	if (id1.isNull()) id1 = root1;

	QDomElement id2;
	if (!id.isEmpty()) {
		id2 = findElementWithAttribute(root2, "id", id);
	}
	if (id2.isNull()) id2 = root2;

	QDomNode node = id2.firstChild();
	while (!node.isNull()) {
		QDomNode nextNode = node.nextSibling();
		id1.appendChild(node);
		node = nextNode;
	}

	return true;
}

QString TextUtils::mergeSvgFinish(QDomDocument & doc) {
	return removeXMLEntities(doc.toString());
}

QString TextUtils::mergeSvg(const QString & svg1, const QString & svg2, const QString & id, bool flip) {

	QDomDocument doc1;
	if (!TextUtils::mergeSvg(doc1, svg1, id)) return ___emptyString___;

	if (!TextUtils::mergeSvg(doc1, svg2, id)) return ___emptyString___;

	if (flip) {
		QDomElement svg = doc1.documentElement();
		QString viewBox = svg.attribute("viewBox");
		QStringList coords = viewBox.split(" ", QString::SkipEmptyParts);
		qreal width = coords[2].toDouble();
		QMatrix matrix;
		matrix.translate(width / 2, 0);
		matrix.scale(-1, 1);
		matrix.translate(-width / 2, 0);
		QHash<QString, QString> attributes;
		attributes.insert("transform", svgMatrix(matrix));
		gWrap(doc1, attributes);
	}

	return mergeSvgFinish(doc1);
}

QString TextUtils::makeSVGHeader(qreal printerScale, qreal dpi, qreal width, qreal height) {

	qreal trueWidth = width / printerScale;
	qreal trueHeight = height / printerScale;

	return 
		QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n%5"
							 "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" "
							 "version=\"1.2\" baseProfile=\"tiny\" "
							 "x=\"0in\" y=\"0in\" width=\"%1in\" height=\"%2in\" "
							 "viewBox=\"0 0 %3 %4\" >\n"
							 )
						.arg(trueWidth)
						.arg(trueHeight)
						.arg(trueWidth * dpi)
						.arg(trueHeight * dpi)
						.arg(TextUtils::CreatedWithFritzingXmlComment);
}

bool TextUtils::isIllustratorFile(const QString &fileContent) {
	return fileContent.contains(AdobeIllustratorIdentifier, Qt::CaseInsensitive);
}

bool TextUtils::isIllustratorFile(const QByteArray & fileContent) {
	return fileContent.contains(AdobeIllustratorIdentifier.toUtf8());
}

bool TextUtils::isIllustratorDoc(const QDomDocument & doc) {
	QDomNodeList children = doc.childNodes();
	for ( int i=0; i < children.count(); ++i ) {
		QDomNode child = children.at(i);
		if (child.nodeType() == QDomNode::CommentNode) {
			if (isIllustratorFile(child.nodeValue())) {
				return true;
			}
		}
	}

	return false;
}

QString TextUtils::removeXMLEntities(QString svgContent) {
	return removeXMLNS(svgContent.remove(HexExpr));
}

QString TextUtils::removeXMLNS(QString svgContent) {
	// TODO: this is a bug in Qt, it would be nice to fix it there
	// but as a stopgap, it would be nice if this function removed all repetitious xmlns attributes, not just the svg one
	QString svgNS = "xmlns=\"http://www.w3.org/2000/svg\"";
	QStringList strings = svgContent.split(svgNS);
	if (strings.count() < 3) return svgContent;

	QString first = strings.takeFirst();
	return first + svgNS + strings.join("");
}

bool TextUtils::cleanSodipodi(QString &content)
{
	// clean out sodipodi stuff
	// TODO: don't bother with the core parts
	int l1 = content.length();
	content.remove(SodipodiDetector);
	if (content.length() != l1) {
		/*
		QFileInfo f(filename);
		QString p = f.absoluteFilePath();
		p.remove(':');
		p.remove('/');
		p.remove('\\');
		QFile fi(QCoreApplication::applicationDirPath() + p);
		bool ok = fi.open(QFile::WriteOnly);
		if (ok) {
			QTextStream out(&fi);
   			out << str;
			fi.close();
		}
		*/
		return true;
	}
	return false;


	/*
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	bool result = doc.setContent(bytes, &errorStr, &errorLine, &errorColumn);
	m_svgXml.clear();
	if (!result) {
		return false;
	}

	SvgFlattener flattener;
	QDomElement root = doc.documentElement();
	flattener.flattenChildren(root);
	SvgFileSplitter::fixStyleAttributeRecurse(root);
	return doc.toByteArray();
	*/
}

bool TextUtils::fixViewboxOrigin(QString &fileContent) {
	QDomDocument svgDom;

	bool fileHasChanged = false;
	if(isIllustratorFile(fileContent)) {
		QString errorMsg;
		int errorLine;
		int errorCol;
		if(!svgDom.setContent(fileContent, true, &errorMsg, &errorLine, &errorCol)) {
			return false;
		}

		QDomElement elem = svgDom.firstChildElement("svg");

		fileHasChanged = moveViewboxToTopLeftCorner(elem);

		if(fileHasChanged) {
			fileContent = svgDom.toString();
		}
	}

	return fileHasChanged;
}

bool TextUtils::moveViewboxToTopLeftCorner(QDomElement &elem) {
	QString attrName = elem.hasAttribute("viewbox")? "viewbox": "viewBox";
	QStringList vals = elem.attribute(attrName).split(" ");
	if(vals.length() == 4 && (vals[0] != "0" || vals[1] != "0")) {
		QString newValue = QString("0 0 %1 %2").arg(vals[2]).arg(vals[3]);
		elem.setAttribute(attrName,newValue);
		return true;
	}
	return false;
}

bool TextUtils::fixPixelDimensionsIn(QString &fileContent) {
	bool isIllustrator = isIllustratorFile(fileContent);
	if (!isIllustrator) return false;

	QDomDocument svgDom;

	QString errorMsg;
	int errorLine;
	int errorCol;
	if(!svgDom.setContent(fileContent, true, &errorMsg, &errorLine, &errorCol)) {
		return false;
	}

	bool fileHasChanged = false;

	if(isIllustrator) {
		QDomElement elem = svgDom.firstChildElement("svg");
		fileHasChanged = pxToInches(elem,"width",isIllustrator);
		fileHasChanged |= pxToInches(elem,"height",isIllustrator);
	}

	if (fileHasChanged) {
		fileContent = removeXMLEntities(svgDom.toString());
	}

	return fileHasChanged;
}

bool TextUtils::pxToInches(QDomElement &elem, const QString &attrName, bool isIllustrator) {
	if (!isIllustrator) return false;

	QString attrValue = elem.attribute(attrName);
	if(attrValue.endsWith("px")) {
		bool ok;
		qreal value = TextUtils::convertToInches(attrValue, &ok, isIllustrator);
		if(ok) {
			QString newValue = QString("%1in").arg(value);
			elem.setAttribute(attrName,newValue);

			return true;
		}
	}
	return false;
}

QString TextUtils::svgMatrix(QMatrix & matrix) {
	return QString("matrix(%1, %2, %3, %4, %5, %6)").arg(matrix.m11()).arg(matrix.m12()).arg(matrix.m21()).arg(matrix.m22()).arg(matrix.dx()).arg(matrix.dy());
}

void TextUtils::setSVGTransform(QDomElement & element, QMatrix & matrix)
{
	element.setAttribute("transform", svgMatrix(matrix));
}

QString TextUtils::svgTransform(const QString & svg, QTransform & transform, bool translate, const QString extras) {
	if (transform.isIdentity()) return svg;

	return QString("<g transform=\"matrix(%1,%2,%3,%4,%5,%6)\" %8 >%7</g>")
			.arg(transform.m11())
			.arg(transform.m12())
			.arg(transform.m21())
			.arg(transform.m22())
			.arg(translate ? transform.dx() : 0.0)   			
			.arg(translate ? transform.dy() : 0.0)  		
			.arg(svg)
			.arg(extras);
}

bool TextUtils::getSvgSizes(QDomDocument & doc, qreal & sWidth, qreal & sHeight, qreal & vbWidth, qreal & vbHeight) 
{
	bool isIllustrator = isIllustratorDoc(doc);

	QDomElement root = doc.documentElement();
	QString swidthStr = root.attribute("width");
	if (swidthStr.isEmpty()) return false;

	QString sheightStr = root.attribute("height");
	if (sheightStr.isEmpty()) return false;

	bool ok;
	sWidth = TextUtils::convertToInches(swidthStr, &ok, isIllustrator);
	if (!ok) return false;

	sHeight = TextUtils::convertToInches(sheightStr, &ok, isIllustrator);
	if (!ok) return false;

	bool vbWidthOK = false;
	bool vbHeightOK = false;
	QString sviewboxStr = root.attribute("viewBox");
	if (!sviewboxStr.isEmpty()) {
		QStringList strings = sviewboxStr.split(" ");
		if (strings.size() == 4) {
			qreal tempWidth = strings[2].toDouble(&vbWidthOK);
			if (vbWidthOK) {
				vbWidth = tempWidth;
			}

			qreal tempHeight= strings[3].toDouble(&vbHeightOK);
			if (vbHeightOK) {
				vbHeight = tempHeight;
			}
		}
	}

	if (vbWidthOK && vbHeightOK) return true;

	// assume that if there's no viewBox, the viewbox is at the right dpi?
	// or should the assumption be 90 or 100?  Illustrator would be 72...
	int multiplier = 90;
	if (isIllustratorFile(doc.toString())) {
		multiplier = 72;
	}

	vbWidth = sWidth * multiplier;
	vbHeight = sHeight * multiplier;
	return true;
}

bool TextUtils::findText(QDomNode & node, QString & text) {
	if (node.isText()) {
		text = node.nodeValue();
		return true;
	}

	QDomNode cnode = node.firstChild();
	while (!cnode.isNull()) {
		if (findText(cnode, text)) return true;

		cnode = cnode.nextSibling();
	}

	return false;
}

qreal TextUtils::convertToInches(const QString & string) {
	bool ok; 
	qreal retval = TextUtils::convertToInches(string, &ok, false);
	if (!ok) return 0;

	return retval;
}

QString TextUtils::escapeAnd(const QString & string) {
	QString s = Qt::escape(string);
	s.replace('\'', "&apos;");
	return s;
}

QString TextUtils::stripNonValidXMLCharacters(const QString & str) 
{
	QString result;
	QChar hs;
	bool in_hs = false;
    foreach (QChar c, str) {
		if (c.isHighSurrogate()) {
			hs = c;
			in_hs = true;
			continue;
		}

        if ((c == 0x9) ||
            (c == 0xA) ||
            (c == 0xD) ||
            ((c >= 0x20) && (c <= 0xD7FF)) ||
            ((c >= 0xE000) && (c <= 0xFFFD))) 
		{
			if (in_hs) {
				result.append(hs);
				in_hs = false;
			}
			result.append(c);
		}
		else {
			in_hs = false;
		}
    }
    return result;
}    

bool TextUtils::addCopper1(const QString & filename, QDomDocument & domDocument, const QString & srcAtt, const QString & destAtt) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QFile file(filename);
	bool result = domDocument.setContent(&file, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		domDocument.clear();			// probably redundant
		return false;
	}

    QDomElement root = domDocument.documentElement();
	QList<QDomElement> elements;
	findElementsWithAttribute(root, "id", elements);
    for (int i = 0; i < elements.count(); i++) {
        QDomElement node = elements.at(i);
        if (node.isNull()) continue;

        QString att = node.attribute("id");
		if (att == destAtt) {
			// already have copper1
			domDocument.clear();
			return false;
		}
    }

	result = false;
    for (int i = 0; i < elements.count(); i++) {
        QDomElement node = elements.at(i);
        if (node.isNull()) continue;

        QString att = node.attribute("id");
		if (att == srcAtt) {
			QDomElement g = domDocument.createElement("g");
			g.setAttribute("id", destAtt);
			node.parentNode().insertAfter(g, node);
			g.appendChild(node);
			result = true;
		}
    }

	if (!result) {
		domDocument.clear();
	}
	//else {
		//QString test = domDocument.toString();
		//DebugDialog::debug("test " + test);
	//}

	return result;

}

QString TextUtils::convertToPowerPrefix(qreal q) {
	initPowerPrefixes();

	for (int i = 0; i < PowerPrefixes.count(); i++) {
		if (q < 100 * PowerPrefixValues[i]) {
			q /= PowerPrefixValues[i];
			return QString::number(q) + PowerPrefixes[i];
		}
	}

	return QString::number(q);
}

qreal TextUtils::convertFromPowerPrefixU(QString & val, const QString & symbol) 
{
	val.replace('u', MicroSymbol);
	return convertFromPowerPrefix(val, symbol);
}

qreal TextUtils::convertFromPowerPrefix(const QString & val, const QString & symbol) 
{
	initPowerPrefixes();

	qreal multiplier = 1;
	QString temp = val;
	if (temp.endsWith(symbol)) {
		temp.chop(symbol.length());
	}

	for (int i = 0; i < PowerPrefixes.count(); i++) {
		if (PowerPrefixes[i].isEmpty()) continue;

		if (temp.endsWith(PowerPrefixes[i], Qt::CaseSensitive)) {
			multiplier = PowerPrefixValues[i];
			temp.chop(PowerPrefixes[i].length());
			break;
		}
	}
	temp = temp.trimmed();
	return temp.toDouble() * multiplier;
}

void TextUtils::initPowerPrefixes() {
	if (PowerPrefixes.count() == 0) {
		PowerPrefixes << "p" << "n" << MicroSymbol  << "u" << "m" << "" << "k" << "M" << "G" << "T";
        PowerPrefixValues << 0.000000000001 << 0.000000001 << 0.000001 << 0.000001 << 0.001 << 1 << 1000 << 1000000 << 1000000000. << 1000000000000.;
	}
}

void TextUtils::collectLeaves(QDomElement & element, int & index, QVector<QDomElement> & leaves) {
	if (element.hasChildNodes()) {
		element.removeAttribute("id");
		QDomElement c = element.firstChildElement();
		while (!c.isNull()) {
			collectLeaves(c, index, leaves);
			c = c.nextSiblingElement();
		}
	}
	else {
		leaves.insert(index, element);
		element.setAttribute("id", QString::number(index++));
	}
}

QMatrix TextUtils::elementToMatrix(QDomElement & element) {
	QString transform = element.attribute("transform");
	if (transform.isEmpty()) return QMatrix();

	return transformStringToMatrix(transform);
}

QMatrix TextUtils::transformStringToMatrix(const QString & transform) {

	// doesn't handle multiple transform attributes
	QList<qreal> floats = getTransformFloats(transform);

	if (transform.startsWith("translate")) {
		return QMatrix().translate(floats[0], (floats.length() > 1) ? floats[1] : 0);
	}
	else if (transform.startsWith("rotate")) {
		if (floats.length() == 1) {
			return QMatrix().rotate(floats[0]);
		}
		else if (floats.length() == 3) {
			return  QMatrix().translate(-floats[1], -floats[2]) * QMatrix().rotate(floats[0]) * QMatrix().translate(floats[1], floats[2]);
		}
	}
	else if (transform.startsWith("matrix")) {
        return QMatrix(floats[0], floats[1], floats[2], floats[3], floats[4], floats[5]);
	}
	else if (transform.startsWith("scale")) {
		return QMatrix().scale(floats[0], floats[1]);
	}
	else if (transform.startsWith("skewX")) {
		return QMatrix().shear(floats[0], 0);
	}
	else if (transform.startsWith("skewY")) {
		return QMatrix().shear(0, floats[0]);
	}

	return QMatrix();
}

QList<qreal> TextUtils::getTransformFloats(QDomElement & element){
	return getTransformFloats(element.attribute("transform"));
}

QList<qreal> TextUtils::getTransformFloats(const QString & transform){
    QList<qreal> list;
    int pos = 0;

	while ((pos = TextUtils::floatingPointMatcher.indexIn(transform, pos)) != -1) {
		list << transform.mid(pos, TextUtils::floatingPointMatcher.matchedLength()).toDouble();
        pos += TextUtils::floatingPointMatcher.matchedLength();
    }

#ifndef QT_NO_DEBUG
   // QString dbg = "got transform params: \n";
    //dbg += transform + "\n";
    //for(int i=0; i < list.size(); i++){
        //dbg += QString::number(list.at(i)) + " ";
    // }
    //DebugDialog::debug(dbg);
#endif

    return list;
}

void TextUtils::gWrap(QDomDocument & domDocument, const QHash<QString, QString> & attributes)
{
	QDomElement g = domDocument.createElement("g");
	foreach (QString key, attributes.keys()) {
		g.setAttribute(key, attributes.value(key, ""));
	}

	QDomNodeList nodeList = domDocument.documentElement().childNodes();
	QList<QDomNode> nodes;
	for (int i = 0; i < nodeList.count(); i++) {
		nodes.append(nodeList.item(i));
	}

	domDocument.documentElement().appendChild(g);
	foreach (QDomNode node, nodes) {
		g.appendChild(node);
	}
}

bool TextUtils::tspanRemove(QString &svg) {
	if (!svg.contains("<tspan")) return false;

	// TODO: if the <text> element has its own text node, this will be ordered first, even if the text node occurs after one or more <tspan> elements
	// this is a very unlikely occurrence, so fixing it is very low priority.

	QDomDocument svgDom;
	QString errorMsg;
	int errorLine;
	int errorCol;
	if(!svgDom.setContent(svg, true, &errorMsg, &errorLine, &errorCol)) {
		return false;
	}

	QList<QDomElement> texts;
	QDomNodeList textNodeList = svgDom.elementsByTagName("text");
    for (int i = 0; i < textNodeList.count(); i++) {
        QDomElement text = textNodeList.item(i).toElement();
		QDomElement tspan = text.firstChildElement("tspan");
		if (tspan.isNull()) continue;

		texts.append(text);
	}

	foreach (QDomElement text, texts) {
		QDomElement g = svgDom.createElement("g");
		text.parentNode().replaceChild(g, text);
		QDomNamedNodeMap attributes = text.attributes();
		for (int i = 0; i < attributes.count(); i++) {
			QDomNode attribute = attributes.item(i);
			g.setAttribute(attribute.nodeName(), attribute.nodeValue());
		}
		QString defaultX = g.attribute("x");
		QString defaultY = g.attribute("y");
		g.removeAttribute("x");
		g.removeAttribute("y");

		copyText(svgDom, g, text, defaultX, defaultY, false);

		QDomElement tspan = text.firstChildElement("tspan");
		while (!tspan.isNull()) {
			copyText(svgDom, g, tspan, defaultX, defaultY, true);
			tspan = tspan.nextSiblingElement("tspan");
		}
	}

	svg = removeXMLEntities(svgDom.toString());
	return true;
}

QDomElement TextUtils::copyText(QDomDocument & svgDom, QDomElement & parent, QDomElement & text, const QString & defaultX, const QString & defaultY, bool copyAttributes)
{
	QDomNode cnode = text.firstChild();
	while (!cnode.isNull()) {
		if (cnode.isText()) {
			QDomElement newText = svgDom.createElement("text");
			parent.appendChild(newText);
			newText.setAttribute("x", defaultX);
			newText.setAttribute("y", defaultY);
			QDomNode textValue = svgDom.createTextNode(cnode.nodeValue());
			newText.appendChild(textValue);

			if (copyAttributes) {
				QDomNamedNodeMap attributes = text.attributes();
				for (int i = 0; i < attributes.count(); i++) {
					QDomNode attribute = attributes.item(i);
					newText.setAttribute(attribute.nodeName(), attribute.nodeValue());
				}
			}

			// normalize means there's only one text child node, so we can return now
			return newText;
		}

		cnode = cnode.nextSibling();
	}

	return ___emptyElement___;
}

void TextUtils::slamStrokeAndFill(QDomElement & element, const QString & stroke, const QString & fill)
{
	// assumes style elements have been normalized already
	QString strokeAtt = element.attribute("stroke");
	QString fillAtt = element.attribute("fill");
	if (!strokeAtt.isEmpty() || !fillAtt.isEmpty()) {
		element.setAttribute("stroke", stroke);
		element.setAttribute("fill", fill);
	}

	QDomElement child = element.firstChildElement();
	while (!child.isNull()) {
		slamStrokeAndFill(child, stroke, fill);
		child = child.nextSiblingElement();
	}
}


struct MatchThing
{
	int pos;
	int len;
	qreal val;
};

QString TextUtils::incrementTemplate(const QString & filename, int pins, qreal increment, MultiplyPinFunction multiFun, CopyPinFunction copyFun) 
{
	QFile file(filename);
	file.open(QFile::ReadOnly);
	QString templateString = file.readAll();
	file.close();

	return incrementTemplateString(templateString, pins, increment, multiFun, copyFun);
}

QString TextUtils::incrementTemplateString(const QString & templateString, int pins, qreal increment, MultiplyPinFunction multiFun, CopyPinFunction copyFun)
{
	QString string;

	QRegExp uMatcher("\\[([\\.\\d]+)\\]");
	MatchThing matchThings[32];
	int pos = 0;
	unsigned int matchThingIndex = 0;
	while ((pos = uMatcher.indexIn(templateString, pos)) != -1) {
		MatchThing * mt = &matchThings[matchThingIndex++];
		mt->pos = pos;
		mt->len = uMatcher.matchedLength();
		mt->val = uMatcher.cap(1).toDouble();
		pos += uMatcher.matchedLength();
		if (matchThingIndex >= sizeof(matchThings) / sizeof(MatchThing)) break;
	}

	for (int i = 0; i < pins; i++) {
		QString argCopy(templateString);
		for (int j = matchThingIndex - 1; j >= 0; j--) {
			MatchThing * mt = &matchThings[j];
			argCopy.replace(mt->pos, mt->len, (*multiFun)(i, increment, mt->val));   
		}
		string += (*copyFun)(i, argCopy);
	}

	return string;
}

QString TextUtils::standardCopyPinFunction(int pin, const QString & argString)
{
	return argString.arg(pin);
}

QString TextUtils::standardMultiplyPinFunction(int pin, qreal increment, qreal value)
{
	return QString::number(value + (pin * increment));
}

