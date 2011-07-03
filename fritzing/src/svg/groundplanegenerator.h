/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

	bool generateGroundPlane(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, QStringList & exceptions, 
							 QGraphicsItem * board, qreal res, const QString & color, const QString & layerName); 
	bool generateGroundPlaneUnit(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, QStringList & exceptions, 
							 QGraphicsItem * board, qreal res, const QString & color, const QString & layerName, QPointF whereToStart); 
	const QStringList & newSVGs();
	void scanImage(QImage & image, qreal bWidth, qreal bHeight, qreal pixelFactor, qreal res, const QString & colorString, const QString & layerName, bool makeConnector, int minRunSize);  
	bool getBoardRects(const QString & boardSvg, QGraphicsItem * board, qreal res, qreal keepoutSpace, QList<QRect> & rects);
	static void scanLines(QImage & image, int bWidth, int bHeight, QList<QRect> & rects, int threshhold, int minRunSize);

public:
	static QString ConnectorName;

protected:
	void splitScanLines(QList<QRect> & rects, QList< QList<int> * > & pieces);
	void joinScanLines(QList<QRect> & rects, QList<QPolygon> & polygons);
	QString makePolySvg(QList<QPolygon> & polygons, qreal res, qreal bWidth, qreal bHeight, qreal pixelFactor, const QString & colorString, const QString & layerName, bool makeConnector);
	QString makeOnePoly(const QPolygon & poly, const QString & colorString, const QString & id);
	qreal calcArea(QPolygon & poly);
	QImage * generateGroundPlaneAux(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, QStringList & exceptions, 
									QGraphicsItem * board, qreal res, qreal & bWidth, qreal & bHeight); 


protected:
	QStringList m_newSVGs;
	
};

#endif
