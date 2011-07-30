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

#include "fsplashscreen.h"
#include "utils/misc.h"

#include <QTextDocument>
#include <QTextCursor>

FSplashScreen::FSplashScreen(const QPixmap & pixmap, Qt::WindowFlags f ) : QSplashScreen(pixmap, f)
{
}

FSplashScreen::~FSplashScreen() {
	foreach (MessageThing * messageThing, m_messages) {
		delete messageThing;
	}
	foreach (PixmapThing * pixmapThing, m_pixmaps) {
		delete pixmapThing;
	}
}


void FSplashScreen::showMessage(const QString &message, QRect rect, int alignment, const QColor &color)
{
	MessageThing * messageThing = new MessageThing;
	messageThing->alignment = alignment;
	messageThing->color = color;
	messageThing->rect = rect;
	messageThing->message = message;
	m_messages.append(messageThing);
	repaint();
}


int FSplashScreen::showPixmap(const QPixmap & pixmap, QPoint point)
{
	PixmapThing * pixmapThing = new PixmapThing;
	pixmapThing->rect = QRect(point, QPoint(-1,-1));
	pixmapThing->pixmap = pixmap;
	m_pixmaps.append(pixmapThing);
	repaint();

	return m_pixmaps.count() - 1;
}

void FSplashScreen::showProgress(int index, double progress) {
	if (index < 0) return;
	if (index >= m_pixmaps.count()) return;

	int w = (int) (this->width() * progress);
	PixmapThing * pixmapThing = m_pixmaps[index];
	pixmapThing->rect.setWidth(w);
	repaint();
}


void FSplashScreen::drawContents ( QPainter * painter )
{
	// copied from QSplashScreen::drawContents
	painter->setRenderHint ( QPainter::Antialiasing, true );				// TODO: might need to be in the stylesheet?

	// pixmaps first, since they go beneath text
	foreach (PixmapThing * pixmapThing, m_pixmaps) {
		painter->drawPixmap(pixmapThing->rect, pixmapThing->pixmap);
	}

	foreach (MessageThing * messageThing, m_messages) {
		painter->setPen(messageThing->color);
		if (Qt::mightBeRichText(messageThing->message)) {
			QTextDocument doc;
	#ifdef QT_NO_TEXTHTMLPARSER
			doc.setPlainText(messageThing->message);
	#else
			doc.setHtml(messageThing->message);
	#endif
			doc.setTextWidth(messageThing->rect.width());
			QTextCursor cursor(&doc);
			cursor.select(QTextCursor::Document);
			QTextBlockFormat fmt;
			fmt.setAlignment(Qt::Alignment(messageThing->alignment));
			cursor.mergeBlockFormat(fmt);
			painter->save();
			painter->translate(messageThing->rect.topLeft());
			doc.drawContents(painter);
			painter->restore();
		} else {
			painter->drawText(messageThing->rect, messageThing->alignment, messageThing->message);
		}
	}
}

