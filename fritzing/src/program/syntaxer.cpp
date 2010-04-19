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

#include "syntaxer.h"
#include "../debugdialog.h"
#include "../utils/textutils.h"

#include <QRegExp>
#include <QXmlStreamReader>

Syntaxer::Syntaxer() : QObject() {
	m_trieRoot = NULL;
}

Syntaxer::~Syntaxer() {
	if (m_trieRoot) {
		delete m_trieRoot;
	}
}

bool Syntaxer::loadSyntax(const QString &filename)
 {
	QFile file(filename);

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) return false;
	if (root.tagName() != "language") return false;

	QDomElement highlighting = root.firstChildElement("highlighting");
	if (highlighting.isNull()) return false;

	QDomElement general = root.firstChildElement("general");
	if (general.isNull()) return false;

	m_name = root.attribute("name");
	m_trieRoot = new TrieNode('\0');

	QDomElement list = highlighting.firstChildElement("list");
	while (!list.isNull()) {
		loadList(list);
		list = list.nextSiblingElement("list");
	}

	QDomElement comments = general.firstChildElement("comments");
	if (!comments.isNull()) {
		QDomElement comment = comments.firstChildElement("comment");
		while (!comment.isNull()) {
			CommentInfo * commentInfo = new CommentInfo(comment.attribute("start"), comment.attribute("end"));
			m_commentInfo.append(commentInfo);
			comment = comment.nextSiblingElement("comment");
		}
	}



	return false;

 }

QString Syntaxer::parseForName(const QString & filename)
{
	QFile file(filename);
	file.open(QFile::ReadOnly);
	QXmlStreamReader xml(&file);
    xml.setNamespaceProcessing(false);

	while (!xml.atEnd()) {
        switch (xml.readNext()) {
			case QXmlStreamReader::StartElement:
				if (xml.name().toString().compare("language") == 0) {
					return xml.attributes().value("name").toString();
				}
				break;
			default:
				break;
		}
	}

	return "";
}

void Syntaxer::loadList(QDomElement & list) {
	QString name = list.attribute("name");
	QDomElement item = list.firstChildElement("item");
	while (!item.isNull()) {
		QString text;
		if (TextUtils::findText(item, text)) {
			m_trieRoot->addString(text.trimmed(), false, new SyntaxerTrieLeaf(name));
		}
		item = item.nextSiblingElement("item");
	}
}

bool Syntaxer::matches(const QString & string, TrieLeaf * & leaf) {
	if (m_trieRoot == NULL) return false;

	QString temp = string;
	return m_trieRoot->matches(temp, leaf);
}

//////////////////////////////////////////////

SyntaxerTrieLeaf::SyntaxerTrieLeaf(QString name) {
	m_name = name;
}

SyntaxerTrieLeaf::~SyntaxerTrieLeaf()
{
}

//////////////////////////////////////////////

CommentInfo::CommentInfo(const QString & start, const QString & end) {
	m_start = start;
	m_end = end;
	if (end.isEmpty()) {
		m_multiLine = false;
	}
	else {
		m_multiLine = true;
	}
}

