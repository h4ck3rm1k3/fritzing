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

#include "htmlinfoview.h"
#include "infographicsview.h"
#include "debugdialog.h"
#include "connectorstuff.h"
#include "connector.h"
#include "fsvgrenderer.h"
#include "layerattributes.h"
#include "dockmanager.h"


#define HTML_EOF "</body>\n</html>"

QString HtmlInfoView::PropsBlockId = "props_id";
QString HtmlInfoView::TagsBlockId = "tags_id";
QString HtmlInfoView::ConnsBlockId = "conns_id";

HtmlInfoView::HtmlInfoView(ReferenceModel *refModel, QWidget * parent) : QFrame(parent) {
	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->setMargin(0);
	lo->setSpacing(0);
	m_webView = new QWebView(this);
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

	//setContent("<html></html>");
	m_currentItem = NULL;
	m_currentSwappingEnabled = false;
	m_refModel = refModel;
	Q_ASSERT(m_refModel);

	connect(m_webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(jsRegister()));
	registerRefModel();

	m_maxPropCount = 0;

	QSettings settings;
	m_blocksVisibility[PropsBlockId] = settings.value(settingsBlockVisibilityName(PropsBlockId),true).toBool();
	m_blocksVisibility[TagsBlockId] = settings.value(settingsBlockVisibilityName(TagsBlockId),true).toBool();
	m_blocksVisibility[ConnsBlockId] = settings.value(settingsBlockVisibilityName(ConnsBlockId),true).toBool();
}

void HtmlInfoView::jsRegister() {
	if (!m_setContentMutex.tryLock()) return;

	m_setContentMutex.unlock();

	// prevent recursion, particularly when setting content to NULL
	disconnect(m_webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(jsRegister()));
	registerCurrentAgain();
	registerRefModel();
	m_webView->page()->mainFrame()->addToJavaScriptWindowObject(
		"infoView", this
	);
	connect(m_webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(jsRegister()));
}

void HtmlInfoView::setBlockVisibility(const QString &blockId, bool value) {
	m_blocksVisibility[blockId] = value;
	QSettings settings;
	settings.setValue(settingsBlockVisibilityName(blockId),QVariant::fromValue(value));
}

QString HtmlInfoView::settingsBlockVisibilityName(const QString &blockId) {
	return "infoView/"+blockId+"Visibility";
}

void HtmlInfoView::registerRefModel() {
	m_webView->page()->mainFrame()->addToJavaScriptWindowObject(
		"refModel", m_refModel
	);
}

void HtmlInfoView::hoverEnterItem(ModelPart * modelPart, bool swappingEnabled) {
	m_currentSwappingEnabled = swappingEnabled;
	QString s = "";
	s += appendItemStuff(modelPart, 0, swappingEnabled, "", false);
	setContent(s);
}

void HtmlInfoView::viewItemInfo(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) {
	
	if (item == NULL) {
		// TODO: it would be nice to do something reasonable in this case
		setNullContent();
		registerInfoGraphicsView(infoGraphicsView, "sketch");
		registerRefModel();
		return;
	}

	m_currentSwappingEnabled = swappingEnabled;

	QString s = appendStuff(item,swappingEnabled);

	//s += appendViewGeometry(item, false); //cprobably not used in alpha -andre/johannes
	//s += appendCurrentGeometry(item, false); //probably not used in alpha -andre/johannes
	setContent(s);
	registerAsCurrentItem(item);
	registerInfoGraphicsView(infoGraphicsView, "sketch");
	registerRefModel();
}

QString HtmlInfoView::appendStuff(ItemBase* item, bool swappingEnabled) {
	Wire *wire = dynamic_cast<Wire*>(item);
	if(wire) {
		return appendWireStuff(wire, wire->id());
	} else {
		return appendItemStuff(item, item->id(), swappingEnabled);
	}
}

void HtmlInfoView::hoverEnterItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * item, bool swappingEnabled) {
	viewItemInfo(infoGraphicsView, item, swappingEnabled);
}


void HtmlInfoView::hoverLeaveItem(ModelPart * modelPart) {
	Q_UNUSED(modelPart);
	//clear();
}


void HtmlInfoView::hoverLeaveItem(InfoGraphicsView * , QGraphicsSceneHoverEvent *, ItemBase * ){
	//clear();
	unregisterCurrentItem();
}

void HtmlInfoView::viewConnectorItemInfo(ConnectorItem * item, bool swappingEnabled) {
	Connector * connector = item->connector();
	if (connector == NULL) return;

	ConnectorStuff * connectorStuff = connector->connectorStuff();
	if (connectorStuff == NULL) return;

	QString s = appendStuff(item->attachedTo(), swappingEnabled);
	s += "<div class='block'>";
	s += blockHeader(tr("Connections"),ConnsBlockId);
	s += blockContainer(ConnsBlockId);
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("conn.")).arg(tr("connected to %n item(s)", "", item->connectionsCount()));
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("name")).arg(connectorStuff->name());
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("type")).arg(Connector::connectorNameFromType(connector->connectorType()));
	s += 		 "</table></div>\n";
	s += "</div>";

	setContent(s);

	if (item->attachedTo() && item->attachedTo() != m_currentItem) {
		registerAsCurrentItem(item->attachedTo());
		registerRefModel();
	}
}

void HtmlInfoView::hoverEnterConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem * item, bool swappingEnabled) {
	Q_UNUSED(igv)
	Q_UNUSED(event)
	viewConnectorItemInfo(item, swappingEnabled);
}

void HtmlInfoView::hoverLeaveConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem *connItem) {
	Q_UNUSED(igv);
	Q_UNUSED(event);
	Q_UNUSED(connItem);
	// clear
}


QString HtmlInfoView::appendViewGeometry(ItemBase * base, bool doLine) {
	if (base == NULL) return "missing base";

	ViewGeometry & viewGeometry = base->getViewGeometry();
	QString s = "";
	s += 		 "<table>\n";
	s += QString("<tr><td class='subhead' colspan='2'>%1</td></tr>\n").arg(tr("Position")) +
		 QString("<tr><td class='label'>x</td><td>%1</td></tr>\n").arg(viewGeometry.loc().x()) +
		 QString("<tr><td class='label'>y</td><td>%1</td></tr>\n").arg(viewGeometry.loc().y());

	if (doLine) {
		s +=	QString("<tr><td class='subhead' colspan='2'>%1</td></tr>\n").arg(tr("line")) +
				QString("<tr><td class='label'>x1 / y1</td><td>%1 / %2</td></tr>\n").arg(viewGeometry.line().x1()).
																				   arg(viewGeometry.line().y1());
				QString("<tr><td class='label'>x2 / y2</td><td>%1 / %2</td></tr>\n").arg(viewGeometry.line().x2()).
																				   arg(viewGeometry.line().y2());
	}

	s += QString("<tr><td class='label'>z</td><td>%1</td></tr>\n").arg(viewGeometry.z());
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg(tr("view")).arg(base->viewIdentifierName());
	s += 		 "</table>\n";

	return s;
}

QString HtmlInfoView::appendCurrentGeometry(ItemBase * item, bool doLine) {
	Q_UNUSED(doLine);

	if (item == NULL) return "missing item";

	QPointF loc = item->pos();
	QString s = "";
	s += 		 "<table>\n";
	s += QString("<tr><td class='subhead' colspan='2'>%1</td></tr>\n").arg(tr("current geometry")) +
		 QString("<tr><td class='label'>x</td><td>%1</td></tr>\n").arg(loc.x()) +
		 QString("<tr><td class='label'>y</td><td>%1</td></tr>\n").arg(loc.y());

	if (item->hasLine()) {
		QLineF line = item->line();
		s += QString("<tr><td class='subhead' colspan='2'>%1</td></tr>\n").arg(tr("line")) +
			 QString("<tr><td class='label'>x1 / y1</td><td>%1,%2</td></tr>\n").arg(line.x1()).
		 																	  arg(line.y1());
		s += QString("<tr><td class='label'>x2 / y2</td><td>%1,%2</td></tr>\n").arg(line.x2()).
		 																	  arg(line.y2());
	}

	s += QString("<tr><td class='label'>z</td><td>%1</td></tr>\n").arg(item->zValue());
	s += 		 "</table>\n";
	return s;
}

QString HtmlInfoView::appendItemStuff(ItemBase* base, long id, bool swappingEnabled) {
	if (base == NULL) return "missing base";

	QString title;
	prepareTitleStuff(base, title);

	QString retval = appendItemStuff(base->modelPart(), id, swappingEnabled, title, base->isPartLabelVisible());
	return retval;
}


QString HtmlInfoView::appendWireStuff(Wire* wire, long id) {
	if (wire == NULL) return "missing base";

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


	QSize size(STANDARD_ICON_IMG_WIDTH, STANDARD_ICON_IMG_HEIGHT);
	QPixmap *pixmap = FSvgRenderer::getPixmap(wire->modelPart()->moduleID(), ViewLayer::Icon, size);

	ModelPart *modelPart = wire->modelPart();
	if (modelPart == NULL) return "missing modelpart";
	if (modelPart->modelPartStuff() == NULL) return "missing modelpart stuff";

	QString s = "";
	if(!title.isNull() && !title.isEmpty()) {
		s += QString("<h1 onclick='editBox(this)' id='title'>%1</h1>\n").arg(title);
	}
	s += 		 "<div class='parttitle'>\n";

	if(pixmap != NULL) {
		s += QString("<img src='%1' width='%2' height='%3' />\n").arg(toHtmlImage(pixmap)).arg(STANDARD_ICON_IMG_WIDTH).arg(STANDARD_ICON_IMG_HEIGHT);
	}

	/*
	// TODO:  put this somewhere more reasonable
	if(!title.isNull() && !title.isEmpty()) {
		s += QString("<input type='checkbox' %1 id='show_part_label' onclick='showPartLabel(this, this.checked)'>show label").arg(wire->isPartLabelVisible() ? "checked='true'" : "");
	}
	*/

	s += 	QString("<h2>%1</h2>\n<p>%2</p>\n").arg(nameString)
											   .arg(modelPart->modelPartStuff()->version());
	s += 		"</div>\n";

	s += "<div class='block'>";
	s += blockHeader(tr("Properties"),PropsBlockId);
	s += blockContainer(PropsBlockId);
#ifndef QT_NO_DEBUG
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("id").arg(id);
#else
	Q_UNUSED(id);
#endif
	QHash<QString,QString> properties = modelPart->modelPartStuff()->properties();
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("family").arg(properties["family"]);
	QString select = wireColorsSelect(wire);
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("color").arg(select);
	select = wireWidthSelect(wire);
	if (!select.isEmpty()) {
		s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("width").arg(select);
	}

	s += 		 "</table></div>\n";
	s += "</div>";

	if(!modelPart->modelPartStuff()->tags().isEmpty()) {
		s += "<div class='block'>";
		s += blockHeader(tr("Tags"),TagsBlockId);
		s += blockContainer(TagsBlockId);
		s += QString("<tr><td colspan='2'>%1</td></tr>\n").arg(modelPart->modelPartStuff()->tags().join(", "));
		s += 		"</table></div>\n";
		s += "</div>";
	}

	//delete pixmap;
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


QString HtmlInfoView::appendItemStuff(ModelPart * modelPart, long id, bool swappingEnabled, const QString title, bool labelIsVisible) {
	Q_UNUSED(labelIsVisible);

	if (modelPart == NULL) return "missing modelpart";
	if (modelPart->modelPartStuff() == NULL) return "missing modelpart stuff";

	QSize size(STANDARD_ICON_IMG_WIDTH, STANDARD_ICON_IMG_HEIGHT);
	QPixmap *pixmap1 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
	QPixmap *pixmap2 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Schematic, size);
	QPixmap *pixmap3 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Copper0, size);

	// TODO (jrc): calling setUpImage here is a hack, best to do it somewhere else
	if (pixmap2 == NULL) {
		LayerAttributes layerAttributes;
		PaletteItemBase::setUpImage(modelPart, ItemBase::SchematicView, ViewLayer::Schematic, layerAttributes);
		pixmap2 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Schematic, size);
	}
	if (pixmap3 == NULL) {
		LayerAttributes layerAttributes;
		PaletteItemBase::setUpImage(modelPart, ItemBase::PCBView, ViewLayer::Copper0, layerAttributes);
		pixmap3 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Copper0, size);
	}

	QString s = "";
	if(!title.isNull() && !title.isEmpty()) {
		s += QString("<h1 onclick='editBox(this)' id='title'>%1</h1>\n").arg(title);
	}
	s += 		 "<div class='parttitle'>\n";
	if(pixmap1 != NULL) {
		s += QString("<img src='%1' width='%2' height='%3' />\n").arg(toHtmlImage(pixmap1)).arg(STANDARD_ICON_IMG_WIDTH).arg(STANDARD_ICON_IMG_HEIGHT);
	}
	if(pixmap2 != NULL) {
		s += QString("<img src='%1' width='%2' height='%3' />\n").arg(toHtmlImage(pixmap2)).arg(STANDARD_ICON_IMG_WIDTH).arg(STANDARD_ICON_IMG_HEIGHT);
		delete pixmap2;
	}
	if(pixmap3 != NULL) {
		s += QString("<img src='%1' width='%2' height='%3' />\n").arg(toHtmlImage(pixmap3)).arg(STANDARD_ICON_IMG_WIDTH).arg(STANDARD_ICON_IMG_HEIGHT);
		delete pixmap3;
	}

	/*
	// TODO:  put this somewhere more reasonable
	if(!title.isNull() && !title.isEmpty()) {
		s += QString("<input type='checkbox' %1 id='show_part_label' onclick='showPartLabel(this, this.checked)'>show label").arg(labelIsVisible ? "checked='true'" : "");
	}
	*/

	s += 		"<div class='parttitle' style='padding-top: 8px; height: 25px;'>\n";
	s += 	QString("<h2>%1</h2>\n<p>%2</p>\n").arg(modelPart->modelPartStuff()->title())
											   .arg("&nbsp;"+modelPart->modelPartStuff()->version());
	s += 		"</div>\n";

	s += "<div class='block'>";
	s += blockHeader(tr("Properties"),PropsBlockId);
	s += blockContainer(PropsBlockId);
#ifndef QT_NO_DEBUG
	s += QString("<tr><td class='label'>%1</td><td>%2</td></tr>\n").arg("id").arg(id);
#else
	Q_UNUSED(id)
#endif
	QHash<QString,QString> properties = modelPart->modelPartStuff()->properties();
	QString family = properties["family"].toLower();

	m_maxPropCount = properties.keys().size() > m_maxPropCount ? properties.keys().size() : m_maxPropCount;
	int rowsLeft = m_maxPropCount;
	for(int i=0; i < properties.keys().size(); i++) {
		QString key = properties.keys()[i];
		QString value = properties[ key ];
		s += propertyHtml(key, value, family, swappingEnabled);
		rowsLeft--;
	}

	for(int i = 0; i < rowsLeft; i++) {
		s += "<tr style='height: 35px; '><td style='border-bottom: 0px;' colspan='2'>&nbsp;</td></tr>\n";
	}
	s += 		 "</table></div>\n";
	s += "</div>";

//	s += QString("<tr><td colspan='2'>%1</td></tr>").
//			arg(QString(modelPart->modelPartStuff()->path()).remove(QDir::currentPath()));
	if(!modelPart->modelPartStuff()->tags().isEmpty()) {
		s += "<div class='block'>";
		s += blockHeader(tr("Tags"),TagsBlockId);
		s += blockContainer(TagsBlockId);
		s += QString("<tr><td colspan='2'>%1</td></tr>\n").arg(modelPart->modelPartStuff()->tags().join(", "));
		s += 		"</table></div>\n";
		s += "</div>";
	}

	return s;
}

QString HtmlInfoView::wireColorsSelect(Wire *wire) {
	QString currColor = wire->colorString();
	if (wire->canChangeColor()) {
		QString retval = QString("<script language='JavaScript'>var oldColor = '%1'</script>\n").arg(currColor);
		retval += QString("<select onchange='setWireColor(\"%1\",%2,this.value)'>\n")
					.arg(wire->instanceTitle())
					.arg(wire->id());
		foreach(QString colorName, Wire::colorNames) {
			QString colorValue = Wire::colorTrans.value(colorName);
			QString selected = colorValue == currColor ? " selected='selected' " : "";
			retval += QString("\t<option value='%2' %3>%1</option>\n").arg(colorName).arg(colorValue).arg(selected);
		}
		retval += "</select>\n";
		return retval;
	}
	else {
		return currColor;
	}
}

QString HtmlInfoView::wireWidthSelect(Wire *wire) {
	if (!wire->canChangeWidth()) {
		return ___emptyString___;
	}

	QString retval = QString("<script language='JavaScript'>var oldWidth = '%1'</script>\n").arg(wire->width());
	retval += QString("<select onchange='setWireWidth(\"%1\",%2,this.value)'>\n")
				.arg(wire->instanceTitle())
				.arg(wire->id());
	foreach(QString widthName, Wire::widthNames) {
		qreal widthValue = Wire::widthTrans.value(widthName);
		QString selected = (widthValue == wire->width()) ? " selected='selected' " : "";
		retval += QString("\t<option value='%2' %3>%1</option>\n").arg(widthName).arg(widthValue).arg(selected);
	}
	retval += "</select>\n";
	return retval;
}

QString HtmlInfoView::propertyHtml(const QString& name, const QString& value, const QString& family, bool dynamic) {
	QStringList values = m_refModel->values(family,name);

	if(!dynamic || name.toLower() == "id" || name.toLower() == "family" || values.size() == 1) {
		return QString("<tr style='height: 35px;'><td class='label'>%1</td><td>%2</td></tr>\n").arg(name).arg(value);
	} else {
		QString options = "";
		QString jsCode = "<script language='JavaScript'>\n";
		jsCode += QString("currProps['%1'] = '%2'; \n").arg(name).arg(value);
		foreach(QString opt, values) {
			options += QString("<option value='%1' %2>%1</option> \n")
				.arg(opt).arg(opt==value?" selected='selected'" : ___emptyString___);
		}
		jsCode += "</script>\n";

		return jsCode+QString("<tr style='height: 35px;'><td class='label'>%1</td><td><select name='%1' id='%1' onchange='doSwap(\"%3\",\"%1\",\"%2\")'>\n%4</select></td></tr>\n")
						.arg(name).arg(value).arg(family).arg(options);
	}
}

QString HtmlInfoView::toHtmlImage(QPixmap *pixmap, const char* format) {
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	pixmap->save(&buffer, format); // writes pixmap into bytes in PNG format
	return QString("data:image/%1;base64,%2").arg(QString(format).toLower()).arg(QString(bytes.toBase64()));
}

void HtmlInfoView::setContent(const QString &html) {

	// using a mutex because setContent can trigger jsRegister which can cause another call to setContent
	if (!m_setContentMutex.tryLock()) {
		return;
	}

	QString fileContent = QString(QString("<html>\n%1<body>\n%2")+HTML_EOF).arg(m_includes).arg(html);
	m_webView->setHtml(fileContent);

	m_setContentMutex.unlock();
	
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

void HtmlInfoView::registerCurrentAgain() {
	registerAsCurrentItem(m_currentItem);
}

bool HtmlInfoView::registerAsCurrentItem(ItemBase *item) {
	// note: must take place after setContent()
	if(item) {
		m_currentItem = item;
		/*
		if(m_currentItem->itemType() != ModelPart::Wire) {
			registerJsObjects("swapper");
		} else {
			registerJsObjects("wireManager");
		}
		*/
		registerJsObjects("sketch");
	} else {
		m_currentItem = NULL;
		setNullContent();
	}
	return m_currentItem != NULL;
}

void HtmlInfoView::registerJsObjects(const QString &parentName) {
	Q_UNUSED(parentName);
	m_webView->page()->mainFrame()->addToJavaScriptWindowObject(
		"currentItem", m_currentItem
	);
}

void HtmlInfoView::unregisterCurrentItem() {
	registerAsCurrentItem(NULL);
}

void HtmlInfoView::unregisterCurrentItemIf(long id) {
	if (m_currentItem == NULL) {
		return;
	}
	if (m_currentItem->id() == id) {
		registerAsCurrentItem(NULL);
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

void HtmlInfoView::registerInfoGraphicsView(InfoGraphicsView * infoGraphicsView, const QString & parentName) {
	// note: must take place after setContent()
	if(infoGraphicsView) {
		m_webView->page()->mainFrame()->addToJavaScriptWindowObject(
			parentName, infoGraphicsView
		);
	}
}

void HtmlInfoView::setNullContent() 
{
	setContent("<html></html>");
}
