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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/


#include <QSqlRecord>
#include <QSqlError>

#include "sqlitereferencemodel.h"
#include "../debugdialog.h"

#define MAX_CONN_TRIES 3

SqliteReferenceModel::SqliteReferenceModel() {
	init();

	m_swappingEnabled = false;
	int tries = 0;
	while(!m_swappingEnabled && tries < MAX_CONN_TRIES) {
		createConnection();
		if(!m_swappingEnabled) {
			deleteConnection();
		}
		tries++;
	}
	/* TODO Mariano: perhaps we should check that there are no parts with
	 * the same family and providing exactly the same properties set
	 */

	if(!m_swappingEnabled) {
		QMessageBox::warning(0,
			qApp->tr("Oops!"),
			qApp->tr("Sorry, we have a problem with the swapping mechanism.\nFritzing still works, but you won't be able to change parts properties."),
			QMessageBox::Ok);
	}

	// just testing
	if(m_swappingEnabled) {
		/*ModelPart *mp = retrieveModelPart(new Part("led","color","red"));
		if(mp) {
			Q_ASSERT(mp->moduleID().startsWith("1234ABDC24"));
		}*/

		// this should fail
		/*mp = retrieveModelPart(new Part("capacitor","p2","Ceramic Capacitor"));
		Q_ASSERT(mp->moduleID() == "1001ABDC02");*/

		/*QStringList ledscolors = values("led","color");
		if(ledscolors.size() > 0) {
			Q_ASSERT(ledscolors.contains("blue"));
			Q_ASSERT(ledscolors.contains("green"));
			Q_ASSERT(ledscolors.contains("red"));
			Q_ASSERT(ledscolors.contains("white"));
			Q_ASSERT(ledscolors.contains("yellow"));
		}*/
	}
}

SqliteReferenceModel::~SqliteReferenceModel() {
	deleteConnection();
}

void SqliteReferenceModel::init() {
	m_init = true;
	PaletteModel::init();
	m_init = false;
}

bool SqliteReferenceModel::createConnection() {
	m_swappingEnabled = true;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(":memory:");
	if (!db.open()) {
		m_swappingEnabled = false;
	} else {
		QSqlQuery query;
		query.exec("CREATE TABLE parts (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"moduleID VARCHAR NOT NULL,\n"
			"family VARCHAR NOT NULL"
		")");
		query.exec("CREATE INDEX idx_part_id ON parts (id ASC)");
		query.exec("CREATE INDEX idx_part_moduleID ON parts (moduleID ASC)");
		query.exec("CREATE INDEX idx_part_family ON parts (family ASC)");

		query.exec("CREATE TABLE properties (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL ,\n"
			"name VARCHAR NOT NULL,\n"
			"value VARCHAR NOT NULL,\n"
			"part_id INTEGER NOT NULL"
		")");
		query.exec("CREATE INDEX idx_property_name ON properties (name ASC)");

		query.exec("CREATE TRIGGER unique_part__moduleID \n"
			"BEFORE INSERT ON parts \n"
			"FOR EACH ROW BEGIN \n"
				"SELECT RAISE(ROLLBACK, 'insert on table \"parts\" violates unique constraint \"unique_part__moduleID\"') \n"
					"WHERE (SELECT count(*) FROM parts WHERE moduleID = NEW.moduleID) > 0; \n"
			"END; "
		);

		foreach(ModelPart* mp, partHash.values()) {									   
			addPartAux(mp);
		}

		db.commit();
	}
	return m_swappingEnabled;
}

void SqliteReferenceModel::deleteConnection() {
	QSqlDatabase::removeDatabase("SQLITE");
}

ModelPart *SqliteReferenceModel::loadPart(const QString & path, bool update) {
	ModelPart *modelPart = PaletteModel::loadPart(path, update);
	if(!m_init) addPart(modelPart, update);
	return modelPart;
}

ModelPart *SqliteReferenceModel::retrieveModelPart(const QString &moduleID) {
	if(moduleID == ___emptyString___) {
		return NULL;
	}
	return partHash[moduleID];
}

ModelPart *SqliteReferenceModel::retrieveModelPart(const QString &family, const QMultiHash<QString, QString> &properties) {
	return retrieveModelPart(Part::from(family, properties));
}

ModelPart *SqliteReferenceModel::retrieveModelPart(const Part *examplePart) {
	return retrieveModelPart(retrieveModuleId(examplePart));
}

QString SqliteReferenceModel::retrieveModuleIdWithRecordedProps(const QString &family) {
	QString moduleID = retrieveModuleId(family,m_recordedProperties);
	m_recordedProperties.clear();
	return moduleID;
}

QString SqliteReferenceModel::retrieveModuleId(const QString &family, const QMultiHash<QString, QString> &properties) {
	return retrieveModuleId(Part::from(family, properties));
}

QString SqliteReferenceModel::retrieveModuleId(const Part *examplePart) {
	PartPropertyList props = examplePart->properties();

	if(props.size() > 0) {
		QString queryStr =
			"SELECT moduleID FROM parts part \n"
			"WHERE part.family = :family AND (1=1 ";
		QString queryDebug =
			"SELECT moduleID FROM parts part \n"
			"WHERE part.family = '"+examplePart->family()+"' AND (1=1 ";
		QHash<QString, QString> params;
		for(int i=0; i<props.size(); i++) {
			PartProperty *prop = props[i];
			QString propParam = QString(":prop%1").arg(i);
			queryStr += QString(" AND EXISTS (SELECT * FROM properties prop WHERE prop.part_id = part.id AND prop.name = %1_name AND prop.value = %1_value)").arg(propParam);
			queryDebug += QString(" AND EXISTS (SELECT * FROM properties prop WHERE prop.part_id = part.id AND prop.name = '"+prop->name()+"' AND prop.value = '"+prop->value()+"')");
			params[propParam+"_name"] = prop->name();
			params[propParam+"_value"] = prop->value();
		}
		queryStr += ")";
		queryDebug += ")";

		QSqlQuery query;
		query.prepare(queryStr);

		query.bindValue(":family",examplePart->family());
		foreach(QString name, params.uniqueKeys()) {
			query.bindValue(name,params[name]);
		}

		QString moduleId = ___emptyString___;
		if(query.exec()) {
			DebugDialog::debug("SQLITE: retrieving: \n"+queryDebug);
			//Q_ASSERT(query.record().count() == 1);
			if(query.next()) {
				moduleId =  query.value(0).toString(); //grab the first
			}
			//Q_ASSERT(!query.next());
			DebugDialog::debug("SQLITE: found: "+moduleId);
			query.clear();
		} else {
			moduleId = ___emptyString___;
			DebugDialog::debug(
				"SQLITE: couldn't retrieve part\n"
				"\t "+query.lastQuery()+"\n"
				"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
				"\t ERROR DB: "+query.lastError().databaseText()+"\n"
			);
		}

		return moduleId;
	} else {
		return ___emptyString___;
	}
}

bool SqliteReferenceModel::addPartAux(ModelPart * newModel) {
	return addPart(Part::from(newModel));
}

bool SqliteReferenceModel::addPart(ModelPart * newModel, bool update) {
	bool result;
	if(update && containsModelPart(newModel->moduleID())) {
		result = updatePart(newModel);
	} else {
		result = addPartAux(newModel);
	}
	return result;
}

bool SqliteReferenceModel::updatePart(ModelPart * newModel) {
	if(m_swappingEnabled) {
		qlonglong partId = this->partId(newModel->moduleID());
		if(partId != -1) {
			removePart(partId);
			removeProperties(partId);
			return addPartAux(newModel);
		} else {
			return false;
		}
	} else {
		return false;
	}

}

bool SqliteReferenceModel::addPart(Part* part) {
	return insertPart(part);
}

bool SqliteReferenceModel::insertPart(Part *part) {
	QSqlQuery query;
	query.prepare("INSERT INTO parts(moduleID, family) VALUES (:moduleID, :family )");
	query.bindValue(":moduleID", part->moduleID());
	query.bindValue(":family", part->family());
	if(query.exec()) {
		part->setId(query.lastInsertId().toLongLong());

		foreach(PartProperty *prop, part->properties()) {
			prop->setPart(part);
			insertProperty(prop);
		}

		query.clear();
	} else {
		DebugDialog::debug(
			"SQLITE: couldn't register part "+part->moduleID()+"\n"
			"\t INSERT INTO parts(moduleID, family) VALUES ('"+part->moduleID()+"'"+", '"+part->family()+"')\n"
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
		m_swappingEnabled = false;
	}

	return true;
}

bool SqliteReferenceModel::insertProperty(PartProperty *property) {
	QSqlQuery query;
	query.prepare("INSERT INTO properties(name, value, part_id) VALUES (:name, :value, :part_id)");
	query.bindValue(":name",property->name());
	query.bindValue(":value",property->value());
	query.bindValue(":part_id",property->part()->id());
	if(!query.exec()) {
		DebugDialog::debug(
			"SQLITE: couldn't register property \n"
			""+QString("\t INSERT INTO properties(name, value, part_id) VALUES ('"+property->name()+"', '"+property->value()+"', %1) \n").arg(property->part()->id())+""
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
		m_swappingEnabled = false;
	} else {
		property->setId(query.lastInsertId().toULongLong());
		query.clear();
	}

	return true;
}

QStringList SqliteReferenceModel::values(const QString &family, const QString &propName, bool distinct) {
	QStringList retval;

	QSqlQuery query;
	query.prepare(QString(
		"SELECT %1 prop.value FROM properties prop JOIN parts PART ON part.id = prop.part_id \n"
		"WHERE part.family = :family AND prop.name = :propName ORDER BY prop.value \n"
		).arg(distinct ? " DISTINCT ":"")
	);
	query.bindValue(":family",family);
	query.bindValue(":propName",propName);

	if(query.exec()) {
		while(query.next()) {
			retval << query.value(0).toString();
		}
		query.clear();
	} else {
		DebugDialog::debug(
			"SQLITE: couldn't retrieve part\n"
			"\t "+query.lastQuery()+"\n"
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
		m_swappingEnabled = false;
	}

	return retval;
}

void SqliteReferenceModel::recordProperty(const QString &name, const QString &value) {
	DebugDialog::debug(QString("RECORDING PROPERTY %1:%2").arg(name).arg(value));
	m_recordedProperties.insert(name,value);
}

bool SqliteReferenceModel::swapEnabled() {
	return m_swappingEnabled;
}

bool SqliteReferenceModel::containsModelPart(const QString & moduleID) {
	return partId(moduleID) != -1;
}

qlonglong SqliteReferenceModel::partId(QString moduleID) {
	qlonglong partId = -1;

	QSqlQuery query;
	query.prepare(
		"SELECT id FROM parts \n"
		"WHERE moduleID = :moduleID "
	);
	query.bindValue(":moduleID",moduleID);

	if(query.exec()) {
		DebugDialog::debug("SQLITE: retrieving id: "+moduleID);
		if(query.next()) {
			partId =  query.value(0).toULongLong(); //grab the first
		}
		DebugDialog::debug(QString("SQLITE: found: %1").arg(partId));
		query.clear();
	} else {
		DebugDialog::debug(
			"SQLITE: couldn't retrieve part\n"
			"\t "+query.lastQuery()+"\n"
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
	}

	return partId;
}

bool SqliteReferenceModel::removePart(qlonglong partId) {
	bool result = true;

	QSqlQuery query;
	query.prepare(
		"DELETE FROM parts \n"
		"WHERE id = :id "
	);
	query.bindValue(":id",partId);

	if(query.exec()) {
		query.clear();
		result = true;
	} else {
		DebugDialog::debug(
			"SQLITE: couldn't delete part\n"
			"\t "+query.lastQuery()+"\n"
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
		result = false;
	}

	return result;
}

bool SqliteReferenceModel::removeProperties(qlonglong partId) {
	bool result = true;

	QSqlQuery query;
	query.prepare(
		"DELETE FROM properties \n"
		"WHERE part_id = :id "
	);
	query.bindValue(":id",partId);

	if(query.exec()) {
		query.clear();
		result = true;
	} else {
		DebugDialog::debug(
			"SQLITE: couldn't delete properties \n"
			"\t "+query.lastQuery()+"\n"
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
		result = false;
	}

	return result;
}
