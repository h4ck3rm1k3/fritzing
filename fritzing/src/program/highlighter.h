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

$Revision: 4043 $:
$Author: cohen@irascible.com $:
$Date: 2010-03-19 13:06:00 +0100 (Fri, 19 Mar 2010) $

********************************************************************/


#ifndef HIGHLIGHTER_H_
#define HIGHLIGHTER_H_

#include <QSyntaxHighlighter>
#include <QTextEdit>

class Highlighter : public QSyntaxHighlighter
{
Q_OBJECT

public:
	Highlighter(QTextEdit * parent);
	~Highlighter();

protected:
	void highlightBlock(const QString & text);

};

#endif /* HIGHLIGHTER_H_ */
