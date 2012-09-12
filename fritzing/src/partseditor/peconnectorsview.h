/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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



#ifndef CONNECTORSVIEW_H
#define CONNECTORSVIEW_H

#include <QFrame>
#include <QTimer>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QMutex>

#include "../items/itembase.h"
#include "../items/wire.h"
#include "../connectors/connectoritem.h"
#include "../connectors/connector.h"
#include "../referencemodel/referencemodel.h"

struct ConnectorMetadata {
    Connector::ConnectorType connectorType;
    QString connectorName;
    QString connectorDescription;
    QString connectorID;
};

class PEConnectorsView : public QScrollArea
{
Q_OBJECT
public:
	PEConnectorsView(QWidget * parent = 0);
	~PEConnectorsView();

    void initConnectors(QList<QDomElement> & connectorList, bool gotZeroConnector);

public:
    static QWidget * makeConnectorForm(const QDomElement & connector, bool gotZeroConnector, int index, QObject * slotHolder, bool alternating);

signals:
    void connectorMetadataChanged(const ConnectorMetadata *);

protected slots:
    void nameEntry();
    void descriptionEntry();
    void typeEntry();
    void connectorCountEntry();

protected:
    void changeConnector();

protected:
    QPointer<QFrame> m_mainFrame;
    int m_connectorCount;
    QMutex m_mutex;

};

#endif
