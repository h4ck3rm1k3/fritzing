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

$Revision: 4116 $:
$Author: cohen@irascible.com $:
$Date: 2010-04-15 15:12:52 +0200 (Thu, 15 Apr 2010) $

********************************************************************/



#include "highlighter.h"
#include "../debugdialog.h"

#include <QRegExp>
#include <stdlib.h>


Highlighter::Highlighter(QTextEdit * textEdit) : QSyntaxHighlighter(textEdit)
{
}

Highlighter::~Highlighter()
{
}

void Highlighter::highlightBlock(const QString &text)
 {
     QTextCharFormat myClassFormat;
     myClassFormat.setFontWeight(QFont::Bold);
     myClassFormat.setForeground(Qt::darkMagenta);
     QString pattern = "\\bhello\\b";

     QRegExp expression(pattern);
     int index = text.indexOf(expression);
     while (index >= 0) {
         int length = expression.matchedLength();
         setFormat(index, length, myClassFormat);
         index = text.indexOf(expression, index + length);
     }
 }
