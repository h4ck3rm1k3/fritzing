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
#include "../utils/textutils.h"
#include "../utils/graphicsutils.h"

#include <QHBoxLayout>
#include <QTextStream>

static const int TheSpacing = 10;
//////////////////////////////////////

PEToolView::PEToolView(QWidget * parent) : QScrollArea(parent) 
{
    m_units = "px";
    m_pegi = NULL;

	QFrame * frame = new QFrame(this);
	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );

    QGroupBox * groupBox = new QGroupBox("element");
	QVBoxLayout * groupLayout = new QVBoxLayout;

    m_svgElement = new QLabel;
    m_svgElement->setWordWrap(false);
    m_svgElement->setTextFormat(Qt::PlainText);
    m_svgElement->setMaximumWidth(400);
    groupLayout->addWidget(m_svgElement);

    QFrame * boundsFrame = new QFrame;
    QHBoxLayout * boundsLayout = new QHBoxLayout;

    QLabel * label = new QLabel("x");
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
    groupLayout->addWidget(boundsFrame);

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
    groupLayout->addWidget(radioFrame);

	
	groupBox->setLayout(groupLayout);
	mainLayout->addWidget(groupBox);

	frame->setLayout(mainLayout);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setWidget(frame);
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
