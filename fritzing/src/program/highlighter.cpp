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

#define STRINGOFFSET 10
#define COMMENTOFFSET 100

QTextCharFormat keywordFormat;
QTextCharFormat commentFormat;
QTextCharFormat stringFormat;

Highlighter::Highlighter(QTextEdit * textEdit) : QSyntaxHighlighter(textEdit)
{
	m_syntaxer = NULL;

	// just temporary
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(Qt::blue);
	commentFormat.setFontItalic(true);
    commentFormat.setForeground(Qt::gray);
	stringFormat.setForeground(Qt::red);
}

Highlighter::~Highlighter()
{
}

void Highlighter::loadStyles(const QString & filename) {
	QFile file(filename);

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) return;
	if (root.tagName() != "styles") return;

	QDomElement style = root.firstChildElement("style");
	while (!style.isNull()) {
		QTextCharFormat * tcf = new QTextCharFormat();
		QColor color(Qt::black);
		QString colorString = style.attribute("color");
		if (!colorString.isEmpty()) {
			color.setNamedColor(colorString);
			tcf->setForeground(QBrush(color));
		}
		QString italicString = style.attribute("italic");
		if (italicString.compare("1") == 0) {
			tcf->setFontItalic(true);
		}
		QString boldString = style.attribute("bold");
		if (boldString.compare("1") == 0) {
			tcf->setFontWeight(QFont::Bold);
		}
		QString underlineString = style.attribute("underline");
		if (underlineString.compare("1") == 0) {
			tcf->setFontUnderline(true);
		}
		
		style = style.nextSiblingElement("style");
	}

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
	int startCommentIndex = -1;
	int startStringIndex = -1;
	const CommentInfo * currentCommentInfo = NULL;
	int pbs = previousBlockState();
	if (pbs <= 0) {
		m_syntaxer->matchCommentStart(text, 0, startCommentIndex, currentCommentInfo);
	}
	else if (pbs >= COMMENTOFFSET) {
		currentCommentInfo = m_syntaxer->getCommentInfo(previousBlockState() - COMMENTOFFSET);
		startCommentIndex = 0;
	}
	else if (pbs == STRINGOFFSET) {
		startStringIndex = 0;
	}

	QString noComment = text;

	while (startCommentIndex >= 0) {
		int endIndex = currentCommentInfo->m_multiLine ? text.indexOf(currentCommentInfo->m_end, startCommentIndex, currentCommentInfo->m_caseSensitive) : text.length();
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(currentCommentInfo->m_index + COMMENTOFFSET);
			commentLength = text.length() - startCommentIndex;
		} 
		else {
			commentLength = endIndex - startCommentIndex + currentCommentInfo->m_end.length();
		}
		noComment.replace(startCommentIndex, commentLength, QString(commentLength, ' '));
		setFormat(startCommentIndex, commentLength, commentFormat);
		m_syntaxer->matchCommentStart(text, startCommentIndex + commentLength, startCommentIndex, currentCommentInfo);
	}

	highlightStrings(startStringIndex, noComment);
	highlightTerms(noComment);
}

void Highlighter::highlightStrings(int startStringIndex, QString & text) {
	if (startStringIndex < 0) {
		startStringIndex = m_syntaxer->matchStringStart(text, 0);
	}

	while (startStringIndex >= 0) {
		int endIndex = m_syntaxer->matchStringEnd(text, startStringIndex + 1);
		int stringLength;
		if (endIndex == -1) {
			setCurrentBlockState(STRINGOFFSET);
			stringLength = text.length() - startStringIndex;
		} 
		else {
			stringLength = endIndex - startStringIndex + 1;
		}
		text.replace(startStringIndex, stringLength, QString(stringLength, ' '));
		setFormat(startStringIndex, stringLength, stringFormat);
		startStringIndex = m_syntaxer->matchStringStart(text, startStringIndex + stringLength);
	}
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
				setFormat(lastWordBreak, b - lastWordBreak, keywordFormat);
			}
		}
		
		lastWordBreak = b + 1;
	}
}

bool Highlighter::isWordChar(QChar c) {
	return c.isLetterOrNumber() || c == '#' || c == '_';
}
