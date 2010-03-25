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

#ifndef GEDAELEMENT2SVG_H
#define GEDAELEMENT2SVG_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariant>

class GedaElement2Svg
{

public:
	GedaElement2Svg();
	QString convert(QString filename);

protected:
	int countArgs(QVector<QVariant> & stack, int ix);
	QString convertPin(QVector<QVariant> & stack, int ix, int argCount, bool mils);
	QString convertPad(QVector<QVariant> & stack, int ix, int argCount, bool mils);
	QString convertArc(QVector<QVariant> & stack, int ix, int argCount, bool mils);
	QString offsetMin(const QString & svg);
	QString unquote(const QString &);
	void fixQuad(int quad, qreal & px, qreal & py);
	int reflectQuad(int angle, int & quad);
	QString getPinID(QString & number, QString & name, bool & repeat);

protected:
	int m_maxX;
	int m_maxY;
	int m_minX;
	int m_minY;
	QStringList m_nameList;
	QStringList m_numberList;
};

#endif // GEDAELEMENT2SVG_H
