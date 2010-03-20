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


#ifndef SVGFILESPLITTER_H_
#define SVGFILESPLITTER_H_

#include <QString>
#include <QByteArray>
#include <QDomElement>
#include <QObject>
#include <QMatrix>
#include <QPainterPath>
#include <QRegExp>

struct PathUserData {
	QString string;
    QMatrix transform;
	qreal sNewWidth;
	qreal sNewHeight;
	qreal vbWidth; 
	qreal vbHeight;
	qreal x;
	qreal y;
	QPainterPath * painterPath;
};

class SvgFileSplitter : public QObject {
	Q_OBJECT

public:
	SvgFileSplitter();
	bool split(const QString & filename, const QString & elementID);
	bool splitString(QString & contents, const QString & elementID);
	const QByteArray & byteArray();
	const QDomDocument & domDocument();
	bool normalize(qreal dpi, const QString & elementID, bool blackOnly);
	QString shift(qreal x, qreal y, const QString & elementID);
	QString elementString(const QString & elementID);
    virtual bool parsePath(const QString & data, const char * slot, PathUserData &, QObject * slotTarget, bool convertHV);
	QPainterPath painterPath(qreal dpi, const QString & elementID);			// note: only partially implemented

public:
	static bool getSvgSizeAttributes(const QString & path, QString & width, QString & height, QString & viewBox);
	static bool changeStrokeWidth(const QString & svg, qreal delta, bool absolute, QByteArray &);
	static bool changeColors(const QString & svg, QString & toColor, QStringList & exceptions, QByteArray &);
	static void changeColors(QDomElement & element, QString & toColor, QStringList & exceptions);
	static void fixStyleAttributeRecurse(QDomElement & element);
	static void fixStyleAttribute(QDomElement & element, QString & style, const QString & attributeName);
    static QList<qreal> getTransformFloats(QDomElement & element);
	static QList<qreal> getTransformFloats(const QString & transform);
	static QMatrix elementToMatrix(QDomElement & element);

protected:
	void normalizeChild(QDomElement & childElement, 
						qreal sNewWidth, qreal sNewHeight,
						qreal vbWidth, qreal vbHeight, bool blackOnly);
	bool normalizeAttribute(QDomElement & element, const char * attributeName, qreal num, qreal denom);
	virtual void shiftChild(QDomElement & element, qreal x, qreal y);
	void setStrokeOrFill(QDomElement & element, bool blackOnly);
	void painterPathChild(QDomElement & element, QPainterPath & ppath);			// note: only partially implemented
	void normalizeTranslation(QDomElement & element, 
							qreal sNewWidth, qreal sNewHeight,
							qreal vbWidth, qreal vbHeight);

protected:
	static void changeStrokeWidth(QDomElement & element, qreal delta, bool absolute);
	static void fixStyleAttribute(QDomElement & element);

protected slots:
	void normalizeCommandSlot(QChar command, bool relative, QList<double> & args, void * userData);
	void shiftCommandSlot(QChar command, bool relative, QList<double> & args, void * userData);
    virtual void rotateCommandSlot(QChar, bool, QList<double> &, void *){}
	void painterPathCommandSlot(QChar command, bool relative, QList<double> & args, void * userData);
	void convertHVSlot(QChar command, bool relative, QList<double> & args, void * userData);

protected:
	QByteArray m_byteArray;
	QDomDocument m_domDocument;

};

#endif
