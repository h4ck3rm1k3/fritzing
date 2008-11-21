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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include "rendererviewthing.h"
#include <QPainter>


FSvgRenderer::FSvgRenderer(QObject * parent) : QSvgRenderer(parent)
{
}

bool FSvgRenderer::load ( const QString & filename ) {
	bool result = QSvgRenderer::load(filename);
	if (result) {
		m_filename = filename;
	}
	return result;
}

const QString & FSvgRenderer::filename() {
	return m_filename;
}


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
	// TODO: cache pixmap

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
