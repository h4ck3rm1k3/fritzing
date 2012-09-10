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

    show in OS button

	first time help?

	disable dragging wires

	change pin count

    connector duplicate op

    swap connector metadata op

    assign connector to selected svg element 

	on first time switching to view, select first connector

    on svg import detect all connector IDs
        if any are invisible, tell user this is obsolete
    
    move connectors with arrow keys, or typed coordinates
	drag and drop later

    load new file
        load image button can be replaced by file menu
	
    import
        eagle lbr
        eagle brd
        kicad footprint and mod?
        gEDA footprint

    for schematic view 
        offer pins, rects, and a selection of standard schematic icons in the parts bin
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
        pcb view missing layers

    holes

    smd vs. tht

    hybrids

    buses 
        connect bus by drawing a wire
        can this be modal? i.e. turn bus mode on and off

    bendable legs

    flip and rotate?

    set flippable

    zoom slider is not correctly synchronized with actual zoom level

    terminal points
        move only with arrow keys (maybe cntl-arrow if arrow is used for connector)
        give presets: center, N, E, S, W, NE, NW, SE, SW

    undo/redo as xml file: use index + guid for uniqueness

    nudge via arrow keys

    taxonomy entry like tag entry?

    new schematic layout specs

    deal with customized svgs
        chip label
        * pin label
        * resistance
        * led color
        pin header stuff
        pin size

    disable family entry?  should always be "custom_"  + original family (unless it already starts with custom_)

    lock svg element, if unlocked then click to change.  If a connector is found, lock to start with

    matrix problem with move and duplicate (i.e. if element inherits a matrix from far above)
        even a problem when inserting hole, pad, or pin
        eliminate internal transforms, then insert inverted matrix, then use untransformed coords based on viewbox
            
    delete all unused svg and fzp files when finished

    only allow appropriate file to be loaded for appropriate view (.mod, .fp, etc.)

    kicad schematic does not match our schematic

    move contrib pcb files to obsolete

    always show terminal point when highlighting

***************************************************/

#include "pemainwindow.h"
#include "metadataview.h"
#include "connectorsview.h"
#include "pecommands.h"
#include "petoolview.h"
#include "pegraphicsitem.h"
#include "kicadmoduledialog.h"
#include "../debugdialog.h"
#include "../model/palettemodel.h"
#include "../sketch/breadboardsketchwidget.h"
#include "../sketch/schematicsketchwidget.h"
#include "../sketch/pcbsketchwidget.h"
#include "../mainwindow/sketchareawidget.h"
#include "../referencemodel/referencemodel.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../utils/folderutils.h"
#include "../mainwindow/fdockwidget.h"
#include "../fsvgrenderer.h"
#include "../partsbinpalette/binmanager/binmanager.h"
#include "../svg/gedaelement2svg.h"
#include "../svg/kicadmodule2svg.h"
#include "../svg/kicadschematic2svg.h"

#include <QCoreApplication>
#include <QSvgGenerator>
#include <QMenuBar>

////////////////////////////////////////////////////

bool GotZeroConnector = false;

bool byID(QDomElement & c1, QDomElement & c2)
{
    int c1id = -1;
    int c2id = -1;
	int ix = IntegerFinder.indexIn(c1.attribute("id"));
    if (ix > 0) c1id = IntegerFinder.cap(0).toInt();
    ix = IntegerFinder.indexIn(c2.attribute("id"));
    if (ix > 0) c2id = IntegerFinder.cap(0).toInt();

    if (c1id == 0 || c2id == 0) GotZeroConnector = true;
	
	return c1id <= c2id;
}

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
    m_guid = FolderUtils::getRandText();
    m_fileIndex = 0;
	m_userPartsFolderPath = FolderUtils::getUserDataStorePath("parts")+"/user/";
	m_userPartsFolderSvgPath = FolderUtils::getUserDataStorePath("parts")+"/svg/user/";

}

PEMainWindow::~PEMainWindow()
{
    // PEGraphicsItems are still holding QDomElement so delete them before m_fzpDocument is deleted
    QList<SketchWidget *> sketchWidgets;
    sketchWidgets << m_breadboardGraphicsView << m_schematicGraphicsView << m_pcbGraphicsView;
    foreach (SketchWidget * sketchWidget, sketchWidgets) {
        foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
            PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
            if (pegi) delete pegi;
        }
    }   
}

void PEMainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
}

void PEMainWindow::initLockedFiles(bool) {
}

void PEMainWindow::initSketchWidgets()
{
    MainWindow::initSketchWidgets();

    m_docs.insert(m_breadboardGraphicsView->viewIdentifier(), &m_breadboardDocument);
    m_docs.insert(m_schematicGraphicsView->viewIdentifier(), &m_schematicDocument);
    m_docs.insert(m_pcbGraphicsView->viewIdentifier(), &m_pcbDocument);

    m_breadboardGraphicsView->setAcceptWheelEvents(false);
    m_schematicGraphicsView->setAcceptWheelEvents(false);
    m_pcbGraphicsView->setAcceptWheelEvents(false);

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
    connect(m_connectorsView, SIGNAL(connectorMetadataChanged(const ConnectorMetadata *)), this, SLOT(connectorMetadataChanged(const ConnectorMetadata *)), Qt::DirectConnection);
}

void PEMainWindow::initDock()
{
    m_binManager = new BinManager(m_refModel, NULL, m_undoStack, this);
    m_binManager->openBin(":/resources/bins/pe.fzb");
}

void PEMainWindow::moreInitDock()
{
    static int MinHeight = 75;
    static int DefaultHeight = 100;

    m_peToolView = new PEToolView();
    connect(m_peToolView, SIGNAL(switchedConnector(const QDomElement &)), this, SLOT(switchedConnector(const QDomElement &)));
    connect(m_peToolView, SIGNAL(lockChanged(bool)), this, SLOT(lockChanged(bool)));
    makeDock(tr("Tools"), m_peToolView, DockMinWidth, DockMinHeight);
    m_peToolView->setMinimumSize(DockMinWidth, DockMinHeight);

	makeDock(BinManager::Title, m_binManager, MinHeight, DefaultHeight);
}

void PEMainWindow::createActions()
{
    createFileMenuActions();
	m_openAct->setStatusTip(tr("Open a file to use as a part image."));
	disconnect(m_openAct, SIGNAL(triggered()), this, SLOT(mainLoad()));
	connect(m_openAct, SIGNAL(triggered()), this, SLOT(loadImage()));
  
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
    bool succeeded = true;
	succeeded =  succeeded && connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_breadboardGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	succeeded =  succeeded && connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_schematicGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	succeeded =  succeeded && connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_pcbGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));

	succeeded =  succeeded && connect(m_pcbGraphicsView, SIGNAL(cursorLocationSignal(double, double)), this, SLOT(cursorLocationSlot(double, double)));
	succeeded =  succeeded && connect(m_breadboardGraphicsView, SIGNAL(cursorLocationSignal(double, double)), this, SLOT(cursorLocationSlot(double, double)));
	succeeded =  succeeded && connect(m_schematicGraphicsView, SIGNAL(cursorLocationSignal(double, double)), this, SLOT(cursorLocationSlot(double, double)));
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

    m_isCore = originalModelPart->isCore();

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

    fzpRoot.setAttribute("moduleId", m_guid);
    QString family = originalModelPart->family();
    if (family.isEmpty()) {
        family = m_guid;
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

    QList<SketchWidget *> sketchWidgets;
    sketchWidgets << m_breadboardGraphicsView << m_schematicGraphicsView << m_pcbGraphicsView << m_iconGraphicsView;
    foreach (SketchWidget * sketchWidget, sketchWidgets) {
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

		QSizeF size = itemBase->size();
		svg = TextUtils::makeSVGHeader(GraphicsUtils::SVGDPI, GraphicsUtils::StandardFritzingDPI, size.width(), size.height()) + svg + "</svg>";
        QString svgPath = makeSvgPath(sketchWidget);
        result = TextUtils::writeUtf8(m_userPartsFolderSvgPath + svgPath, svg);
        if (!result) {
            QMessageBox::critical(NULL, tr("Parts Editor"), QString("Unable to write svg to  %1").arg(svgPath));
		    return;
        }

        QDomElement view = views.firstChildElement(ViewIdentifierClass::viewIdentifierXmlName(sketchWidget->viewIdentifier()));
        QDomElement layers = view.firstChildElement("layers");
        if (layers.isNull()) {
            QMessageBox::critical(NULL, tr("Parts Editor"), QString("Unable to parse fzp file  %1").arg(originalModelPart->path()));
		    return;
        }

        layers.setAttribute("image", svgPath);
    }

    reload();

    switchedConnector(m_peToolView->currentConnector());
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

void PEMainWindow::connectorMetadataChanged(const ConnectorMetadata * cmd)
{
    ConnectorMetadata oldcmd;

    QDomElement connector = findConnector(cmd->connectorID);
    if (connector.isNull()) return;

    oldcmd.connectorID = connector.attribute("id");
    oldcmd.connectorType = Connector::Male;
    if (connector.attribute("type").compare("female", Qt::CaseInsensitive) == 0) oldcmd.connectorType = Connector::Female;
    else if (connector.attribute("type").compare("pad", Qt::CaseInsensitive) == 0) oldcmd.connectorType = Connector::Pad;
    oldcmd.connectorName = connector.attribute("name");
    QDomElement description = connector.firstChildElement("description");
    oldcmd.connectorDescription = description.text();

    ChangeConnectorMetadataCommand * ccmc = new ChangeConnectorMetadataCommand(this, oldcmd, *cmd, NULL);
    ccmc->setText(tr("Change connector %1").arg(cmd->connectorName));
    ccmc->setSkipFirstRedo();
    changeConnectorElement(connector, *cmd);
    m_undoStack->waitPush(ccmc, SketchWidget::PropChangeDelay);
}

QDomElement PEMainWindow::findConnector(const QString & id) 
{
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement connectors = root.firstChildElement("connectors");
    QDomElement connector = connectors.firstChildElement("connector");
    while (!connector.isNull()) {
        if (id.compare(connector.attribute("id")) == 0) {
            return connector;
        }
        connector = connector.nextSiblingElement("connector");
    }

    return QDomElement();
}


void PEMainWindow::changeConnectorMetadata(const ConnectorMetadata & cmd, bool updateDisplay) {
    QDomElement connector = findConnector(cmd.connectorID);
    if (connector.isNull()) return;

    changeConnectorElement(connector, cmd);
    if (updateDisplay) {
        initConnectors();
    }
}

void PEMainWindow::changeConnectorElement(QDomElement & connector, const ConnectorMetadata & cmd)
{
    connector.setAttribute("name", cmd.connectorName);
    QString type = "male";
    if (cmd.connectorType == Connector::Female) type = "female";
    else if (cmd.connectorType == Connector::Pad) type = "pad";
    connector.setAttribute("type", type);
    QDomElement description = connector.firstChildElement("description");
    TextUtils::replaceElementChildText(m_fzpDocument, connector, "description", cmd.connectorDescription);
}

void PEMainWindow::initSvgTree(ItemBase * itemBase, QDomDocument & domDocument) 
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    QFile file(itemBase->filename());
    if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("unable to parse svg: %1 %2 %3").arg(errorStr).arg(errorLine).arg(errorColumn));
        return;
	}

    QDomElement root = domDocument.documentElement();
    TextUtils::elevateTransform(root);
    TextUtils::gornTree(domDocument);   //

    FSvgRenderer renderer;
    renderer.loadSvg(domDocument.toByteArray(), "", false);

	QSizeF defaultSizeF = renderer.defaultSizeF();
	QRectF viewBox = renderer.viewBoxF();

    double z = 5000;
    QList<QDomElement> traverse;
    traverse << domDocument.documentElement();
    while (traverse.count() > 0) {
        QList<QDomElement> next;
        foreach (QDomElement element, traverse) {
            QString tagName = element.tagName();
            if      (tagName.compare("rect") == 0);
            else if (tagName.compare("g") == 0);
            else if (tagName.compare("circle") == 0);
            else if (tagName.compare("ellipse") == 0);
            else if (tagName.compare("path") == 0);
            else if (tagName.compare("svg") == 0);
            else if (tagName.compare("line") == 0);
            else if (tagName.compare("polyline") == 0);
            else if (tagName.compare("polygon") == 0);
            else if (tagName.compare("use") == 0);
            else if (tagName.compare("text") == 0);
            else continue;

            QString id = element.attribute("id");
            QRectF r = renderer.boundsOnElement(id);
            QMatrix matrix = renderer.matrixForElement(id);
            QString oldid = element.attribute("oldid");
            if (!oldid.isEmpty()) {
                element.setAttribute("id", oldid);
                element.removeAttribute("oldid");
            }
            QRectF bounds = matrix.mapRect(r);
	        bounds.setRect(bounds.x() * defaultSizeF.width() / viewBox.width(), 
							   bounds.y() * defaultSizeF.height() / viewBox.height(), 
							   bounds.width() * defaultSizeF.width() / viewBox.width(), 
							   bounds.height() * defaultSizeF.height() / viewBox.height());

            // known Qt bug: boundsOnElement returns zero width and height for text elements.
            if (bounds.width() > 0 && bounds.height() > 0) {
                PEGraphicsItem * pegItem = new PEGraphicsItem(0, 0, bounds.width(), bounds.height());
                pegItem->setPos(itemBase->pos() + bounds.topLeft());
                pegItem->setZValue(z);
                itemBase->scene()->addItem(pegItem);
                pegItem->setElement(element);
                pegItem->setOffset(bounds.topLeft());
                connect(pegItem, SIGNAL(highlightSignal(PEGraphicsItem *)), this, SLOT(highlightSlot(PEGraphicsItem *)));
                connect(pegItem, SIGNAL(mouseReleased(PEGraphicsItem *)), this, SLOT(pegiMouseReleased(PEGraphicsItem *)));

                /* 
                QString string;
                QTextStream stream(&string);
                element.save(stream, 0);
                DebugDialog::debug("........");
                DebugDialog::debug(string);
                */
            }

            QDomElement child = element.firstChildElement();
            while (!child.isNull()) {
                next.append(child);
                child = child.nextSiblingElement();
            }
        }
        z++;
        traverse.clear();
        foreach (QDomElement element, next) traverse.append(element);
        next.clear();
    }
}

void PEMainWindow::highlightSlot(PEGraphicsItem * pegi) {
    if (m_peToolView) {
        m_peToolView->highlightElement(pegi);
    }
}

void PEMainWindow::initConnectors() {
    QDomElement root = m_fzpDocument.documentElement();
    QDomElement connectors = root.firstChildElement("connectors");
    QDomElement connector = connectors.firstChildElement("connector");
    QList<QDomElement> connectorList;
    while (!connector.isNull()) {
        connectorList.append(connector);
        connector = connector.nextSiblingElement("connector");
    }

    qSort(connectorList.begin(), connectorList.end(), byID);

    m_connectorsView->initConnectors(connectorList, GotZeroConnector);
    m_peToolView->initConnectors(connectorList, GotZeroConnector);
}

void PEMainWindow::switchedConnector(const QDomElement & element)
{
    if (m_currentGraphicsView == NULL) return;

    QString id, terminalID;
    if (!getConnectorIDs(element, m_currentGraphicsView, id, terminalID)) return;

    QList<PEGraphicsItem *> pegiList;
    foreach (QGraphicsItem * item, m_currentGraphicsView->scene()->items()) {
        PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
        if (pegi) pegiList.append(pegi);
    }

    bool gotTerminal = false;
    QPointF terminalPoint;
    if (!terminalID.isEmpty()) {
        foreach (PEGraphicsItem * pegi, pegiList) {
            QDomElement pegiElement = pegi->element();
            if (pegiElement.attribute("id").compare(terminalID) == 0) {
                terminalPoint = pegi->rect().center();
                gotTerminal = true;
                break;
            }
        }
    }

    bool gotOne = false;
    foreach (PEGraphicsItem * pegi, pegiList) {
        QDomElement pegiElement = pegi->element();
        if (pegiElement.attribute("id").compare(id) == 0) {
            if (gotTerminal) {
                pegi->setTerminalPoint(terminalPoint);
            }
            else {
                pegi->setTerminalPoint(pegi->rect().center());
            }
            gotOne = true;
            pegi->showTerminalPoint(true);
            pegi->setHighlighted(true);
            break;
        }
    }
    m_peToolView->setLock(gotOne);
    foreach (PEGraphicsItem * pegi, pegiList) {
        pegi->setAcceptedMouseButtons(gotOne ? Qt::NoButton : Qt::LeftButton);
    }
}

void PEMainWindow::loadImage() 
{
    if (m_currentGraphicsView == NULL) return;

	QStringList extras;
	extras.append("");
	extras.append("");
	QString imageFiles;
	if (m_currentGraphicsView->viewIdentifier() == ViewIdentifierClass::PCBView) {
		imageFiles = tr("Image & Footprint Files (%1 %2 %3 %4 %5);;SVG Files (%1);;JPEG Files (%2);;PNG Files (%3);;gEDA Footprint Files (%4);;Kicad Module Files (%5)");   // 
		extras[0] = "*.fp";
		extras[1] = "*.mod";
	}
	else {
		imageFiles = tr("Image Files (%1 %2 %3);;SVG Files (%1);;JPEG Files (%2);;PNG Files (%3)%4%5");
	}

	if (m_currentGraphicsView->viewIdentifier() == ViewIdentifierClass::SchematicView) {
		extras[0] = "*.lib";
		imageFiles = tr("Image & Footprint Files (%1 %2 %3 %4);;SVG Files (%1);;JPEG Files (%2);;PNG Files (%3);;Kicad Schematic Files (%4)%5");   // 
	}

    QString initialPath = FolderUtils::openSaveFolder();
    ItemBase * itemBase = m_items.value(m_currentGraphicsView->viewIdentifier(), NULL);
    if (itemBase) {
        initialPath = itemBase->filename();
    }
	QString origPath = FolderUtils::getOpenFileName(this,
		tr("Open Image"),
		initialPath,
		imageFiles.arg("*.svg").arg("*.jpg *.jpeg").arg("*.png").arg(extras[0]).arg(extras[1])
	);

	if (origPath.isEmpty()) {
		return; // Cancel pressed
	} 

    QString newPath = origPath;
	if (!newPath.endsWith(".svg")) {
		try {
			newPath = createSvgFromImage(newPath);
		}
		catch (const QString & msg) {
    		QMessageBox::warning(
    			NULL,
    			tr("Conversion problem"),
    			tr("Unable to load image file: \n%1").arg(msg)
    		);
			return;
		}
	}

	if (!newPath.isEmpty()) {
		ChangeSvgCommand * csc = new ChangeSvgCommand(this, m_currentGraphicsView, itemBase->filename(), newPath, NULL);
        QFileInfo info(origPath);
        csc->setText(QString("Load '%1'").arg(info.fileName()));
        m_undoStack->waitPush(csc, SketchWidget::PropChangeDelay);
	}
}

QString PEMainWindow::createSvgFromImage(const QString &origFilePath) {

	QString newFilePath = m_userPartsFolderSvgPath + makeSvgPath(m_currentGraphicsView);
	if (origFilePath.endsWith(".fp")) {
		// this is a geda footprint file
		GedaElement2Svg geda;
		QString svg = geda.convert(origFilePath, false);
		return saveSvg(svg, newFilePath);
	}

	if (origFilePath.endsWith(".lib")) {
		// Kicad schematic library file
		QStringList defs = KicadSchematic2Svg::listDefs(origFilePath);
		if (defs.count() == 0) {
			throw tr("no schematics found in %1").arg(origFilePath);
		}

		QString def;
		if (defs.count() > 1) {
			KicadModuleDialog kmd(tr("schematic part"), origFilePath, defs, this);
			int result = kmd.exec();
			if (result != QDialog::Accepted) {
				return "";
			}

			def = kmd.selectedModule();
		}
		else {
			def = defs.at(0);
		}

		KicadSchematic2Svg kicad;
		QString svg = kicad.convert(origFilePath, def);
		return saveSvg(svg, newFilePath);
	}

	if (origFilePath.endsWith(".mod")) {
		// Kicad footprint (Module) library file
		QStringList modules = KicadModule2Svg::listModules(origFilePath);
		if (modules.count() == 0) {
			throw tr("no footprints found in %1").arg(origFilePath);
		}

		QString module;
		if (modules.count() > 1) {
			KicadModuleDialog kmd("footprint", origFilePath, modules, this);
			int result = kmd.exec();
			if (result != QDialog::Accepted) {
				return "";
			}

			module = kmd.selectedModule();
		}
		else {
			module = modules.at(0);
		}

		KicadModule2Svg kicad;
		QString svg = kicad.convert(origFilePath, module, false);
		return saveSvg(svg, newFilePath);
	}

	// deal with png, jpg, etc.:


/* %1=witdh in mm
 * %2=height in mm
 * %3=width in local coords
 * %4=height in local coords
 * %5=binary data
 */
/*	QString svgTemplate =
"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
"	<svg width='%1mm' height='%2mm' viewBox='0 0 %3 %4' xmlns='http://www.w3.org/2000/svg'\n"
"		xmlns:xlink='http://www.w3.org/1999/xlink' version='1.2' baseProfile='tiny'>\n"
"		<g fill='none' stroke='black' vector-effect='non-scaling-stroke' stroke-width='1'\n"
"			fill-rule='evenodd' stroke-linecap='square' stroke-linejoin='bevel' >\n"
"			<image x='0' y='0' width='%3' height='%4'\n"
"				xlink:href='data:image/png;base64,%5' />\n"
"		</g>\n"
"	</svg>";

	QPixmap pixmap(origFilePath);
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	pixmap.save(&buffer,"png"); // writes pixmap into bytes in PNG format

	QString svgDom = svgTemplate
		.arg(pixmap.widthMM()).arg(pixmap.heightMM())
		.arg(pixmap.width()).arg(pixmap.height())
		.arg(QString("data:image/png;base64,%2").arg(QString(bytes.toBase64())));

	QFile destFile(newFilePath);
	if(!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::information(NULL, "", "file not created");
		if(!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
				QMessageBox::information(NULL, "", "file not created 2");
			}
	}
	QTextStream out(&destFile);
	out << svgDom;
	destFile.close();
	qDebug() << newFilePath;
	bool existsResult = QFileInfo(newFilePath).exists();
	Q_ASSERT(existsResult);
*/

	QImage img(origFilePath);
	QSvgGenerator svgGenerator;
	svgGenerator.setResolution(90);
	svgGenerator.setFileName(newFilePath);
	QSize sz = img.size();
    svgGenerator.setSize(sz);
	svgGenerator.setViewBox(QRect(0, 0, sz.width(), sz.height()));
	QPainter svgPainter(&svgGenerator);
	svgPainter.drawImage(QPoint(0,0), img);
	svgPainter.end();

	return newFilePath;
}

QString PEMainWindow::makeSvgPath(SketchWidget * sketchWidget)
{
    QString viewName = ViewIdentifierClass::viewIdentifierNaturalName(sketchWidget->viewIdentifier());
    return QString("%1/%2_%3.svg").arg(viewName).arg(m_guid).arg(m_fileIndex++);
}

QString PEMainWindow::saveSvg(const QString & svg, const QString & newFilePath) {
    if (!TextUtils::writeUtf8(newFilePath, svg)) {
        throw tr("unable to open temp file %1").arg(newFilePath);
    }
	return newFilePath;
}

void PEMainWindow::changeSvg(SketchWidget * sketchWidget, const QString & filename) {
    QDomElement fzpRoot = m_fzpDocument.documentElement();
    QDomElement views = fzpRoot.firstChildElement("views");
    QDomElement view = views.firstChildElement(ViewIdentifierClass::viewIdentifierXmlName(sketchWidget->viewIdentifier()));
    QDomElement layers = view.firstChildElement("layers");
    QFileInfo info(filename);
    QDir dir = info.absoluteDir();
    QString shortName = dir.dirName() + "/" + info.fileName();
    layers.setAttribute("image", shortName);

    foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
        PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
        if (pegi) delete pegi;
    }

    foreach (ItemBase * itemBase, m_items.values()) {
        foreach(ItemBase * lk, itemBase->layerKin()) {
            delete lk;
        }
        delete itemBase;
    }
    m_items.clear();

    reload();
}

QString PEMainWindow::saveFzp() {
    QDir dir = QDir::temp();
    dir.mkdir(m_guid);
    dir.cd(m_guid);
    QString fzpPath = dir.absoluteFilePath(QString("%1_%2.fzp").arg(m_guid).arg(m_fileIndex++));   
    DebugDialog::debug("temp path " + fzpPath);
    TextUtils::writeUtf8(fzpPath, m_fzpDocument.toString());
    return fzpPath;
}

void PEMainWindow::reload() {
    QString fzpPath = saveFzp();
    ModelPart * modelPart = new ModelPart(&m_fzpDocument, fzpPath, ModelPart::Part);

    long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));

    ItemBase * iconItem = m_iconGraphicsView->addItem(modelPart, m_iconGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    ItemBase * breadboardItem = m_breadboardGraphicsView->addItem(modelPart, m_breadboardGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    ItemBase * schematicItem = m_schematicGraphicsView->addItem(modelPart, m_schematicGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    ItemBase * pcbItem = m_pcbGraphicsView->addItem(modelPart, m_pcbGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_metadataView->initMetadata(m_fzpDocument);

    initConnectors();

    initSvgTree(breadboardItem, m_breadboardDocument);
    initSvgTree(schematicItem, m_schematicDocument);
    initSvgTree(pcbItem, m_pcbDocument);

    m_items.insert(m_iconGraphicsView->viewIdentifier(), iconItem);
    m_items.insert(m_breadboardGraphicsView->viewIdentifier(), breadboardItem);
    m_items.insert(m_schematicGraphicsView->viewIdentifier(), schematicItem);
    m_items.insert(m_pcbGraphicsView->viewIdentifier(), pcbItem);

    QTimer::singleShot(10, this, SLOT(initZoom()));
}

void PEMainWindow::lockChanged(bool state) {
    foreach (QGraphicsItem * item, m_currentGraphicsView->scene()->items()) {
        PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
        if (pegi) {
            pegi->setAcceptedMouseButtons(state ? Qt::NoButton : Qt::LeftButton);
        }
    }
}

void PEMainWindow::pegiMouseReleased(PEGraphicsItem * pegi)
{
    QString newGorn = pegi->element().attribute("gorn");
    QDomDocument * doc = m_docs.value(m_currentGraphicsView->viewIdentifier());
    QDomElement root = doc->documentElement();
    QDomElement newGornElement = TextUtils::findElementWithAttribute(root, "gorn", newGorn);
    if (newGornElement.isNull()) {
        return;
    }

    QDomElement currentConnectorElement = m_peToolView->currentConnector();
    QString id, terminalID;
    if (!getConnectorIDs(currentConnectorElement, m_currentGraphicsView, id, terminalID)) {
        return;
    }

    QDomElement oldGornElement = TextUtils::findElementWithAttribute(root, "id", id);
    QString oldGorn = oldGornElement.attribute("gorn");
    QString oldGornTerminal;
    if (!terminalID.isEmpty()) {
        QDomElement element = TextUtils::findElementWithAttribute(root, "id", terminalID);
        oldGornTerminal = element.attribute("gorn");
    }

    if (newGornElement.attribute("gorn").compare(oldGornElement.attribute("gorn")) == 0) {
        // no change
        return;
    }

    RelocateConnectorSvgCommand * rcsc = new RelocateConnectorSvgCommand(this, m_currentGraphicsView, id, terminalID, oldGorn, oldGornTerminal, newGorn, "", NULL);
    rcsc->setText(tr("Relocate connector %1").arg(currentConnectorElement.attribute("name")));
    m_undoStack->waitPush(rcsc, SketchWidget::PropChangeDelay);
}

void PEMainWindow::createFileMenu() {
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_revertAct);

    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_closeAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);


    m_fileMenu->addSeparator();
    //m_fileMenu->addAction(m_pageSetupAct);
    m_fileMenu->addAction(m_printAct);

	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_quitAct);

    connect(m_fileMenu, SIGNAL(aboutToShow()), this, SLOT(updateFileMenu()));
}

bool PEMainWindow::getConnectorIDs(const QDomElement & element, SketchWidget * sketchWidget, QString & id, QString & terminalID) {
    QString viewName = ViewIdentifierClass::viewIdentifierXmlName(sketchWidget->viewIdentifier());
    QDomElement views = element.firstChildElement("views");
    QDomElement view = views.firstChildElement(viewName);
    QDomElement p = view.firstChildElement("p");
    if (p.isNull()) return false;

    id = p.attribute("svgId");
    if (id.isEmpty()) return false;

    terminalID = p.attribute("terminalId");
    return true;
}

void PEMainWindow::relocateConnectorSvg(SketchWidget * sketchWidget, const QString & id, const QString & terminalID,
                const QString & oldGorn, const QString & oldGornTerminal, const QString & newGorn, const QString & newGornTerminal)
{
    QDomDocument * doc = m_docs.value(m_currentGraphicsView->viewIdentifier());
    QDomElement root = doc->documentElement();

    QDomElement oldGornElement = TextUtils::findElementWithAttribute(root, "gorn", oldGorn);
    QDomElement oldGornTerminalElement;
    if (!oldGornTerminal.isEmpty()) {
        oldGornTerminalElement = TextUtils::findElementWithAttribute(root, "gorn", oldGornTerminal);
    }
    QDomElement newGornElement = TextUtils::findElementWithAttribute(root, "gorn", newGorn);
    QDomElement newGornTerminalElement;
    if (!newGornTerminal.isEmpty()) {
        newGornTerminalElement = TextUtils::findElementWithAttribute(root, "gorn", newGornTerminal);
    }

    if (oldGornElement.isNull()) return;
    if (newGornElement.isNull()) return;

    oldGornElement.setAttribute("id", "");
    if (!oldGornTerminalElement.isNull()) oldGornTerminalElement.setAttribute("id", "");
    newGornElement.setAttribute("id", id);
    if (!newGornTerminalElement.isNull()) newGornTerminalElement.setAttribute("id", terminalID);

    foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
        PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
        if (pegi == NULL) continue;

        QDomElement element = pegi->element();
        if (element.attribute("gorn").compare(newGorn) == 0) {
            pegi->setHighlighted(true);
            switchedConnector(m_peToolView->currentConnector());
            break;
        }
    }
}

bool PEMainWindow::save() {
    if (m_isCore) {
        saveAs();
    }

    return true;
}

bool PEMainWindow::saveAs() {
    return true;
}



