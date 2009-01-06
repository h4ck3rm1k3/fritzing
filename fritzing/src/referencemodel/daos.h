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




#ifndef DAOS_H_
#define DAOS_H_

#include <QList>
#include "../misc.h"

class Part;

class PartProperty {
public:
	PartProperty() {}
	PartProperty(qlonglong id, const QString &name, const QString &value, Part *part = NULL) {
		m_id = id;
		m_name = name;
		m_value = value;
		m_part = part;
	}
	PartProperty(const QString &name, const QString &value, Part *part = NULL) {
		m_name = name;
		m_value = value;
		m_part = part;
	}

	qlonglong id() const {return m_id;}
	void setId(qlonglong id) {m_id = id;}

	const QString &name() const {return m_name;}
	void setName(const QString &name) {
		m_name = name.toLower().trimmed();
		//Q_ASSERT(m_name != ___emptyString___);
	}

	const QString &value() const {return m_value;}
	void setValue(const QString &value) {
		m_value = value.toLower().trimmed();
		//Q_ASSERT(m_value != ___emptyString___);
	}

	Part *part() {return m_part;}
	void setPart(Part *part) {m_part = part;}

	static PartProperty* from(const QString &name, const QString &value, Part *part=NULL) {
		return new PartProperty(name, value, part);
	}

private:
	qlonglong m_id;
	QString m_name;
	QString m_value;
	Part * m_part;
};

typedef QList<PartProperty*> PartPropertyList;

class Part {
public:
	Part() {}
	Part(qlonglong id, const QString &moduleID, const QString &family, const PartPropertyList &properties) {
		setId(id);
		setModuleID(moduleID);
		setFamily(family);
		setProperties(properties);
	}
	Part(const QString &moduleID, const QString &family, const PartPropertyList &properties) {
		m_id = -1;
		setModuleID(moduleID);
		setFamily(family);
		setProperties(properties);
	}
	Part(const QString &family, const PartPropertyList &properties) {
		m_id = -1;
		m_moduleID = ___emptyString___;
		setFamily(family);
		setProperties(properties);
	}
	Part(const QString &family, const QString &propname, const QString &propvalue) {
		m_id = -1;
		m_moduleID = ___emptyString___;
		setFamily(family);

		m_properties << new PartProperty(propname, propvalue, this);
	}
	Part(const QString &family) {
		m_id = -1;
		m_moduleID = ___emptyString___;
		setFamily(family);
	}

	qlonglong id() const {return m_id;}
	void setId(qlonglong id) {m_id = id;}

	const QString &moduleID() const {return m_moduleID;}
	void setModuleID(const QString &moduleID) {
		m_moduleID = moduleID;
		Q_ASSERT(m_moduleID != ___emptyString___);
		//Q_ASSERT(!m_moduleID.isNull());
	}

	const QString & family() const {return m_family;}
	void setFamily(const QString &family) {
		m_family = family.toLower().trimmed();
		//Q_ASSERT(m_family != ___emptyString___);
	}

	PartPropertyList properties() const {return m_properties;}
	void setProperties(const PartPropertyList &properties) {
		//Q_ASSERT(m_properties.size() == 0);
		m_properties = properties;
		//Q_ASSERT(m_properties.size() > 0);
	}

	void addProperty(PartProperty *property) {
		m_properties << property;
	}

	static Part *from(const QString &family, const QMultiHash<QString,QString> &properties) {
		Part *part = new Part(family);
		if(properties.size() > 0) {
			PartPropertyList partprops;
			foreach(QString name, properties.uniqueKeys()) {
				foreach(QString value, properties.values(name)) {
					partprops << PartProperty::from(name, value, part);
				}
			}
			part->setProperties(partprops);
		}
		return part;
	}

	static Part *from(const QString &moduleID, const QString &family, const QMultiHash<QString, QString> &properties) {
		Part *part = from(family, properties);
		part->setModuleID(moduleID);
		return part;
	}

	static Part *from(ModelPart *modelPart) {
		QString moduleID = modelPart->moduleID();
		Q_ASSERT(moduleID != ___emptyString___);
		//Q_ASSERT(!moduleID.isNull());

		QHash<QString,QString> props = modelPart->modelPartStuff()->properties();
		Q_ASSERT(props.contains("family"));

		QString family = props["family"];
		props.remove("family");
		return from(moduleID, family,props);
	}

private:
	qlonglong m_id;
	QString m_moduleID;
	QString m_family;
	PartPropertyList m_properties;
};

#endif /* DAOS_H_ */
