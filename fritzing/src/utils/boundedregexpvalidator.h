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

$Revision: 2829 $:
$Author: cohen@irascible.com $:
$Date: 2009-04-17 00:22:27 +0200 (Fri, 17 Apr 2009) $

********************************************************************/

#ifndef BOUNDEDREGEXPVALIDATOR_H
#define BOUNDEDREGEXPVALIDATOR_H

#include <QRegExpValidator>
#include <limits>

typedef qreal (*Converter)(const QString &);

class BoundedRegExpValidator : public QRegExpValidator 
{
public:
	BoundedRegExpValidator(QObject * parent) : QRegExpValidator(parent) {
		m_max = std::numeric_limits<double>::max();
		m_min = std::numeric_limits<double>::min();
		m_converter = NULL;
	}

	void setBounds(qreal min, qreal max) {
		m_min = min;
		m_max = max;
	}

	void setConverter(Converter converter) {
		m_converter = converter;
	}

	QValidator::State validate ( QString & input, int & pos ) const {
		QValidator::State state = QRegExpValidator::validate(input, pos);
		if (state == QValidator::Invalid) return state;
		if (state == QValidator::Intermediate) return state;
		if (m_converter == NULL) return state;

		qreal converted = m_converter(input);
		if (converted < m_min) return QValidator::Invalid;
		if (converted > m_max) return QValidator::Invalid;

		return QValidator::Acceptable;
	}

protected:
	qreal m_min;
	qreal m_max;
	Converter m_converter;
};

#endif