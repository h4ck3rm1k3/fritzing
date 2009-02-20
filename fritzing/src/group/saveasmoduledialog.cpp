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

#include "saveasmoduledialog.h"
#include "../sketchwidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QApplication>

/////////////////////////////////////

SaveAsModuleDialog::SaveAsModuleDialog(SketchWidget * sketchWidget, QWidget *parent)
	: QDialog(parent)
{
	m_sketchWidget = sketchWidget;
	this->setWindowTitle(QObject::tr("Save as Module"));

	QVBoxLayout * vLayout = new QVBoxLayout(this);

	QLabel * label = new QLabel(QObject::tr("To make a connector available to users of this module, click it; click again to make it unavailable."), this);
	label->setWordWrap(true);
	vLayout->addWidget(label);

	sketchWidget->scene()->installEventFilter(this);

	QGraphicsView * gv = new QGraphicsView(this);
	gv->setMinimumSize(300, 300);
	gv->setScene(sketchWidget->scene());
	gv->show();

	vLayout->addWidget(gv);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);

}

SaveAsModuleDialog::~SaveAsModuleDialog()
{
	foreach (ConnectorItem * connectorItem, m_externalConnectorItems) {
		connectorItem->setChosen(false);
	}

	if (m_sketchWidget) {
		m_sketchWidget->scene()->removeEventFilter(this);
	}
}

bool SaveAsModuleDialog::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type()) {
		case QEvent::GraphicsSceneMousePress:
			handleSceneMousePress(event);
			return true;

		case QEvent::KeyPress:
		case QEvent::KeyRelease:			
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:			
		case QEvent::MouseButtonDblClick:
		case QEvent::WinEventAct:
		case QEvent::GraphicsSceneMouseDoubleClick:
		case QEvent::GraphicsSceneMouseRelease:
			return true;
			
		default:
			//DebugDialog::debug(QString("got event %1").arg(event->type()));
			break;
	}


	return QObject::eventFilter(obj, event);
}

void SaveAsModuleDialog::handleSceneMousePress(QEvent *event)
{
	QGraphicsSceneMouseEvent * sceneMouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
	QList<QGraphicsItem *> items = m_sketchWidget->scene()->items(sceneMouseEvent->scenePos());
	foreach (QGraphicsItem * item, items) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;

		if (m_externalConnectorItems.contains(connectorItem)) {
			m_externalConnectorItems.removeOne(connectorItem);
			connectorItem->setChosen(false);
		}
		else {
			m_externalConnectorItems.append(connectorItem);
			connectorItem->setChosen(true);
		}
		connectorItem->update();
		QApplication::processEvents();
	}
}

