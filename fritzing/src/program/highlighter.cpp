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
#include "syntaxer.h"

#include "../debugdialog.h"

#include <QRegExp>
#include <stdlib.h>

enum BlockState {
	InComment = 1
};


Highlighter::Highlighter(QTextEdit * textEdit) : QSyntaxHighlighter(textEdit)
{
	m_syntaxer = NULL;
}

Highlighter::~Highlighter()
{
}

void Highlighter::setSyntaxer(Syntaxer * syntaxer) {
	m_syntaxer = syntaxer;
}

void Highlighter::highlightBlock(const QString &text)
{
	if (!m_syntaxer) return;

    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setForeground(Qt::darkMagenta);



	int lastWordBreak = 0;
	int textLength = text.length();
	int b;
	while (lastWordBreak < textLength) {
		for (b = lastWordBreak; b < textLength; b++) {
			if (!isWordChar(text.at(b))) break;
		}
		
		if (b > lastWordBreak) {
			TrieLeaf * leaf = NULL;
			if (m_syntaxer->matches(text.mid(lastWordBreak, b - lastWordBreak), leaf)) {
				setFormat(lastWordBreak, b - lastWordBreak, myClassFormat);
			}
		}
		
		lastWordBreak = b + 1;
	}
}

bool Highlighter::isWordChar(QChar c) {
	return c.isLetterOrNumber() || c == '#' || c == '_';
}
