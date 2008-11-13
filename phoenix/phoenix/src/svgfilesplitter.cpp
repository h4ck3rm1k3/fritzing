#include "svgfilesplitter.h"

#include "misc.h"
#include "debugdialog.h"
#include <QDomDocument>
#include <QFile>

SvgFileSplitter::SvgFileSplitter()
{
}

const bool SvgFileSplitter::split(const QString & filename, const QString & elementID)
{
	m_byteArray.clear();

	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return false;
	}

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument domDocument;

	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "svg") {
		return false;
	}

	QDomElement element = findElementWithAttribute(root, "id", elementID);
	if (element.isNull()) return false;

	while (!root.firstChild().isNull()) {
		root.removeChild(root.firstChild());
	}

	root.appendChild(element);
	m_byteArray = domDocument.toByteArray();

	return true;
}

QDomElement SvgFileSplitter::findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue) {
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

const QByteArray & SvgFileSplitter::byteArray() {
	return m_byteArray;
}
