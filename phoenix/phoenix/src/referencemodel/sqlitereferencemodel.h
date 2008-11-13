/*
 * (c) Fachhochschule Potsdam
 */

#ifndef SQLITEREFERENCEMODEL_H_
#define SQLITEREFERENCEMODEL_H_

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QApplication>

#include "referencemodel.h"
#include "daos.h"

class SqliteReferenceModel : public ReferenceModel {
	Q_OBJECT
	public:
		SqliteReferenceModel();
		~SqliteReferenceModel();

		ModelPart *loadPart(const QString & path, bool update);

		ModelPart *retrieveModelPart(const QString &moduleID);
		ModelPart *retrieveModelPart(const QString &family, const QMultiHash<QString /*name*/, QString /*value*/> &properties);
		ModelPart *retrieveModelPart(const Part *examplePart);
		QString retrieveModuleId(const Part *examplePart);

		bool addPart(ModelPart * newModel, bool update);
		bool updatePart(ModelPart * newModel);
		bool addPart(Part* part);

		bool swapEnabled();
		bool containsModelPart(const QString & moduleID);

	public slots:
		void recordProperty(const QString &name, const QString &value);
		QString retrieveModuleIdWithRecordedProps(const QString &family);
		QString retrieveModuleId(const QString &family, const QMultiHash<QString /*name*/, QString /*value*/> &properties);
		QStringList values(const QString &family, const QString &propName, bool distinct=true);

	protected:
		void init();

	private:
		bool addPartAux(ModelPart * newModel);

		bool createConnection();
		void deleteConnection();
		bool insertPart(Part *part);
		bool insertProperty(PartProperty *property);
		qlonglong partId(QString moduleID);
		bool removePart(qlonglong partId);
		bool removeProperties(qlonglong partId);

		volatile bool m_swappingEnabled;
		bool m_init;
		QMultiHash<QString /*name*/, QString /*value*/> m_recordedProperties;
};

#endif /* SQLITEREFERENCEMODEL_H_ */
