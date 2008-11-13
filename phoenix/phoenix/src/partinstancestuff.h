/*
 * (c) Fachhochschule Potsdam
 */

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

	const QHash<QString,QString> & properties();
	void setProperties(const QHash<QString,QString> &properties);

protected:
	void loadText(QDomElement parent, QString tagName, QString &field);

	QDomDocument* m_domDocument;

	QString m_title;
	QHash<QString,QString> m_properties;
};
#endif /* PARTINSTANCESTUFF_H_ */
