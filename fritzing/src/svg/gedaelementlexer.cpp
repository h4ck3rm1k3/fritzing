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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include "gedaelementlexer.h"
#include "gedaelementgrammar_p.h"
#include "../utils/textutils.h"
#include <qdebug.h>

GedaElementLexer::GedaElementLexer(const QString &source)
{

	//m_stringMatcher.setPattern("\"([^\"]*)\"");
	m_stringMatcher.setPattern("\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"");
	m_integerMatcher.setPattern("[-+]?\\b\\d+\\b");			
	m_hexMatcher.setPattern("0[xX][0-9a-fA-F]+\\b");			
    m_source = clean(source);
    m_chars = m_source.unicode();
    m_size = m_source.size();
	qDebug() << m_source;
    m_pos = 0;
    m_current = next();
}

GedaElementLexer::~GedaElementLexer()
{
}

QString GedaElementLexer::clean(const QString & source) {
	// clean it up to make it easier to parse
	
	QString s1 = source;
	QString s2 = s1.replace(TextUtils::FindWhitespace, " ");
	return s2;
}

int GedaElementLexer::lex()
{
	while (true) {
		if (m_hexMatcher.indexIn(m_source, m_pos - 1) == m_pos - 1) {
			bool ok;
			m_currentNumber = m_source.mid(m_pos - 1, m_hexMatcher.matchedLength()).toLong(&ok, 16);
			m_pos += m_hexMatcher.matchedLength() - 1;
			next();
			return GedaElementGrammar::NUMBER;
		}
		else if (m_integerMatcher.indexIn(m_source, m_pos - 1) == m_pos - 1) {
			m_currentNumber = m_source.mid(m_pos - 1, m_integerMatcher.matchedLength()).toLong();
			m_pos += m_integerMatcher.matchedLength() - 1;
			next();
			return GedaElementGrammar::NUMBER;
		}
		else if (m_stringMatcher.indexIn(m_source, m_pos - 1) == m_pos - 1) {
			m_currentString = m_source.mid(m_pos - 1, m_stringMatcher.matchedLength());
			m_pos += m_stringMatcher.matchedLength() - 1;
			next();
			return GedaElementGrammar::STRING;
		}
		else if (m_current.isSpace()) {
			next();
			continue;
		}
		else if (m_current.isNull()) {
			return GedaElementGrammar::EOF_SYMBOL;
		} 
		else if (m_current == QLatin1Char('(')) {
			next();
			return GedaElementGrammar::LEFTPAREN;
		} 
		else if (m_current == QLatin1Char(')')) {
			next();
			return GedaElementGrammar::RIGHTPAREN;
		} 
		else if (m_current == QLatin1Char('[')) {
			next();
			return GedaElementGrammar::LEFTBRACKET;
		} 
		else if (m_current == QLatin1Char(']')) {
			next();
			return GedaElementGrammar::RIGHTBRACKET;
		} 
		else if (m_source.indexOf("elementline", m_pos - 1, Qt::CaseInsensitive) == m_pos - 1) {
			m_currentCommand = "elementline";
			m_pos += m_currentCommand.length() - 1;
			next();
			return GedaElementGrammar::ELEMENTLINE;
		} 
		else if (m_source.indexOf("elementarc", m_pos - 1, Qt::CaseInsensitive) == m_pos - 1) {
			m_currentCommand = "elementarc";
			m_pos += m_currentCommand.length() - 1;
			next();
			return GedaElementGrammar::ELEMENTARC;
		} 
		else if (m_source.indexOf("element", m_pos - 1, Qt::CaseInsensitive) == m_pos - 1) {
			m_currentCommand = "element";
			m_pos += m_currentCommand.length() - 1;
			next();
			return GedaElementGrammar::ELEMENT;
		} 
		else if (m_source.indexOf("pad", m_pos - 1, Qt::CaseInsensitive) == m_pos - 1) {
			m_currentCommand = "pad";
			m_pos += m_currentCommand.length() - 1;
			next();
			return GedaElementGrammar::PAD;
		} 
		else if (m_source.indexOf("pin", m_pos - 1, Qt::CaseInsensitive) == m_pos - 1) {
			m_currentCommand = "pin";
			m_pos += m_currentCommand.length() - 1;
			next();
			return GedaElementGrammar::PIN;
		} 

		
		return -1;
	}
}

QChar GedaElementLexer::next()
{
    if (m_pos < m_size)
        m_current = m_chars[m_pos++];
    else
        m_current = QChar();
    return m_current;
}

QString GedaElementLexer::currentCommand() {
	return m_currentCommand;
}

double GedaElementLexer::currentNumber() {
	return m_currentNumber;
}

QString GedaElementLexer::currentString() {
	return m_currentString;
}
