/*
 * (c) Fachhochschule Potsdam
 */

#include "partinstancestuff.h"

PartInstanceStuff::PartInstanceStuff() {
	m_domDocument = NULL;
	m_title = QString::null;
}

PartInstanceStuff::PartInstanceStuff(QDomDocument * domDocument, const QString & path) {
	m_domDocument = domDocument;

        Q_UNUSED(path);
	//TODO Mariano: perhaps we should grab the instance title right here
	/*QDomElement root = domDocument->documentElement();
	if (root.isNull()) {
		return;
	}

	if (root.tagName() != "module") {
		return;
	}

	loadText(root, "title", m_title);*/
}

void PartInstanceStuff::loadText(QDomElement parent, QString tagName, QString &field) {
	QDomElement tagElement = parent.firstChildElement(tagName);
	if (!tagElement.isNull()) {
		field = tagElement.text();
	}
}

const QString & PartInstanceStuff::title() {
	return m_title;
}
void PartInstanceStuff::setTitle(QString title) {
	m_title = title;
}
