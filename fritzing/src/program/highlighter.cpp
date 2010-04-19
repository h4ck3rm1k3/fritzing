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

#include "highlighter.h"
#include "syntaxer.h"

#include "../debugdialog.h"

#include <QRegExp>
#include <stdlib.h>


QTextCharFormat myClassFormat;
QTextCharFormat commentFormat;

Highlighter::Highlighter(QTextEdit * textEdit) : QSyntaxHighlighter(textEdit)
{
	m_syntaxer = NULL;

	// just temporary
    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setForeground(Qt::darkMagenta);
    commentFormat.setFontWeight(QFont::Bold);
    commentFormat.setForeground(Qt::gray);
	commentFormat.setFontItalic(true);
}

Highlighter::~Highlighter()
{
}

void Highlighter::setSyntaxer(Syntaxer * syntaxer) {
	m_syntaxer = syntaxer;
}

Syntaxer * Highlighter::syntaxer() {
	return m_syntaxer;
}

void Highlighter::highlightBlock(const QString &text)
{
	if (!m_syntaxer) return;
	
	if (text.isEmpty()) {
		setCurrentBlockState(previousBlockState());
		return;
	}

	setCurrentBlockState(0);
	int startIndex = -1;
	const CommentInfo * currentCommentInfo = NULL;
	if (previousBlockState() <= 0) {
		m_syntaxer->matchCommentStart(text, 0, startIndex, currentCommentInfo);
	}
	else {
		currentCommentInfo = m_syntaxer->getCommentInfo(previousBlockState() - 1);
		startIndex = 0;
	}

	QString noComment = text;

	while (startIndex >= 0) {
		int endIndex = currentCommentInfo->m_multiLine ? text.indexOf(currentCommentInfo->m_end, startIndex, currentCommentInfo->m_caseSensitive) : text.length();
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(currentCommentInfo->m_index + 1);
			commentLength = text.length() - startIndex;
		} 
		else {
			commentLength = endIndex - startIndex + currentCommentInfo->m_end.length();
		}
		noComment.replace(startIndex, commentLength, QString(commentLength, ' '));
		setFormat(startIndex, commentLength, commentFormat);
		m_syntaxer->matchCommentStart(text, startIndex + commentLength, startIndex, currentCommentInfo);
	}

	highlightTerms(noComment);
}


void Highlighter::highlightTerms(const QString & text) {
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
