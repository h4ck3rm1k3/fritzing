/*
 * (c) Fachhochschule Potsdam
 */

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
