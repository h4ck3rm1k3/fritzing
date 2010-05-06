/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include <QWebFrame>
#include <QBuffer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QLabel>
#include <QPalette>

#include "htmlinfoview.h"
#include "infoviewwebpage.h"
#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../connectors/connectorshared.h"
#include "../connectors/connector.h"
#include "../fsvgrenderer.h"
#include "../dockmanager.h"
#include "../utils/flineedit.h"


#define HTML_EOF "</body>\n</html>"

QString HtmlInfoView::PropsBlockId = "props_id";
QString HtmlInfoView::TagsBlockId = "tags_id";
QString HtmlInfoView::ConnsBlockId = "conns_id";

QPixmap * NoIcon = NULL;

const int HtmlInfoView::STANDARD_ICON_IMG_WIDTH = 32;
const int HtmlInfoView::STANDARD_ICON_IMG_HEIGHT = 32;
const int IconSpace = 3;

/////////////////////////////////////

QLabel * addLabel(QHBoxLayout * hboxLayout, QPixmap * pixmap) {
	QLabel * label = new QLabel();
	QPalette palette = label->palette();
	palette.setColor(QPalette::Window, QColor(0xc2, 0xc2, 0xc2));
	label->setPalette(palette);
	label->setAutoFillBackground(true);
	label->setPixmap(*pixmap);
	label->setFixedSize(pixmap->size());
	hboxLayout->addWidget(label);
	hboxLayout->addSpacing(IconSpace);

	return label;
}

//////////////////////////////////////

HtmlInfoView::HtmlInfoView(QWidget * parent) : QScrollArea(parent) 
{
        this->setWidgetResizable(true);
        QFrame * mainFrame = new QFrame(this);
	mainFrame->setObjectName("infoViewMainFrame");

	m_lastSwappingEnabled = false;
	m_lastItemBase = NULL;
	m_infoGraphicsView = NULL;
	m_setContentTimer.setSingleShot(true);
	m_setContentTimer.setInterval(10);
	connect(&m_setContentTimer, SIGNAL(timeout()), this, SLOT(setContent()));
	QVBoxLayout *lo = new QVBoxLayout(mainFrame);
	lo->setMargin(0);
	lo->setSpacing(0);
	lo->setSizeConstraint( QLayout::SetMinAndMaxSize );

        /* Part Title */

	m_titleEdit = new FLineEdit(mainFrame);
	m_titleEdit->setObjectName("instanceTitleEditor");

	connect(m_titleEdit, SIGNAL(editingFinished()), this, SLOT(setInstanceTitle()));
	connect(m_titleEdit, SIGNAL(mouseEnter()), this, SLOT(instanceTitleEnter()));
	connect(m_titleEdit, SIGNAL(mouseLeave()), this, SLOT(instanceTitleLeave()));
	connect(m_titleEdit, SIGNAL(editable(bool)), this, SLOT(instanceTitleEditable(bool)));

	setInstanceTitleColors(m_titleEdit, QColor(0xb3, 0xb3, 0xb3), QColor(0x57, 0x57, 0x57));
	m_titleEdit->setAutoFillBackground(true);
	lo->addWidget(m_titleEdit);

        /* Part Icons */

	if (NoIcon == NULL) {
		NoIcon = new QPixmap(":/resources/images/icons/noicon.png");
	}

	QFrame * frame = new QFrame(mainFrame);
	frame->setObjectName("IconFrame");

	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setContentsMargins (0, 0, 0, 0);
	hboxLayout->addSpacing(IconSpace);
	m_icon1 = addLabel(hboxLayout, NoIcon);
	m_icon2 = addLabel(hboxLayout, NoIcon);
	m_icon3 = addLabel(hboxLayout, NoIcon);
	hboxLayout->addSpacerItem(new QSpacerItem(IconSpace, 1, QSizePolicy::Expanding));
	hboxLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );
	frame->setLayout(hboxLayout);
	lo->addWidget(frame);

        /* Part Properties (WebKit) */

	m_webView = new QWebView(mainFrame);
	m_webView->setObjectName("infoViewWebView");
	
	m_infoViewWebPage = new InfoViewWebPage(this, m_webView);
	m_webView->setPage(m_infoViewWebPage);
	lo->addWidget(m_webView);

	mainFrame->setLayout(lo);

	this->setWidget(mainFrame);

	m_webView->setContextMenuPolicy(Qt::PreventContextMenu);
	m_includes = "";

	QFile styleSheet(":/resources/styles/infoview.css");
	if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/infoview.css");
	} else {
		m_includes += QString("<style>\n%1\n</style>\n").arg(QString(styleSheet.readAll()));
	}

	QFile javascript(":/resources/js/infoview.js");
	if (!javascript.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/js/infoview.js");
	} else {
		m_includes += QString("<script language='JavaScript'>\n%1\n</script>\n").arg(QString(javascript.readAll()));
	}

	// TODO Mariano: not working this way
	//m_includes = "\t<link rel'stylesheet' type='text/css' href='/resources/styles/infoview.css' />\n";
	//m_includes+= "\t<script src=':/resources/js/infoview.js' type='text/javascript'></script>\n";

	m_currentItem = NULL;
	m_currentSwappingEnabled = false;

	connect(m_webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(jsRegister()));

	QSettings settings;
	m_blocksVisibility[PropsBlockId] = settings.value(settingsBlockVisibilityName(PropsBlockId),true).toBool();
	m_blocksVisibility[TagsBlockId] = settings.value(settingsBlockVisibilityName(TagsBlockId),true).toBool();
	m_blocksVisibility[ConnsBlockId] = settings.value(settingsBlockVisibilityName(ConnsBlockId),true).toBool();
}

HtmlInfoView::~HtmlInfoView() {
}

void HtmlInfoView::cleanup() {
	if (NoIcon) {
		delete NoIcon;
		NoIcon = NULL;
	}
}

void HtmlInfoView::viewItemInfo(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) 
{
	viewItemInfoAux(infoGraphicsView, item, swappingEnabled);
	m_lastItemBase = item;
	m_lastSwappingEnabled = swappingEnabled;
}

void HtmlInfoView::hoverEnterItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * item, bool swappingEnabled) {
	viewItemInfoAux(infoGraphicsView, item, swappingEnabled);
}

void HtmlInfoView::hoverLeaveItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * itemBase){
	Q_UNUSED(itemBase);
	//DebugDialog::debug(QString("hoverLeaveItem itembase %1").arg(itemBase ? itemBase->instanceTitle() : "NULL"));
	viewItemInfoAux(infoGraphicsView, m_lastItemBase, m_lastSwappingEnabled);

}

void HtmlInfoView::viewConnectorItemInfo(InfoGraphicsView * infoGraphicsView, ConnectorItem * item, bool swappingEnabled) {
	if (item->attachedTo() != m_lastItemBase) return;

	Connector * connector = item->connector();
	if (connector == NULL) return;

	ConnectorShared * connectorShared = connector->connectorShared();
	if (connectorShared == NULL) return;

	QString s = appendStuff(item->attachedTo(), swappingEnabled);
	s += "<div class='block'>";
	s += blockHeader(tr("Connections"),ConnsBlockId);
	s += blockContainer(ConnsBlockId);
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("conn.")).arg(tr("connected to %n item(s)", "", item->connectionsCount()));
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("name")).arg(connectorShared->name());
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("type")).arg(Connector::connectorNameFromType(connector->connectorType()));
	s += 		 "</table></div>\n";
	s += "</div>";

	setCurrentItem(item->attachedTo());
	m_infoGraphicsView = infoGraphicsView;
	setContent(s);
}

void HtmlInfoView::hoverEnterConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem * item, bool swappingEnabled) {
	Q_UNUSED(event)
	viewConnectorItemInfo(igv, item, swappingEnabled);
}

void HtmlInfoView::hoverLeaveConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem *connItem) {
	Q_UNUSED(igv);
	Q_UNUSED(event);
	Q_UNUSED(connItem);
}

void HtmlInfoView::viewItemInfoAux(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) {

	if (item == NULL) {
		setNullContent();
		return;
	}

	m_currentSwappingEnabled = swappingEnabled;

	QString s = appendStuff(item,swappingEnabled);
	setCurrentItem(item);
	m_infoGraphicsView = infoGraphicsView;
	setContent(s);
}

QString HtmlInfoView::appendStuff(ItemBase* item, bool swappingEnabled) {
	Wire *wire = dynamic_cast<Wire*>(item);
	if(wire) {
		return appendWireStuff(wire, wire->id(), swappingEnabled);
	} else {
		return appendItemStuff(item, item->id(), swappingEnabled);
	}
}

QString HtmlInfoView::appendWireStuff(Wire* wire, long id, bool swappingEnabled) {
	if (wire == NULL) return "missing base";

	ModelPart *modelPart = wire->modelPart();
	if (modelPart == NULL) return "missing modelpart";
	if (modelPart->modelPartShared() == NULL) return "missing modelpart stuff";

	QString autoroutable = wire->getAutoroutable() ? tr("(autoroutable)") : "";
	QString nameString = tr("Wire");
	if(wire->getRatsnest()) {
		nameString = tr("Rat's nest wire");
	} else if(wire->getTrace()) {
		nameString = tr("Trace wire %1").arg(autoroutable);
	} else if(wire->getJumper()) {
		nameString = tr("Jumper wire %1").arg(autoroutable);
	}

	setUpTitle(wire);
	setUpIcons(wire->modelPart());

	QString s = "";
	s += partTitle(nameString, modelPart->version());

	s += "<div class='block'>";
	s += blockHeader(tr("Properties"),PropsBlockId);
	s += blockContainer(PropsBlockId);
#ifndef QT_NO_DEBUG
	s += idString(id, modelPart->moduleID());
#else
	Q_UNUSED(id);
#endif
	displayProps(modelPart, wire, swappingEnabled, s);
	addTags(modelPart, s);

	return s;
}

QString HtmlInfoView::appendItemStuff(ItemBase* base, long id, bool swappingEnabled) {
	if (base == NULL) return "missing base";

	return appendItemStuff(base, base->modelPart(), id, swappingEnabled, base->isPartLabelVisible());
}

QString HtmlInfoView::appendItemStuff(ItemBase * itemBase, ModelPart * modelPart, long id, bool swappingEnabled,bool labelIsVisible) {
	Q_UNUSED(labelIsVisible);

	if (modelPart == NULL) return "missing modelpart";
	if (modelPart->modelPartShared() == NULL) return "missing modelpart stuff";

	setUpTitle(itemBase);
	setUpIcons(modelPart);

	QString s = "";
	s += partTitle((itemBase) ? itemBase->title() : modelPart->title(), modelPart->version());

	s += "<div class='block'>";
	s += blockHeader(tr("Properties"),PropsBlockId);
	s += blockContainer(PropsBlockId);
#ifndef QT_NO_DEBUG
	s += idString(id, modelPart->moduleID());
	if (itemBase) {
		PaletteItemBase * paletteItemBase = qobject_cast<PaletteItemBase *>(itemBase);
		if (paletteItemBase != NULL) {
			QString svgpath = paletteItemBase->filename();
			if (!svgpath.isEmpty()) {
				s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("svg").arg(svgpath);
			}
		}
	}
	else {
		FSvgRenderer * renderer = FSvgRenderer::getByModuleID(modelPart->moduleID(), ViewLayer::Icon);
		if (renderer != NULL) {
			QString iconpath = renderer->filename();
			if (!iconpath.isEmpty()) {
				s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("svg").arg(iconpath);
			}
		}
	}
	if (modelPart->modelPartShared()) {
		QString fzppath = modelPart->path();
		if (!fzppath.isEmpty()) {
			s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("fzp").arg(fzppath);
		}
	}

#else
	Q_UNUSED(id)
#endif

	displayProps(modelPart, itemBase, swappingEnabled, s);
	addTags(modelPart, s);

	return s;
}

void HtmlInfoView::setContent(const QString &html) {
	//DebugDialog::debug("html set content");
	m_setContentTimer.stop();
	if (html.compare(m_savedContent) == 0) {
		//DebugDialog::debug("same content bail");
		return;
	}

	m_content = html;
	m_setContentTimer.start();
}

void HtmlInfoView::setContent() {

	//DebugDialog::debug("html info view set content");

	QString fileContent = QString(QString("<html>\n%1<body>\n%2")+HTML_EOF).arg(m_includes).arg(m_content);
	m_webView->setHtml(fileContent);
	m_savedContent = m_content;

	/*QFile file("/tmp/infoview.html");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	         return;
	QTextStream out(&file);
	out << fileContent;
	file.close();*/
}

QSize HtmlInfoView::sizeHint() const {
	return QSize(DockManager::DockDefaultWidth, DockManager::InfoViewDefaultHeight);
}


void HtmlInfoView::setCurrentItem(ItemBase * item) {
	m_currentItem = item;
	m_infoViewWebPage->setCurrentItem(item);
}

void HtmlInfoView::registerAsCurrentItem(ItemBase *item) {
	if(item == NULL) {
		setNullContent();
	}
}

void HtmlInfoView::unregisterCurrentItem() {
	setCurrentItem(NULL);
	registerAsCurrentItem(NULL);
}

void HtmlInfoView::unregisterCurrentItemIf(long id) {
	if (m_currentItem == NULL) {
		return;
	}
	if (m_currentItem->id() == id) {
		unregisterCurrentItem();
	}
}

ItemBase *HtmlInfoView::currentItem() {
	return m_currentItem;
}

void HtmlInfoView::reloadContent(InfoGraphicsView * infoGraphicsView) {
	if(m_currentItem) {
		viewItemInfo(infoGraphicsView, m_currentItem, m_currentSwappingEnabled);
	}
}

QString HtmlInfoView::blockHeader(const QString &title, const QString &blockId) {
	return QString(
			"<table><tr><td class='subhead'>%1</td><td class='subhead' align='right'>"
			"<a class='hideShowControl' href='#' onclick='toggleVisibility(this,\"%2\")'>%3</a></td></tr></table>\n"
		).arg(title).arg(blockId).arg(m_blocksVisibility[blockId] ? "[-]":"[+]");
}

QString HtmlInfoView::blockVisibility(const QString &blockId) {
	if(!m_blocksVisibility[blockId]) {
		return " style='display: none' ";
	}
	return "";
}

QString HtmlInfoView::blockContainer(const QString &blockId) {
	return QString("<div id='%1' %2><table>\n").arg(blockId).arg(blockVisibility(blockId));
}


void HtmlInfoView::setNullContent()
{
	setUpTitle(NULL);
	setUpIcons(NULL);
	setContent("");
}

void HtmlInfoView::setInstanceTitle() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (edit == NULL) return;
	if (!edit->isEnabled()) return;
	if (m_infoGraphicsView == NULL) return;
	if (m_currentItem == NULL) return;

	DebugDialog::debug(QString("set instance title to %1").arg(edit->text()));
	m_infoGraphicsView->setInstanceTitle(m_currentItem->id(), edit->text(), true, false);
}

void HtmlInfoView::instanceTitleEnter() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (edit->isEnabled()) {
		setInstanceTitleColors(edit, QColor(0xc8, 0xc8, 0xc8), QColor(0x57, 0x57, 0x57));
	}
}

void HtmlInfoView::instanceTitleLeave() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (edit->isEnabled()) {
		setInstanceTitleColors(edit, QColor(0xb3, 0xb3, 0xb3), QColor(0x57, 0x57, 0x57));
	}
}

void HtmlInfoView::instanceTitleEditable(bool editable) {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (editable) {
		setInstanceTitleColors(edit, QColor(0xfc, 0xfc, 0xfc), QColor(0x00, 0x00, 0x00));
	}
	else {
		setInstanceTitleColors(edit, QColor(0xb3, 0xb3, 0xb3), QColor(0x57, 0x57, 0x57));
	}
}

void HtmlInfoView::setInstanceTitleColors(FLineEdit * edit, const QColor & base, const QColor & text) {
	edit->setStyleSheet(QString("background: rgb(%1,%2,%3); color: rgb(%4,%5,%6);")
		.arg(base.red()).arg(base.green()).arg(base.blue())
		.arg(text.red()).arg(text.green()).arg(text.blue()) );
}

void HtmlInfoView::jsRegister() {
	m_webView->page()->mainFrame()->addToJavaScriptWindowObject( "infoView", this);
}

void HtmlInfoView::setBlockVisibility(const QString &blockId, bool value) {
	m_blocksVisibility[blockId] = value;
	QSettings settings;
	settings.setValue(settingsBlockVisibilityName(blockId),QVariant::fromValue(value));
}

QString HtmlInfoView::settingsBlockVisibilityName(const QString &blockId) {
	return "infoView/"+blockId+"Visibility";
}

void HtmlInfoView::setUpTitle(ItemBase * itemBase) 
{
	if(itemBase) {
		QString title = itemBase->instanceTitle();
		if (title.isEmpty()) {
			// assumes a part with an empty title only comes from the parts bin palette
			m_titleEdit->setEnabled(false);
			title = itemBase->title();
		}
		else {
			m_titleEdit->setEnabled(true);
		}
		m_titleEdit->setText(title);
	}
	else {
		m_titleEdit->setEnabled(false);
		m_titleEdit->setText("");
	}

}

void HtmlInfoView::setUpIcons(ModelPart * modelPart) {
	QPixmap *pixmap1 = NULL;
	QPixmap *pixmap2 = NULL;
	QPixmap *pixmap3 = NULL;

	QSize size = NoIcon->size();

	if (modelPart != NULL) {
		pixmap1 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
		pixmap2 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Schematic, size);
		pixmap3 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Copper0, size);
		if (pixmap1 == NULL) {
			ItemBase::setUpImage(modelPart, ViewIdentifierClass::IconView, ViewLayer::Icon);
			pixmap1 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
		}
		if (pixmap2 == NULL) {
			ItemBase::setUpImage(modelPart, ViewIdentifierClass::SchematicView, ViewLayer::Schematic);
			pixmap2 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Schematic, size);
		}
		if (pixmap3 == NULL) {
			ItemBase::setUpImage(modelPart, ViewIdentifierClass::PCBView, ViewLayer::Copper0);
			pixmap3 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Copper0, size);
		}
	}

	if (pixmap1 == NULL) pixmap1 = NoIcon;
	if (pixmap2 == NULL) pixmap2 = NoIcon;
	if (pixmap3 == NULL) pixmap3 = NoIcon;

	m_icon1->setPixmap(*pixmap1);
	m_icon2->setPixmap(*pixmap2);
	m_icon3->setPixmap(*pixmap3);

	if (pixmap1 != NoIcon) delete pixmap1;
	if (pixmap2 != NoIcon) delete pixmap2;
	if (pixmap3 != NoIcon) delete pixmap3;
}

void HtmlInfoView::addTags(ModelPart * modelPart, QString & s) {
	if(!modelPart->tags().isEmpty()) {
		s += "<div class='block'>";
		s += blockHeader(tr("Tags"),TagsBlockId);
		s += blockContainer(TagsBlockId);
		s += QString("<tr><td colspan='2'>%1</td></tr>\n").arg(modelPart->tags().join(", "));
		s += 		"</table></div>\n";
		s += "</div>";
	}

}

QString HtmlInfoView::partTitle(const QString & title, const QString & version) {
	QString s = "<div class='parttitle'>\n";
	s += QString("<h2>%1</h2>\n<p>%2</p>\n").arg(title) .arg(version);
	s += "</div>\n";
	return s;
}

void HtmlInfoView::displayProps(ModelPart * modelPart, ItemBase * itemBase, bool swappingEnabled, QString & s) 
{
	QHash<QString,QString> properties = modelPart->properties();
	QString family = properties.value("family", "").toLower();
	QString basis("<tr style='height: 35px;'><td class='label'>%1</td><td>%2</td></tr>\n");
	if (itemBase) {
		itemBase->prepareProps();
	}
	foreach(QString key, properties.keys()) {
		QString value = properties.value(key,"");
		QString translatedName = ItemBase::translatePropertyName(key);
		QString resultKey, resultValue;
		bool result = false;
		if (itemBase != NULL) {
			result = itemBase->collectExtraInfoHtml(family, key, value, swappingEnabled, resultKey, resultValue);
		}
		if (result) {
			s += basis.arg(resultKey).arg(resultValue);
		}
		else {
			s += basis.arg(translatedName).arg(value);
		}
	}
}


QString HtmlInfoView::idString(long id, const QString & moduleID) {
	return QString("<tr><td class='label'>%1</td><td>%2 %3</td></tr>\n").arg("id").arg(id).arg(moduleID);
}
