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

#include <QFrame>
#include <QBoxLayout>
#include <QLabel>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "toolhelp.h"

ToolHelp::ToolHelp(
		const QString &text,
		const QString imageName,
		QBoxLayout::Direction direction
	) : QGraphicsProxyWidget()
{
	QFrame *frame = new QFrame();
	frame->setObjectName("toolHelp"+imageName);
	QBoxLayout *layout = new QBoxLayout(direction, frame);
	layout->setSpacing(3);
	layout->setMargin(0);

	QLabel *label = new QLabel(frame);
	label->setText(text);
	label->setObjectName("toolHelpText"+imageName);

	QLabel *image = new QLabel(frame);
	image->setPixmap(QPixmap(QString(":/resources/images/inViewHelp%1Arrow.png").arg(imageName)));

	layout->addWidget(label);
	layout->addWidget(image);

	setWidget(frame);

	QFile styleSheet(":/resources/styles/inviewhelp.qss");
    if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/inviewhelp.qss");
	} else {
		frame->setStyleSheet(styleSheet.readAll());
	}
}

//QVariant ToolHelp::itemChange(GraphicsItemChange change, const QVariant &value) {
	//QPointF pos = value.toPointF();
	// Map the original button-down position back to local coordinates.
	//QPointF buttonDownPos = mapFromScene(pos);

	//newScale = scale * distanceToPoint(pos) / distanceToPoint(buttonDownPos);

	// Apply the new transformation
	//QMatrix matrix = scene()->views()[0]->matrix();
	//setMatrix(matrix.scale(1/matrix.m11(),1/matrix.m11()));
	//QGraphicsProxyWidget::itemChange(change,value);
//}

