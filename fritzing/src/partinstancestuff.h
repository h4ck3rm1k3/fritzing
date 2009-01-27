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



#ifndef PARTINSTANCESTUFF_H_
#define PARTINSTANCESTUFF_H_

#include <QDomDocument>
#include <QHash>

class PartInstanceStuff {

public:
	PartInstanceStuff();
	PartInstanceStuff(QDomDocument *, const QString & path);

	const QString & title();
	void setTitle(QString title);

	const QString & text();
	void setText(QString text);

	const QHash<QString,QString> & properties();
	void setProperties(const QHash<QString,QString> &properties);

protected:
	void loadTagText(QDomElement parent, QString tagName, QString &field);

	QDomDocument* m_domDocument;

	QString m_title;
	QString m_text;
	QHash<QString,QString> m_properties;
};
#endif /* PARTINSTANCESTUFF_H_ */
