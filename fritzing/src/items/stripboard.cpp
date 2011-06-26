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
#include "../connectors/busshared.h"
#include "../connectors/connectorshared.h"

#include <QCursor>
#include <QBitmap>


//////////////////////////////////////////////////

// TODO:
//
//	cursor(s) for mouse hover? and down
//	change bus config at mouse release
//	disconnect/reconnect affected parts
//	undo


static QCursor * SpotFaceCutterCursor = NULL;

/////////////////////////////////////////////////////////////////////

Stripbit::Stripbit(const QPainterPath & path, QGraphicsItem * parent = 0) : QGraphicsPathItem(path, parent)
{
	if (SpotFaceCutterCursor == NULL) {
		QBitmap bitmap(":resources/images/spot_face_cutter.bmp");
		SpotFaceCutterCursor = new QCursor(bitmap, bitmap, 0, 31);
	}

	setZValue(-999);

	m_inHover = m_removed = false;
	setAcceptsHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setCursor(*SpotFaceCutterCursor);
}

Stripbit::~Stripbit() {
}

void Stripbit::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	qreal newOpacity = 1;
	if (m_removed) {
		if (m_inHover) newOpacity = 0.55;
		else newOpacity = 0.3;
	}
	else {
		if (m_inHover) newOpacity = 0.6;
		else newOpacity = 1;
	}

	qreal opacity = painter->opacity();
	painter->setOpacity(newOpacity);
	QGraphicsPathItem::paint(painter, option, widget);
	painter->setOpacity(opacity);

}

void Stripbit::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
	if (!event->buttons() && Qt::LeftButton) {
		event->ignore();
		return;
	}

	event->accept();
	m_removed = !m_removed;
	m_inHover = false;
	update();

	//DebugDialog::debug("got press");
}

void Stripbit::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
}

void Stripbit::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (!event->buttons() && Qt::LeftButton) return;

	//DebugDialog::debug("got move");

	Stripbit * other = NULL;
	foreach(QGraphicsItem * item, scene()->items(event->scenePos())) {
		other = dynamic_cast<Stripbit *>(item);
		if (other) break;
	}

	if (!other) return;

	//DebugDialog::debug("got other");

	if (other->removed() == m_removed) return;

	//DebugDialog::debug("change other");

	other->setRemoved(m_removed);
	other->update();
}

void Stripbit::hoverEnterEvent( QGraphicsSceneHoverEvent * event ) 
{
	Q_UNUSED(event);
	m_inHover = true;
	update();
}

void Stripbit::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) 
{
	Q_UNUSED(event);
	m_inHover = false;
	update();
}

void Stripbit::setRemoved(bool removed) {
	m_removed = removed;
}

bool Stripbit::removed() {
	return m_removed;
}

/////////////////////////////////////////////////////////////////////

Stripboard::Stripboard( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: Perfboard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (!viewIdentifier == ViewIdentifierClass::BreadboardView) return;

	QList<BusShared *> buses;
	int x, y;
	getXY(x, y, m_size);
	for (int i = 0; i < y; i++) {
		BusShared * busShared = new BusShared(QString::number(i));
		buses.append(busShared);
	}

	foreach (Connector * connector, modelPart->connectors().values()) {
		ConnectorShared * connectorShared = connector->connectorShared();
		int cx, cy;
		getXY(cx, cy, connectorShared->name());
		BusShared * busShared = buses.at(cy);
		busShared->addConnectorShared(connectorShared);
	}

	modelPart->initBuses();
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
