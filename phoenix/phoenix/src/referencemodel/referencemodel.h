/*
 * (c) Fachhochschule Potsdam
 */

#ifndef REFERENCEMODEL_H_
#define REFERENCEMODEL_H_

#include "../palettemodel.h"
#include "daos.h"

class ReferenceModel : public PaletteModel {
	Q_OBJECT
	public:
		virtual ModelPart *loadPart(const QString & path, bool update) = 0;

		virtual ModelPart *retrieveModelPart(const QString &moduleID) = 0;
		virtual ModelPart *retrieveModelPart(const QString &family, const QMultiHash<QString /*name*/, QString /*value*/> &properties) = 0;
		virtual ModelPart *retrieveModelPart(const Part *examplePart) = 0;
		virtual QString retrieveModuleId(const Part *examplePart) = 0;

		virtual bool addPart(ModelPart * newModel, bool update) = 0;
		virtual bool updatePart(ModelPart * newModel) = 0;
		virtual bool addPart(Part* part) = 0;

		virtual bool swapEnabled() = 0;

	public slots:
		virtual void recordProperty(const QString &name, const QString &value) = 0;
		virtual QString retrieveModuleIdWithRecordedProps(const QString &family) = 0;
		virtual QString retrieveModuleId(const QString &family, const QMultiHash<QString /*name*/, QString /*value*/> &properties) = 0;
		virtual QStringList values(const QString &family, const QString &propName, bool distinct=true) = 0;
};

#endif /* REFERENCEMODEL_H_ */
