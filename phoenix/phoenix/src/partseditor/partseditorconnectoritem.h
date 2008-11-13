/*
 * (c) Fachhochschule Potsdam
 */


#ifndef PARTSEDITORCONNECTORITEM_H_
#define PARTSEDITORCONNECTORITEM_H_

#include "../connectoritem.h"


class PartsEditorConnectorItem: public ConnectorItem {
	public:
		PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo);
		void highlight(const QString &connId);
		void removeFromModel();
		void setConnector(Connector *connector);
		void setMismatching(bool isMismatching);

	protected:
		void setSelectedColor(const QColor &color = selectedColor);
		void setNotSelectedColor(const QColor &color = notSelectedColor);
		void removeErrorIcon();
		void addErrorIcon();
		void addBorder();
		void removeBorder();
		void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget );

		QGraphicsSvgItem *m_errorIcon;
		bool m_withBorder;
	public:
		static QColor selectedColor;
		static QColor notSelectedColor;
		static QColor selectedPenColor;
		static qreal selectedPenWidth;
};

#endif /* PARTSEDITORCONNECTORITEM_H_ */
