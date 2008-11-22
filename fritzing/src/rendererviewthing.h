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

#ifndef RENDERERVIEWTHING_H
#define RENDERERVIEWTHING_H

#include <QHash>
#include <QSvgRenderer>

#include "viewthing.h"
#include "viewlayer.h"


class FSvgRenderer : public QSvgRenderer
{
public:
	FSvgRenderer(QObject * parent = 0);

	bool load(const QString & filename);
	bool load ( const QByteArray & contents);     // for SvgSplitter loads
	const QString & filename();

protected:
	QString m_filename;
};

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
