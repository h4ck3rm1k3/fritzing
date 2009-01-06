/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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
