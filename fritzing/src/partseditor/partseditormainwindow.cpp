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



#include "partseditormainwindow.h"
#include "pcbxml.h"
#include "../debugdialog.h"
#include "../fdockwidget.h"
#include "../waitpushundostack.h"
#include "editabletextwidget.h"
#include "partseditorview.h"
#include "partseditorviewswidget.h"
#include "editabledatewidget.h"
#include "hashpopulatewidget.h"
#include "partspecificationswidget.h"
#include "partconnectorswidget.h"
#include "../palettemodel.h"
#include "../sketchmodel.h"

#include <QtGui>
#include <QCryptographicHash>
#include <QRegExpValidator>
#include <QRegExp>
#include <stdlib.h>


QString PartsEditorMainWindow::TitleFreshStartText;
QString PartsEditorMainWindow::LabelFreshStartText;
QString PartsEditorMainWindow::DescriptionFreshStartText;
QString PartsEditorMainWindow::TaxonomyFreshStartText;
QString PartsEditorMainWindow::TagsFreshStartText;
QString PartsEditorMainWindow::FooterText;
QString PartsEditorMainWindow::UntitledPartName;
QString PartsEditorMainWindow::___partsEditorName___;

const QString PartsEditorMainWindow::templatePath = "/docs/templates/";

int PartsEditorMainWindow::UntitledPartIndex = 1;
PartsEditorMainWindow *PartsEditorMainWindow::m_lastOpened = NULL;
int PartsEditorMainWindow::m_closedBeforeCount = 0;

#ifdef QT_NO_DEBUG
	#define CORE_EDITION_ENABLED false
#else
	#define CORE_EDITION_ENABLED true
#endif

void PartsEditorMainWindow::initText() {
	UntitledPartName = tr("Untitled Part");
	TitleFreshStartText = tr("Please find a name for me!");
	LabelFreshStartText = tr("Please provide a label");
	DescriptionFreshStartText = tr("You could tell a little bit about this part");
	TaxonomyFreshStartText = tr("Please classify this part");
	TagsFreshStartText = tr("You can add your tags to make searching easier");
	FooterText = tr("<i>created by</i> %1 <i>on</i> %2");
	___partsEditorName___ = tr("Parts Editor");

}

PartsEditorMainWindow::PartsEditorMainWindow(long id, QWidget * parent, ModelPart *modelPart, bool fromTemplate, bool asMainWindow)
	: FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension(), parent)
{
    QFile styleSheet(":/resources/styles/partseditor.qss");
    m_mainFrame = new QFrame(this);
    m_mainFrame->setObjectName("partsEditor");

    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open :/resources/styles/partseditor.qss");
    } else {
    	m_mainFrame->setStyleSheet(styleSheet.readAll()+___MacStyle___);
    }

    resize(500,700);

	m_id = id;
	m_partUpdated = false;

	setAttribute(Qt::WA_DeleteOnClose, true);

	m_paletteModel = new PaletteModel(false, false);

	if(modelPart == NULL){
		if (fromTemplate && !createTemplate()){
			QMessageBox::critical(this, tr("Parts Editor"),
	                           tr("Error! Cannot create part file.\n"),
	                           QMessageBox::Close);
	        // this seems hacky but maybe it's ok
	        QTimer::singleShot(60, this, SLOT(close()));
			return;
		}
		m_lastOpened = this;
		m_updateEnabled = false;
	} else {
		// user only allowed to save parts, once he has saved it as a new one
		m_updateEnabled = modelPart->isCore()? CORE_EDITION_ENABLED: false;
		m_fileName = modelPart->modelPartShared()->path();
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
	if(asMainWindow) {
		createFooter();
	} else {
		m_footerFrame = NULL;
	}

	layout()->setMargin(0);
	layout()->setSpacing(0);


	QGridLayout *layout = new QGridLayout(m_mainFrame);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_headerFrame,0,0);
	layout->addWidget(m_centerFrame,1,0);
	if(asMainWindow) {
		layout->addWidget(m_footerFrame,2,0);
	}
	setCentralWidget(m_mainFrame);

    if(fromTemplate) {
    	m_views->loadViewsImagesFromModel(m_paletteModel, m_sketchModel->root());
    }

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	QSettings settings;
	if (!settings.value("peditor/state").isNull()) {
		restoreState(settings.value("peditor/state").toByteArray());
	}
	if (!settings.value("peditor/geometry").isNull()) {
		restoreGeometry(settings.value("peditor/geometry").toByteArray());
	}

	installEventFilter(this);
}

PartsEditorMainWindow::~PartsEditorMainWindow()
{
	if (m_sketchModel) {
		//delete m_sketchModel;
		//delete m_paletteModel;
	}
}

void PartsEditorMainWindow::createHeader(ModelPart *modelPart) {
	m_headerFrame = new QFrame();
	m_headerFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
	m_headerFrame->setObjectName("header");
	m_headerFrame->setStyleSheet("padding: 2px; padding-bottom: 0;");

	int iconViewSize = 50;
	QGraphicsItem *startItem = modelPart? NULL: PartsEditorMainWindow::emptyViewItem("icon_icon.png",___emptyString___);
	m_iconViewImage = new PartsEditorView(
		ViewIdentifierClass::IconView, createTempFolderIfNecessary(), false, startItem, m_headerFrame, iconViewSize
	);
	m_iconViewImage->setFixedSize(iconViewSize,iconViewSize);
	m_iconViewImage->setObjectName("iconImage");
	m_iconViewImage->setSketchModel(m_sketchModel);
	m_iconViewImage->setUndoStack(m_undoStack);
	m_iconViewImage->addViewLayer(new ViewLayer(ViewLayer::Icon, true, 2.5));
	m_iconViewImage->setViewLayerIDs(ViewLayer::Icon, ViewLayer::Icon, ViewLayer::Icon, ViewLayer::Icon, ViewLayer::Icon, ViewLayer::Icon);
	if(modelPart) {
		m_iconViewImage->loadFromModel(m_paletteModel, modelPart);
	}

	QString linkStyle = "font-size: 10px; color: white; text-decoration: none;";
	QLabel *button = new QLabel(
						QString("<a style='%2' href='#'>%1</a>")
							.arg(tr("image ..."))
							.arg(linkStyle),
						this);
	button->setObjectName("iconBrowseButton");
	button->setFixedWidth(iconViewSize);
	button->setFixedHeight(20);
	//m_iconViewImage->addFixedToBottomRight(button);
	connect(button, SIGNAL(linkActivated(const QString&)), m_iconViewImage, SLOT(loadFile()));

	QString title = modelPart ? modelPart->modelPartShared()->title() : TitleFreshStartText;
	m_title = new EditableLineWidget(title,m_undoStack,m_headerFrame,"",modelPart,true);
	m_title->setObjectName("partTitle");
	m_title->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding));

	QGridLayout *headerLayout = new QGridLayout();
	headerLayout->addWidget(m_iconViewImage,0,0);
	headerLayout->addWidget(button,1,0);
	headerLayout->addWidget(m_title,0,1);
	headerLayout->setVerticalSpacing(0);
	headerLayout->setMargin(0);
	m_headerFrame->setLayout(headerLayout);
}

void PartsEditorMainWindow::createCenter(ModelPart *modelPart) {
	m_moduleId = modelPart ? modelPart->modelPartShared()->moduleID() : "";
	m_version  = modelPart ? modelPart->modelPartShared()->version() : "";
	m_uri      = modelPart ? modelPart->modelPartShared()->uri() : "";

	m_centerFrame = new QFrame();
	m_centerFrame->setObjectName("center");

	QList<QWidget*> specWidgets;

	m_connsInfo = new ConnectorsInfoWidget(m_undoStack,this);
	m_views = new PartsEditorViewsWidget(m_sketchModel, m_undoStack, m_connsInfo, this);

	QString label = modelPart ? modelPart->modelPartShared()->label() : LabelFreshStartText;
	m_label = new EditableLineWidget(label,m_undoStack,this,tr("Label"),modelPart);

	QString description = modelPart ? modelPart->modelPartShared()->description() : DescriptionFreshStartText;
	m_description = new EditableTextWidget(description,m_undoStack,this,tr("Description"),modelPart);

	/*QString taxonomy = modelPart ? modelPart->modelPartShared()->taxonomy() : TAXONOMY_FRESH_START_TEXT;
	m_taxonomy = new EditableLineWidget(taxonomy,m_undoStack,this,tr("Taxonomy"),modelPart);
	QRegExp regexp("[\\w+\\.]+\\w$");
	m_taxonomy->setValidator(new QRegExpValidator(regexp,this));*/

	QStringList readOnlyKeys;
	readOnlyKeys << "family" << "voltage" << "type";

	QHash<QString,QString> initValues;
	if(modelPart) {
		initValues = modelPart->modelPartShared()->properties();
	} else {
		initValues["family"] = "";
		//initValues["voltage"] = "";
		//initValues["type"] = "Through-Hole";
	}

	m_properties = new HashPopulateWidget(tr("Properties"),initValues,readOnlyKeys,m_undoStack,this);

	QString tags = modelPart ? modelPart->modelPartShared()->tags().join(", ") : TagsFreshStartText;
	m_tags = new EditableLineWidget(tags,m_undoStack,this,tr("Tags"),modelPart);


	m_author = new EditableLineWidget(
		modelPart ? modelPart->modelPartShared()->author() : QString(getenvUser()),
		m_undoStack, this, tr("Author"),true);
	connect(
		m_author,SIGNAL(editionCompleted(QString)),
		this,SLOT(updateDateAndAuthor()));

	m_createdOn = new EditableDateWidget(
		modelPart ? modelPart->modelPartShared()->date() : QDate::currentDate(),
		m_undoStack,this, tr("Created/Updated on"),true);
	connect(
		m_createdOn,SIGNAL(editionCompleted(QString)),
		this,SLOT(updateDateAndAuthor()));

	m_createdByText = new QLabel(FooterText.arg(m_author->text()).arg(m_createdOn->text()));
	m_createdByText->setObjectName("createdBy");

	specWidgets << m_label << m_description /*<< m_taxonomy*/ << m_properties << m_tags << m_author << m_createdOn << m_createdByText;

	m_connsInfo->setViews(m_views);

	connect(m_connsInfo, SIGNAL(repaintNeeded()), m_views, SLOT(repaint()));
	connect(m_connsInfo, SIGNAL(drawConnector(Connector*)), m_views, SLOT(drawConnector(Connector*)));
	connect(
		m_connsInfo, SIGNAL(drawConnector(ViewIdentifierClass::ViewIdentifier, Connector*)),
		m_views, SLOT(drawConnector(ViewIdentifierClass::ViewIdentifier, Connector*))
	);
	connect(
		m_connsInfo, SIGNAL(setMismatching(ViewIdentifierClass::ViewIdentifier, const QString&, bool)),
		m_views, SLOT(setMismatching(ViewIdentifierClass::ViewIdentifier, const QString&, bool))
	);
	connect(m_connsInfo, SIGNAL(removeConnectorFrom(const QString&,ViewIdentifierClass::ViewIdentifier)),
			m_views, SLOT(removeConnectorFrom(const QString&,ViewIdentifierClass::ViewIdentifier)));
	connect(m_views, SIGNAL(connectorSelectedInView(const QString&)),
			m_connsInfo, SLOT(connectorSelectedInView(const QString&)));
	m_views->showTerminalPointsCheckBox()->setChecked(false);

	connect(
		m_views, SIGNAL(connectorsFound(QList<Connector*>)),
		m_connsInfo, SLOT(connectorsFound(QList<Connector*>))
	);

	m_tabWidget = new QTabWidget(m_centerFrame);
	m_tabWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_tabWidget->addTab(new PartSpecificationsWidget(specWidgets,this),tr("Specifications"));
	m_tabWidget->addTab(new PartConnectorsWidget(m_connsInfo,this),tr("Connectors"));

	QGridLayout *tabLayout = new QGridLayout(m_tabWidget);
	tabLayout->setMargin(0);
	tabLayout->setSpacing(0);

	QSplitter *splitter = new QSplitter(Qt::Vertical,this);
	splitter->addWidget(m_views);
	splitter->addWidget(m_tabWidget);

	QGridLayout *mainLayout = new QGridLayout(m_centerFrame);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(splitter,0,0,1,1);
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
	footerLayout->addSpacerItem(new QSpacerItem(40,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_saveAsNewPartButton);
	footerLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
	footerLayout->addWidget(m_saveButton);
	footerLayout->addSpacerItem(new QSpacerItem(15,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	footerLayout->addWidget(m_cancelButton);
	footerLayout->addSpacerItem(new QSpacerItem(40,0,QSizePolicy::Minimum,QSizePolicy::Minimum));
	m_footerFrame->setLayout(footerLayout);
}


QGraphicsItem *PartsEditorMainWindow::emptyViewItem(QString iconFile, QString text) {
	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap(":/resources/images/"+iconFile));
	icon->setAlignment(Qt::AlignHCenter);

	QLabel *label = NULL;
	if(!text.isNull() && !text.isEmpty()) {
		label = new QLabel(text);
		label->setAlignment(Qt::AlignHCenter);
	}

	QFrame *container = new QFrame();
	container->setStyleSheet("background-color: transparent;");
	QVBoxLayout *mainLo = new QVBoxLayout(container);
	mainLo->setMargin(0);
	mainLo->setSpacing(0);

	QHBoxLayout *lo1 = new QHBoxLayout();
	lo1->addWidget(icon);
	lo1->setMargin(0);
	lo1->setSpacing(0);
	mainLo->addLayout(lo1);

	if(label) {
		QHBoxLayout *lo2 = new QHBoxLayout();
		lo2->addWidget(label);
		lo2->setMargin(0);
		lo2->setSpacing(0);
		mainLo->addLayout(lo2);
	}

	QGraphicsProxyWidget *item = new QGraphicsProxyWidget();
	item->setWidget(container);

	return item;
}

// creates a temp directory and copies all template .fz and .svg files
// returns false if directory can't be created
bool PartsEditorMainWindow::createTemplate(){
	QDir srcDir = QDir(":/resources/part-template");
	DebugDialog::debug("template source: " + srcDir.path());

	QDir randDir = createTempFolderIfNecessary();

	replicateDir(srcDir,randDir);

	QFile tempFile(QCoreApplication::applicationDirPath() + templatePath);
	tempFile.copy(randDir.path() + "/core/template" + FritzingPartExtension);

	m_fileName = randDir.path() + "/core/template" + FritzingPartExtension;
	DebugDialog::debug("created temp part: " + m_fileName);

	return true;
}

const QDir& PartsEditorMainWindow::createTempFolderIfNecessary() {
	if(m_tempDir.path() == ".") {
		QString randext = getRandText();
		m_tempDir = QDir(QDir::tempPath());
		Q_ASSERT(m_tempDir.mkdir(randext));
		Q_ASSERT(m_tempDir.cd(randext));
	}
	return m_tempDir;
}

void PartsEditorMainWindow::loadPcbFootprint(){
	QString pcbFootprintFile;
	//this->loadFileIntoView(m_pcbSvgFile, m_pcbView, ItemBase::PCBView, "copper0");
	pcbFootprintFile = QFileDialog::getOpenFileName(this,
       tr("Open Image"), QApplication::applicationFilePath(), tr("SVG Files (*.fzfp)"));

    QFile file(pcbFootprintFile);
    QDomDocument doc("footprint");
    if (!file.open(QIODevice::ReadOnly)) {
    	 DebugDialog::debug("cannot open fzfp file");
         return;
    }
    if (!doc.setContent(&file)) {
    	DebugDialog::debug("cannot parse fzfp xml");
        file.close();
        return;
    }
    file.close();
    PcbXML *footprint = new PcbXML(doc.documentElement());
    DebugDialog::debug(footprint->getSvgFile());
}

bool PartsEditorMainWindow::save() {
	if(validateMinRequirements()) {
		return FritzingWindow::save();
	} else {
		return false;
	}
}

bool PartsEditorMainWindow::saveAs() {
	if(validateMinRequirements()) {
		QString fileNameBU = m_fileName;
		QString moduleIdBU = m_moduleId;

		m_moduleId = ___emptyString___;
		QString title = m_title->text();

		m_fileName = title != ___emptyString___ ? title+FritzingPartExtension : m_fileName;

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
				tr("There's already a file with this name.\nPlease, specify a new filename"),
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

		Qt::CaseSensitivity cs = Qt::CaseSensitive;
	#ifdef Q_WS_WIN
		// seems to be necessary for Windows: getApplicationSubFolderPath() returns a string starting with "c:"
		// but the file dialog returns a string beginning with "C:"
		cs = Qt::CaseInsensitive;
	#endif

		QString filename = !m_fileName.startsWith(userPartsFolderPath, cs)
				? userPartsFolderPath+m_fileName
				: m_fileName;
		QString guid = "__"+getRandText()+FritzingPartExtension;;
		if(!alreadyHasExtension(filename)) {
			filename += guid;
		} else {
			filename.replace(FritzingPartExtension,guid);
		}

		saveAsAux(filename);

		m_updateEnabled = true;
		updateSaveButton();

		return true;
	} else {
		return false;
	}
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

    m_sketchModel->root()->setModelPartShared(modelPartShared());

	QString fileNameAux = QFileInfo(fileName).fileName();
	m_views->copySvgFilesToDestiny(fileNameAux);
	m_iconViewImage->copySvgFileToDestiny(fileNameAux);

	m_sketchModel->save(fileName, true);

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);

    m_partUpdated = true;

    m_fileName = fileName;
    //setCurrentFile(fileName);

   // mark the stack clean so we update the window dirty flag
    m_undoStack->setClean();
    setTitle();
}

void PartsEditorMainWindow::updateDateAndAuthor() {
	m_createdByText->setText(FooterText.arg(m_author->text()).arg(m_createdOn->text()));
}

ModelPartShared* PartsEditorMainWindow::modelPartShared() {
	ModelPartShared* shared = new ModelPartShared();

	if(m_moduleId.isNull() || m_moduleId.isEmpty()) {
		m_moduleId = getRandText();
	}

	shared->setModuleID(m_moduleId);
	shared->setUri(m_uri);
	shared->setVersion(m_version);

	shared->setAuthor(m_author->text());
	shared->setTitle(m_title->text());
	shared->setDate(m_createdOn->text());
	shared->setLabel(m_label->text());
	//stuff->setTaxonomy(m_taxonomy->text());
	shared->setDescription(m_description->text());

	QStringList tags = m_tags->text().split(", ");
	shared->setTags(tags);
	shared->setProperties(m_properties->hash());

	m_views->aboutToSave();
	shared->setConnectorsShared(m_connsInfo->connectorsShared());

	return shared;
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
		if(m_partUpdated) emit partUpdated(m_fileName);
		emit closed(m_id);
	} else {
		event->ignore();
	}

	QSettings settings;
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
	return FritzingPartExtension;
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
			FritzingWindow *parent = dynamic_cast<FritzingWindow*>(parentWidget());
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

bool PartsEditorMainWindow::event(QEvent * e) {
	switch (e->type()) {
		case QEvent::WindowActivate:
			emit changeActivationSignal(true);
			break;
		case QEvent::WindowDeactivate:
			emit changeActivationSignal(false);
			break;
		default:
			break;
	}
	return FritzingWindow::event(e);
}

bool PartsEditorMainWindow::validateMinRequirements() {
	if(!m_iconViewImage->isEmpty()) {
		return true;
	} else {
		QMessageBox::information(this, tr("Icon needed"), tr("Please, provide an icon image for this part"));
		return false;
	}
}
