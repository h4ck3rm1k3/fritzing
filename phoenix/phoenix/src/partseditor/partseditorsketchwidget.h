#ifndef PARTSEDITORSKETCHWIDGET_H
#define PARTSEDITORSKETCHWIDGET_H
//
#include "../sketchwidget.h"
#include "partseditorpaletteitem.h"
//
class PartsEditorSketchWidget : public SketchWidget
{

Q_OBJECT

public:
    PartsEditorSketchWidget(ItemBase::ViewIdentifier, QWidget *parent=0);
	void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void loadSvgFile(StringPair *path, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, QString layer);

signals:
	void connectorsFound(ItemBase::ViewIdentifier viewId, QStringList connNames);

protected:
	void clearScene();
	ItemBase * addItemAux(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, PaletteItem* paletteItem, bool doConnectors);
};
#endif
