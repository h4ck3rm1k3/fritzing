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

#include "groundplanegenerator.h"
#include "svgfilesplitter.h"
#include "../fsvgrenderer.h"
#include "../debugdialog.h"
#include "../version/version.h"
#include "../utils/folderutils.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../items/wire.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QDate>
#include <QTextStream>
#include <qmath.h>
#include <limits>

static const int MILS = 5;			// operate on a 5 mil scale
static const int THRESHOLD = 192;

QString GroundPlaneGenerator::ConnectorName = "connector0pad";

//  !!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!!!  IMPORTANT NOTE:  QRect::right() and QRect::bottom() are off by one--this is a known Qt problem 
//  !!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!!!					  one workaround might be to switch to QRectF
//  !!!!!!!!!!!!!!!!!!!

GroundPlaneGenerator::GroundPlaneGenerator()
{
}

GroundPlaneGenerator::~GroundPlaneGenerator() {
}

bool GroundPlaneGenerator::getBoardRects(const QString & boardSvg, QGraphicsItem * board, qreal res, qreal keepoutSpace, QList<QRect> & rects)
{
	QByteArray boardByteArray;
    QString tempColor("#000000");
	QStringList exceptions;
    if (!SvgFileSplitter::changeColors(boardSvg, tempColor, exceptions, boardByteArray)) {
		return false;
	}

	QRectF br = board->sceneBoundingRect();
	qreal bWidth = res * br.width() / FSvgRenderer::printerScale();
	qreal bHeight = res * br.height() / FSvgRenderer::printerScale();
	QImage image(bWidth, bHeight, QImage::Format_RGB32);
	image.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
	image.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);
	image.fill(0xffffffff);

	QSvgRenderer renderer(boardByteArray);
	QPainter painter;
	painter.begin(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	renderer.render(&painter);
	painter.end();

#ifndef QT_NO_DEBUG
	//image.save("getBoardRects.png");
#endif

	QColor keepaway(255,255,255);
	int threshold = 1;

	// now add keepout area to the border
	QImage image2 = image.copy();
	painter.begin(&image2);
	painter.fillRect(0, 0, image2.width(), keepoutSpace, keepaway);
	painter.fillRect(0, image2.height() - keepoutSpace, image2.width(), keepoutSpace, keepaway);
	painter.fillRect(0, 0, keepoutSpace, image2.height(), keepaway);
	painter.fillRect(image2.width() - keepoutSpace, 0, keepoutSpace, image2.height(), keepaway);

	for (int y = 0; y < image.height(); y++) {
		QRgb* scanLine = (QRgb *) image.scanLine(y);
		for (int x = 0; x < image.width(); x++) {
			QRgb current = *(scanLine + x);
			int gray = qGray(current);
			if (gray <= threshold) {			
				continue;
			}

			painter.fillRect(x - keepoutSpace, y - keepoutSpace, keepoutSpace + keepoutSpace, keepoutSpace + keepoutSpace, keepaway);
		}
	}
	painter.end();

#ifndef QT_NO_DEBUG
	//image2.save("getBoardRects2.png");
#endif

	scanLines(image2, bWidth, bHeight, rects, threshold, 1);

	// combine parallel equal-sized rects
	int ix = 0;
	while (ix < rects.count()) {
		QRect r = rects.at(ix++);
		for (int j = ix; j < rects.count(); j++) {
			QRect s = rects.at(j);
			if (s.bottom() == r.bottom()) {
				// on same row; keep going
				continue;
			}

			if (s.top() > r.bottom() + 1) {
				// skipped row, can't join
				break;
			}

			if (s.left() == r.left() && s.right() == r.right()) {
				// join these
				r.setBottom(s.bottom());
				rects.removeAt(j);
				ix--;
				rects.replace(ix, r);
				break;
			}
		}
	}

	return true;
}

bool GroundPlaneGenerator::generateGroundPlaneUnit(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, 
												   QStringList & exceptions, QGraphicsItem * board, qreal res, const QString & color, const QString & layerName,
												   QPointF whereToStart) 
{
	qreal bWidth, bHeight;
	QImage * image = generateGroundPlaneAux(boardSvg, boardImageSize, svg, copperImageSize, exceptions, board, res, bWidth, bHeight);
	if (image == NULL) return false;

	QPoint s(qRound(res * (whereToStart.x() - board->pos().x()) / FSvgRenderer::printerScale()),
			qRound(res * (whereToStart.y() - board->pos().y()) / FSvgRenderer::printerScale()));

	QRgb pixel = image->pixel(s);
	int gray = qGray(pixel);
	if (gray <= THRESHOLD) {
		// starting off in bad territory
		delete image;
		return false;
	}

	// step 1 flood fill white to red  (keep max locations)

	int minY = image->height();
	int maxY = 0;
	int minX = image->width();
	int maxX = 0;
	QList<QPoint> stack;
	stack << s;
	while (stack.count() > 0) {
		QPoint p = stack.takeFirst();
		if (p.x() < 0) continue;
		if (p.y() < 0) continue;
		if (p.x() >= image->width()) continue;
		if (p.y() >= image->height()) continue;

		QRgb pixel = image->pixel(p);
		if (pixel == 0xffff0000) continue;			// already been here

		int gray = qGray(pixel);
		if (gray <= THRESHOLD) continue;			// black; bail

		image->setPixel(p,  0xffff0000);
		if (p.x() > maxX) maxX = p.x();
		if (p.x() < minX) minX = p.x();
		if (p.y() > maxY) maxY = p.y();
		if (p.y() < minY) minY = p.y();

		stack.append(QPoint(p.x() - 1, p.y()));
		stack.append(QPoint(p.x() + 1, p.y()));
		stack.append(QPoint(p.x(), p.y() - 1));
		stack.append(QPoint(p.x(), p.y() + 1));
	}

	//image->save("testPoly1.png");

	// step 2 replace white with black

	for (int y = 0; y < image->height(); y++) {
		QRgb* scanLine = (QRgb *) image->scanLine(y);
		for (int x = 0; x < image->width(); x++) {
			QRgb pixel = *(scanLine + x);
			if (pixel == 0xffff0000) continue;

			image->setPixel(x, y,  0xff000000);
		}
	}

	//image->save("testPoly2.png");

	// step 3 replace red with white

	for (int y = 0; y < image->height(); y++) {
		QRgb* scanLine = (QRgb *) image->scanLine(y);
		for (int x = 0; x < image->width(); x++) {
			QRgb pixel = *(scanLine + x);
			if (pixel != 0xffff0000) continue;

			image->setPixel(x, y,  0xffffffff);
		}
	}

	//image->save("testPoly3.png");

	scanImage(*image, bWidth, bHeight, MILS, res, color, layerName, true, 8, true, QSizeF(.05, .05));
	delete image;
	return true;
}


bool GroundPlaneGenerator::generateGroundPlane(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, 
												QStringList & exceptions, QGraphicsItem * board, qreal res, const QString & color, const QString & layerName) 
{

	qreal bWidth, bHeight;
	QImage * image = generateGroundPlaneAux(boardSvg, boardImageSize, svg, copperImageSize, exceptions, board, res, bWidth, bHeight);
	if (image == NULL) return false;

	scanImage(*image, bWidth, bHeight, MILS, res, color, layerName, true, 8, true, QSizeF(.05, .05));
	delete image;
	return true;
}

QImage * GroundPlaneGenerator::generateGroundPlaneAux(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, 
													QStringList & exceptions, QGraphicsItem * board, qreal res, qreal & bWidth, qreal & bHeight) 
{
	QByteArray boardByteArray;
    QString tempColor("#ffffff");
    if (!SvgFileSplitter::changeColors(boardSvg, tempColor, exceptions, boardByteArray)) {
		return NULL;
	}

	/*
	QFile file0("testGroundFillBoard.svg");
	file0.open(QIODevice::WriteOnly);
	QTextStream out0(&file0);
	out0 << boardByteArray;
	file0.close();
	*/


	QByteArray copperByteArray;
	if (!SvgFileSplitter::changeStrokeWidth(svg, 50, false, copperByteArray)) {
		return NULL;
	}

	/*
	QFile file1("testGroundFillCopper.svg");
	file1.open(QIODevice::WriteOnly);
	QTextStream out1(&file1);
	out1 << copperByteArray;
	file1.close();
	*/

	qreal svgWidth = res * qMax(boardImageSize.width(), copperImageSize.width()) / FSvgRenderer::printerScale();
	qreal svgHeight = res * qMax(boardImageSize.height(), copperImageSize.height()) / FSvgRenderer::printerScale();

	QRectF br =  board->sceneBoundingRect();
	bWidth = res * br.width() / FSvgRenderer::printerScale();
	bHeight = res * br.height() / FSvgRenderer::printerScale();
	QImage * image = new QImage(qMax(svgWidth, bWidth), qMax(svgHeight, bHeight), QImage::Format_RGB32);
	image->setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
	image->setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);
	image->fill(0x0);

	QSvgRenderer renderer(boardByteArray);
	QPainter painter;
	painter.begin(image);
	painter.setRenderHint(QPainter::Antialiasing);
	renderer.render(&painter, QRectF(0, 0, res * boardImageSize.width() / FSvgRenderer::printerScale(), res * boardImageSize.height() / FSvgRenderer::printerScale()));
	painter.end();

	//image->save("testGroundFillBoard.png");

	// "blur" the image a little
	QSvgRenderer renderer2(copperByteArray);
	painter.begin(image);
	QRectF bounds(0, 0, res * copperImageSize.width() / FSvgRenderer::printerScale(), res * copperImageSize.height() / FSvgRenderer::printerScale());
	renderer2.render(&painter, bounds);
	bounds.moveTo(1, 0);
	renderer2.render(&painter, bounds);
	bounds.moveTo(-1, 0);
	renderer2.render(&painter, bounds);
	bounds.moveTo(0, 1);
	renderer2.render(&painter, bounds);
	bounds.moveTo(0, -1);
	renderer2.render(&painter, bounds);
	bounds.moveTo(1, 1);
	renderer2.render(&painter, bounds);
	bounds.moveTo(-1, -1);
	renderer2.render(&painter, bounds);
	bounds.moveTo(-1, 1);
	renderer2.render(&painter, bounds);
	bounds.moveTo(1, -1);
	renderer2.render(&painter, bounds);
	painter.end();

	//image->save("testGroundFillCopper.png");
	return image;
}

void GroundPlaneGenerator::scanImage(QImage & image, qreal bWidth, qreal bHeight, qreal pixelFactor, qreal res, 
									 const QString & colorString, const QString & layerName, bool makeConnector, 
									 int minRunSize, bool makeOffset, QSizeF minSizeInches)  
{
	QList<QRect> rects;
	scanLines(image, bWidth, bHeight, rects, THRESHOLD, minRunSize);
	QList< QList<int> * > pieces;
	splitScanLines(rects, pieces);
	foreach (QList<int> * piece, pieces) {
		QList<QPolygon> polygons;
		QList<QRect> newRects;
		foreach (int i, *piece) {
			QRect r = rects.at(i);
			newRects.append(QRect(r.x() * pixelFactor, r.y() * pixelFactor, (r.width() * pixelFactor) + 1, pixelFactor + 1));    // + 1 is for off-by-one converting rects to polys
		}

		// note: there is always one
		joinScanLines(newRects, polygons);
		QPointF offset;
		QString pSvg = makePolySvg(polygons, res, bWidth, bHeight, pixelFactor, colorString, layerName, makeConnector, makeOffset ? &offset : NULL, minSizeInches);
		if (pSvg.isEmpty()) continue;

		m_newSVGs.append(pSvg);
		if (makeOffset) {
			offset *= FSvgRenderer::printerScale();
			m_newOffsets.append(offset);			// offset now in pixels
		}

		/*
		QFile file4("testPoly.svg");
		file4.open(QIODevice::WriteOnly);
		QTextStream out4(&file4);
		out4 << pSvg;
		file4.close();
		*/

	}

	/*
	QString newSvg = QString("<svg xmlns='http://www.w3.org/2000/svg' width='%1in' height='%2in' viewBox='0 0 %3 %4' >\n")
		.arg(bWidth / res)
		.arg(bHeight / res)
		.arg(bWidth * MILS)
		.arg(bHeight * MILS);
	newSvg += "<g id='groundplane'>\n";

	// ?split each line into two lines (l1, l2) and add a terminal point at the left of l1 and the right of l2?

	ix = 0;
	foreach (QRectF r, rects) {
		newSvg += QString("<rect x='%1' y='%2' width='%3' height='%4' id='connector%5pad' fill='%6'  />\n")
			.arg(r.left() * MILS)
			.arg(r.top() * MILS)
			.arg(r.width() * MILS)
			.arg(MILS)
			.arg(ix++).
			.arg(ViewLayer::Copper0Color);
	}
	newSvg += "</g>\n</svg>\n";
	*/

}

void GroundPlaneGenerator::scanLines(QImage & image, int bWidth, int bHeight, QList<QRect> & rects, int threshold, int minRunSize)
{
	// threshold should be between 0 and 255 exclusive; smaller will include more of the svg
	for (int y = 0; y < bHeight; y++) {
		bool inWhite = false;
		int whiteStart = 0;
		QRgb* scanLine = (QRgb *) image.scanLine(y);
		for (int x = 0; x < bWidth; x++) {
			QRgb current = *(scanLine + x);
			//if (current != 0xff000000 && current != 0xffffffff) {
				//DebugDialog::debug(QString("current %1").arg(current,0,16));
			//}
			int gray = qGray(current);
			if (inWhite) {
				if (gray > threshold) {			// qBlue(current) == 0xff    gray > 128
					// another white pixel, keep moving
					continue;
				}

				// got black: close up this segment;
				inWhite = false;
				if (x - whiteStart < minRunSize) {
					// not a big enough section
					continue;
				}

				rects.append(QRect(whiteStart, y, x - whiteStart, 1));
			}
			else {
				if (gray <= threshold) {		// qBlue(current) != 0xff				
					// another black pixel, keep moving
					continue;
				}

				inWhite = true;
				whiteStart = x;
			}
		}
		if (inWhite) {
			// close up the last segment
			if (bWidth - whiteStart >= minRunSize) {
				rects.append(QRect(whiteStart, y, bWidth - whiteStart, 1));
			}
		}
	}
}

void GroundPlaneGenerator::splitScanLines(QList<QRect> & rects, QList< QList<int> * > & pieces) 
{
	// combines vertically adjacent scanlines into "pieces"
	int ix = 0;
	int prevFirst = -1;
	int prevLast = -1;
	while (ix < rects.count()) {
		int first = ix;
		QRectF firstR = rects.at(ix);
		while (++ix < rects.count()) {
			QRectF nextR = rects.at(ix);
			if (nextR.y() != firstR.y()) {
				break;
			}
		}
		int last = ix - 1;  // this was a lookahead so step back one
		if (prevFirst >= 0) {
			for (int i = first; i <= last; i++) {
				QRectF candidate = rects.at(i);
				int gotCount = 0;
				for (int j = prevFirst; j <= prevLast; j++) {
					QRectF prev = rects.at(j);
					if (prev.y() + 1 != candidate.y()) {
						// skipped a line; no intersection possible
						break;
					}

					if ((prev.x() + prev.width() <= candidate.x()) || (candidate.x() + candidate.width() <= prev.x())) {
						// candidate and prev didn't intersect
						continue;
					}

					if (++gotCount > 1) {
						QList<int> * piecei = NULL;
						QList<int> * piecej = NULL;
						foreach (QList<int> * piece, pieces) {
							if (piece->contains(j)) {
								piecej = piece;
								break;
							}
						}
						foreach (QList<int> * piece, pieces) {
							if (piece->contains(i)) {
								piecei = piece;
								break;
							}
						}
						if (piecei != NULL && piecej != NULL) {
							if (piecei != piecej) {
								foreach (int b, *piecej) {
									piecei->append(b);
								}
								piecej->clear();
								pieces.removeOne(piecej);
								delete piecej;
							}
							piecei->append(i);
						}
						else {
							DebugDialog::debug("we are really screwed here, what should we do about it?");
						}
					}
					else {
						// put the candidate (i) in j's piece
						foreach (QList<int> * piece, pieces) {
							if (piece->contains(j)) {
								piece->append(i);
								break;
							}
						}
					}
				}

				if (gotCount == 0) {
					// candidate is an orphan line at this point
					QList<int> * piece = new QList<int>;
					piece->append(i);
					pieces.append(piece);
				}

			}
		}
		else {
			for (int i = first; i <= last; i++) {
				QList<int> * piece = new QList<int>;
				piece->append(i);
				pieces.append(piece);
			}
		}

		prevFirst = first;
		prevLast = last;
	}

	foreach (QList<int> * piece, pieces) {
		qSort(*piece);
	}
}

void GroundPlaneGenerator::joinScanLines(QList<QRect> & rects, QList<QPolygon> & polygons) {
	QList< QList<int> * > pieces;
	int ix = 0;
	int prevFirst = -1;
	int prevLast = -1;
	while (ix < rects.count()) {
		int first = ix;
		QRectF firstR = rects.at(ix);
		while (++ix < rects.count()) {
			QRectF nextR = rects.at(ix);
			if (nextR.y() != firstR.y()) {
				break;
			}
		}
		int last = ix - 1;  
		if (prevFirst >= 0) {
			QVector<int> holdPrevs(last - first + 1);
			QVector<int> gotCounts(last - first + 1);
			for (int i = first; i <= last; i++) {
				int index = i - first;
				holdPrevs[index] = 0;
				gotCounts[index] = 0;
				QRectF candidate = rects.at(i);
				for (int j = prevFirst; j <= prevLast; j++) {
					QRectF prev = rects.at(j);

					if ((prev.x() + prev.width() <= candidate.x()) || (candidate.x() + candidate.width() <= prev.x())) {
						// candidate and prev didn't intersect
						continue;
					}

					holdPrevs[index] = j;
					gotCounts[index]++;
				}
				if (gotCounts[index] > 1) {
					holdPrevs[index] = -1;			// clear this to allow one of the others in this scanline to capture a previous
				}
			}
			for (int i = first; i <= last; i++) {
				int index = i - first;

				bool gotOne = false;
				if (gotCounts[index] == 1) {
					bool unique = true;
					for (int j = first; j <= last; j++) {
						if (j - first == index) continue;			// don't compare against yourself

						if (holdPrevs[index] == holdPrevs[j - first]) {
							unique = false;
							break;
						}
					}

					if (unique) {
						// add this to the previous chunk
						gotOne = true;
						foreach (QList<int> * piece, pieces) {
							if (piece->contains(holdPrevs[index])) {
								piece->append(i);
								break;
							
							}
						}
					}
				}
				if (!gotOne) {
					// start a new chunk
					holdPrevs[index] = -1;						// allow others to capture the prev
					QList<int> * piece = new QList<int>;
					piece->append(i);
					pieces.append(piece);
				}
			}
		}
		else {
			for (int i = first; i <= last; i++) {
				QList<int> * piece = new QList<int>;
				piece->append(i);
				pieces.append(piece);
			}
		}

		prevFirst = first;
		prevLast = last;
	}

	foreach (QList<int> * piece, pieces) {
		//QPolygon poly(rects.at(piece->at(0)), true);
		//for (int i = 1; i < piece->length(); i++) {
			//QPolygon temp(rects.at(piece->at(i)), true);
			//poly = poly.united(temp);
		//}

		// no need to close polygon; SVG automatically closes path
		
		QPolygon poly;

		// left side
		for (int i = 0; i < piece->length(); i++) {
			QRect r = rects.at(piece->at(i));
			if ((poly.count() > 0) && (poly.last().x() == r.left())) {
				poly.pop_back();
			}
			else {
				poly.append(QPoint(r.left(), r.top()));
			}
			poly.append(QPoint(r.left(), r.bottom()));
		}
		// right side
		for (int i = piece->length() - 1; i >= 0; i--) {
			QRect r = rects.at(piece->at(i));
			if ((poly.count() > 0) && (poly.last().x() == r.right())) {
				poly.pop_back();
			}
			else {
				poly.append(QPoint(r.right(), r.bottom()));
			}
			poly.append(QPoint(r.right(), r.top()));
		}

		

		polygons.append(poly);
		delete piece;
	}
}

QString GroundPlaneGenerator::makePolySvg(QList<QPolygon> & polygons, qreal res, qreal bWidth, qreal bHeight, qreal pixelFactor, 
										const QString & colorString, const QString & layerName, bool makeConnectorFlag, QPointF * offset, 
										QSizeF minSizeInches) 
{
	int minX = 0;
	int minY = 0;
	if (offset != NULL) {
		minY = std::numeric_limits<int>::max();
		int maxY = std::numeric_limits<int>::min();
		minX = minY;
		int maxX = maxY;

		foreach (QPolygon polygon, polygons) {
			foreach (QPoint p, polygon) {
				if (p.x() > maxX) maxX = p.x();
				if (p.x() < minX) minX = p.x();
				if (p.y() > maxY) maxY = p.y();
				if (p.y() < minY) minY = p.y();
			}
		}

		bWidth = (maxX - minX) / pixelFactor;
		bHeight = (maxY - minY) / pixelFactor;
		offset->setX(minX / (res * pixelFactor));		// inches
		offset->setY(minY / (res * pixelFactor));		// inches
	}

	if ((bWidth / res < minSizeInches.width()) && (bHeight / res < minSizeInches.height())) {
		return "";
	}

	QString pSvg = QString("<svg xmlns='http://www.w3.org/2000/svg' width='%1in' height='%2in' viewBox='0 0 %3 %4' >\n")
		.arg(bWidth / res)
		.arg(bHeight / res)
		.arg(bWidth * pixelFactor)
		.arg(bHeight * pixelFactor);
	pSvg += QString("<g id='%1'>\n").arg(layerName);
	if (makeConnectorFlag) {
		makeConnector(polygons, res, pixelFactor, colorString, minX, minY, pSvg);
	}
	else {
		foreach (QPolygon poly, polygons) {
			pSvg += makeOnePoly(poly, colorString, "", minX, minY);
		}
	}

	pSvg += "</g>\n</svg>\n";

	return pSvg;
}

void GroundPlaneGenerator::makeConnector(QList<QPolygon> & polygons, qreal res, qreal pixelFactor, const QString & colorString, int minX, int minY, QString & pSvg)
{
	//	see whether the standard circular connector will fit somewhere inside a polygon:
	//	http://stackoverflow.com/questions/4279478/maximum-circle-inside-a-non-convex-polygon
	//	or maybe this is useful, e.g. treating the circle as a square:  
	//	http://stackoverflow.com/questions/4833802/check-if-polygon-is-inside-a-polygon

	//	code presently uses a version of the Poles of Inaccessibility algorithm:

	static const qreal standardConnectorWidth = .075;		 // inches
	qreal targetDiameter = res * pixelFactor * standardConnectorWidth;
	qreal targetDiameterAnd = targetDiameter * 1.25;
	qreal targetRadius = targetDiameter / 2;
	qreal targetRadiusAnd = targetDiameterAnd / 2;
	qreal targetRadiusAndSquared = targetRadiusAnd * targetRadiusAnd;
	foreach (QPolygon poly, polygons) {
		QRect boundingRect = poly.boundingRect(); 
		if (boundingRect.width() < targetDiameterAnd) continue;
		if (boundingRect.height() < targetDiameterAnd) continue;

		QList<QLineF> polyLines;
		int count = poly.count();
		for (int i = 0; i < count; i++) {
			QLineF lp(poly[i], poly[(i + 1) % count]);
			polyLines.append(lp);
		}

		int xDivisor = qRound(boundingRect.width() / targetRadius);
		int yDivisor = qRound(boundingRect.height() / targetRadius);

		qreal dx = (boundingRect.width() - targetDiameterAnd) / xDivisor;
		qreal dy = (boundingRect.height() - targetDiameterAnd) / yDivisor;
		qreal x;
		qreal y = boundingRect.top() + targetRadiusAnd - dy;
		for (int iy = 0; iy <= yDivisor; iy++) {
			y += dy;
			x = boundingRect.left() + targetRadiusAnd - dx;
			for (int ix = 0; ix <= xDivisor; ix++) {
				x += dx;
				if (!poly.containsPoint(QPoint(qRound(x), qRound(y)), Qt::OddEvenFill)) continue;

				bool gotOne = true;
				foreach (QLineF line, polyLines) {
					qreal distance, dx, dy;
					bool atEndpoint;
					GraphicsUtils::distanceFromLine(x, y, line.p1().x(), line.p1().y(), line.p2().x(), line.p2().y(), 
													dx, dy, distance, atEndpoint);
					if (distance <= targetRadiusAndSquared) {
						gotOne = false;
						break;
					}
				}

				if (!gotOne) continue;

				foreach (QPolygon poly, polygons) {
					pSvg += makeOnePoly(poly, colorString, "", minX, minY);
				}

				pSvg += QString("<g id='%1'><circle cx='%2' cy='%3' r='%4' fill='%5' stroke='none' stroke-width='0' /></g>\n")
					.arg(ConnectorName)
					.arg(x - minX)
					.arg(y - minY)
					.arg(targetRadius)
					.arg(colorString);


				return;
			}
		}
	}

	// couldn't find anything big enough above, so
	// try to find a poly with an area that's big enough to click, but not so big as to get in the way
	int useIndex = -1;
	QList<qreal> areas;
	qreal divisor = res * pixelFactor * res * pixelFactor;
	foreach (QPolygon poly, polygons) {
		areas.append(calcArea(poly) / divisor);
	}
		
	for (int i = 0; i < areas.count(); i++) {
		if (areas.at(i) > 0.1 && areas.at(i) < 0.25) {
			useIndex = i;
			break;
		}
	}
	if (useIndex < 0) {
		for (int i = 0; i < areas.count(); i++) {
			if (areas.at(i) > 0.1) {
				useIndex = i;
				break;
			}
		}
	}
	if (useIndex < 0) {
		pSvg += QString("<g id='%1'>\n").arg(ConnectorName);
		foreach (QPolygon poly, polygons) {
			pSvg += makeOnePoly(poly, colorString, "", minX, minY);
		}
		pSvg += "</g>";
	}
	else {
		int ix = 0;
		for (int i = 0; i < polygons.count(); i++) {
			if (i == useIndex) {
				// has to appear inside a g element
				pSvg += QString("<g id='%1'>\n").arg(ConnectorName);
				pSvg += makeOnePoly(polygons.at(i), colorString, "", minX, minY);
				pSvg += "</g>";
			}
			else {
				pSvg += makeOnePoly(polygons.at(i), colorString, FSvgRenderer::NonConnectorName + QString::number(ix++), minX, minY);
			}
		}
	}
}

qreal GroundPlaneGenerator::calcArea(QPolygon & poly) {
	qreal total = 0;
	for (int ix = 0; ix < poly.count(); ix++) {
		QPoint p0 = poly.at(ix);
		QPoint p1 = poly.at((ix + 1) % poly.count());
		total += (p0.x() * p1.y() - p1.x() * p0.y());
	}
	return qAbs(total / 2.0);
}

QString GroundPlaneGenerator::makeOnePoly(const QPolygon & poly, const QString & colorString, const QString & id, int minX, int minY) {
	QString idString;
	if (!id.isEmpty()) {
		idString = QString("id='%1'").arg(id);
	}
	QString polyString = QString("<polygon fill='%1' %2 points='\n").arg(colorString).arg(idString);
	int space = 0;
	foreach (QPoint p, poly) {
		polyString += QString("%1,%2 %3").arg(p.x() - minX).arg(p.y() - minY).arg((++space % 8 == 0) ?  "\n" : "");
	}
	polyString += "'/>\n";
	return polyString;
}

const QStringList & GroundPlaneGenerator::newSVGs() {
	return m_newSVGs;
}

const QList<QPointF> & GroundPlaneGenerator::newOffsets() {
	return m_newOffsets;
}
