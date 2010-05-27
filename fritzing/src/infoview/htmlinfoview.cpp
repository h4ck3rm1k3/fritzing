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

QHash<QString, QPixmap *> HtmlInfoView::m_pixmaps;

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

	m_lastTitleItemBase = NULL;
	m_lastTagsModelPart = NULL;
	m_lastConnectorItem = NULL;
	m_lastIconModelPart = NULL;
	m_lastPropsModelPart = NULL;
	m_lastPropsItemBase = NULL;

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
	iconFrame->setLayout(hboxLayout);
	vlo->addWidget(iconFrame);

	QFrame * tFrame = new QFrame(mainFrame);
	tFrame->setObjectName("partTitleFrame");
	hboxLayout = new QHBoxLayout();
	hboxLayout->setContentsMargins (0, 0, 0, 0);
	m_partTitle = new QLabel(tFrame);
	m_partTitle->setObjectName("partTitle");
	hboxLayout->addWidget(m_partTitle);
	hboxLayout->addSpacerItem(new QSpacerItem(IconSpace, 1, QSizePolicy::Expanding));
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
	pev->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
	vlo->addWidget(pev);

	ExpandableView * tev = new ExpandableView(tr("Tags"), this);
	tev->setProperty("blockid", TagsBlockId);
	connect(tev, SIGNAL(expanded(bool)), this, SLOT(viewExpanded(bool)));
	m_tagsLabel = new QLabel(this);
	tev->setChildFrame(m_tagsLabel);
	tev->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
	vlo->addWidget(tev);


	ExpandableView * cev = new ExpandableView(tr("Connections"), this);
	cev->setProperty("blockid", ConnsBlockId);
	connect(cev, SIGNAL(expanded(bool)), this, SLOT(viewExpanded(bool)));
	QFrame * connFrame = new QFrame(this);
        QGridLayout * connLayout = new QGridLayout(connFrame);
	connLayout->setMargin(0);
	connFrame->setLayout(connLayout);

	QLabel * descrLabel = new QLabel(tr("conn."), this);
	descrLabel->setObjectName("connectionsLabel");
	m_connDescr = new QLabel(this);
        connLayout->addWidget(descrLabel, 0, 0);
        connLayout->addWidget(m_connDescr, 0, 1);

	QLabel * nameLabel = new QLabel(tr("name"), this);
	nameLabel->setObjectName("connectionsLabel");
	m_connName = new QLabel(this);
        connLayout->addWidget(nameLabel, 1, 0);
        connLayout->addWidget(m_connName, 1, 1);

	QLabel * typeLabel = new QLabel(tr("type"), this);
	typeLabel->setObjectName("connectionsLabel");
	m_connType = new QLabel(this);
        connLayout->addWidget(typeLabel, 2, 0);
        connLayout->addWidget(m_connType, 2, 1);

	cev->setChildFrame(connFrame);
	cev->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

	vlo->addWidget(cev);

	vlo->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

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
	foreach (PropThing * propThing, m_propThings) {
		delete propThing;
	}
	m_propThings.clear();

	foreach (QPixmap * pixmap, m_pixmaps.values()) {
		delete pixmap;
	}
	m_pixmaps.clear();
}

void HtmlInfoView::cleanup() {
	if (NoIcon) {
		delete NoIcon;
		NoIcon = NULL;
	}
}

void HtmlInfoView::viewItemInfo(InfoGraphicsView * infoGraphicsView, ItemBase* item, bool swappingEnabled) 
{
	m_setContentTimer.stop();
	m_pendingInfoGraphicsView = infoGraphicsView;
	m_lastItemBase = m_pendingItemBase = item;
	m_lastSwappingEnabled = m_pendingSwappingEnabled = swappingEnabled;
	m_setContentTimer.start();
}

void HtmlInfoView::hoverEnterItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * item, bool swappingEnabled) {
	m_setContentTimer.stop();
	m_pendingInfoGraphicsView = infoGraphicsView;
	m_pendingItemBase = item;
	m_pendingSwappingEnabled = swappingEnabled;
	m_setContentTimer.start();
}

void HtmlInfoView::hoverLeaveItem(InfoGraphicsView * infoGraphicsView, QGraphicsSceneHoverEvent *, ItemBase * itemBase){
	Q_UNUSED(itemBase);
	//DebugDialog::debug(QString("hoverLeaveItem itembase %1").arg(itemBase ? itemBase->instanceTitle() : "NULL"));
	m_setContentTimer.stop();
	m_pendingInfoGraphicsView = infoGraphicsView;
	m_pendingItemBase = m_lastItemBase;
	m_pendingSwappingEnabled = m_lastSwappingEnabled;
	m_setContentTimer.start();
}

void HtmlInfoView::viewConnectorItemInfo(ConnectorItem * connectorItem) {

	int count = connectorItem ? connectorItem->connectionsCount() : 0;
	if (m_lastConnectorItem == connectorItem && m_lastConnectorItemCount == count) return;

	m_lastConnectorItem = connectorItem;
	m_lastConnectorItemCount = count;

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
	Q_UNUSED(swappingEnabled)
	Q_UNUSED(igv)
	viewConnectorItemInfo(item);
}

void HtmlInfoView::hoverLeaveConnectorItem(InfoGraphicsView *igv, QGraphicsSceneHoverEvent *event, ConnectorItem *connItem) {
	Q_UNUSED(event);
	Q_UNUSED(connItem);
	Q_UNUSED(igv);
	viewConnectorItemInfo(NULL);
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
	if (swappingEnabled) {
		if (wire->getRatsnest()) {
			nameString = tr("Rat's nest wire");
		} 
		else if(wire->getTrace()) {
			nameString = tr("Trace wire %1").arg(autoroutable);
		} 
		else if(wire->getJumper()) {
			nameString = tr("Jumper wire %1").arg(autoroutable);
		}
	}
	else {
		 nameString = modelPart->description();
	}
	partTitle(nameString, modelPart->version());

	setUpTitle(wire);
	setUpIcons(wire->modelPart());
	m_location->setText(QString("(%1,%2)").arg(wire->pos().x()).arg(wire->pos().y()));

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

	QString nameString;
	if (swappingEnabled) {
		nameString = (itemBase) ? itemBase->title() : modelPart->title();
	}
	else {
		nameString = modelPart->description();
	}
	partTitle(nameString, modelPart->version());

	displayProps(modelPart, itemBase, swappingEnabled);
	addTags(modelPart);
}

void HtmlInfoView::setContent()
{
	m_setContentTimer.stop();
	//DebugDialog::debug(QString("start updating %1").arg(QTime::currentTime().toString("HH:mm:ss.zzz")));
	if (m_pendingItemBase == NULL) {
		setNullContent();
		m_setContentTimer.stop();
		return;
	}

	//DebugDialog::debug(QString("pending %1").arg(m_pendingItemBase->title()));
	m_currentSwappingEnabled = m_pendingSwappingEnabled;

	appendStuff(m_pendingItemBase, m_pendingSwappingEnabled);
	setCurrentItem(m_pendingItemBase);
	m_infoGraphicsView = m_pendingInfoGraphicsView;

	m_setContentTimer.stop();
	//DebugDialog::debug(QString("end   updating %1").arg(QTime::currentTime().toString("HH:mm:ss.zzz")));

}

QSize HtmlInfoView::sizeHint() const {
	return QSize(DockManager::DockDefaultWidth, DockManager::InfoViewDefaultHeight);
}

void HtmlInfoView::setCurrentItem(ItemBase * item) {
	m_currentItem = item;
}

void HtmlInfoView::unregisterCurrentItem() {
        m_setContentTimer.stop();
	setCurrentItem(NULL);
        m_pendingItemBase = NULL;
        m_setContentTimer.start();
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
	if (itemBase == m_lastTitleItemBase) return;

	m_lastTitleItemBase = itemBase;
	if (itemBase) {
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
	if (m_lastIconModelPart == modelPart) return;
	
	m_lastIconModelPart = modelPart;

	QPixmap *pixmap0 = NULL;
	QPixmap *pixmap1 = NULL;
	QPixmap *pixmap2 = NULL;
	QPixmap *pixmap3 = NULL;

	QSize size = NoIcon->size();

	if (modelPart != NULL) {
		pixmap0 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
		pixmap1 = getPixmap(modelPart, ViewIdentifierClass::BreadboardView);
		pixmap2 = getPixmap(modelPart, ViewIdentifierClass::SchematicView);
		pixmap3 = getPixmap(modelPart, ViewIdentifierClass::PCBView);
		if (pixmap0 == NULL) {
			ItemBase::setUpImage(modelPart, ViewIdentifierClass::IconView, ViewLayer::Icon);
			pixmap0 = FSvgRenderer::getPixmap(modelPart->moduleID(), ViewLayer::Icon, size);
		}
	}

	QPixmap* use1 = pixmap1;
	QPixmap* use2 = pixmap2;
	QPixmap* use3 = pixmap3;

	// use the icon image instead of the breadboard image, unless the item doesn't have a breadboard view
	if (pixmap1 && pixmap2 && pixmap3) {
		use1 = pixmap0;
		if (pixmap2 == pixmap1) {
			use2 = pixmap0;
		}
		if (pixmap3 == pixmap1) {
			use3 = pixmap0;
		}
	}
	else if (pixmap3) {
		use3 = pixmap0;
	}
	else if (pixmap1) {
		use1 = pixmap0;
	}
	else if (pixmap2) {
		use2 = pixmap0;
	}

	if (use1 == NULL) use1 = NoIcon;
	if (use2 == NULL) use2 = NoIcon;
	if (use3 == NULL) use3 = NoIcon;

	m_icon1->setPixmap(*use1);
	m_icon2->setPixmap(*use2);
	m_icon3->setPixmap(*use3);

	if (pixmap0) delete pixmap0;
}

void HtmlInfoView::addTags(ModelPart * modelPart) {
	if (m_tagsLabel == NULL) return;

	if (m_lastTagsModelPart == modelPart) return;

	m_lastTagsModelPart = modelPart;

	if (modelPart == NULL || modelPart->tags().isEmpty()) {
		m_tagsLabel->setText("");
		return;
	}

	m_tagsLabel->setText(modelPart->tags().join(", "));
}

void HtmlInfoView::partTitle(const QString & title, const QString & version) {
	if (m_partTitle == NULL) return;

	if (m_lastPartTitle == title && m_lastPartVersion == version) return;

	m_lastPartTitle = title;
	m_lastPartVersion = version;

	m_partTitle->setText(title);
	m_partVersion->setText(version);

}

void HtmlInfoView::displayProps(ModelPart * modelPart, ItemBase * itemBase, bool swappingEnabled) 
{
	bool repeatPossible = (modelPart == m_lastPropsModelPart && itemBase == m_lastPropsItemBase && swappingEnabled == m_lastPropsSwappingEnabled);
	if (repeatPossible && modelPart == NULL && itemBase == NULL) {
		DebugDialog::debug("display props bail");
		return;
	}

	m_propLayout->setEnabled(false);

	if (repeatPossible) {
		DebugDialog::debug(QString("repeat possible %1").arg(repeatPossible));
	}

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

		QWidget * oldPlugin = propThing->m_plugin;
		propThing->m_plugin = NULL;

		QString value = properties.value(key,"");
		QString translatedName = ItemBase::translatePropertyName(key);
		QString resultKey, resultValue;
		QWidget * resultWidget = oldPlugin;
		bool result = false;
		if (itemBase != NULL) {
			result = itemBase->collectExtraInfo(propThing->m_name->parentWidget(), family, key, value, swappingEnabled, resultKey, resultValue, resultWidget);
		}

		QString newName;
		QString newValue;
		QWidget * newWidget = NULL;
		if (result) {
			newName = resultKey;
			if (resultWidget) {
				newWidget = resultWidget;
				if (resultWidget != oldPlugin) {
					propThing->m_layout->addWidget(resultWidget);
				}
				else {
					oldPlugin = NULL;
				}
                //DebugDialog::debug(QString("adding %1 %2").arg(newName).arg((long) resultWidget, 0, 16));
				propThing->m_plugin = resultWidget;
			}
			else {
				newValue = resultValue;
			}
		}
		else {
			newName = translatedName;
			newValue = value;
		}
		
		if (oldPlugin) {
			clearPropThingPlugin(propThing, oldPlugin);
		}

		if (propThing->m_name->text().compare(newName) != 0) {
			propThing->m_name->setText(newName);
		}
		propThing->m_name->setVisible(true);

		if (newWidget == NULL && propThing->m_value->text().compare(newValue) != 0) {
			propThing->m_value->setText(newValue);
		}
		propThing->m_value->setVisible(newWidget == NULL);
		ix++;
	}

	for (int jx = ix; jx < m_propThings.count(); jx++) {
		PropThing * propThing = m_propThings.at(jx);
		propThing->m_name->setVisible(false);
		propThing->m_value->setVisible(false);
		if (propThing->m_plugin) {
			propThing->m_plugin->setVisible(false);
		}
	}

	m_propLayout->setEnabled(true);


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
		clearPropThingPlugin(propThing, propThing->m_plugin);
		propThing->m_plugin = NULL;		
	}
}

void HtmlInfoView::clearPropThingPlugin(PropThing * propThing, QWidget * plugin) 
{
    //DebugDialog::debug(QString("clearing %1").arg((long) plugin, 0, 16));

	propThing->m_layout->removeWidget(plugin);
	plugin->setVisible(false);
	plugin->deleteLater();
}


QPixmap * HtmlInfoView::getPixmap(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	if (!modelPart->hasViewFor(viewIdentifier)) return NULL;

	QString baseName = modelPart->hasBaseNameFor(viewIdentifier);
	if (baseName.isEmpty()) return NULL;

	QString filename = ItemBase::getSvgFilename(modelPart->modelPartShared(), baseName);
	if (filename.isEmpty()) return NULL;

	QPixmap * cached = m_pixmaps.value(filename, NULL);
	if (cached) {
		return cached;
	}

	QSvgRenderer renderer(filename);

	QSize size = NoIcon->size();
	QPixmap * pixmap = new QPixmap(size);
	pixmap->fill(Qt::transparent);
	QPainter painter(pixmap);
	// preserve aspect ratio
	QSize def = renderer.defaultSize();
	qreal newW = size.width();
	qreal newH = newW * def.height() / def.width();
	if (newH > size.height()) {
		newH = size.height();
		newW = newH * def.width() / def.height();
	}
	QRectF bounds((size.width() - newW) / 2.0, (size.height() - newH) / 2.0, newW, newH);
	renderer.render(&painter, bounds);
	painter.end();

	m_pixmaps.insert(filename, pixmap);

	return pixmap;
}
