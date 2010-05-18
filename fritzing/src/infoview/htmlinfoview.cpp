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
#include <QHBoxLayout>
#include <QSettings>
#include <QPalette>
#include <QFormLayout>

#include "htmlinfoview.h"
#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../connectors/connectorshared.h"
#include "../connectors/connector.h"
#include "../fsvgrenderer.h"
#include "../dockmanager.h"
#include "../utils/flineedit.h"
#include "../utils/expandableview.h"


#define HTML_EOF "</body>\n</html>"

QPixmap * NoIcon = NULL;

const int HtmlInfoView::STANDARD_ICON_IMG_WIDTH = 32;
const int HtmlInfoView::STANDARD_ICON_IMG_HEIGHT = 32;
const int IconSpace = 3;

QString HtmlInfoView::PropsBlockId = "props_id";
QString HtmlInfoView::TagsBlockId = "tags_id";
QString HtmlInfoView::ConnsBlockId = "conns_id";

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

	m_partTitle = NULL;
	m_partVersion = NULL;
	m_connDescr = NULL;
	m_tagsLabel = NULL;
	m_lastSwappingEnabled = false;
	m_lastItemBase = NULL;
	m_infoGraphicsView = NULL;
	m_setContentTimer.setSingleShot(true);
	m_setContentTimer.setInterval(10);
	connect(&m_setContentTimer, SIGNAL(timeout()), this, SLOT(setContent()));
	QVBoxLayout *vlo = new QVBoxLayout(mainFrame);
	vlo->setMargin(0);
	vlo->setSpacing(0);
	vlo->setSizeConstraint( QLayout::SetMinAndMaxSize );

        /* Part Title */

	m_titleEdit = new FLineEdit(mainFrame);
	m_titleEdit->setObjectName("instanceTitleEditor");

	connect(m_titleEdit, SIGNAL(editingFinished()), this, SLOT(setInstanceTitle()));
	connect(m_titleEdit, SIGNAL(mouseEnter()), this, SLOT(instanceTitleEnter()));
	connect(m_titleEdit, SIGNAL(mouseLeave()), this, SLOT(instanceTitleLeave()));
	connect(m_titleEdit, SIGNAL(editable(bool)), this, SLOT(instanceTitleEditable(bool)));

	setInstanceTitleColors(m_titleEdit, QColor(0xb3, 0xb3, 0xb3), QColor(0x57, 0x57, 0x57));
	m_titleEdit->setAutoFillBackground(true);
	vlo->addWidget(m_titleEdit);

        /* Part Icons */

	if (NoIcon == NULL) {
		NoIcon = new QPixmap(":/resources/images/icons/noicon.png");
	}

	QFrame * iconFrame = new QFrame(mainFrame);
	iconFrame->setObjectName("IconFrame");

	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setContentsMargins (0, 0, 0, 0);
	hboxLayout->addSpacing(IconSpace);
	m_icon1 = addLabel(hboxLayout, NoIcon);
	m_icon2 = addLabel(hboxLayout, NoIcon);
	m_icon3 = addLabel(hboxLayout, NoIcon);
	m_location = new QLabel();
	hboxLayout->addWidget(m_location);
	hboxLayout->addSpacerItem(new QSpacerItem(IconSpace, 1, QSizePolicy::Expanding));
	hboxLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );
	iconFrame->setLayout(hboxLayout);
	vlo->addWidget(iconFrame);

	QFrame * tFrame = new QFrame(mainFrame);
	hboxLayout = new QHBoxLayout();
	hboxLayout->setContentsMargins (0, 0, 0, 0);
	m_partTitle = new QLabel(tFrame);
	m_partTitle->setObjectName("partTitle");
	hboxLayout->addWidget(m_partTitle);
	hboxLayout->addSpacerItem(new QSpacerItem(IconSpace, 1, QSizePolicy::Expanding));
	hboxLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );
	m_partVersion = new QLabel(tFrame);
	m_partVersion->setObjectName("partTitle");
	hboxLayout->addWidget(m_partVersion);
	tFrame->setLayout(hboxLayout);
	vlo->addWidget(tFrame);

	ExpandableView * pev = new ExpandableView(tr("Properties"), this);
	pev->setProperty("blockid", PropsBlockId);
	connect(pev, SIGNAL(expanded(bool)), this, SLOT(viewExpanded(bool)));
	QFrame * propFrame = new QFrame(this);
	m_propLayout = new QGridLayout(propFrame);
	m_propLayout->setMargin(0);
	propFrame->setLayout(m_propLayout);
	pev->setChildFrame(propFrame);
	vlo->addWidget(pev);

	ExpandableView * tev = new ExpandableView(tr("Tags"), this);
	tev->setProperty("blockid", TagsBlockId);
	connect(tev, SIGNAL(expanded(bool)), this, SLOT(viewExpanded(bool)));
	m_tagsLabel = new QLabel(this);
	tev->setChildFrame(m_tagsLabel);
	vlo->addWidget(tev);

	ExpandableView * cev = new ExpandableView(tr("Connections"), this);
	cev->setProperty("blockid", ConnsBlockId);
	connect(cev, SIGNAL(expanded(bool)), this, SLOT(viewExpanded(bool)));
	QFrame * connFrame = new QFrame(this);
	QFormLayout * connLayout = new QFormLayout(propFrame);
	connLayout->setMargin(0);
	connLayout->setLabelAlignment(Qt::AlignLeft);
	connFrame->setLayout(connLayout);

	QLabel * descrLabel = new QLabel(tr("conn."), this);
	descrLabel->setObjectName("connectionsLabel");
	m_connDescr = new QLabel(this);
	connLayout->addRow(descrLabel, m_connDescr);

	QLabel * nameLabel = new QLabel(tr("name"), this);
	nameLabel->setObjectName("connectionsLabel");
	m_connName = new QLabel(this);
	connLayout->addRow(nameLabel, m_connName);

	QLabel * typeLabel = new QLabel(tr("type"), this);
	typeLabel->setObjectName("connectionsLabel");
	m_connType = new QLabel(this);
	connLayout->addRow(typeLabel, m_connType);

	cev->setChildFrame(connFrame);
	vlo->addWidget(cev);

	mainFrame->setLayout(vlo);

	this->setWidget(mainFrame);

	m_currentItem = NULL;
	m_currentSwappingEnabled = false;

	QSettings settings;
	if (!settings.value(settingsBlockVisibilityName(PropsBlockId),true).toBool()) {
		pev->expanderClicked();
	}
	if (!settings.value(settingsBlockVisibilityName(TagsBlockId),true).toBool()) {
		tev->expanderClicked();
	}
	if (!settings.value(settingsBlockVisibilityName(ConnsBlockId),true).toBool()) {
		cev->expanderClicked();
	}
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

void HtmlInfoView::viewConnectorItemInfo(ConnectorItem * connectorItem) {
	Connector * connector = NULL;
	ConnectorShared * connectorShared = NULL;
	if (connectorItem) {
		if (connectorItem->attachedTo() != m_lastItemBase) {
			return;
		}

		connector = connectorItem->connector();
		connectorShared = connector->connectorShared();
	
		QPointF p = connectorItem->sceneAdjustedTerminalPoint(NULL);
		m_location->setText(QString("%1 (%2,%3)").arg(m_location->text()).arg(p.x()).arg(p.y()));
	}

	if (m_connDescr) {
		m_connDescr->setText(connectorItem ? tr("connected to %n item(s)", "", connectorItem->connectionsCount()) : "");
		m_connName->setText(connectorShared ? connectorShared->name() : "");
		m_connType->setText(connector ? Connector::connectorNameFromType(connector->connectorType()) : "");
	}

}

void HtmlInfoView::hoverEnterConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem * item, bool swappingEnabled) {
	Q_UNUSED(event)
	//QString s = appendStuff(connectorItem->attachedTo(), swappingEnabled);
	//setCurrentItem(item->attachedTo());
	//m_infoGraphicsView = infoGraphicsView;
	//setContent(s);

	viewConnectorItemInfo(item);
}

void HtmlInfoView::hoverLeaveConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem *connItem) {
	Q_UNUSED(event);
	Q_UNUSED(connItem);
	viewConnectorItemInfo(NULL);
}

void HtmlInfoView::viewItemInfoAux(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) {

	if (item == NULL) {
		setNullContent();
		return;
	}

	m_currentSwappingEnabled = swappingEnabled;

	appendStuff(item, swappingEnabled);
	setCurrentItem(item);
	m_infoGraphicsView = infoGraphicsView;
}

void HtmlInfoView::appendStuff(ItemBase* item, bool swappingEnabled) {
	Wire *wire = dynamic_cast<Wire*>(item);
	if (wire) {
		appendWireStuff(wire, swappingEnabled);
	} else {
		appendItemStuff(item, swappingEnabled);
	}
}

void HtmlInfoView::appendWireStuff(Wire* wire, bool swappingEnabled) {
	if (wire == NULL) return;

	ModelPart *modelPart = wire->modelPart();
	if (modelPart == NULL) return;
	if (modelPart->modelPartShared() == NULL) return;

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
	m_location->setText(QString("(%1,%2)").arg(wire->pos().x()).arg(wire->pos().y()));

	partTitle(nameString, modelPart->version());

	displayProps(modelPart, wire, swappingEnabled);
	addTags(modelPart);

}

void HtmlInfoView::appendItemStuff(ItemBase* base, bool swappingEnabled) {
	if (base == NULL) return;

	appendItemStuff(base, base->modelPart(), swappingEnabled, base->isPartLabelVisible());
}

void HtmlInfoView::appendItemStuff(ItemBase * itemBase, ModelPart * modelPart, bool swappingEnabled, bool labelIsVisible) {
	Q_UNUSED(labelIsVisible);

	if (modelPart == NULL) return;
	if (modelPart->modelPartShared() == NULL) return;

	setUpTitle(itemBase);
	setUpIcons(modelPart);
	m_location->setText(QString("(%1,%2)").arg(itemBase->pos().x()).arg(itemBase->pos().y()));

	partTitle((itemBase) ? itemBase->title() : modelPart->title(), modelPart->version());

	displayProps(modelPart, itemBase, swappingEnabled);
	addTags(modelPart);
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

void HtmlInfoView::setNullContent()
{
	setUpTitle(NULL);
	partTitle("", "");
	setUpIcons(NULL);
	displayProps(NULL, NULL, false);
	addTags(NULL);
	viewConnectorItemInfo(NULL);
	m_location->setText("");
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

void HtmlInfoView::addTags(ModelPart * modelPart) {
	if (m_tagsLabel == NULL) return;

	if (modelPart == NULL || modelPart->tags().isEmpty()) {
		m_tagsLabel->setText("");
		return;
	}

	m_tagsLabel->setText(modelPart->tags().join(", "));
}

void HtmlInfoView::partTitle(const QString & title, const QString & version) {
	if (m_partTitle == NULL) return;

	m_partTitle->setText(title);
	m_partVersion->setText(version);

}

void HtmlInfoView::displayProps(ModelPart * modelPart, ItemBase * itemBase, bool swappingEnabled) 
{
	QStringList keys;
	QHash<QString, QString> properties;
	QString family;
	if (modelPart) {
		properties = modelPart->properties();
		family = properties.value("family", "").toLower();
		if (itemBase) {
			itemBase->prepareProps();
		}

		// ensure family is first;
		keys = properties.keys();
		keys.removeOne("family");
		keys.push_front("family");

#ifndef QT_NO_DEBUG
		properties.insert("id", QString("%1 %2").arg(itemBase ? QString::number(itemBase->id()) : "").arg(modelPart->moduleID()));
		keys.insert(1, "id");

		int insertAt = 2;

		if (itemBase) {
			PaletteItemBase * paletteItemBase = qobject_cast<PaletteItemBase *>(itemBase);
			if (paletteItemBase != NULL) {
				properties.insert("svg", paletteItemBase->filename());
				keys.insert(insertAt++, "svg");
			}
		}
		else {
			FSvgRenderer * renderer = FSvgRenderer::getByModuleID(modelPart->moduleID(), ViewLayer::Icon);
			if (renderer != NULL) {
				properties.insert("svg", renderer->filename());
				keys.insert(insertAt++, "svg");
			}
		}
		if (modelPart->modelPartShared()) {
			properties.insert("fzp",  modelPart->path());
			keys.insert(insertAt++, "fzp");
		}	
#endif

	}

	int ix = 0;
	foreach(QString key, keys) {
		if (ix >= m_propThings.count()) {
			PropThing * propThing = new PropThing;
			propThing->m_plugin = NULL;
			m_propThings.append(propThing);

			QLabel * propNameLabel = new QLabel(this);
			propNameLabel->setObjectName("connectionsLabel");
			propNameLabel->setWordWrap(true);
			propThing->m_name = propNameLabel;
			m_propLayout->addWidget(propNameLabel, ix, 0);

			QFrame * valueFrame = new QFrame(this);
			QVBoxLayout * vlayout = new QVBoxLayout(valueFrame);
			propThing->m_layout = vlayout;
			vlayout->setMargin(0);

			QLabel * propValueLabel = new QLabel(valueFrame);
			vlayout->addWidget(propValueLabel);
			propThing->m_value = propValueLabel;
			m_propLayout->addWidget(valueFrame, ix, 1);
		}

		PropThing * propThing = m_propThings.at(ix);
		clearPropThingPlugin(propThing);

		QString value = properties.value(key,"");
		QString translatedName = ItemBase::translatePropertyName(key);
		QString resultKey, resultValue;
		QWidget * resultWidget = NULL;
		bool result = false;
		if (itemBase != NULL) {
			result = itemBase->collectExtraInfo(propThing->m_name->parentWidget(), family, key, value, swappingEnabled, resultKey, resultValue, resultWidget);
		}

		if (result) {
			propThing->m_name->setText(resultKey);
			if (resultWidget) {
				propThing->m_layout->addWidget(resultWidget);
				propThing->m_plugin = resultWidget;
				propThing->m_value->setVisible(false);
			}
			else {
				propThing->m_value->setText(resultValue);
				propThing->m_value->setVisible(true);
			}
		}
		else {
			propThing->m_name->setText(translatedName);
			propThing->m_value->setText(value);
			propThing->m_value->setVisible(true);
		}

		propThing->m_name->setVisible(true);
		ix++;
	}

	for (int jx = ix; jx < m_propThings.count(); jx++) {
		PropThing * propThing = m_propThings.at(jx);
		propThing->m_name->setVisible(false);
		propThing->m_value->setVisible(false);
		clearPropThingPlugin(propThing);
	}

	/*
	foreach (PropThing * propThing, m_propThings) {
		if (propThing->m_layout->count() > 1) {
			DebugDialog::debug(QString("too many %1").arg(propThing->m_layout->count()));
		}
	}
	*/
}

void HtmlInfoView::viewExpanded(bool value) {
	QObject * view = sender();
	if (view == NULL) return;

	QString blockId = view->property("blockid").toString();
	if (!blockId.isEmpty()) {
		QSettings settings;
		settings.setValue(settingsBlockVisibilityName(blockId),QVariant::fromValue(value));
	}
}

void HtmlInfoView::clearPropThingPlugin(PropThing * propThing) 
{
	if (propThing->m_plugin) {
		propThing->m_layout->removeWidget(propThing->m_plugin);
		delete propThing->m_plugin;
		propThing->m_plugin = NULL;		
	}
}
