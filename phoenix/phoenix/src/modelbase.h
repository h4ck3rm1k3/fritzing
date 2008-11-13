#ifndef MODELBASE_H
#define MODELBASE_H

#include <QObject>
#include "modelpart.h"

class ModelBase : public QObject
{

public:
	ModelBase(bool makeRoot);
	virtual ~ModelBase();

	ModelPart * root();
	virtual ModelPart* retrieveModelPart(const QString & moduleID);
	virtual ModelPart * addModelPart(ModelPart * parent, ModelPart * copyChild);
	virtual bool load(const QString & fileName, ModelBase* refModel, bool doConnections);
	void save(const QString & fileName, bool asPart=false);
	virtual ModelPart * addPart(QString newPartPath, bool addToReference);
	virtual bool addPart(ModelPart * modelPart, bool update);


protected:
	ModelPart * m_root;
	ModelBase * m_referenceModel;

};

#endif
