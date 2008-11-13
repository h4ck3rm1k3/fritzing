/*
 * (c) Fachhochschule Potsdam
 */

//#include <QXmlQuery>

#include "partseditorconnectorviewimagewidget.h"
#include "partseditorconnectoritem.h"
#include "../debugdialog.h"

PartsEditorConnectorViewImageWidget::PartsEditorConnectorViewImageWidget(ItemBase::ViewIdentifier viewId, QWidget *parent, int size)
	: PartsEditorAbstractViewImage(viewId, parent, size)
{
}

void PartsEditorConnectorViewImageWidget::mousePressEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	return;
}

void PartsEditorConnectorViewImageWidget::loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart) {
	PartsEditorAbstractViewImage::loadFromModel(paletteModel, modelPart);
	m_item->removeFromModel();
	m_item->highlightConnectors("");
}

void PartsEditorConnectorViewImageWidget::addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath) {
	if(!modelPart) {
		QString path = svgFilePath->first+(svgFilePath->second != ___emptyString___ ? "/" : "")+svgFilePath->second;
		modelPart = createFakeModelPart(path, svgFilePath->second);
	}

	PartsEditorAbstractViewImage::addItemInPartsEditor(modelPart,svgFilePath);
	m_item->removeFromModel();

	emit connectorsFound(this->m_viewIdentifier,m_item->connectors());

	m_item->highlightConnectors("");
}

void PartsEditorConnectorViewImageWidget::informConnectorSelection(const QString &connId) {
	if(m_item) {
		m_item->highlightConnectors(connId);
	}
}

void PartsEditorConnectorViewImageWidget::setMismatching(ItemBase::ViewIdentifier viewId, const QString &id, bool mismatching) {
	if(m_item && viewId == m_viewIdentifier) {
		for (int i = 0; i < m_item->childItems().count(); i++) {
			PartsEditorConnectorItem * connectorItem = dynamic_cast<PartsEditorConnectorItem *>(m_item->childItems()[i]);
			if(connectorItem == NULL) continue;

			if(connectorItem->connector()->connectorStuffID() == id) {
				connectorItem->setMismatching(mismatching);
			}
		}
	}
}
