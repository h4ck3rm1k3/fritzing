/*
 * (c) Fachhochschule Potsdam
 */

#ifndef CONNECTORWIDGET_H_
#define CONNECTORWIDGET_H_

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QHash>
#include <QLineEdit>

#include "../itembase.h"
#include "../modelpart.h"
#include "../connector.h"
#include "../connectorstuff.h"

class ConnectorIdValidator : public QRegExpValidator {
	public:
		ConnectorIdValidator(QWidget *parent = 0) : QRegExpValidator(QRegExp("\\w+"),parent) {};
};

class ConnectorNameComboBox : public QComboBox {
	public:
		ConnectorNameComboBox(int *prevSelIdx, QWidget *parent = 0);
	protected:
		void focusOutEvent( QFocusEvent * event );
		int * m_prevSelIdx;
};

class ConnectorsWidget : public QWidget {
	Q_OBJECT

	public:
		ConnectorsWidget(QWidget *parent = 0);
		QList<ConnectorStuff*> connectorsInfo();

	public slots:
		void connectorsFound(ItemBase::ViewIdentifier viewId, QStringList connNames);
		void updateInfo(ModelPart *);

	signals:
		void breadboardConnectorSelected(QString connName);
		void schematicConnectorSelected(QString connName);
		void pcbConnectorSelected(QString connName);

	protected slots:
		void setUsrDefConnectorsEditable();
		void setUsrDefConnectorsUneditable();
		void showConnInfoIfAnyAndSavePrev(int idx);
		void updateUserDefConnsComboBox();

	protected:
		class ConnectorsWidgetHelpClass {
			public:
				ConnectorsWidgetHelpClass(QString id) {
					this->id = id;
					name = "";
					description = "";
					idxTypeCB = -1;
					idxBreadCB = -1;
					idxSchemCB = -1;
					idxPcbCB = -1;
				};

				QString id;
				QString name;
				QString description;
				int idxTypeCB;
				int idxBreadCB;
				int idxSchemCB;
				int idxPcbCB;
		};

		void clearChildren();
		void keepConnInfo();
		void addConnectorStuff(ConnectorStuff * connStuff);
		int itemIndex(QComboBox* opts, QString text);

		QHash<int /* connector position in combobox*/, ConnectorsWidgetHelpClass*> m_connInfo;
		QHash<ItemBase::ViewIdentifier,QComboBox*> m_comboBoxes;

		QPushButton * m_newButton;
		QComboBox * m_userDefConnectors;

		QLineEdit * m_connId;
		QLineEdit * m_connName;
		QLineEdit * m_connDesc;
		QComboBox * m_connType;

		QComboBox * m_breadConnectors;
		QComboBox * m_schemConnectors;
		QComboBox * m_pcbConnectors;

		int m_prevComboBoxIdx;
};

#endif /* CONNECTORWIDGET_H_ */
