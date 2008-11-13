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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/



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
