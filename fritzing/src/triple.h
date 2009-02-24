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

$Revision: 2467 $:
$Author: cohen@irascible.com $:
$Date: 2009-02-23 19:06:21 +0100 (Mon, 23 Feb 2009) $

********************************************************************/

#ifndef TRIPLE_H
#define TRIPLE_H

template <class T1, class T2, class T3>
struct Triple {
public:
	Triple() {}

	Triple(T1 _first, T2 _second, T3 _third) {
		first = _first;
		second = _second;
		third = _third;
	}

	T1 first;
	T2 second;
	T3 third;
};

struct StringTriple : public Triple<QString, QString, QString> {
	StringTriple() : Triple<QString, QString, QString>() {}
	StringTriple(QString _first, QString _second, QString _third)
		: Triple<QString, QString, QString>(_first, _second, _third) {}
};


#endif
