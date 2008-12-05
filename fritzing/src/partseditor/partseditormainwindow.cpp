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



#include "partseditormainwindow.h"
#include "pcbxml.h"
#include "../debugdialog.h"
#include "../fdockwidget.h"
#include "../waitpushundostack.h"
#include "editabletextwidget.h"
#include "partseditorviewimagewidget.h"
#include "partsymbolswidget.h"
#include "partspecificationswidget.h"

#include <QtGui>
#include <QCryptographicHash>
#include <QRegExpValidator>
#include <QRegExp>
#include <stdlib.h>

const QString ___partsEditorName___ = QObject::tr("Parts Editor");
const QString PartsEditorMainWindow::templatePath = "/docs/templates/";
const QString PartsEditorMainWindow::UntitledPartName = "Untitled Part";
int PartsEditorMainWindow::UntitledPartIndex = 1;
PartsEditorMainWindow *PartsEditorMainWindow::m_lastOpened = NULL;
int PartsEditorMainWindow::m_closedBeforeCount = 0;

#ifdef QT_NO_DEBUG
	#define CORE_EDITION_ENABLED false
#else
	#define CORE_EDITION_ENABLED true
#endif

PartsEditorMainWindow::PartsEditorMainWindow(long id, QWidget * parent, Qt::WFlags f, ModelPart *modelPart, bool fromTemplate)
	: FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension(), parent, f)
{
    QFile styleSheet(":/resources/styles/partseditor.qss");

    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open :/resources/styles/partseditor.qss");
    } else {
    	setStyleSheet(styleSheet.readAll()+___MacStyle___);
    }

    resize(500,700);

	m_id = id;

	setAttribute(Qt::WA_DeleteOnClose, true);

	m_paletteModel = new PaletteModel(false, false);

	if(modelPart == NULL){
		if (fromTemplate && !createTemplate()){
			QMessageBox::critical(this, tr("Part Editor"),
	                           tr("Error! Cannot create part file.\n"),
	                           QMessageBox::Close);
	        // this seems hacky but maybe it's ok
	        QTimer::singleShot(60, this, SLOT(close()));
			return;
		}
		m_lastOpened = this;
		m_updateEnabled = CORE_EDITION_ENABLED;
	} else {
		m_updateEnabled = CORE_EDITION_ENABLED || !modelPart->isCore();
		m_fileName = modelPart->modelPartStuff()->path();
		setTitle();
		UntitledPartIndex--; // TODO Mariano: not good enough
	}

	if(!fromTemplate) {
		m_sketchModel = new SketchModel(true);
	} else {
		modelPart = m_paletteModel->loadPart(m_fileName);
		m_sketchModel = new SketchModel(modelPart);
	}

	ModelPart *mp = fromTemplate ? modelPart : NULL;

	createHeader(mp);
	createCenter(mp);
	createFooter();

	layout()->setMargin(0);
	layout()->setSpacing(0);


	QFrame * frame = new QFrame();
	QGridLayout *layout = new QGridLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_headerFrame,0,0);
	layout->addWidget(m_centerFrame,1,0);
	layout->addWidget(m_footerFrame,2,0);
	frame->setLayout(layout);
	setCentralWidget(frame);

    if(fromTemplate) {
    	m_symbols->loadViewsImagesFromModel(m_paletteModel, m_sketchModel->root());
    }

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	QSettings settings("Fritzing","Fritzing");
	if(!settings.value("peditor/state").isNull()) {
		restoreState(settings.value("peditor/state").toByteArray());
		restoreGeometry(settings.value("peditor/geometry").toByteArray());
	}

	installEventFilter(this);
}

void PartsEditorMainWindow::createHeader(ModelPart *modelPart) {
	m_headerFrame = new QFrame();
	m_headerFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
	m_headerFrame->setObjectName("header");

	m_iconViewImage = new PartsEditorViewImageWidget(ItemBase::IconView, createTempFolderIfNecessary(), 0, m_headerFrame, 50);
	m_iconViewImage->setObjectName("iconImage");
	m_iconViewImage->setSketchModel(m_sketchModel);
	m_iconViewImage->setUndoStack(m_undoStack);
	m_iconViewImage->addViewLayer(new ViewLayer(ViewLayer::Icon, true, 2.5));
	m_iconViewImage->setViewLayerIDs(ViewLayer::Icon, ViewLayer::Icon, ViewLayer::Icon, ViewLayer::Icon);
	if(modelPart) {
		m_iconViewImage->loadFromModel(m_paletteModel, modelPart);
	}

	QString title = modelPart ? modelPart->modelPartStuff()->title() : TITLE_FRESH_START_TEXT;
	m_title = new EditableLineWidget(title,m_undoStack,m_headerFrame,"",modelPart,true);
	m_title->setObjectName("partTitle");
	m_title->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));

	QGridLayout *headerLayout = new QGridLayout();
	headerLayout->addWidget(m_iconViewImage,0,0);
	headerLayout->addWidget(m_title,0,1);
	m_headerFrame->setLayout(headerLayout);
}

void PartsEditorMainWindow::createCenter(ModelPart *modelPart) {
	m_moduleId = modelPart ? modelPart->modelPartStuff()->moduleID() : "";
	m_version  = modelPart ? modelPart->modelPartStuff()->version() : "";
	m_uri      = modelPart ? modelPart->modelPartStuff()->uri() : "";

	m_centerFrame = new QFrame();
	m_centerFrame->setObjectName("center");

	QList<QWidget*> specWidgets;
	m_symbols = new PartSymbolsWidget(m_sketchModel, m_undoStack, this);

	QString label = modelPart ? modelPart->modelPartStuff()->label() : LABEL_FRESH_START_TEXT;
	m_label = new EditableLineWidget(label,m_undoStack,this,tr("Label"),modelPart);

	QString description = modelPart ? modelPart->modelPartStuff()->description() : DESCRIPTION_FRESH_START_TEXT;
	m_description = new EditableTextWidget(description,m_undoStack,this,tr("Description"),modelPart);

	/*QString taxonomy = modelPart ? modelPart->modelPartStuff()->taxonomy() : TAXONOMY_FRESH_START_TEXT;
	m_taxonomy = new EditableLineWidget(taxonomy,m_undoStack,this,tr("Taxonomy"),modelPart);
	QRegExp regexp("[\\w+\\.]+\\w$");
	m_taxonomy->setValidator(new QRegExpValidator(regexp,this));*/

	QStringList readOnlyKeys;
	readOnlyKeys << "family" << "voltage" << "type";

	QHash<QString,QString> initValues;
	if(modelPart) {
		initValues = modelPart->modelPartStuff()->properties();
	} else {
		initValues["family"] = "";
		//initValues["voltage"] = "";
		//initValues["type"] = "Through-Hole";
	}

	m_properties = new HashPopulateWidget(tr("Properties"),initValues,readOnlyKeys,m_undoStack,this);

	QString tags = modelPart ? modelPart->modelPartStuff()->tags().join(", ") : TAGS_FRESH_START_TEXT;
	m_tags = new EditableLineWidget(tags,m_undoStack,this,tr("Tags"),modelPart);

	m_author = new EditableLineWidget(
		modelPart ? modelPart->modelPartStuff()->author() : QString(getenv("USER")),
		m_undoStack, this, tr("Author"),true);
	connect(
		m_author,SIGNAL(editionCompleted(QString)),
		this,SLOT(updateDateAndAuthor()));

	m_createdOn = new EditableDateWidget(
		modelPart ? modelPart->modelPartStuff()->date() : QDate::currentDate(),
		m_undoStack,this, tr("Created/Updated on"),true);
	connect(
		m_createdOn,SIGNAL(editionCompleted(QString)),
		this,SLOT(updateDateAndAuthor()));

	m_createdByText = new QLabel(FOOTER_TEXT.arg(m_author->text()).arg(m_createdOn->text()));
	m_createdByText->setObjectName("createdBy");

	specWidgets << m_symbols << m_label << m_description /*<< m_taxonomy*/ << m_properties << m_tags << m_author << m_createdOn << m_createdByText;

	QList<QWidget*> connsWidgets;
	m_connsInfo = new ConnectorsInfoWidget(m_undoStack,this);
	m_connsViews = new ConnectorsViewsWidget(m_symbols, m_sketchModel, m_undoStack, m_connsInfo, this);

	connect(m_connsInfo, SIGNAL(repaintNeeded()), m_connsViews, SLOT(repaint()));

	connect(
		m_symbols, SIGNAL(connectorsFound(QList<Connector*>)),
		m_connsInfo, SLOT(connectorsFound(QList<Connector*>))
	);
	connsWidgets << m_connsViews << m_connsInfo;

	m_tabWidget = new QTabWidget(m_centerFrame);
	m_tabWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding));
	m_tabWidget->addTab(new PartSpecificationsWidget(specWidgets,this),tr("Specifications"));
	m_tabWidget->addTab(new PartConnectorsWidget(connsWidgets,this),tr("Connectors"));

	QGridLayout *tabLayout = new QGridLayout();
	tabLayout->setMargin(0);
	tabLayout->setSpacing(0);
	m_tabWidget->setLayout(tabLayout);

	QGridLayout *mainLayout = new QGridLayout();
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(m_tabWidget,0,0,1,1);
	m_centerFrame->setLayout(mainLayout);

}

void PartsEditorMainWindow::connectWidgetsToSave(const QList<QWidget*> &widgets) {
	for(int i=0; i < widgets.size(); i++) {
		connect(m_saveAsNewPartButton,SIGNAL(clicked()),widgets[i],SLOT(informEditionCompleted()));
		connect(m_saveButton,SIGNAL(clicked()),widgets[i],SLOT(informEditionCompleted()));
	}
}

void PartsEditorMainWindow::createFooter() {
	m_footerFrame = new QFrame();
	m_footerFrame->setObjectName("footer");
	m_footerFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

	m_saveAsNewPartButton = new QPushButton(tr("save as new part"));
	m_saveAsNewPartButton->setObjectName("saveAsPartButton");
	m_saveAsNewPartButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

	m_saveButton = new QPushButton(tr("save"));
	m_saveButton->setObjectName("saveAsButton");
	m_saveButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	updateSaveButton();

	QList<QWidget*> editableLabelWidgets;
	editableLabelWidgets << m_title << m_label << m_description /*<< m_taxonomy*/ << m_tags << m_author << m_createdOn << m_connsInfo ;
	connectWidgetsToSave(editableLabelWidgets);

	connect(m_saveAsNewPartButton, SIGNAL(clicked()), this, SLOT(saveAs()));
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()));

	m_cancelButton = new QPushButton(tr("cancel"));
	m_cancelButton->setObjectName("cancelButton");
	m_cancelButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *footerLayout = new QHBoxLayout;

	footerLayout->setMargin(0);
	footerLayout->setSpacing(0);
	footerLayout->addWidget(m_saveAsNewPartButton);
	footerLayout->addWidget(m_saveButton);
	footerLayout->addWidget(m_cancelButton);
	m_footerFrame->setLayout(footerLayout);
}


QGraphicsItem *PartsEditorMainWindow::emptyViewItem(QString iconFile, QString text) {
	QGraphicsItem * parentITem = new QGraphicsSvgItem();
	QPixmap pixmap = QPixmap("resources/images/"+iconFile);
	QGraphicsItem *iconItem = new QGraphicsPixmapItem(pixmap,parentITem);
	qreal scaleFactor = 20.0/*px*/ / pixmap.width();
	iconItem->scale(scaleFactor,scaleFactor);
	if(!text.isNull() && !text.isEmpty()) {
		QGraphicsTextItem *textItem = new QGraphicsTextItem(text,parentITem);
		Q_UNUSED(textItem)
	}
	return parentITem;
}

// creates a temp directory and copies all template .fz and .svg files
// returns false if directory can't be created
bool PartsEditorMainWindow::createTemplate(){
	QDir srcDir = QDir(":/resources/part-template");
	DebugDialog::debug("template source: " + srcDir.path());

	QDir randDir = createTempFolderIfNecessary();

	replicateDir(srcDir,randDir);

	QFile tempFile(QCoreApplication::applicationDirPath() + templatePath);
	tempFile.copy(randDir.path() + "/core/template.fz");

	m_fileName = randDir.path() + "/core/template.fz";
	DebugDialog::debug("created temp part: " + m_fileName);

	return true;
}

const QDir& PartsEditorMainWindow::createTempFolderIfNecessary() {
	if(m_tempDir.path() == ".") {
		QString randext = getRandText();
		m_tempDir = QDir(QDir::tempPath());
		if(!m_tempDir.mkdir(randext)) return ___emptyDir___;
		if(!m_tempDir.cd(randext)) return ___emptyDir___;
	}
	return m_tempDir;
}

void PartsEditorMainWindow::loadPcbFootprint(){
	QString pcbFootprintFile;
	//this->loadFileIntoView(m_pcbSvgFile, m_pcbView, ItemBase::PCBView, "copper0");
	pcbFootprintFile = QFileDialog::getOpenFileName(this,
       tr("Open Image"), QApplication::applicationFilePath(), tr("SVG Files (*.fzp)"));

    QFile file(pcbFootprintFile);
    QDomDocument doc("footprint");
    if (!file.open(QIODevice::ReadOnly)) {
    	 DebugDialog::debug("cannot open fzp file");
         return;
    }
    if (!doc.setContent(&file)) {
    	DebugDialog::debug("cannot parse fzp xml");
        file.close();
        return;
    }
    file.close();
    PcbXML *footprint = new PcbXML(doc.documentElement());
    DebugDialog::debug(footprint->getSvgFile());
}

bool PartsEditorMainWindow::saveAs() {
	QString fileNameBU = m_fileName;
	QString moduleIdBU = m_moduleId;

	m_moduleId = ___emptyString___;
	QString title = m_title->text();

	m_fileName = title != ___emptyString___ ? title+FritzingExtension : m_fileName;

	// TODO Mariano: This folder should be defined in the preferences... some day
	QString userPartsFolderPath = getApplicationSubFolderPath("parts")+"/user/";

	bool firstTime = true; // Perhaps the user wants to use the default file name, confirm first
	while(m_fileName.isEmpty()
		  || QFileInfo(userPartsFolderPath+m_fileName).exists()
		  || (isEmptyFileName(m_fileName,untitledFileName()) && firstTime)
		) {
		bool ok;
		m_fileName = QInputDialog::getText(
			this,
			tr("Save as new part"),
			tr("Please, specify a new filename"),
			QLineEdit::Normal,
			m_fileName,
			&ok
		);
		firstTime = false;
		if (!ok) {
			m_moduleId = moduleIdBU;
			m_fileName = fileNameBU;
			return false;
		}
	}

	QString filename = userPartsFolderPath+m_fileName;
	if(!alreadyHasExtension(filename)) {
		filename += FritzingExtension;
	}

	saveAsAux(filename);

	m_updateEnabled = true;
	updateSaveButton();

	return true;
}

void PartsEditorMainWindow::saveAsAux(const QString & fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    file.close();

    updateDateAndAuthor();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_sketchModel->root()->setModelPartStuff(modelPartStuff());
	m_sketchModel->save(fileName, true);

	m_symbols->copySvgFilesToDestiny();
	m_iconViewImage->copySvgFileToDestiny();

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);

    emit partUpdated(fileName);

    m_fileName = fileName;
    //setCurrentFile(fileName);

   // mark the stack clean so we update the window dirty flag
    m_undoStack->setClean();
    setTitle();
}

void PartsEditorMainWindow::updateDateAndAuthor() {
	m_createdByText->setText(FOOTER_TEXT.arg(m_author->text()).arg(m_createdOn->text()));
}

ModelPartStuff* PartsEditorMainWindow::modelPartStuff() {
	ModelPartStuff* stuff = new ModelPartStuff();

	if(m_moduleId.isNull() || m_moduleId.isEmpty()) {
		m_moduleId = getRandText();
	}

	stuff->setModuleID(m_moduleId);
	stuff->setUri(m_uri);
	stuff->setVersion(m_version);

	stuff->setAuthor(m_author->text());
	stuff->setTitle(m_title->text());
	stuff->setDate(m_createdOn->text());
	stuff->setLabel(m_label->text());
	//stuff->setTaxonomy(m_taxonomy->text());
	stuff->setDescription(m_description->text());

	QStringList tags = m_tags->text().split(", ");
	stuff->setTags(tags);
	stuff->setProperties(m_properties->hash());

	stuff->setConnectorsStuff(m_connsInfo->connectorsStuffs());

	return stuff;
}

void PartsEditorMainWindow::cleanUp() {
	bool decUntitled = m_fileName.startsWith(untitledFileName());
	if(decUntitled) {
		if(m_lastOpened == this) {
			UntitledPartIndex -= m_closedBeforeCount;
			UntitledPartIndex--;
			m_closedBeforeCount = 0;
		} else {
			m_closedBeforeCount++;
		}
	}
	if(m_tempDir.path() != ".") {
		rmdir(m_tempDir);
		m_tempDir = QDir();
	}
}

void PartsEditorMainWindow::parentAboutToClose() {
	if(beforeClosing(false)) {
		cleanUp();
	}
}

void PartsEditorMainWindow::closeEvent(QCloseEvent *event) {
	if(beforeClosing()) {
		cleanUp();
		QMainWindow::closeEvent(event);
		emit closed(m_id);
	} else {
		event->ignore();
	}

	QSettings settings("Fritzing","Fritzing");
	settings.setValue("peditor/state",saveState());
	settings.setValue("peditor/geometry",saveGeometry());
}

const QDir& PartsEditorMainWindow::tempDir() {
	return createTempFolderIfNecessary();
}

const QString PartsEditorMainWindow::untitledFileName() {
	return UntitledPartName;
}

int &PartsEditorMainWindow::untitledFileCount() {
	return UntitledPartIndex;
}

const QString PartsEditorMainWindow::fileExtension() {
	return FritzingExtension;
}

const QString PartsEditorMainWindow::defaultSaveFolder() {
	return QDir::currentPath()+"/parts/user/";
}

void PartsEditorMainWindow::updateSaveButton() {
	m_saveButton->setEnabled(m_updateEnabled);
}

bool PartsEditorMainWindow::eventFilter(QObject *object, QEvent *event) {
	if (object == this && event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
		if(keyEvent && keyEvent->matches(QKeySequence::Close)) {
			this->close();
			event->ignore();
			QCoreApplication::processEvents();
#ifdef Q_WS_MAC
			MainWindow *parent = dynamic_cast<MainWindow*>(parentWidget());
			if(parent) {
				parent->notClosableForAWhile();
			}
#endif
			return true;
		}
	}
	return QMainWindow::eventFilter(object, event);
}

const QString PartsEditorMainWindow::fritzingTitle() {
	QString fritzing = FritzingWindow::fritzingTitle();
	return tr("%1 %2").arg(fritzing).arg(___partsEditorName___);
}
