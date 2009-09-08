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

$Revision: 1671 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-28 12:20:31 +0100 (Fri, 28 Nov 2008) $

********************************************************************/

#ifndef GEDAELEMENTLEXER_H
#define GEDAELEMENTLEXER_H

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QRegExp>

class GedaElementLexer
{
public:
    GedaElementLexer(const QString &source);
    ~GedaElementLexer();
    int lex();
	QString currentCommand();
	double currentNumber();
	QString currentString();

protected:
    QChar next();
	QString clean(const QString & source);

protected:
    QString m_source;
    const QChar *m_chars;
    int m_size;
    int m_pos;
    QChar m_current;
	QString m_currentCommand;
	long m_currentNumber;
	QString m_currentString;
	QRegExp m_integerMatcher;
	QRegExp m_hexMatcher;
	QRegExp m_stringMatcher;
};

#endif
