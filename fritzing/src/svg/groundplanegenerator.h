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
							 QGraphicsItem * board, double res, const QString & color, const QString & layerName, double blurBy); 
	bool generateGroundPlaneUnit(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, QStringList & exceptions, 
							 QGraphicsItem * board, double res, const QString & color, const QString & layerName, QPointF whereToStart, double blurBy); 
	void scanImage(QImage & image, double bWidth, double bHeight, double pixelFactor, double res, 
					const QString & colorString, const QString & layerName, bool makeConnector, 
					int minRunSize, bool makeOffset, QSizeF minAreaInches, double minDimensionInches, QPointF offsetPolygons);  
	void scanOutline(QImage & image, double bWidth, double bHeight, double pixelFactor, double res, 
					const QString & colorString, const QString & layerName, bool makeConnector, int minRunSize, bool makeOffset, QSizeF minAreaInches, double minDimensionInches);  
	static void scanLines(QImage & image, int bWidth, int bHeight, QList<QRect> & rects, int threshhold, int minRunSize);
	bool getBoardRects(const QString & boardSvg, QGraphicsItem * board, double res, double keepoutSpace, QList<QRect> & rects);
	const QStringList & newSVGs();
	const QList<QPointF> & newOffsets();

public:
	static QString ConnectorName;

protected:
	void splitScanLines(QList<QRect> & rects, QList< QList<int> * > & pieces);
	void joinScanLines(QList<QRect> & rects, QList<QPolygon> & polygons);
	QString makePolySvg(QList<QPolygon> & polygons, double res, double bWidth, double bHeight, double pixelFactor, const QString & colorString, const QString & layerName, 
							bool makeConnectorFlag, QPointF * offset, QSizeF minAreaInches, double minDimensionInches, QPointF polygonOffset);
	QString makeOnePoly(const QPolygon & poly, const QString & colorString, const QString & id, int minX, int minY);
	double calcArea(QPolygon & poly);
	QImage * generateGroundPlaneAux(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, QStringList & exceptions, 
									QGraphicsItem * board, double res, double & bWidth, double & bHeight, double blurBy); 
	void makeConnector(QList<QPolygon> & polygons, double res, double pixelFactor, const QString & colorString, int minX, int minY, QString & svg);
	bool tryNextPoint(int x, int y, QImage & image, QList<QPoint> & points);
	void collectBorderPoints(QImage & image, QList<QPoint> & points);


protected:
	QStringList m_newSVGs;
	QList<QPointF> m_newOffsets;
};

#endif
