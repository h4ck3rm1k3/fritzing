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

#include "miniview.h"
#include "../debugdialog.h"
#include "../fgraphicsscene.h"
#include "miniviewcontainer.h"

static const QColor NormalColor(0x70, 0x70, 0x70);
static const QColor HoverColor(0x00, 0x00, 0x00);
static const QColor PressedColor(84, 24, 44 /*0xff, 0xff, 0xff */);
static const int FontPixelSize = 11;
static const QString FontFamily = "Droid Sans";

MiniView::MiniView(QWidget *parent )
	: QGraphicsView(parent)
{
	m_otherView = NULL;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_selected = false;
}

MiniView::~MiniView()
{
}

void MiniView::setTitle(const QString & title) {
	m_title = title;
	QFont font;
	font.setFamily(FontFamily);
	font.setWeight(QFont::Bold);
	font.setPixelSize(FontPixelSize);
	setFont(font);
	QFontMetrics metrics = fontMetrics();
	QRect br = metrics.boundingRect(m_title);
	parentWidget()->setMinimumWidth(br.width());
}

void MiniView::drawBackground(QPainter * painter, const QRectF & rect) {
	QGraphicsView::drawBackground(painter, rect);

	painter->save();
	QRect vp = painter->viewport(); 
	painter->setWindow(vp);
	painter->setTransform(QTransform());
	//painter->fillRect(0, 0, 10, 10, QBrush(QColor(Qt::blue)));
	QPen pen(m_titleColor, 1);
	painter->setPen(pen);
	QFont font;
	font.setFamily(FontFamily);
	font.setWeight(m_titleWeight);
	font.setPixelSize(FontPixelSize);
	painter->setFont(font);
	QFontMetrics metrics = painter->fontMetrics();
	m_lastHeight = metrics.descent() + metrics.ascent();
	int h = 0;  // metrics.descent();
	int y = vp.bottom() - h - 2;
	QRect br = metrics.boundingRect(m_title);
	int x = vp.left() + ((vp.width() - br.width()) / 2);
	painter->drawText(QPointF(x, y), m_title);
	painter->restore();
}

void MiniView::paintEvent ( QPaintEvent * event ) {
    //DebugDialog::debug("mini view paint event");
    if (scene()) {
        ((FGraphicsScene *) scene())->setDisplayHandles(false);
    }
    QGraphicsView::paintEvent(event);
}

void MiniView::setView(QGraphicsView * view) {
	m_otherView = view;
	this->setBackgroundBrush(view->backgroundBrush());
	QGraphicsView::setScene(view->scene());
}

void MiniView::updateSceneRect ( const QRectF & rect ) {
	QGraphicsView::updateSceneRect(rect);
	fitInView ( rect, Qt::KeepAspectRatio );
	QMatrix matrix = this->matrix();
	if (matrix.m11() < 0 && matrix.m22() < 0) {
		// bug (in Qt?) scales the matrix negatively;  try flipping the scale
		//DebugDialog::debug(QString("updatescenerect rect:%1 %2 %3 %4' m11:%5 m22:%6")
						   //.arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height())
						   //.arg(matrix.m11()).arg(matrix.m22()) );
		QMatrix m2(-matrix.m11(), matrix.m12(), matrix.m21(), -matrix.m22(), matrix.dx(), matrix.dy());
		setMatrix(m2);
	}

	emit rectChangedSignal();
}

void MiniView::resizeEvent ( QResizeEvent * event )
{
	QGraphicsView::resizeEvent(event);
	fitInView(sceneRect(), Qt::KeepAspectRatio);
	emit rectChangedSignal();
}

void MiniView::mousePressEvent(QMouseEvent *) {
	// for some reason, this mousePressEvent isn't detected by setting an event filter on the miniview
	// maybe because the event happens in the scene and get swallowed before the filter gets it?
	emit miniViewMousePressedSignal();
}

QGraphicsView * MiniView::view() {
	return m_otherView;
}

void MiniView::mouseMoveEvent(QMouseEvent * event) {
	Q_UNUSED(event);			// stops hover events when user hovers over QGraphicsItems in the navigator
}

bool MiniView::viewportEvent(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		// stops hover events when user hovers over QGraphicsItems in the navigator
		event->setAccepted(true);
        return true;
	}

	return QGraphicsView::viewportEvent(event);
}

void MiniView::navigatorMousePressedSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == this->parentWidget()) {
		m_selected = true;
		m_titleColor = PressedColor;
		m_titleWeight = QFont::Bold;
		qobject_cast<MiniViewContainer *>(parentWidget())->hideHandle(false);
	}
	else {
		m_selected = false;
		m_titleWeight = QFont::Normal;
		m_titleColor = NormalColor;
		qobject_cast<MiniViewContainer *>(parentWidget())->hideHandle(true);
	}

	//QSize sz = size();
	//repaint(0, sz.height() - m_lastHeight, sz.width(), m_lastHeight);
	this->setBackgroundBrush(backgroundBrush());				// only way I've found so far to force a repaint of the background

}

void MiniView::navigatorMouseEnterSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == this->parentWidget()) {
		if (!m_selected) {
			m_titleColor = HoverColor;
			//QSize sz = size();
			//repaint(0, sz.height() - m_lastHeight, sz.width(), m_lastHeight);
			this->setBackgroundBrush(backgroundBrush());				// only way I've found so far to force a repaint of the background
		}
	}
}

void MiniView::navigatorMouseLeaveSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == this->parentWidget()) {
		if (!m_selected) {
			m_titleColor = NormalColor;
			//QSize sz = size();
			//repaint(0, sz.height() - m_lastHeight, sz.width(), m_lastHeight);
			this->setBackgroundBrush(backgroundBrush());				// only way I've found so far to force a repaint of the background
		}
	}
}
