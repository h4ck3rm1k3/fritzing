/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "petoolview.h"
#include "pegraphicsitem.h"
#include "connectorsview.h"
#include "../utils/textutils.h"
#include "../utils/graphicsutils.h"

#include <QHBoxLayout>
#include <QTextStream>
#include <QSplitter>
#include <QPushButton>

static const int TheSpacing = 10;

//////////////////////////////////////

PEToolView::PEToolView(QWidget * parent) : QWidget(parent) 
{
    this->setObjectName("PEToolView");

    m_units = "px";
    m_pegi = NULL;

    QVBoxLayout * mainLayout = new QVBoxLayout;

    QSplitter * splitter = new QSplitter(Qt::Vertical);
    mainLayout->addWidget(splitter);

    QFrame * connectorsFrame = new QFrame;
    QVBoxLayout * connectorsLayout = new QVBoxLayout;

    QPushButton * button = new QPushButton(tr("Load image..."));
    connectorsLayout->addWidget(button);
    connect(button, SIGNAL(pressed()), this, SLOT(loadImageSlot()));

    QLabel * label = new QLabel(tr("Connector List"));
    connectorsLayout->addWidget(label);

    m_connectorListWidget = new QListWidget();
	connect(m_connectorListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(switchConnector(QListWidgetItem *, QListWidgetItem *)));

    connectorsLayout->addWidget(m_connectorListWidget);

    connectorsFrame->setLayout(connectorsLayout);
    splitter->addWidget(connectorsFrame);

    QFrame * connectorFrame = new QFrame;
    QVBoxLayout * connectorLayout = new QVBoxLayout;

    m_connectorInfoGroupBox = new QGroupBox;
    m_connectorInfoLayout = new QVBoxLayout;
    m_connectorInfoWidget = new QFrame;
    m_connectorInfoLayout->addWidget(m_connectorInfoWidget);
    m_connectorInfoGroupBox->setLayout(m_connectorInfoLayout);
    connectorLayout->addWidget(m_connectorInfoGroupBox);

    QGroupBox * svgGroupBox = new QGroupBox("SVG Element Info");
	QVBoxLayout * svgGroupLayout = new QVBoxLayout;

    m_elementLock = new QCheckBox(tr("Locked"));
    m_elementLock->setChecked(true);
    m_elementLock->setToolTip(tr("Unlock to modify the current connector's location"));
    connect(m_elementLock, SIGNAL(clicked(bool)), this, SLOT(lockChangedSlot(bool)));
    svgGroupLayout->addWidget(m_elementLock);

    m_svgElement = new QLabel;
    m_svgElement->setWordWrap(false);
    m_svgElement->setTextFormat(Qt::PlainText);
    m_svgElement->setMaximumWidth(400);
    svgGroupLayout->addWidget(m_svgElement);

    QFrame * boundsFrame = new QFrame;
    QHBoxLayout * boundsLayout = new QHBoxLayout;

    label = new QLabel("x");
    boundsLayout->addWidget(label);
    m_x = new QLabel;
    boundsLayout->addWidget(m_x);
    boundsLayout->addSpacing(TheSpacing);

    label = new QLabel("y");
    boundsLayout->addWidget(label);
    m_y = new QLabel;
    boundsLayout->addWidget(m_y);
    boundsLayout->addSpacing(TheSpacing);

    label = new QLabel(tr("width"));
    boundsLayout->addWidget(label);
    m_width = new QLabel;
    boundsLayout->addWidget(m_width);
    boundsLayout->addSpacing(TheSpacing);

    label = new QLabel(tr("height"));
    boundsLayout->addWidget(label);
    m_height = new QLabel;
    boundsLayout->addWidget(m_height);

    boundsLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    boundsFrame->setLayout(boundsLayout);
    svgGroupLayout->addWidget(boundsFrame);

    QFrame * radioFrame = new QFrame;
    QHBoxLayout * radioLayout = new QHBoxLayout;

    m_in = new QRadioButton("in");
    connect(m_in, SIGNAL(clicked()), this, SLOT(changeUnits()));
    radioLayout->addWidget(m_in);
    radioLayout->addSpacing(TheSpacing);

    m_mm = new QRadioButton("mm");
    connect(m_mm, SIGNAL(clicked()), this, SLOT(changeUnits()));
    radioLayout->addWidget(m_mm);
    radioLayout->addSpacing(TheSpacing);

    m_px = new QRadioButton("px");
    connect(m_px, SIGNAL(clicked()), this, SLOT(changeUnits()));
    m_px->setChecked(true);
    radioLayout->addWidget(m_px);

    radioLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    radioFrame->setLayout(radioLayout);
    svgGroupLayout->addWidget(radioFrame);
	
	svgGroupBox->setLayout(svgGroupLayout);

    connectorLayout->addWidget(svgGroupBox);
    connectorLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    connectorFrame->setLayout(connectorLayout);

	splitter->addWidget(connectorFrame);

	//this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//this->setWidget(splitter);

    this->setLayout(mainLayout);
}

PEToolView::~PEToolView() 
{
}

void PEToolView::highlightElement(PEGraphicsItem * pegi) {
    m_pegi = pegi;
    if (pegi == NULL) {
        m_svgElement->setText("");
        m_x->setText("");
        m_y->setText("");
        m_width->setText("");
        m_height->setText("");
        return;
    }

    QString string;
    QTextStream stream(&string);
    pegi->element().save(stream, 0);
    string = TextUtils::killXMLNS(string);
    string.replace("\n", " ");
    m_svgElement->setText(string);
    QPointF p = pegi->offset();
    m_x->setText(convertUnits(p.x()));
    m_y->setText(convertUnits(p.y()));
    QRectF r = pegi->rect();
    m_width->setText(convertUnits(r.width()));
    m_height->setText(convertUnits(r.height()));
}

void PEToolView::changeUnits() {
    QRadioButton * radio = qobject_cast<QRadioButton *>(sender());
    if (radio == NULL) return;

    m_units = radio->text();
    highlightElement(m_pegi);
}

QString PEToolView::convertUnits(double val)
{
    if (m_units.compare("in") == 0) {
        return QString::number(val / GraphicsUtils::SVGDPI);
    }
    else if (m_units.compare("mm") == 0) {
        return QString::number(val * 25.4 / GraphicsUtils::SVGDPI);
    }

    return QString::number(val);
}

void PEToolView::initConnectors(QList<QDomElement> & connectorList, bool gotZeroConnector) {
    m_gotZeroConnector = gotZeroConnector;
    m_connectorListWidget->clear();  // deletes QListWidgetItems
    m_connectorList = connectorList;

    int ix = 0;
    foreach (QDomElement connector, connectorList) {
		QListWidgetItem *item = new QListWidgetItem;
		item->setData(Qt::DisplayRole, connector.attribute("name"));
		item->setData(Qt::UserRole, ix++);
		m_connectorListWidget->addItem(item);
    }

    if (m_connectorListWidget->count() > 0) {
        m_connectorListWidget->setCurrentRow(0);
    }
}

void PEToolView::switchConnector(QListWidgetItem * current, QListWidgetItem * previous) {
    Q_UNUSED(previous);

    if (m_connectorInfoWidget) {
        delete m_connectorInfoWidget;
        m_connectorInfoWidget = NULL;
    }

    if (current == NULL) return;

    int index = current->data(Qt::UserRole).toInt();
    QDomElement element = m_connectorList.at(index);

    m_connectorInfoWidget = ConnectorsView::makeConnectorForm(element, m_gotZeroConnector, index, this, false);
    m_connectorInfoLayout->addWidget(m_connectorInfoWidget);
    m_connectorInfoGroupBox->setTitle(tr("Connector %1").arg(element.attribute("name")));

    emit switchedConnector(element);
}

void PEToolView::loadImageSlot() {
    emit loadImage();
}

void PEToolView::setLock(bool lock) {
    if (m_elementLock) m_elementLock->setChecked(lock);
}

void PEToolView::lockChangedSlot(bool state)
{
    emit lockChanged(state);
}

void PEToolView::nameEntry() {
}

void PEToolView::typeEntry() {
}

void PEToolView::descriptionEntry() {

}