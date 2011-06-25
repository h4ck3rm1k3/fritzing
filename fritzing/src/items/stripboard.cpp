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

#include "stripboard.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "moduleidnames.h"
#include "../connectors/connectoritem.h"

/////////////////////////////////////////////////////////////////////

Stripbit::Stripbit(const QPainterPath & path, QGraphicsItem * parent = 0) : QGraphicsPathItem(path, parent)
{
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemIsSelectable, false);

}

Stripbit::~Stripbit() {
}

/////////////////////////////////////////////////////////////////////

Stripboard::Stripboard( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: Perfboard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
}

Stripboard::~Stripboard() {
}

void Stripboard::setProp(const QString & prop, const QString & value) 
{
	Perfboard::setProp(prop, value);
}

QString Stripboard::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi) 
{
	return Perfboard::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}


QString Stripboard::genFZP(const QString & moduleid)
{
	QString fzp = Perfboard::genFZP(moduleid);
	fzp.replace("perfboard", "stripboard");
	fzp.replace("Perfboard", "Stripboard");
	fzp.replace("stripboard.svg", "perfboard.svg");
	fzp.replace("Stripboard.svg", "Perfboard.svg");
	return fzp;
}

bool Stripboard::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	return Perfboard::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Stripboard::addedToScene()
{
    Perfboard::addedToScene();
	if (this->scene() == NULL) return;

	QList<QGraphicsItem *> items = childItems();

	int x, y;
	getXY(x, y, m_size);
	ConnectorItem * ciFirst = NULL;
	ConnectorItem * ciNext = NULL;
	foreach (QGraphicsItem * item, items) {
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(item);
		if (ci == NULL) continue;

		int cx, cy;
		getXY(cx, cy, ci->connectorSharedName());
		if (cy == 0 && cx == 0) {
			ciFirst = ci;
			break;
		}
	}
	foreach (QGraphicsItem * item, items) {
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(item);
		if (ci == NULL) continue;

		int cx, cy;
		getXY(cx, cy, ci->connectorSharedName());
		if (cy == 0 && cx == 1) {
			ciNext = ci;
			break;
		}
	}

	if (ciFirst == NULL) return;
	if (ciNext == NULL) return;

	QRectF r1 = ciFirst->rect();
	QRectF r2 = ciNext->rect();

	qreal h = r1.height();
	qreal w = r2.center().x() - r1.center().x();

	r1.moveTo(-(r1.width() / 2), 0);
	r2.moveTo(w - (r2.width() / 2), 0);

	QPainterPath pp1;
	pp1.addRect(0, 0, w / 2, h);
	pp1.arcTo(r1, 90, -180);
	pp1.addRect(w / 2, 0, w / 2, h);
	pp1.moveTo(w, 0);
	pp1.arcTo(r2, 90, 180);

	//QPainterPath pp3;

	//pp3.moveTo(w, h);

	//QPainterPath pp4 = pp1.subtracted(pp3);

	//pp1.moveTo(0, 0);
	//QPainterPath pp2;
	//pp2.addRect(w / 2, 0, w / 2, h);
	//pp2.moveTo(w / 2, 0);
	//QPainterPath pp3;
	//pp3.addPath(pp1);
	//pp3.addPath(pp2);

	foreach (QGraphicsItem * item, items) {
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(item);
		if (ci == NULL) continue;

		int cx, cy;
		getXY(cx, cy, ci->connectorSharedName());
		if (cx >= x - 1) {
			// don't need a stripbit after the last column
			continue;
		}

		Stripbit * stripbit = new Stripbit(pp1, this);
		stripbit->setPen(Qt::NoPen);
		// TODO: don't hardcode this color
		stripbit->setBrush(QColor(0xc4, 0x9c, 0x59));
		QRectF r = ci->rect();
		stripbit->setPos(r.center().x(), r.top());
	}

}

void Stripboard::changeBoardSize() 
{
	Perfboard::changeBoardSize();
}

QString Stripboard::genModuleID(QMap<QString, QString> & currPropsMap)
{
	QString size = currPropsMap.value("size");
	return size + ModuleIDNames::StripboardModuleIDName;
}

QString Stripboard::makeBreadboardSvg(const QString & size) 
{
	return Perfboard::makeBreadboardSvg(size);
}
