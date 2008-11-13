#ifndef SKETCHMODEL_H
#define SKETCHMODEL_H

#include "modelpart.h"
#include "modelbase.h"

#include <QTextStream>

class SketchModel : public ModelBase
{

public:
	SketchModel(bool makeRoot);
	SketchModel(ModelPart * root);
	void removeModelPart(ModelPart *);

};
#endif
