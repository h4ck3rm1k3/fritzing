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

#include "textutils.h"
#include "misc.h"
#include <QRegExp>
#include <QBuffer>

const QRegExp TextUtils::FindWhitespace("[\\s]+");
const QRegExp TextUtils::SodipodiDetector("((inkscape)|(sodipodi)):[^=\\s]+=\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"");

const QRegExp HexExpr("&#x[0-9a-fA-F];");   // &#x9; &#xa; &#xd;

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

bool TextUtils::squashNotElement(QString & svg, const QString & elementName, const QString &attName, const QRegExp &matchContent) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	if (!doc.setContent(svg, &errorStr, &errorLine, &errorColumn)) return false;

	QDomElement root = doc.documentElement();
	QString tagName = root.tagName();
	bool result = false;
    squashNotElement(root, elementName, attName, matchContent, result);
	root.setTagName(tagName);
	if (result) {
		svg = removeXMLEntities(doc.toString());
	}
	return result;
}

void TextUtils::squashNotElement(QDomElement & element, const QString & elementName, const QString &attName, const QRegExp &matchContent, bool & result) {
	if (element.tagName().compare(elementName) != 0) {
		element.setTagName("g");
		result = true;
	}
    else {
        if (!attName.isEmpty()) {
            QString att = element.attribute(attName);
            if (att.isEmpty()) {
                element.setTagName("g");
                result = true;
            }
            else {
                if (!matchContent.isEmpty()) {
                    if (matchContent.indexIn(att) < 0) {
                        element.setTagName("g");
                        result = true;
                    }
                }
            }
        }
    }

	QDomElement child = element.firstChildElement();
	while (!child.isNull()) {
        squashNotElement(child, elementName, attName, matchContent, result);
		child = child.nextSiblingElement();
	}
}

bool TextUtils::squashElement(QString & svg, const QString & elementName, const QString &attName, const QRegExp &matchContent) {
    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if (!doc.setContent(svg, &errorStr, &errorLine, &errorColumn)) return false;

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

    if (result) {
        svg = removeXMLEntities(doc.toString());
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

QString TextUtils::mergeSvg(const QString & svg1, const QString & svg2) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc1;
	if (!doc1.setContent(svg1, &errorStr, &errorLine, &errorColumn)) return ___emptyString___;

	QDomDocument doc2;
	if (!doc2.setContent(svg2, &errorStr, &errorLine, &errorColumn)) return ___emptyString___;

	QDomElement root1 = doc1.documentElement();
	if (root1.tagName() != "svg") return ___emptyString___;

	QDomElement root2 = doc2.documentElement();
	if (root2.tagName() != "svg") return ___emptyString___;

	QDomNode node = root2.firstChild();
	while (!node.isNull()) {
		QDomNode nextNode = node.nextSibling();
		root1.appendChild(node);
		node = nextNode;
	}

	return removeXMLEntities(doc1.toString());
}

QString TextUtils::toHtmlImage(QPixmap *pixmap, const char* format) {
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	pixmap->save(&buffer, format); // writes pixmap into bytes in PNG format
	return QString("data:image/%1;base64,%2").arg(QString(format).toLower()).arg(QString(bytes.toBase64()));
}

QString TextUtils::makeSVGHeader(qreal printerScale, qreal dpi, qreal width, qreal height) {

	qreal trueWidth = width / printerScale;
	qreal trueHeight = height / printerScale;

	return 
		QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> "
							 "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" "
							 "version=\"1.2\" baseProfile=\"tiny\" "
							 "x=\"0in\" y=\"0in\" width=\"%1in\" height=\"%2in\" "
							 "viewBox=\"0 0 %3 %4\" >")
						.arg(trueWidth)
						.arg(trueHeight)
						.arg(trueWidth * dpi)
						.arg(trueHeight * dpi);
}

bool TextUtils::isIllustratorFile(const QString &fileContent) {
	return fileContent.contains("<!-- Generator: Adobe Illustrator", Qt::CaseInsensitive);
}


QString TextUtils::removeXMLEntities(QString svgContent) {
	return svgContent.remove(HexExpr);
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
		fileHasChanged =  pxToInches(elem,"width",isIllustrator);
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

