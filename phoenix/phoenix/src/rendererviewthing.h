#ifndef RENDERERVIEWTHING_H
#define RENDERERVIEWTHING_H

#include <QHash>
#include <QSvgRenderer>

#include "viewthing.h"
#include "viewlayer.h"

class RendererViewThing : public ViewThing
{

public:
	RendererViewThing();

	QSvgRenderer * get(long /* ViewLayer::ViewLayerID */);
	void set(long /* ViewLayer::ViewLayerID */, QSvgRenderer *);

	QPixmap *getPixmap(ViewLayer::ViewLayerID viewLayerId, QSize size);

protected:
	QHash<long /* ViewLayer::ViewLayerID */, QSvgRenderer *> m_hash;

};

#endif
