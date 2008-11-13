/*
 * (c) Fachhochschule Potsdam
 */

#ifndef SINGLECONNECTORINFOWIDGET_H_
#define SINGLECONNECTORINFOWIDGET_H_

#include "../connector.h"
#include "../connectorstuff.h"
#include "abstractconnectorinfowidget.h"
#include "editablelinewidget.h"
#include "editabletextwidget.h"
#include "mismatchingconnectorwidget.h"

class ConnectorTypeWidget : public QLabel {
	public:
		ConnectorTypeWidget(Connector::ConnectorType type = Connector::Female, QWidget *parent=0);
		Connector::ConnectorType type();
		const QString &typeAsStr();
		void setType(Connector::ConnectorType type);

	friend class SingleConnectorInfoWidget;
	protected:
		void mousePressEvent(QMouseEvent *);
		void toggleValue();
		void cancel();

		bool m_isSelected;
		volatile bool m_isInEditionMode;
		Connector::ConnectorType m_typeBackUp;

		static const QString FemaleSymbol;
		static const QString MaleSymbol;
};

class SingleConnectorInfoWidget : public AbstractConnectorInfoWidget {
	Q_OBJECT
	public:
		SingleConnectorInfoWidget(WaitPushUndoStack *undoStack, Connector* connector=0, QWidget *parent=0);
		void setSelected(bool selected, bool doEmitChange=true);
		void setInEditionMode(bool inEditionMode);
		bool isInEditionMode();
		QSize sizeHint() const;
		QSize minimumSizeHint() const;
		QSize maximumSizeHint() const;

		Connector * connector();

		QString id();
		QString name();
		QString description();
		QString type();

		MismatchingConnectorWidget *toMismatching(ItemBase::ViewIdentifier viewId);

	protected slots:
		void editionCompleted();
		void editionCanceled();

	signals:
		void editionStarted();
		void editionFinished();
		//void connectorSelected(const QString&);

	protected:
		void toStandardMode();
		void toEditionMode();
		void startEdition();

		void mousePressEvent(QMouseEvent * event);

		QFrame *m_noEditFrame;
		QLabel *m_nameLabel;
		QLabel *m_descLabel;
		ConnectorTypeWidget *m_type;

		QLineEdit *m_nameEdit;
		QTextEdit *m_descEdit;

		QPushButton *m_acceptButton;
		QPushButton *m_cancelButton;

		WaitPushUndoStack *m_undoStack;
		Connector *m_connector;

		volatile bool m_isInEditionMode;
};

#endif /* SINGLECONNECTORINFOWIDGET_H_ */
