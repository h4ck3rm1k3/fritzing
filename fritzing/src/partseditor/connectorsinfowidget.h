/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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



#ifndef CONNECTORSINFOWIDGET_H_
#define CONNECTORSINFOWIDGET_H_

#include <QFrame>
#include <QLabel>

#include "singleconnectorinfowidget.h"
#include "mismatchingconnectorwidget.h"

class ConnectorsInfoWidget : public QFrame{
	Q_OBJECT
	public:
		ConnectorsInfoWidget(WaitPushUndoStack *undoStack, QWidget *parent=0);
		const QList<ConnectorStuff *> connectorsStuffs();

	public slots:
		void connectorsFound(QList<Connector *>);
		void informConnectorSelection(const QString &);
		void informEditionCompleted();
		void syncNewConnectors(ItemBase::ViewIdentifier viewId, const QList<Connector*> &conns);

	signals:
		void connectorSelected(const QString &);
		void editionCompleted();
		void existingConnector(ItemBase::ViewIdentifier viewId, const QString &id, Connector*);
		void setMismatching(ItemBase::ViewIdentifier viewId, const QString &connId, bool mismatching);

	protected slots:
		void updateLayout();
		void selectionChanged(AbstractConnectorInfoWidget* selected);

	protected:
		void addConnectorInfo(MismatchingConnectorWidget* mcw);
		void addConnectorInfo(QString id);
		void addConnectorInfo(Connector *conn);
		void addMismatchingConnectorInfo(MismatchingConnectorWidget *mcw);
		void addMismatchingConnectorInfo(ItemBase::ViewIdentifier viewID, QString connId);
		QGridLayout *scrollContentLayout();
		bool eventFilter(QObject *obj, QEvent *event);
		void setSelected(AbstractConnectorInfoWidget * newSelected);
		void selectNext();
		void selectPrev();

		void clearMismatchingForView(ItemBase::ViewIdentifier viewId);
		void singleToMismatchingNotInView(ItemBase::ViewIdentifier viewId, const QStringList &connIds);

		bool existingConnId(const QString &id);
		MismatchingConnectorWidget* existingMismatchingConnector(const QString &id);
		void removeMismatchingConnectorInfo(MismatchingConnectorWidget* mcw);
		void removeConnectorInfo(SingleConnectorInfoWidget *sci);
		Connector* findConnector(const QString &id);

		QHash<QString /*connId*/, QMultiHash<ItemBase::ViewIdentifier, SvgIdLayer*> > m_connectorsPins;
		QFrame *m_scrollContent;
		QFrame *m_mismatchersFrame;
		QFrame *m_mismatchersFrameParent;

		AbstractConnectorInfoWidget *m_selected;

		QList<SingleConnectorInfoWidget*> m_connsInfo;
		QList<MismatchingConnectorWidget*> m_mismatchConnsInfo;

		WaitPushUndoStack *m_undoStack;
};

#endif /* CONNECTORSINFOWIDGET_H_ */
