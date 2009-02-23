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
#include "../zoomablegraphicsview.h"
#include "../partseditor/partspecificationswidget.h"
#include "../partseditor/partseditormainwindow.h"
#include "../partseditor/editablelinewidget.h"
#include "../partseditor/editabletextwidget.h"
#include "../partseditor/editabledatewidget.h"
#include "../partseditor/hashpopulatewidget.h"

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

	QFrame * centerFrame = new QFrame();
	centerFrame->setObjectName("center");

	QList<QWidget*> specWidgets;

	QLabel * prompt = new QLabel(QObject::tr("<html><body>"
											"<p>To make a connector \"external\", so that parts outside this module can connect to it, click it;"
											"click again to make it unavailable for connecting.  "
											"To make selection easier, use the mouse wheel to zoom in and out.</p>"
											"<p>Don't forget to scroll down and fill out the description and other fields that describe your module.</p>"
											"</body></html>"), 
											
								this);
	prompt->setWordWrap(true);


	sketchWidget->scene()->installEventFilter(this);

	QGraphicsView * gv = new ZoomableGraphicsView(this);
	gv->setMinimumSize(300, 300);
	gv->setScene(sketchWidget->scene());
	gv->show();


	ModelPart * modelPart = NULL;
	WaitPushUndoStack * undoStack = new WaitPushUndoStack();

	QString label = PartsEditorMainWindow::LabelFreshStartText;
	EditableLineWidget * labelWidget = new EditableLineWidget(label,undoStack,this,tr("Label"),modelPart);

	QString description = PartsEditorMainWindow::DescriptionFreshStartText;
	EditableTextWidget * descriptionWidget = new EditableTextWidget(description,undoStack,this,tr("Description"),modelPart);

	QStringList readOnlyKeys;
	readOnlyKeys << "family" << "voltage" << "type";

	QHash<QString,QString> initValues;
	initValues["family"] = "";
	HashPopulateWidget * propertiesWidget = new HashPopulateWidget(tr("Properties"),initValues,readOnlyKeys,undoStack,this);

	QString tags = PartsEditorMainWindow::TagsFreshStartText;
	EditableLineWidget * tagsWidget = new EditableLineWidget(tags,undoStack,this,tr("Tags"),modelPart);

	EditableLineWidget * authorWidget = new EditableLineWidget(
		QString(getenv("USER")),
		undoStack, this, tr("Author"),true);
	
	/*connect(
		m_author,SIGNAL(editionCompleted(QString)),
		this,SLOT(updateDateAndAuthor()));
	*/

	EditableDateWidget * createdOnWidget = new EditableDateWidget(
		QDate::currentDate(),
		undoStack,this, tr("Created/Updated on"),true);
	
	/*connect(
		m_createdOn,SIGNAL(editionCompleted(QString)),
		this,SLOT(updateDateAndAuthor()));
	*/

	QLabel * createdByTextWidget = new QLabel(PartsEditorMainWindow::FooterText.arg(authorWidget->text()).arg(createdOnWidget->text()));
	createdByTextWidget->setObjectName("createdBy");

	specWidgets << labelWidget << descriptionWidget  << propertiesWidget << tagsWidget << authorWidget << createdOnWidget << createdByTextWidget;

	PartSpecificationsWidget * partSpecWidget = new PartSpecificationsWidget(specWidgets,this);

	QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));



	QGridLayout *frameLayout = new QGridLayout(centerFrame);
	frameLayout->setMargin(0);
	frameLayout->setSpacing(0);
	frameLayout->addWidget(partSpecWidget,0,0,1,1);
	centerFrame->setLayout(frameLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(prompt);
	mainLayout->addWidget(gv);
	mainLayout->addWidget(centerFrame);
	mainLayout->addWidget(buttonBox);
	this->setMinimumSize(QSize(400, 400));
	this->setLayout(mainLayout);
	
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

