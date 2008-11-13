/*
 * (c) Fachhochschule Potsdam
 */

#ifndef MISMATCHINGCONNECTORWIDGET_H_
#define MISMATCHINGCONNECTORWIDGET_H_

#include <QLabel>
#include "abstractconnectorinfowidget.h"
#include "../itembase.h"

class MismatchingConnectorWidget : public AbstractConnectorInfoWidget {
	Q_OBJECT
	public:
		MismatchingConnectorWidget(ItemBase::ViewIdentifier viewId, const QString &connId, QWidget *parent, bool isInView = true, Connector* conn = NULL);
		void setSelected(bool selected, bool doEmitChange=true);
		bool onlyMissingThisView(ItemBase::ViewIdentifier viewId);
		void addViewPresence(ItemBase::ViewIdentifier viewId);
		void removeViewPresence(ItemBase::ViewIdentifier viewId);
		const QString &connId();
		Connector *prevConn();
		QList<ItemBase::ViewIdentifier> views();
		bool presentInAllViews();

	protected:
		void mousePressEvent(QMouseEvent * event);
		QString viewsString();

		QList<ItemBase::ViewIdentifier> m_missingViews;
		QString m_connId;
		Connector *m_prevConn; // If this connector info used to be a not mismatching one, we save that info here

		QLabel *m_connIdLabel;
		QLabel *m_connMsgLabel;

		static QList<ItemBase::ViewIdentifier> AllViews;
};

#endif /* MISMATCHINGCONNECTORWIDGET_H_ */
