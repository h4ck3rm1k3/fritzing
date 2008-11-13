#include "partseditorsketchwidget.h"
#include "partseditorpaletteitem.h"
#include "../layerkinpaletteitem.h"
#include "../debugdialog.h"

QT_BEGIN_NAMESPACE

PartsEditorSketchWidget::PartsEditorSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent)
	: SketchWidget(viewIdentifier, parent, 300)
{

}

void PartsEditorSketchWidget::mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *){
	DebugDialog::debug("got connector mouse press.  not yet implemented...");
	return;
}

void PartsEditorSketchWidget::loadSvgFile(StringPair *path, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, QString layer) {
	PartsEditorPaletteItem * item = new PartsEditorPaletteItem(modelPart, viewIdentifier, path, layer);
	//if(item->connectors().size() > 0) {
		//emit connectorsFound(this->m_viewIdentifier,item->connectors());
	//}
	this->addItem(modelPart, BaseCommand::CrossView, item->getViewGeometry(),item->id(),item);
}

ItemBase * PartsEditorSketchWidget::addItemAux(ModelPart * modelPart, const ViewGeometry & /*viewGeometry*/, long /*id*/, PaletteItem * paletteItem, bool doConnectors)
{
	if(paletteItem == NULL) {
		paletteItem = new PartsEditorPaletteItem(modelPart, m_viewIdentifier);
	}
	modelPart->initConnectors();    // is a no-op if connectors already in place
	return addPartItem(modelPart, paletteItem, doConnectors);
}

void PartsEditorSketchWidget::clearScene() {
	QGraphicsScene * scene = this->scene();
	//QList<QGraphicsItem*> items;
	for(int i=0; i < scene->items().size(); i++){
		ItemBase * itemBase = ItemBase::extractTopLevelItemBase(scene->items()[i]);
		if (itemBase == NULL) {
			//items << scene->items()[i];
			continue;
		}

		this->deleteItem(itemBase, true, true);
	}

	/*for(int i=0; i < items.size(); i++) {
		scene->removeItem(items[i]);
	}*/
}
