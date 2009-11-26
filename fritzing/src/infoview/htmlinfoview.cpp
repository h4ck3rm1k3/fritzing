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
#include "../layerattributes.h"
#include "../dockmanager.h"
#include "../utils/flineedit.h"


#define HTML_EOF "</body>\n</html>"

QString HtmlInfoView::PropsBlockId = "props_id";
QString HtmlInfoView::TagsBlockId = "tags_id";
QString HtmlInfoView::ConnsBlockId = "conns_id";

const int HtmlInfoView::STANDARD_ICON_IMG_WIDTH = 32;
const int HtmlInfoView::STANDARD_ICON_IMG_HEIGHT = 32;
const int IconSpace = 3;

//////////////////////////////////////

HtmlInfoView::HtmlInfoView(QWidget * parent) : QFrame(parent) 
{
	m_lastInfoGraphicsView = NULL;
	m_lastModelPart = NULL;
	m_lastItemBase = NULL;
	m_infoGraphicsView = NULL;
	m_setContentTimer.setSingleShot(true);
	m_setContentTimer.setInterval(10);
	connect(&m_setContentTimer, SIGNAL(timeout()), this, SLOT(setContent()));
	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->setMargin(0);
	lo->setSpacing(0);
	m_webView = new QWebView(this);
	m_infoViewWebPage = new InfoViewWebPage(this, m_webView);
	m_webView->setPage(m_infoViewWebPage);
	lo->addWidget(m_webView);

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

	m_maxPropCount = 0;

	QSettings settings;
	m_blocksVisibility[PropsBlockId] = settings.value(settingsBlockVisibilityName(PropsBlockId),true).toBool();
	m_blocksVisibility[TagsBlockId] = settings.value(settingsBlockVisibilityName(TagsBlockId),true).toBool();
	m_blocksVisibility[ConnsBlockId] = settings.value(settingsBlockVisibilityName(ConnsBlockId),true).toBool();
}

HtmlInfoView::~HtmlInfoView() {
}

void HtmlInfoView::hoverEnterItem(InfoGraphicsView * igv, ModelPart * modelPart, bool swappingEnabled) {
	//viewModelPartInfoAux(igv, modelPart, swappingEnabled);
}

void HtmlInfoView::viewModelPartInfo(InfoGraphicsView * igv, ModelPart * modelPart, bool swappingEnabled) {
	viewModelPartInfoAux(igv, modelPart, swappingEnabled);
	m_lastModelPart = modelPart;
}

void HtmlInfoView::viewItemInfo(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) 
{
	viewItemInfoAux(infoGraphicsView, item, swappingEnabled);
	m_lastItemBase = item;
	m_lastInfoGraphicsView = infoGraphicsView;
}

void HtmlInfoView::hoverEnterItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * item, bool swappingEnabled) {
	//viewItemInfoAux(infoGraphicsView, item, swappingEnabled);
}

void HtmlInfoView::hoverLeaveItem(InfoGraphicsView * infoGraphicsView, ModelPart *) {
	if (m_lastModelPart) {
		//viewModelPartInfoAux(infoGraphicsView, m_lastModelPart, false);
	}
	else {
		//viewItemInfoAux(m_lastInfoGraphicsView, m_lastItemBase, true);
	}
}

void HtmlInfoView::hoverLeaveItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * ){
	//viewItemInfoAux(infoGraphicsView, m_lastItemBase, true);
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

void HtmlInfoView::viewModelPartInfoAux(InfoGraphicsView * igv, ModelPart * modelPart, bool swappingEnabled) {
	if (modelPart == NULL) {
		if (m_lastItemBase) {
			viewItemInfoAux(m_lastInfoGraphicsView, m_lastItemBase, true);
		}
		else {
			setNullContent();
		}

		return;
	}

	m_currentSwappingEnabled = swappingEnabled;
	QString s = "";
	s += appendItemStuff(NULL, modelPart, 0, swappingEnabled, "", false);
	m_infoGraphicsView = igv;
	setContent(s);
}

void HtmlInfoView::viewItemInfoAux(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) {

	if (item == NULL) {
		// TODO: it would be nice to do something reasonable in this case
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
		return appendWireStuff(wire, wire->id());
	} else {
		return appendItemStuff(item, item->id(), swappingEnabled);
	}
}

QString HtmlInfoView::appendItemStuff(ItemBase* base, long id, bool swappingEnabled) {
	if (base == NULL) return "missing base";

	QString title;
	prepareTitleStuff(base, title);

	QString retval = appendItemStuff(base, base->modelPart(), id, swappingEnabled, title, base->isPartLabelVisible());
	return retval;
}


QString HtmlInfoView::appendWireStuff(Wire* wire, long id) {
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

	QString title;
	prepareTitleStuff(wire, title);

	QString s = "";
	if(!title.isNull() && !title.isEmpty()) {
		s += QString("<object type='application/x-qt-plugin' classid='title' id='title' width='100%' height='30px'><param name='title' value='%1'/></object>").arg(title);
	}
	s += 		 "<div class='parttitle'>\n";

	m_modelPart = wire->modelPart();
	s += QString("<object type='application/x-qt-plugin' classid='PartIcons' width='%1px' height='%2px'><param name='moduleid' value='%3'/></object>")
		.arg(3 * (STANDARD_ICON_IMG_WIDTH + IconSpace))
		.arg(STANDARD_ICON_IMG_HEIGHT)
		.arg(wire->modelPart()->moduleID());

	s += 	QString("<h2>%1</h2>\n<p>%2</p>\n").arg(nameString)
											   .arg(modelPart->modelPartShared()->version());
	s += 		"</div>\n";

	s += "<div class='block'>";
	s += blockHeader(tr("Properties"),PropsBlockId);
	s += blockContainer(PropsBlockId);
#ifndef QT_NO_DEBUG
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("id").arg(id);
#else
	Q_UNUSED(id);
#endif
	QHash<QString,QString> properties = modelPart->modelPartShared()->properties();
	QString family = properties.value("family", "");
	foreach (QString prop, properties.keys()) {
		QString returnProp, returnValue;
		QStringList nothing;
		bool display = wire->collectExtraInfoHtml(family, prop, properties.value(prop, ""), false, returnProp, returnValue);
		if (display) {
			s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(returnProp).arg(returnValue);
		}
	}

	s += 		 "</table></div>\n";
	s += "</div>";

	if(!modelPart->modelPartShared()->tags().isEmpty()) {
		s += "<div class='block'>";
		s += blockHeader(tr("Tags"),TagsBlockId);
		s += blockContainer(TagsBlockId);
		s += QString("<tr><td colspan='2'>%1</td></tr>\n").arg(modelPart->modelPartShared()->tags().join(", "));
		s += 		"</table></div>\n";
		s += "</div>";
	}

	return s;
}

void HtmlInfoView::prepareTitleStuff(ItemBase *base, QString &title) {

	if(base) {
		title = base->instanceTitle();
	}
	else {
		title = ItemBase::partInstanceDefaultTitle;
	}
}


QString HtmlInfoView::appendItemStuff(ItemBase * itemBase, ModelPart * modelPart, long id, bool swappingEnabled, const QString title, bool labelIsVisible) {
	Q_UNUSED(labelIsVisible);

	if (modelPart == NULL) return "missing modelpart";
	if (modelPart->modelPartShared() == NULL) return "missing modelpart stuff";


	QString s = "";
	if(!title.isNull() && !title.isEmpty()) {
		s += QString("<object type='application/x-qt-plugin' classid='title' id='title' width='100%' height='30px'><param name='title' value='%1'/></object>").arg(title);
	}
	s += 		 "<div class='parttitle'>\n";

	m_modelPart = modelPart;
	s += QString("<object type='application/x-qt-plugin' classid='PartIcons' width='%1px' height='%2px'><param name='moduleid' value='%3'/></object>")
		.arg(3 * (STANDARD_ICON_IMG_WIDTH + IconSpace))
		.arg(STANDARD_ICON_IMG_HEIGHT)
		.arg(modelPart->moduleID());

	s += 		"<div class='parttitle' style='padding-top: 8px; height: 25px;'>\n";
	s += 	QString("<h2>%1</h2>\n<p>%2</p>\n").arg((itemBase) ? itemBase->title() : modelPart->title())
											   .arg("&nbsp;"+modelPart->modelPartShared()->version());
	s += 		"</div>\n";

	s += "<div class='block'>";
	s += blockHeader(tr("Properties"),PropsBlockId);
	s += blockContainer(PropsBlockId);
#ifndef QT_NO_DEBUG
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("id").arg(id);
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
		QString fzppath = modelPart->modelPartShared()->path();
		if (!fzppath.isEmpty()) {
			s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("fzp").arg(fzppath);
		}
	}

#else
	Q_UNUSED(id)
#endif
	QHash<QString,QString> properties = modelPart->modelPartShared()->properties();
	QString family = properties["family"].toLower();

	m_maxPropCount = properties.keys().size() > m_maxPropCount ? properties.keys().size() : m_maxPropCount;
	int rowsLeft = m_maxPropCount;
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
			bool collectValues = swappingEnabled && 
								 (key.compare("family", Qt::CaseInsensitive) != 0) &&
								 (key.compare("id", Qt::CaseInsensitive) != 0); 
			result = itemBase->collectExtraInfoHtml(family, key, value, collectValues, resultKey, resultValue);
		}
		if (result) {
			s += basis.arg(resultKey).arg(resultValue);
		}
		else {
			s += basis.arg(translatedName).arg(value);
		}

		rowsLeft--;
	}

	// always keep the same number of rows in the table even if there are fewer properties
	for(int i = 0; i < rowsLeft; i++) {
		s += "<tr style='height: 35px; '><td style='border-bottom: 0px;' colspan='2'>&nbsp;</td></tr>\n";
	}
	s += 		 "</table></div>\n";
	s += "</div>";

//	s += QString("<tr><td colspan='2'>%1</td></tr>").
//			arg(QString(modelPart->modelPartShared()->path()).remove(QDir::currentPath()));
	if(!modelPart->modelPartShared()->tags().isEmpty()) {
		s += "<div class='block'>";
		s += blockHeader(tr("Tags"),TagsBlockId);
		s += blockContainer(TagsBlockId);
		s += QString("<tr><td colspan='2'>%1</td></tr>\n").arg(modelPart->modelPartShared()->tags().join(", "));
		s += 		"</table></div>\n";
		s += "</div>";
	}

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
	setContent("");
}

void addLabel(QHBoxLayout * hboxLayout, QPixmap * pixmap) {
	QLabel * label = new QLabel();
	QPalette palette = label->palette();
	palette.setColor(QPalette::Window, QColor(0xc2, 0xc2, 0xc2));
	label->setPalette(palette);
	label->setAutoFillBackground(true);
	label->setPixmap(*pixmap);
	label->setFixedSize(pixmap->size());
	hboxLayout->addWidget(label);
	hboxLayout->addSpacing(IconSpace);
}

QObject * HtmlInfoView::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {
	Q_UNUSED(url);

	// TODO (jrc): calling all this icon stuff in htmlInfoView isn't nice but I haven't figured out a better location
	// since you can't count on having the ItemBase

	if (classid.compare("title", Qt::CaseInsensitive) == 0) {
		FLineEdit * lineEdit = new FLineEdit(parent);
		lineEdit->setObjectName("instanceTitleEditor");
		//QFont font = lineEdit->font();
		//font.setPointSize(14);
		//font.setBold(true);
		//lineEdit->setFont(font);
		for (int i = 0; i < paramNames.count(); i++) {
			if (paramNames[i].compare("title", Qt::CaseInsensitive) == 0) {
				lineEdit->setText(paramValues[i]);
				break;
			}
		}

		connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(setInstanceTitle()));
		connect(lineEdit, SIGNAL(mouseEnter()), this, SLOT(instanceTitleEnter()));
		connect(lineEdit, SIGNAL(mouseLeave()), this, SLOT(instanceTitleLeave()));
		connect(lineEdit, SIGNAL(editable(bool)), this, SLOT(instanceTitleEditable(bool)));

		setInstanceTitleColors(lineEdit, QColor(0xb3, 0xb3, 0xb3), QColor(0x57, 0x57, 0x57));
		lineEdit->setAutoFillBackground(true);

		return lineEdit;
	}

	if (classid.compare("PartIcons", Qt::CaseInsensitive) != 0) {
		return NULL;
	}

	if (paramValues.count() < 1) return NULL;

	QString moduleID;
	for (int i = 0; i < paramNames.count(); i++) {
		if (paramNames[i].compare("moduleid", Qt::CaseInsensitive) == 0) {
			moduleID = paramValues.at(i);
			break;
		}
	}

	ModelPart * modelPart = m_modelPart;
	if (modelPart == NULL) return NULL;

	QSize size(STANDARD_ICON_IMG_WIDTH, STANDARD_ICON_IMG_HEIGHT);
	QPixmap *pixmap1 = FSvgRenderer::getPixmap(moduleID, ViewLayer::Icon, size);
	QPixmap *pixmap2 = FSvgRenderer::getPixmap(moduleID, ViewLayer::Schematic, size);
	QPixmap *pixmap3 = FSvgRenderer::getPixmap(moduleID, ViewLayer::Copper0, size);

	if (pixmap1 == NULL) {
		LayerAttributes layerAttributes;
		ItemBase::setUpImage(modelPart, ViewIdentifierClass::IconView, ViewLayer::Icon, layerAttributes);
		pixmap1 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
	}
	if (pixmap2 == NULL) {
		LayerAttributes layerAttributes;
		ItemBase::setUpImage(modelPart, ViewIdentifierClass::SchematicView, ViewLayer::Schematic, layerAttributes);
		pixmap2 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Schematic, size);
	}
	if (pixmap3 == NULL) {
		LayerAttributes layerAttributes;
		ItemBase::setUpImage(modelPart, ViewIdentifierClass::PCBView, ViewLayer::Copper0, layerAttributes);
		pixmap3 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Copper0, size);
	}

	if (pixmap1 == NULL && pixmap2 == NULL && pixmap3 == NULL) return NULL;

	QFrame * frame = new QFrame(parent);
	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setContentsMargins (0, 0, 0, 0);

	if(pixmap1 != NULL) {
		addLabel(hboxLayout, pixmap1);
		delete pixmap1;
	}
	if(pixmap2 != NULL) {
		addLabel(hboxLayout, pixmap2);
		delete pixmap2;
	}
	if(pixmap3 != NULL) {
		addLabel(hboxLayout, pixmap3);
		delete pixmap3;
	}

	frame->setLayout(hboxLayout);

	return frame;
}


void HtmlInfoView::setInstanceTitle() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (edit == NULL) return;
	if (m_infoGraphicsView == NULL) return;
	if (m_currentItem == NULL) return;

	m_infoGraphicsView->setInstanceTitle(m_currentItem->id(), edit->text(), true, true, false);
}

void HtmlInfoView::instanceTitleEnter() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	setInstanceTitleColors(edit, QColor(0xc8, 0xc8, 0xc8), QColor(0x57, 0x57, 0x57));
}

void HtmlInfoView::instanceTitleLeave() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	setInstanceTitleColors(edit, QColor(0xb3, 0xb3, 0xb3), QColor(0x57, 0x57, 0x57));
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

