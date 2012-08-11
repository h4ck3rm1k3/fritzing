/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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

// copying a table from one SQWL database to another http://sqlite.phxsoftware.com/forums/t/285.aspx

#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>

#include "sqlitereferencemodel.h"
#include "../debugdialog.h"

#define MAX_CONN_TRIES 3

void debugError(bool result, QSqlQuery & query) {
    if (result) return;

    QSqlError error = query.lastError();
    DebugDialog::debug(QString("%1 %2 %3").arg(error.text()).arg(error.number()).arg(error.type()));
}

SqliteReferenceModel::SqliteReferenceModel() {
	m_swappingEnabled = false;
	m_lastWasExactMatch = true;
}

void SqliteReferenceModel::loadAll(bool fastLoad, const QString & databaseName)
{
	initParts(fastLoad);

	int tries = 0;
	while(!m_swappingEnabled && tries < MAX_CONN_TRIES) {
		createConnection(databaseName);
		if(!m_swappingEnabled) {
			deleteConnection();
		}
		tries++;
	}
	/* TODO Mariano: perhaps we should check that there are no parts with
	 * the same family and providing exactly the same properties set
	 */

	if(!m_swappingEnabled) {
		QMessageBox::warning(NULL,
			QObject::tr("Oops!"),
			QObject::tr("Sorry, we have a problem with the swapping mechanism.\nFritzing still works, but you won't be able to change parts properties."),
			QMessageBox::Ok);
	}

}

SqliteReferenceModel::~SqliteReferenceModel() {
	deleteConnection();
}

void SqliteReferenceModel::initParts(bool fastLoad) {
	m_init = true;
	PaletteModel::initParts(fastLoad);
	m_init = false;
}

bool SqliteReferenceModel::createConnection(const QString & databaseName) {
	m_swappingEnabled = true;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(databaseName.isEmpty() ? ":memory:" : databaseName);
	if (!db.open()) {
		m_swappingEnabled = false;
	} else {
		QSqlQuery query;
		bool result = query.exec("CREATE TABLE parts (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"moduleID TEXT NOT NULL,\n"
			"family TEXT NOT NULL,\n"
			"version REAL,\n"
	        "fritzing_version TEXT,\n"
            "author TEXT,\n"
			"title TEXT,\n"
			"label TEXT,\n"
			"date TEXT,\n"
			"description TEXT,\n"
			"taxonomy TEXT,\n"
            "core TEXT NOT NULL\n"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_part_id ON parts (id ASC)");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_part_moduleID ON parts (moduleID ASC)");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_part_family ON parts (family ASC)");
        debugError(result, query);


        result = query.exec("CREATE TABLE views (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"viewname TEXT NOT NULL,\n"             // char b=breadboard, p=pcb, s=schematic, i=icon
			"image TEXT NOT NULL,\n"
			"layer0 INTEGER,\n"                         // ViewLayer::ViewLayerID
			"layer1 INTEGER,\n"                         
			"layer2 INTEGER,\n"                         
			"layer3 INTEGER,\n"                         
			"layer4 INTEGER,\n"                         
            "part_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_view_part_id ON views (part_id ASC)");
        debugError(result, query);

        result = query.exec("CREATE TABLE connectors (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"connectorid TEXT NOT NULL,\n"
			"type TEXT NOT NULL,\n"
			"name TEXT NOT NULL,\n"
			"description TEXT NOT NULL,\n"
            "part_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_connector_part_id ON connectors (part_id ASC)");
        debugError(result, query);

        result = query.exec("CREATE TABLE connectorlayers (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"view TEXT NOT NULL,\n"
			"layer INTEGER NOT NULL,\n"                     // ViewLayer::ViewLayerID
			"svgid TEXT NOT NULL,\n"
			"hybrid INTEGER NOT NULL,\n"
			"terminalid TEXT,\n"
            "connector_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_connectorlayer_connector_id ON connectorlayers (connector_id ASC)");
        debugError(result, query);

        result = query.exec("CREATE TABLE buses (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"name TEXT NOT NULL,\n"
            "part_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_bus_part_id ON buses (part_id ASC)");
        debugError(result, query);

        result = query.exec("CREATE TABLE busmembers (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\n"
			"connectorid TEXT NOT NULL,\n"
            "bus_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_busmember_bus_id ON busmembers (bus_id ASC)");
        debugError(result, query);

		result = query.exec("CREATE TABLE tags (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL ,\n"
			"tag TEXT NOT NULL,\n"
            "part_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_tag_part_id ON tags (part_id ASC)");
        debugError(result, query);

		result = query.exec("CREATE TABLE properties (\n"
			"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL ,\n"
			"name TEXT NOT NULL,\n"
			"value TEXT NOT NULL,\n"
			"part_id INTEGER NOT NULL"
		")");
        debugError(result, query);
		result = query.exec("CREATE INDEX idx_property_name ON properties (name ASC)");
        debugError(result, query);

		result = query.exec("CREATE TRIGGER unique_part__moduleID \n"
			"BEFORE INSERT ON parts \n"
			"FOR EACH ROW BEGIN \n"
				"SELECT RAISE(ROLLBACK, 'insert on table \"parts\" violates unique constraint \"unique_part__moduleID\"') \n"
					"WHERE (SELECT count(*) FROM parts WHERE moduleID = NEW.moduleID) > 0; \n"
			"END; "
		);
        debugError(result, query);

		foreach(ModelPart* mp, m_partHash.values()) {
			addPartAux(mp);
		}

		db.commit();

	}
	return m_swappingEnabled;
}

void SqliteReferenceModel::deleteConnection() {
	QSqlDatabase::removeDatabase("SQLITE");
}

ModelPart *SqliteReferenceModel::loadPart(const QString & path, bool update, bool fastLoad) {
	ModelPart *modelPart = PaletteModel::loadPart(path, update, fastLoad);
	if (modelPart == NULL) return modelPart;

	if (!m_init) addPart(modelPart, update);
	return modelPart;
}

ModelPart *SqliteReferenceModel::retrieveModelPart(const QString &moduleID) {
	if (moduleID.isEmpty()) {
		return NULL;
	}
	return m_partHash.value(moduleID, NULL);
}

ModelPart *SqliteReferenceModel::retrieveModelPart(const QString &family, const QMultiHash<QString, QString> &properties) {
	Part *part = Part::from(family, properties);
	ModelPart *retval = retrieveModelPart(part);
	delete part;
	return retval;
}

ModelPart *SqliteReferenceModel::retrieveModelPart(const Part *examplePart) {
	return retrieveModelPart(retrieveModuleId(examplePart,___emptyString___, false));
}

QString SqliteReferenceModel::retrieveModuleIdWith(const QString &family, const QString &propertyName, bool closestMatch) {
	QString moduleID = retrieveModuleId(family,m_recordedProperties,propertyName, closestMatch);
	m_recordedProperties.clear();
	return moduleID;
}

QString SqliteReferenceModel::retrieveModuleId(const QString &family, const QMultiHash<QString, QString> &properties, const QString &propertyName, bool closestMatch) {
	Part * part = Part::from(family, properties);
	if (part == NULL) return "";

	QString moduleID = retrieveModuleId(part, propertyName, closestMatch);
	delete part;
	return moduleID;
}

QString SqliteReferenceModel::retrieveModuleId(const Part *examplePart, const QString &propertyName, bool closestMatch) {
	PartPropertyList props = examplePart->properties();
	QString propertyValue;

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
			if(prop->name() == propertyName) {
				propertyValue = prop->value();
			}
		}
		queryStr += ") order by part.core desc";
		queryDebug += ") order by part.core desc";

		QSqlQuery query;
		query.prepare(queryStr);

		query.bindValue(":family",examplePart->family());
		foreach(QString name, params.uniqueKeys()) {
			query.bindValue(name,params[name]);
		}

		QString moduleId;
		if(query.exec()) {
			//DebugDialog::debug("SQLITE: retrieving: \n"+queryDebug);
			if(query.next()) {
				moduleId =  query.value(0).toString(); //grab the first
			}
			//DebugDialog::debug("SQLITE: found: "+moduleId);
			query.clear();
		} else {
			DebugDialog::debug(
				"SQLITE: couldn't retrieve part\n"
				"\t "+query.lastQuery()+"\n"
				"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
				"\t ERROR DB: "+query.lastError().databaseText()+"\n"
			);
		}

		if(!moduleId.isEmpty()) {
			m_lastWasExactMatch = true;
			return moduleId;
		} else if(closestMatch || !propertyName.isEmpty()) {
			m_lastWasExactMatch = false;
			return closestMatchId(examplePart, propertyName, propertyValue);
		} else {
			return ___emptyString___;
		}
	} else {
		return ___emptyString___;
	}
}

QString SqliteReferenceModel::closestMatchId(const Part *examplePart, const QString &propertyName, const QString &propertyValue) {
	QStringList possibleMatches = getPossibleMatches(examplePart,propertyName, propertyValue);
	return getClosestMatch(examplePart, possibleMatches);
}

QStringList SqliteReferenceModel::getPossibleMatches(const Part *examplePart, const QString &propertyName, const QString &propertyValue) {
	QStringList result;
	QString queryStr =
		"SELECT moduleID FROM parts part \n"
		"WHERE part.family = :family AND EXISTS ( \n"
			"SELECT * FROM properties prop \n"
			"WHERE prop.part_id = part.id %1 \n"
		") ";
	queryStr = queryStr.arg((propertyName.isEmpty()) ? "" : "AND prop.name = :prop_name  AND prop.value = :prop_value ");
	QString dbgQueryStr =
		"SELECT moduleID FROM parts part \n"
		"WHERE part.family = "+examplePart->family()+" AND EXISTS ( \n"
			"SELECT * FROM properties prop \n"
			"WHERE prop.part_id = part.id %1 \n"
		") ";
    dbgQueryStr = dbgQueryStr.arg((propertyName.isEmpty()) ? QString("") : QString("AND prop.name = ")+propertyName+"  AND prop.value = "+propertyValue);
	QSqlQuery query;
	query.prepare(queryStr);

	query.bindValue(":family",examplePart->family());
	if (!propertyName.isEmpty()) {
		query.bindValue(":prop_name",propertyName);
		query.bindValue(":prop_value",propertyValue);
	}

	Q_UNUSED(dbgQueryStr);
	//DebugDialog::debug(dbgQueryStr);

	if(query.exec()) {
		while(query.next()) {
			result << query.value(0).toString();
		}
		//DebugDialog::debug(QString("SQLITE: %1 possible matches found").arg(result.size()));
		query.clear();
	} else {
		DebugDialog::debug(
			"SQLITE: couldn't retrieve part\n"
			"\t "+query.lastQuery()+"\n"
			"\t ERROR DRIVER: "+query.lastError().driverText()+"\n"
			"\t ERROR DB: "+query.lastError().databaseText()+"\n"
		);
	}

	return result;
}

QString SqliteReferenceModel::getClosestMatch(const Part *examplePart, QStringList possibleMatches) {
	int propsInCommonCount = 0;
	int propsInCommonCountAux = 0;
	QString result;
	foreach(QString modId, possibleMatches) {
		propsInCommonCountAux = countPropsInCommon(examplePart,retrieveModelPart(modId));
		if(propsInCommonCountAux > propsInCommonCount) {
			result = modId;
			propsInCommonCount = propsInCommonCountAux;
		}
	}
	return result;
}

int SqliteReferenceModel::countPropsInCommon(const Part *part1, const ModelPart *part2) {
	if (part1 == NULL || part2 == NULL) {
		DebugDialog::debug("countPropsInCommon failure");
		return 0;
	}

	int result = 0;
	PartPropertyList props1 = part1->properties();
	QMultiHash<QString,QString> props2 = part2->properties();
	foreach(PartProperty *prop1, props1) {
		QStringList values = props2.values(prop1->name());
		if(values.contains(prop1->value())) {
			result++;
		}
	}
	return result;
}

bool SqliteReferenceModel::lastWasExactMatch() {
	return m_lastWasExactMatch;
}

bool SqliteReferenceModel::addPartAux(ModelPart * newModel) {
	try {
		Part *part = Part::from(newModel);
		bool result = addPart(part);
		delete part;
		return result;
	}
	catch (const char * msg) {
		DebugDialog::debug(msg);
	}
	catch (const QString & msg) {
		DebugDialog::debug(msg);
	}
	catch (...) {
		DebugDialog::debug("SqliteReferenceModel::addPartAux failure");
	}

	return NULL;
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

ModelPart * SqliteReferenceModel::addPart(QString newPartPath, bool addToReference, bool updateIdAlreadyExists)
{
	return PaletteModel::addPart(newPartPath, addToReference, updateIdAlreadyExists);
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
	query.prepare("INSERT INTO parts(moduleID, family, core) VALUES (:moduleID, :family, :core )");
	query.bindValue(":moduleID", part->moduleID());
	query.bindValue(":family", part->family());
	query.bindValue(":core", part->isCore());
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
			"\t INSERT INTO parts(moduleID, family, core) VALUES ('"+part->moduleID()+"', '"+part->family()+"', '"+part->isCore()+"')\n"
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
		//DebugDialog::debug("SQLITE: retrieving id: "+moduleID);
		if(query.next()) {
			partId =  query.value(0).toULongLong(); //grab the first
		}
		//DebugDialog::debug(QString("SQLITE: found: %1").arg(partId));
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

QString SqliteReferenceModel::partTitle(const QString moduleID) {
	ModelPart *mp = retrieveModelPart(moduleID);
	if(mp) {
		return mp->modelPartShared()->title();
	} else {
		return ___emptyString___;
	}
}

