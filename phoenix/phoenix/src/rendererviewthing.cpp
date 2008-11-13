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
