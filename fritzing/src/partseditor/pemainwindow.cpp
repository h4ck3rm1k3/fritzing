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

/* TO DO ******************************************

	clean up menus

	add connectors tab

	underlying data structure

	first time help?

	disable dragging wires

	change pin count

    connector duplicate op

    swap connector metadata op

	add status bar to connectors tab

    on svg import detect all connector IDs
        if any are invisible, tell user this is obsolete

    allow user to select connectors by driving through svg elements
    
    move connectors with arrow keys, or typed coordinates
	drag and drop later
	
    connector metadata as palette?

    eagle lbr
    eagle brd
    kicad footprint and mod?
    gEDA footprint

    for schematic view 
        offer generate option
        offer pins, rects (or lines), and a selection of standard schematic icons
        text?

    for breadboard view
        import 
        generate ICs, dips, sips, breakouts

    for pcb view
        pads, pins (circle, rect, oblong)
        lines and curves?
        import silkscreen
        text?

    allow but discourage png imports

    for svg import check for flaws:
        internal coords
        corel draw not saved for presentation
        inkscape not saved as plain
        inkscape scaling?
        illustrator px
        <gradient>, <pattern>, <marker>, <tspan>, etc.

    holes

    smd vs. tht

    buses 
        connect bus by drawing a wire
        can this be modal? i.e. turn bus mode on and off

    bendable legs

    flip and rotate

    terminal points
        display as part of connectorItem
        move only with arrow keys (maybe cntl-arrow if arrow is used for connector)
        give presets: center, N, E, S, W, NE, NW, SE, SW

    undo/redo as xml file: use index + guid for uniqueness

    nudge via arrow keys

    taxonomy?

    new schematic layout specs

    deal with customized svgs
        chip label
        * pin label
        * resistance
        * led color
        pin header stuff
        pin size

    disable family entry?  should always be "custom_"  + original family (unless it already starts with custom_)

    allow rulers and other tools?


***************************************************/

#include "pemainwindow.h"
#include "metadataview.h"
#include "connectorsview.h"
#include "pecommands.h"
#include "../debugdialog.h"
#include "../model/palettemodel.h"
#include "../sketch/breadboardsketchwidget.h"
#include "../sketch/schematicsketchwidget.h"
#include "../sketch/pcbsketchwidget.h"
#include "../sketchareawidget.h"
#include "../referencemodel/referencemodel.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../utils/folderutils.h"


#ifdef QT_NO_DEBUG
	#define CORE_EDITION_ENABLED false
#else
	#define CORE_EDITION_ENABLED false
#endif

////////////////////////////////////////////////////

IconSketchWidget::IconSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : SketchWidget(viewIdentifier, parent)
{
	m_shortName = QObject::tr("ii");
	m_viewName = QObject::tr("Icon View");
	initBackgroundColor();
}

void IconSketchWidget::addViewLayers() {
	addIconViewLayers();
}

/////////////////////////////////////////////////////

PEMainWindow::PEMainWindow(PaletteModel * paletteModel, ReferenceModel * referenceModel, QWidget * parent)
	: MainWindow(paletteModel, referenceModel, parent)
{
    m_settingsPrefix = "pe/";
}

PEMainWindow::~PEMainWindow()
{
}

void PEMainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
}

void PEMainWindow::initLockedFiles(bool) {
}

void PEMainWindow::initSketchWidgets()
{
    MainWindow::initSketchWidgets();

	m_iconGraphicsView = new IconSketchWidget(ViewIdentifierClass::IconView, this);
	initSketchWidget(m_iconGraphicsView);
	m_iconWidget = new SketchAreaWidget(m_iconGraphicsView,this);
	m_tabWidget->addWidget(m_iconWidget);
    initSketchWidget(m_iconGraphicsView);

    m_metadataView = new MetadataView(this);
	SketchAreaWidget * sketchAreaWidget = new SketchAreaWidget(m_metadataView, this);
	m_tabWidget->addWidget(sketchAreaWidget);
    connect(m_metadataView, SIGNAL(metadataChanged(const QString &, const QString &)), this, SLOT(metadataChanged(const QString &, const QString &)));
    connect(m_metadataView, SIGNAL(tagsChanged(const QStringList &)), this, SLOT(tagsChanged(const QStringList &)));
    connect(m_metadataView, SIGNAL(propertiesChanged(const QHash<QString, QString> &)), this, SLOT(propertiesChanged(const QHash<QString, QString> &)));

    m_connectorsView = new ConnectorsView(this);
	sketchAreaWidget = new SketchAreaWidget(m_connectorsView, this);
	m_tabWidget->addWidget(sketchAreaWidget);
    connect(m_connectorsView, SIGNAL(connectorsChanged(QList<ConnectorMetadata *> &)), this, SLOT(connectorsChanged(QList<ConnectorMetadata *> &)), Qt::DirectConnection);
}

void PEMainWindow::initDock()
{
}

void PEMainWindow::moreInitDock()
{
}

void PEMainWindow::createActions()
{
    createFileMenuActions();
    createEditMenuActions();
    createViewMenuActions();
    createHelpMenuActions();
}

void PEMainWindow::createMenus()
{
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createHelpMenu();
}

QList<QWidget*> PEMainWindow::getButtonsForView(ViewIdentifierClass::ViewIdentifier) {
	QList<QWidget*> retval;
    return retval;
}

void PEMainWindow::connectPairs() {
}

QMenu *PEMainWindow::breadboardWireMenu() {
    return NULL;
}
    
QMenu *PEMainWindow::breadboardItemMenu() {
    return NULL;
}

QMenu *PEMainWindow::schematicWireMenu() {
    return NULL;
}
    
QMenu *PEMainWindow::schematicItemMenu() {
    return NULL;
}

QMenu *PEMainWindow::pcbWireMenu() {
    return NULL;
}
    
QMenu *PEMainWindow::pcbItemMenu() {
    return NULL;
}

void PEMainWindow::setInitialItem(PaletteItem * paletteItem) {
    ModelPart * originalModelPart = NULL;
    if (paletteItem == NULL) {
        originalModelPart = m_paletteModel->retrieveModelPart("generic_ic_dip_8_300mil");
    }
    else {
        originalModelPart = paletteItem->modelPart();
    }

    long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));

    QFile file(originalModelPart->path());
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = m_fzpDocument.setContent(&file, &errorStr, &errorLine, &errorColumn);
	if (!result) {
        QMessageBox::critical(NULL, tr("Parts Editor"), QString("Unable to load fzp from %1").arg(originalModelPart->path()));
		return;
	}

    QDomElement fzpRoot = m_fzpDocument.documentElement();
    QDomElement views = fzpRoot.firstChildElement("views");
    QDomElement author = fzpRoot.firstChildElement("author");
    if (author.isNull()) {
        author = m_fzpDocument.createElement("author");
        fzpRoot.appendChild(author);
    }
    TextUtils::replaceChildText(m_fzpDocument, author, QString(getenvUser()));
    QDomElement date = fzpRoot.firstChildElement("date");
    if (date.isNull()) {
        date = m_fzpDocument.createElement("date");
        fzpRoot.appendChild(date);
    }
    TextUtils::replaceChildText(m_fzpDocument, date, QDate::currentDate().toString());



	QString userPartsFolderPath = FolderUtils::getUserDataStorePath("parts")+"/user/";
	QString userPartsFolderSvgPath = FolderUtils::getUserDataStorePath("parts")+"/svg/user/";
    QString guid = FolderUtils::getRandText();
    fzpRoot.setAttribute("moduleId", guid);
    QString family = originalModelPart->family();
    if (family.isEmpty()) {
        family = guid;
    }
    else if (!family.startsWith("custom_")) {
        family = "custom_" + family;
    }
    QDomElement properties = fzpRoot.firstChildElement("properties");
    QDomElement prop = properties.firstChildElement("property");
    bool gotProp = false;
    while (!prop.isNull()) {
        QString name = prop.attribute("name");
        if (name.compare("hole size", Qt::CaseInsensitive) == 0) {
            gotProp = true;
            TextUtils::replaceChildText(m_fzpDocument, prop, family);
            break;
        }

        prop = prop.nextSiblingElement("property");
    }
    if (!gotProp) {
        QDomElement prop = m_fzpDocument.createElement("property");
        properties.appendChild(prop);
        prop.setAttribute("name", "family");
        TextUtils::replaceChildText(m_fzpDocument, prop, family);
    }

    QHash<SketchWidget *, QString> sketchWidgets;
    sketchWidgets.insert(m_breadboardGraphicsView, "breadboard");
    sketchWidgets.insert(m_schematicGraphicsView, "schematic"),
    sketchWidgets.insert(m_pcbGraphicsView, "pcb");
    sketchWidgets.insert(m_iconGraphicsView, "icon");
    foreach (SketchWidget * sketchWidget, sketchWidgets.keys()) {
        ItemBase * itemBase = originalModelPart->viewItem(sketchWidget->viewIdentifier());
        if (itemBase == NULL) continue;
        if (!itemBase->hasCustomSVG()) continue;

        QHash<QString, QString> svgHash;
        QString svg = "";
		foreach (ViewLayer * vl, sketchWidget->viewLayers().values()) {
			svg += itemBase->retrieveSvg(vl->viewLayerID(), svgHash, false, GraphicsUtils::StandardFritzingDPI);
		}

        if (svg.isEmpty()) {
            DebugDialog::debug(QString("pe: missing custom svg %1").arg(originalModelPart->moduleID()));
            continue;
        }

        QString prefix = sketchWidgets.value(sketchWidget);
        QString prefixPath = prefix + "/" + guid + ".svg";
		QSizeF size = itemBase->size();
		svg = TextUtils::makeSVGHeader(GraphicsUtils::SVGDPI, GraphicsUtils::StandardFritzingDPI, size.width(), size.height()) + svg + "</svg>";
        QString svgPath = userPartsFolderSvgPath + prefixPath;
        result = TextUtils::writeUtf8(svgPath, svg);
        if (!result) {
            QMessageBox::critical(NULL, tr("Parts Editor"), QString("Unable to write svg to  %1").arg(svgPath));
		    return;
        }

        QDomElement view = views.firstChildElement(prefix + "View");
        QDomElement layers = view.firstChildElement("layers");
        if (layers.isNull()) {
            QMessageBox::critical(NULL, tr("Parts Editor"), QString("Unable to parse fzp file  %1").arg(originalModelPart->path()));
		    return;
        }

        layers.setAttribute("image", prefixPath);
    }

    QString fzpPath = userPartsFolderPath + guid + ".fzp";
    TextUtils::writeUtf8(fzpPath, m_fzpDocument.toString());

    ModelPart * modelPart = new ModelPart(&m_fzpDocument, fzpPath, ModelPart::Part);

    m_iconGraphicsView->addItem(modelPart, m_iconGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_breadboardGraphicsView->addItem(modelPart, m_breadboardGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_schematicGraphicsView->addItem(modelPart, m_schematicGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_pcbGraphicsView->addItem(modelPart, m_pcbGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_metadataView->initMetadata(m_fzpDocument);
    m_connectorsView->initConnectors(m_fzpDocument);


    QTimer::singleShot(10, this, SLOT(initZoom()));
}

bool PEMainWindow::eventFilter(QObject *object, QEvent *event) 
{
	return QMainWindow::eventFilter(object, event);
}

void PEMainWindow::initHelper()
{
}

void PEMainWindow::initZoom() {
    m_breadboardGraphicsView->fitInWindow();
    m_schematicGraphicsView->fitInWindow();
    m_pcbGraphicsView->fitInWindow();
    m_iconGraphicsView->fitInWindow();
}

void PEMainWindow::setTitle() {
    setWindowTitle(tr("New Parts Editor"));
}

void PEMainWindow::createViewMenuActions() {
    MainWindow::createViewMenuActions();

	m_showIconAct = new QAction(tr("Show Icon"), this);
	m_showIconAct->setShortcut(tr("Ctrl+4"));
	m_showIconAct->setStatusTip(tr("Show the icon view"));
	connect(m_showIconAct, SIGNAL(triggered()), this, SLOT(showIconView()));

	m_showMetadataViewAct = new QAction(tr("Show Metatdata"), this);
	m_showMetadataViewAct->setShortcut(tr("Ctrl+5"));
	m_showMetadataViewAct->setStatusTip(tr("Show the metadata view"));
	connect(m_showMetadataViewAct, SIGNAL(triggered()), this, SLOT(showMetadataView()));

    m_showConnectorsViewAct = new QAction(tr("Show Connectors"), this);
	m_showConnectorsViewAct->setShortcut(tr("Ctrl+6"));
	m_showConnectorsViewAct->setStatusTip(tr("Show the connector metatdata in a list view"));
	connect(m_showConnectorsViewAct, SIGNAL(triggered()), this, SLOT(showConnectorsView()));

}

void PEMainWindow::createViewMenu() {
    MainWindow::createViewMenu();

    bool afterNext = false;
    foreach (QAction * action, m_viewMenu->actions()) {
        if (action == m_showPCBAct) {
            afterNext = true;
        }
        else if (afterNext) {
            m_viewMenu->insertAction(action, m_showIconAct);
            m_viewMenu->insertAction(action, m_showMetadataViewAct);
            m_viewMenu->insertAction(action, m_showConnectorsViewAct);
            break;
        }
    }
}

void PEMainWindow::showMetadataView() {
    this->m_tabWidget->setCurrentIndex(4);
}

void PEMainWindow::showConnectorsView() {
    this->m_tabWidget->setCurrentIndex(5);
}

void PEMainWindow::showIconView() {
    this->m_tabWidget->setCurrentIndex(3);
}

void PEMainWindow::metadataChanged(const QString & name, const QString & value)
{
    if (name.compare("family") == 0) {
        QHash<QString, QString> oldProperties = getOldProperties();
        QHash<QString, QString> newProperties(oldProperties);
        newProperties.insert("family", value);
    
        ChangePropertiesCommand * cpc = new ChangePropertiesCommand(this, oldProperties, newProperties, NULL);
        cpc->setText(tr("Change family to %1").arg(value));
        cpc->setSkipFirstRedo();
        changeProperties(newProperties, false);
        m_undoStack->waitPush(cpc, SketchWidget::PropChangeDelay);

        return;
    }

    QString menuText = (name.compare("description") == 0) ? tr("Change description") : tr("Change %1 to '%2'").arg(name).arg(value);

    // called from metadataView
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement element = root.firstChildElement(name);
    QString oldValue = element.text();
    ChangeMetadataCommand * cmc = new ChangeMetadataCommand(this, name, oldValue, value, NULL);
    cmc->setText(menuText);
    cmc->setSkipFirstRedo();
    changeMetadata(name, value, false);
    m_undoStack->waitPush(cmc, SketchWidget::PropChangeDelay);
}

void PEMainWindow::changeMetadata(const QString & name, const QString & value, bool updateDisplay)
{
    // called from command object
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement element = root.firstChildElement(name);
    QString oldValue = element.text();
    TextUtils::replaceChildText(m_fzpDocument, element, value);

    if (updateDisplay) {
        m_metadataView->initMetadata(m_fzpDocument);
    }
}

void PEMainWindow::tagsChanged(const QStringList & newTags)
{
    // called from metadataView
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement tags = root.firstChildElement("tags");
    QDomElement tag = tags.firstChildElement("tag");
    QStringList oldTags;
    while (!tag.isNull()) {
        oldTags << tag.text();
        tag = tag.nextSiblingElement("tag");
    }

    ChangeTagsCommand * ctc = new ChangeTagsCommand(this, oldTags, newTags, NULL);
    ctc->setText(tr("Change tags"));
    ctc->setSkipFirstRedo();
    changeTags(newTags, false);
    m_undoStack->waitPush(ctc, SketchWidget::PropChangeDelay);
}

void PEMainWindow::changeTags(const QStringList & newTags, bool updateDisplay)
{
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement tags = root.firstChildElement("tags");
    QDomElement tag = tags.firstChildElement("tag");
    while (!tag.isNull()) {
        tags.removeChild(tag);
        tag = tags.firstChildElement("tag");
    }

    foreach (QString newTag, newTags) {
        QDomElement tag = m_fzpDocument.createElement("tag");
        tags.appendChild(tag);
        TextUtils::replaceChildText(m_fzpDocument, tag, newTag);
    }

    if (updateDisplay) {
        m_metadataView->initMetadata(m_fzpDocument);
    }
}

void PEMainWindow::propertiesChanged(const QHash<QString, QString> & newProperties)
{
    // called from metadataView
    QHash<QString, QString> oldProperties = getOldProperties();

    ChangePropertiesCommand * cpc = new ChangePropertiesCommand(this, oldProperties, newProperties, NULL);
    cpc->setText(tr("Change properties"));
    cpc->setSkipFirstRedo();
    changeProperties(newProperties, false);
    m_undoStack->waitPush(cpc, SketchWidget::PropChangeDelay);
}


void PEMainWindow::changeProperties(const QHash<QString, QString> & newProperties, bool updateDisplay)
{
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement properties = root.firstChildElement("properties");
    QDomElement prop = properties.firstChildElement("property");
    while (!prop.isNull()) {
        properties.removeChild(prop);
        prop = properties.firstChildElement("property");
    }

    foreach (QString name, newProperties.keys()) {
        QDomElement prop = m_fzpDocument.createElement("property");
        properties.appendChild(prop);
        prop.setAttribute("name", name);
        TextUtils::replaceChildText(m_fzpDocument, prop, newProperties.value(name));
    }

    if (updateDisplay) {
        m_metadataView->initMetadata(m_fzpDocument);
    }
}

QHash<QString, QString> PEMainWindow::getOldProperties() 
{
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement properties = root.firstChildElement("properties");
    QDomElement prop = properties.firstChildElement("property");
    QHash<QString, QString> oldProperties;
    while (!prop.isNull()) {
        QString name = prop.attribute("name");
        QString value = prop.text();
        oldProperties.insert(name, value);
        prop = prop.nextSiblingElement("property");
    }

    return oldProperties;
}

void PEMainWindow::connectorsChanged(QList<ConnectorMetadata *> & connectorMetadataList)
{
}
