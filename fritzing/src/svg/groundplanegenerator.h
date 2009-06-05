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

$Revision: 2676 $:
$Author: cohen@irascible.com $:
$Date: 2009-03-21 03:10:39 +0100 (Sat, 21 Mar 2009) $

********************************************************************/

#ifndef GROUNDPLANEGENERATOR_H
#define GROUNDPLANEGENERATOR_H

#include <QImage>
#include <QList>
#include <QRect>
#include <QPolygon>
#include <QString>
#include <QStringList>
#include <QGraphicsItem>

class GroundPlaneGenerator
{

public:
	GroundPlaneGenerator();
	~GroundPlaneGenerator();

	bool start(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, const QString &suffix, const QString & baseName, QStringList & exceptions, QGraphicsItem * board); 
	QStringList & newPartPaths();

protected:
	void scanLines(QImage & image, int bWidth, int bHeight, QList<QRect> & rects);
	void splitScanLines(QList<QRect> & rects, QList< QList<int> * > & pieces);
	void joinScanLines(QList<QRect> & rects, QList<QPolygon> & polygons);
	QString makePolyFzp(QList<QPolygon> & polygons, const QString & moduleID);
	QString makePolySvg(QList<QPolygon> & polygons, int res, qreal bWidth, qreal bHeight);

protected:
	QStringList m_newPartPaths;
};

#endif
