/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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


#ifndef SVGFILESPLITTER_H_
#define SVGFILESPLITTER_H_

#include <QString>
#include <QByteArray>
#include <QDomElement>
#include <QObject>

struct PathUserData {
	QString string;
	qreal sNewWidth;
	qreal sNewHeight;
	qreal vbWidth; 
	qreal vbHeight;
	qreal x;
	qreal y;
};

class SvgFileSplitter : public QObject {
	Q_OBJECT

public:
	SvgFileSplitter();
	bool split(const QString & filename, const QString & elementID);
	const QByteArray & byteArray();
	const QDomDocument & domDocument();
	bool normalize(qreal dpi, const QString & elementID);
	QString shift(qreal x, qreal y, const QString & elementID);
	QString elementString(const QString & elementID);

protected:
	QDomElement findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue);
	void normalizeChild(QDomElement & childElement, 
						qreal sNewWidth, qreal sNewHeight,
						qreal vbWidth, qreal vbHeight);
	bool normalizeAttribute(QDomElement & element, const char * attributeName, qreal num, qreal denom);
	void shiftChild(QDomElement & element, qreal x, qreal y);
	bool shiftAttribute(QDomElement & element, const char * attributeName, qreal d);
	bool parsePath(const QString & data, const char * slot, PathUserData &);
	void setStrokeOrFill(QDomElement & element);

protected slots:
	void normalizeCommandSlot(QChar command, bool relative, QList<double> & args, void * userData);
	void shiftCommandSlot(QChar command, bool relative, QList<double> & args, void * userData);

protected:
	QByteArray m_byteArray;
	QDomDocument m_domDocument;

};

#endif
