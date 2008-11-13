/*
 * (c) Fachhochschule Potsdam
 */

#ifndef SVGFILESPLITTER_H_
#define SVGFILESPLITTER_H_

#include <QString>
#include <QByteArray>
#include <QDomElement>

class SvgFileSplitter {
	
public:
	SvgFileSplitter();
	const bool split(const QString & filename, const QString & elementID);
	const QByteArray & byteArray();

protected:
	QDomElement findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue);


protected:
	QByteArray m_byteArray;
};

#endif
