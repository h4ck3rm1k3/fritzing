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
#include "peconnectorsview.h"
#include "../utils/textutils.h"
#include "../utils/graphicsutils.h"

#include <QHBoxLayout>
#include <QTextStream>
#include <QSplitter>
#include <QPushButton>
#include <QLineEdit>

static const int TheSpacing = 10;

//////////////////////////////////////

PEDoubleSpinBox::PEDoubleSpinBox(QWidget * parent) : QDoubleSpinBox(parent)
{
}

void PEDoubleSpinBox::stepBy(int steps)
{
    double amount;
    emit getSpinAmount(amount);
    setSingleStep(amount);
    QDoubleSpinBox::stepBy(steps);
}

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

    QLabel * label = new QLabel(tr("Connector List"));
    connectorsLayout->addWidget(label);

    m_connectorListWidget = new QListWidget();
	connect(m_connectorListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(switchConnector(QListWidgetItem *, QListWidgetItem *)));

    connectorsLayout->addWidget(m_connectorListWidget);

    connectorsFrame->setLayout(connectorsLayout);
    splitter->addWidget(connectorsFrame);

    m_connectorInfoGroupBox = new QGroupBox;
    m_connectorInfoLayout = new QVBoxLayout;

    m_connectorInfoWidget = new QFrame;
    m_connectorInfoLayout->addWidget(m_connectorInfoWidget);

    m_svgElement = new QLabel;
    m_svgElement->setWordWrap(false);
    m_svgElement->setTextFormat(Qt::PlainText);
    m_svgElement->setMaximumWidth(400);
    m_connectorInfoLayout->addWidget(m_svgElement);

    m_elementLock = new QCheckBox(tr("SVG Element Locked"));
    m_elementLock->setChecked(true);
    m_elementLock->setToolTip(tr("Unlock to modify the current connector's location"));
    connect(m_elementLock, SIGNAL(clicked(bool)), this, SLOT(lockChangedSlot(bool)));
    m_connectorInfoLayout->addWidget(m_elementLock);



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
    m_connectorInfoLayout->addWidget(boundsFrame);

    QGroupBox * anchorGroupBox = new QGroupBox("Anchor point");
    QVBoxLayout * anchorGroupLayout = new QVBoxLayout;

    QFrame * posRadioFrame = new QFrame;
    QHBoxLayout * posRadioLayout = new QHBoxLayout;

    QList<QString> positionNames;
    positionNames << "Center" << "N" << "E" << "S" << "W";
    QList<QString> trPositionNames;
    trPositionNames << tr("Center") << tr("N") << tr("E") << tr("S") << tr("W");
    for (int i = 0; i < positionNames.count(); i++) {
        QPushButton * button = new QPushButton(trPositionNames.at(i));
        button->setProperty("how", positionNames.at(i));
        connect(button, SIGNAL(clicked()), this, SLOT(buttonChangeAnchor()));
        posRadioLayout->addWidget(button);
        m_buttons.append(button);
    }

    posRadioLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    posRadioFrame->setLayout(posRadioLayout);
    anchorGroupLayout->addWidget(posRadioFrame);

    QFrame * posNumberFrame = new QFrame;
    QHBoxLayout * posNumberLayout = new QHBoxLayout;

    label = new QLabel("x");
    posNumberLayout->addWidget(label);

    m_terminalPointX = new PEDoubleSpinBox;
    m_terminalPointX->setDecimals(4);
    posNumberLayout->addWidget(m_terminalPointX);
    connect(m_terminalPointX, SIGNAL(getSpinAmount(double &)), this, SLOT(getSpinAmountSlot(double &)), Qt::DirectConnection);
    connect(m_terminalPointX, SIGNAL(valueChanged(double)), this, SLOT(anchorPointEntry()));
    connect(m_terminalPointX, SIGNAL(valueChanged(const QString &)), this, SLOT(anchorPointEntry()));

    posNumberLayout->addSpacing(TheSpacing);

    label = new QLabel("y");
    posNumberLayout->addWidget(label);

    m_terminalPointY = new PEDoubleSpinBox;
    m_terminalPointY->setDecimals(4);
    posNumberLayout->addWidget(m_terminalPointY);
    connect(m_terminalPointY, SIGNAL(getSpinAmount(double &)), this, SLOT(getSpinAmountSlot(double &)), Qt::DirectConnection);
    connect(m_terminalPointY, SIGNAL(valueChanged(double)), this, SLOT(anchorPointEntry()));
    connect(m_terminalPointY, SIGNAL(valueChanged(double)), this, SLOT(anchorPointEntry()));

    posNumberLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    posNumberFrame->setLayout(posNumberLayout);
    anchorGroupLayout->addWidget(posNumberFrame);

    anchorGroupBox->setLayout(anchorGroupLayout);
    m_connectorInfoLayout->addWidget(anchorGroupBox);

    QFrame * radioFrame = new QFrame;
    QHBoxLayout * radioLayout = new QHBoxLayout;

    QStringList radioNames;
    radioNames << "in" << "mm" << "px";
    foreach (QString name, radioNames) {
        QRadioButton * radioButton = new QRadioButton(name);
        connect(radioButton, SIGNAL(clicked()), this, SLOT(changeUnits()));
        radioLayout->addWidget(radioButton);
        radioLayout->addSpacing(TheSpacing);
        radioButton->setChecked(radioButton->text().compare(m_units) == 0);
    }

    radioLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    radioFrame->setLayout(radioLayout);
    m_connectorInfoLayout->addWidget(radioFrame);

	m_connectorInfoLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_connectorInfoGroupBox->setLayout(m_connectorInfoLayout);

	splitter->addWidget(m_connectorInfoGroupBox);

	//this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//this->setWidget(splitter);

    this->setLayout(mainLayout);

    m_connectorListWidget->resize(m_connectorListWidget->width(), 0);

    enableChanges(false);
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
        enableChanges(false);
        return;
    }

    QString string;
    QTextStream stream(&string);
    pegi->element().save(stream, 0);
    string = TextUtils::killXMLNS(string);
    string.replace("\n", " ");
    m_svgElement->setText(string);
    QPointF p = pegi->offset();
    m_x->setText(convertUnitsStr(p.x()));
    m_y->setText(convertUnitsStr(p.y()));
    QRectF r = pegi->rect();
    m_width->setText(convertUnitsStr(r.width()));
    m_height->setText(convertUnitsStr(r.height()));

    enableChanges(!m_elementLock->isChecked());
}

void PEToolView::enableChanges(bool enabled)
{
    foreach (QPushButton * button, m_buttons) button->setEnabled(enabled);
    m_terminalPointX->setEnabled(enabled);
    m_terminalPointY->setEnabled(enabled);

}

void PEToolView::changeUnits() {
    QRadioButton * radio = qobject_cast<QRadioButton *>(sender());
    if (radio == NULL) return;

    m_units = radio->text();
    highlightElement(m_pegi);

}

QString PEToolView::convertUnitsStr(double val)
{
    return QString::number(convertUnits(val));
}

double PEToolView::convertUnits(double val)
{
    if (m_units.compare("in") == 0) {
        return val / GraphicsUtils::SVGDPI;
    }
    else if (m_units.compare("mm") == 0) {
        return val * 25.4 / GraphicsUtils::SVGDPI;
    }

    return val;
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

    int pos = 99999;
    for (int ix = 0; ix < m_connectorInfoLayout->count(); ix++) {
        QLayoutItem * item = m_connectorInfoLayout->itemAt(ix);
        if (item->widget() == m_elementLock) {
            pos = ix - 1;
            break;
        }
    }

    m_connectorInfoWidget = PEConnectorsView::makeConnectorForm(element, m_gotZeroConnector, index, this, false);
    m_connectorInfoLayout->insertWidget(pos, m_connectorInfoWidget);
    m_connectorInfoGroupBox->setTitle(tr("Connector %1").arg(element.attribute("name")));

    emit switchedConnector(element);
}

void PEToolView::setLock(bool lock) {
    if (m_elementLock) {
        m_elementLock->setChecked(lock);
        enableChanges(!lock);
    }
}

void PEToolView::lockChangedSlot(bool state)
{
    enableChanges(!state);
    emit lockChanged(state);
}

void PEToolView::nameEntry() {
}

void PEToolView::typeEntry() {
}

void PEToolView::descriptionEntry() {

}

QDomElement PEToolView::currentConnector() {
    QListWidgetItem * item = m_connectorListWidget->currentItem();
    int index = item->data(Qt::UserRole).toInt();
    return m_connectorList.at(index);
}

void PEToolView::setTerminalPointCoords(QPointF p) {
    m_terminalPointX->setValue(convertUnits(p.x()));
    m_terminalPointY->setValue(convertUnits(p.y()));
}

void PEToolView::setTerminalPointLimits(QSizeF sz) {
    m_terminalPointX->setRange(0, sz.width());
    m_terminalPointY->setRange(0, sz.height());
}

void PEToolView::buttonChangeAnchor() {
    QString how = sender()->property("how").toString();
    emit terminalPointChanged(how);
}

void PEToolView::anchorPointEntry()
{
    if (sender() == m_terminalPointX) {
        emit terminalPointChanged("x", m_terminalPointX->value());
    }
    else if (sender() == m_terminalPointY) {
       emit terminalPointChanged("y", m_terminalPointY->value());
    }
}

void PEToolView::getSpinAmountSlot(double & d) {
    emit getSpinAmount(d);
}
