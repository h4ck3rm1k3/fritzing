/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#include "rendererviewthing.h"
#include <QSvgRenderer>
#include <QPainter>

RendererViewThing::RendererViewThing(  )
	: ViewThing()
{

}

void RendererViewThing::set(long /* ViewLayer::ViewLayerID */ layerID, QSvgRenderer * renderer) {
	m_hash.insert(layerID, renderer);
}

QSvgRenderer * RendererViewThing::get(long /* ViewLayer::ViewLayerID */ layerID) {
	return m_hash.value(layerID);
}

QPixmap *RendererViewThing::getPixmap(ViewLayer::ViewLayerID viewLayerId, QSize size) {
	QPixmap *pixmap = NULL;
	QSvgRenderer * renderer = get((long)viewLayerId);
	if (renderer) {
		pixmap = new QPixmap(size);
		pixmap->fill(Qt::transparent);
		QPainter painter(pixmap);
		renderer->render(&painter);
		painter.end();
	}
	return pixmap;
}
