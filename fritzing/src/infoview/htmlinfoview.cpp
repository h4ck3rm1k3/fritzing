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
#include <QFontMetricsF>
#include <qmath.h>

#include "htmlinfoview.h"
#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../connectors/connectorshared.h"
#include "../connectors/connector.h"
#include "../fsvgrenderer.h"
#include "../dockmanager.h"
#include "../utils/flineedit.h"


#define HTML_EOF "</body>\n</html>"

QPixmap * NoIcon = NULL;

const int HtmlInfoView::STANDARD_ICON_IMG_WIDTH = 32;
const int HtmlInfoView::STANDARD_ICON_IMG_HEIGHT = 32;
const int IconSpace = 0;

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


TagLabel::TagLabel(QWidget * parent) : QLabel(parent) 
{
}

QSize TagLabel::sizeHint() const
{
	QSize hint = QLabel::sizeHint();
	QString t = text();
	if (t.isEmpty()) return hint;

	QFontMetricsF fm(this->font());
	qreal textWidth = fm.width(t);

	QWidget * w = this->window();
	QPoint pos(0,0);
	pos = this->mapTo(w, pos);

	int myWidth = w->width() - pos.x() - 10;
	if (textWidth < myWidth) {
		return hint;
	}

	int lines = qCeil(textWidth / myWidth);
	int h = lines * fm.height();

	return QSize(myWidth, h);
}

//////////////////////////////////////

HtmlInfoView::HtmlInfoView(QWidget * parent) : QScrollArea(parent) 
{
    this->setWidgetResizable(true);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
	m_tagsTextLabel = NULL;
	m_lastSwappingEnabled = false;
	m_lastItemBase = NULL;
	m_infoGraphicsView = NULL;
	m_setContentTimer.setSingleShot(true);
	m_setContentTimer.setInterval(10);
	connect(&m_setContentTimer, SIGNAL(timeout()), this, SLOT(setContent()));
	QVBoxLayout *vlo = new QVBoxLayout(mainFrame);
	vlo->setSpacing(0);
	vlo->setContentsMargins(0, 0, 0, 0);
	vlo->setSizeConstraint( QLayout::SetMinAndMaxSize );

        /* Part Title */

	m_titleEdit = new FLineEdit(mainFrame);
	m_titleEdit->setObjectName("instanceTitleEditor");

	connect(m_titleEdit, SIGNAL(editingFinished()), this, SLOT(setInstanceTitle()));
	connect(m_titleEdit, SIGNAL(mouseEnter()), this, SLOT(instanceTitleEnter()));
	connect(m_titleEdit, SIGNAL(mouseLeave()), this, SLOT(instanceTitleLeave()));
	connect(m_titleEdit, SIGNAL(editable(bool)), this, SLOT(instanceTitleEditable(bool)));

	setInstanceTitleColors(m_titleEdit, QColor(0xaf, 0xaf, 0xb4), QColor(0x00, 0x00, 0x00)); //b3b3b3, 575757
	m_titleEdit->setAutoFillBackground(true);
	vlo->addWidget(m_titleEdit);

        /* Part Icons */

	if (NoIcon == NULL) {
		NoIcon = new QPixmap(":/resources/images/icons/noicon.png");
	}

	QFrame * iconFrame = new QFrame(mainFrame);
	iconFrame->setObjectName("iconFrame");

	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->addSpacing(IconSpace);
	m_icon1 = addLabel(hboxLayout, NoIcon);
	m_icon2 = addLabel(hboxLayout, NoIcon);
	m_icon3 = addLabel(hboxLayout, NoIcon);

	m_partVersion = new QLabel();
	m_partVersion->setObjectName("infoViewPartTitle");
	hboxLayout->addWidget(m_partVersion);
	
	m_locationUnits = NULL;
	m_location = NULL;

	/*
	hboxLayout->addSpacing(20);
	
	m_locationUnits = new QComboBox();
	m_location = new QLabel();

	m_locationUnits->addItem("in");
	m_locationUnits->addItem("mm");
	m_locationUnits->setCurrentIndex(0);
	m_locationUnits->setVisible(false);
	hboxLayout->addWidget(m_locationUnits);

	hboxLayout->addWidget(m_location);
	connect(m_locationUnits, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLocation()));
	*/

	hboxLayout->addSpacerItem(new QSpacerItem(IconSpace, 1, QSizePolicy::Expanding));
	iconFrame->setLayout(hboxLayout);
	vlo->addWidget(iconFrame);

	m_partTitle = new TagLabel(this);
	m_partTitle->setWordWrap(true);
	m_partTitle->setObjectName("infoViewPartTitle");
	vlo->addWidget(m_partTitle);

	m_proplabel = new QLabel(tr("Properties"), NULL);
	m_proplabel->setObjectName("expandableViewLabel");
	vlo->addWidget(m_proplabel);

	m_propFrame = new QFrame(this);
	m_propFrame->setObjectName("infoViewPropertyFrame");
	m_propLayout = new QGridLayout(m_propFrame);
	m_propLayout->setSpacing(0);
	m_propLayout->setContentsMargins(0, 0, 0, 0);
	m_propFrame->setLayout(m_propLayout);
	vlo->addWidget(m_propFrame);

	m_taglabel = new QLabel(tr("Tags"), NULL);
	m_taglabel->setObjectName("expandableViewLabel");
	vlo->addWidget(m_taglabel);

	m_tagsTextLabel = new TagLabel(this);
	m_tagsTextLabel->setWordWrap(true);
	m_tagsTextLabel->setObjectName("tagsValue");
	vlo->addWidget(m_tagsTextLabel);

	m_connLabel = new QLabel(tr("Connections"), NULL);
	m_connLabel->setObjectName("expandableViewLabel");
	vlo->addWidget(m_connLabel);

	m_connFrame = new QFrame(this);
	m_connFrame->setObjectName("connectionsFrame");

    QGridLayout * connLayout = new QGridLayout(m_connFrame);
	connLayout->setSpacing(0);
	connLayout->setContentsMargins(0, 0, 0, 0);
	m_connFrame->setLayout(connLayout);

	QLabel * descrLabel = new QLabel(tr("conn."), this);
	descrLabel->setObjectName("connectionsLabel");
	m_connDescr = new QLabel(this);
	m_connDescr->setObjectName("connectionsValue");
    connLayout->addWidget(descrLabel, 0, 0);
    connLayout->addWidget(m_connDescr, 0, 1);

	QLabel * nameLabel = new QLabel(tr("name"), this);
	nameLabel->setObjectName("connectionsLabel");
	m_connName = new QLabel(this);
	m_connName->setObjectName("connectionsValue");
    connLayout->addWidget(nameLabel, 1, 0);
    connLayout->addWidget(m_connName, 1, 1);

	QLabel * typeLabel = new QLabel(tr("type"), this);
	typeLabel->setObjectName("connectionsLabel");
	m_connType = new QLabel(this);
	m_connType->setObjectName("connectionsValue");
	connLayout->addWidget(typeLabel, 2, 0);
    connLayout->addWidget(m_connType, 2, 1);

	vlo->addWidget(m_connFrame);

	vlo->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

	mainFrame->setLayout(vlo);

	this->setWidget(mainFrame);

	m_currentItem = NULL;
	m_currentSwappingEnabled = false;

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
	}
	else {
		 nameString = modelPart->description();
	}
	partTitle(nameString, modelPart->version());

	setUpTitle(wire);
	setUpIcons(wire->modelPart());
	setUpLocation(swappingEnabled ? wire : NULL);

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

	setUpLocation(swappingEnabled ? itemBase : NULL);

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

	m_connFrame->setVisible(true);
	m_proplabel->setVisible(true);
	m_taglabel->setVisible(true);
	m_connLabel->setVisible(true);
	m_propFrame->setVisible(true);

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
	setUpLocation(NULL);
	m_connFrame->setVisible(false);
	m_propFrame->setVisible(false);
	m_proplabel->setVisible(false);
	m_taglabel->setVisible(false);
	m_connLabel->setVisible(false);
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
		setInstanceTitleColors(edit, QColor(0xeb, 0xeb, 0xee), QColor(0x00, 0x00, 0x00)); //c8c8c8, 575757
	}
}

void HtmlInfoView::instanceTitleLeave() {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (edit->isEnabled()) {
		setInstanceTitleColors(edit, QColor(0xd2, 0xd2, 0xd7), QColor(0x00, 0x00, 0x00)); //b3b3b3, 575757
	}
}

void HtmlInfoView::instanceTitleEditable(bool editable) {
	FLineEdit * edit = dynamic_cast<FLineEdit *>(sender());
	if (editable) {
		setInstanceTitleColors(edit, QColor(0xff, 0xff, 0xff), QColor(0x00, 0x00, 0x00)); //fcfcfc, 000000
	}
	else {
		setInstanceTitleColors(edit, QColor(0xd2, 0xd2, 0xd7), QColor(0x00, 0x00, 0x00)); //b3b3b3, 57575
	}
}

void HtmlInfoView::setInstanceTitleColors(FLineEdit * edit, const QColor & base, const QColor & text) {
	edit->setStyleSheet(QString("background: rgb(%1,%2,%3); color: rgb(%4,%5,%6);")
		.arg(base.red()).arg(base.green()).arg(base.blue())
		.arg(text.red()).arg(text.green()).arg(text.blue()) );
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
			QString error;
			ItemBase::setUpImage(modelPart, ViewIdentifierClass::IconView, ViewLayer::Icon, ViewLayer::ThroughHoleThroughTop_OneLayer, error);
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
	if (m_tagsTextLabel == NULL) return;

	if (m_lastTagsModelPart == modelPart) return;

	m_lastTagsModelPart = modelPart;

	if (modelPart == NULL || modelPart->tags().isEmpty()) {
		m_tagsTextLabel->setText("");
		return;
	}

	m_tagsTextLabel->setText(modelPart->tags().join(", "));
}

void HtmlInfoView::partTitle(const QString & title, const QString & version) {
	if (m_partTitle == NULL) return;

	if (m_lastPartTitle == title && m_lastPartVersion == version) return;

	m_lastPartTitle = title;
	m_lastPartVersion = version;

	m_partTitle->setText(title);
	if (!version.isEmpty()) {
		m_partVersion->setText(tr("v. %1").arg(version));
	}
	else m_partVersion->setText("");
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
	QString partNumber;
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

		// ensure part number  is last
		partNumber = properties.value(ModelPartShared::PartNumberPropertyName, "").toLower();
		keys.removeOne(ModelPartShared::PartNumberPropertyName);
		
#ifndef QT_NO_DEBUG
		properties.insert("id", QString("%1 %2 %3")
			.arg(itemBase ? QString::number(itemBase->id()) : "")
			.arg(modelPart->moduleID())
			.arg(itemBase ? ViewLayer::viewLayerNameFromID(itemBase->viewLayerID()) : "")
		);
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

	// ensure part number is last
	if (itemBase != NULL && itemBase->hasPartNumberProperty()) {
		keys.append(ModelPartShared::PartNumberPropertyName);
	}

	int ix = 0;
	foreach(QString key, keys) {
		if (ix >= m_propThings.count()) {
			PropThing * propThing = new PropThing;
			propThing->m_plugin = NULL;
			m_propThings.append(propThing);

			QLabel * propNameLabel = new QLabel(this);
			propNameLabel->setObjectName("propNameLabel");
			propNameLabel->setWordWrap(true);
			propThing->m_name = propNameLabel;
			m_propLayout->addWidget(propNameLabel, ix, 0);

			QFrame * valueFrame = new QFrame(this);
			valueFrame->setObjectName("valueFrame");
			QVBoxLayout * vlayout = new QVBoxLayout(valueFrame);
			vlayout->setSpacing(0);
			vlayout->setContentsMargins(0, 0, 0, 0);
			propThing->m_layout = vlayout;

			QLabel * propValueLabel = new QLabel(valueFrame);
			propValueLabel->setObjectName("propValueLabel");
			propValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
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

void HtmlInfoView::setUpLocation(ItemBase * itemBase) {
	if (m_location == NULL) return;
	if (m_locationUnits == NULL) return;

	if (itemBase == NULL) {
		m_location->setText("");
		m_locationUnits->setVisible(false);
		return;
	}

	m_locationUnits->setVisible(true);
	QPointF p = itemBase->pos();
	qreal x = p.x() / FSvgRenderer::printerScale();
	qreal y = p.y() / FSvgRenderer::printerScale();

	qreal units = (m_locationUnits->currentText().compare("in") == 0) ? 1.0 : 25.4;

	if (itemBase->itemType() == ModelPart::Wire) {
		Wire * wire = qobject_cast<Wire *>(itemBase);
		QPointF q = p + wire->line().p2();
		m_location->setText(tr("x:%1 %2\ny:%3 %4")
			.arg(x * units, 0, 'f', 3)
			.arg(q.x() * units / FSvgRenderer::printerScale(), 0, 'f', 3)
			.arg(y * units, 0, 'f', 3)
			.arg(q.y() * units / FSvgRenderer::printerScale(), 0, 'f', 3)
		);
		return;
	}

	m_location->setText(tr("x:%1\ny:%2")
		.arg(x * units, 0, 'f', 3)
		.arg(y * units, 0, 'f', 3)
	);
}

void HtmlInfoView::updateLocation() {
	setUpLocation(m_currentItem);
}
