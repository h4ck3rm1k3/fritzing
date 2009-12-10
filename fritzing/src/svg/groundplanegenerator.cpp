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

#include <QPainter>
#include <QSvgRenderer>
#include <QDate>
#include <QTextStream>

#define MINYSECTION 4
#define MILS 10			// operate on a 10 mil scale

GroundPlaneGenerator::GroundPlaneGenerator()
{
}

GroundPlaneGenerator::~GroundPlaneGenerator() {
}

bool GroundPlaneGenerator::start(const QString & boardSvg, QSizeF boardImageSize, const QString & svg, QSizeF copperImageSize, 
								 QStringList & exceptions, QGraphicsItem * board, qreal res) 
{
	QByteArray boardByteArray;
    QString tempColor("#ffffff");
    if (!SvgFileSplitter::changeColors(boardSvg, tempColor, exceptions, boardByteArray)) {
		return false;
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
		return false;
	}

	/*
	QFile file1("testGroundFill.svg");
	file1.open(QIODevice::WriteOnly);
	QTextStream out1(&file1);
	out1 << copperByteArray;
	file1.close();
	*/

	qreal svgWidth = res * qMax(boardImageSize.width(), copperImageSize.width()) / FSvgRenderer::printerScale();
	qreal svgHeight = res * qMax(boardImageSize.height(), copperImageSize.height()) / FSvgRenderer::printerScale();

	QRectF br =  board->sceneBoundingRect();
	qreal bWidth = res * br.width() / FSvgRenderer::printerScale();
	qreal bHeight = res * br.height() / FSvgRenderer::printerScale();
	QImage image(qMax(svgWidth, bWidth), qMax(svgHeight, bHeight), QImage::Format_RGB32);
	image.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
	image.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);
	image.fill(0x0);

	QSvgRenderer renderer(boardByteArray);
	QPainter painter;
	painter.begin(&image);
	renderer.render(&painter, QRectF(0, 0, res * boardImageSize.width() / FSvgRenderer::printerScale(), res * boardImageSize.height() / FSvgRenderer::printerScale()));
	painter.end();

	//image.save("testGroundFillBoard.png");

	QSvgRenderer renderer2(copperByteArray);
	painter.begin(&image);
	renderer2.render(&painter, QRectF(0, 0, res * copperImageSize.width() / FSvgRenderer::printerScale(), res * copperImageSize.height() / FSvgRenderer::printerScale()));
	painter.end();

	//image.save("testGroundFill.png");
	if (bHeight > image.height()) bHeight = image.height();
	if (bWidth > image.width()) bWidth = image.width();

	scanImage(image, bWidth, bHeight, MILS, res, "#ffbf00", "groundplane");
	return true;
}

void GroundPlaneGenerator::scanImage(QImage & image, qreal bWidth, qreal bHeight, qreal pixelFactor, qreal res, const QString & colorString, const QString & layerName)  
{
	QList<QRect> rects;
	scanLines(image, bWidth, bHeight, rects);
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
		QString pSvg = makePolySvg(polygons, res, bWidth, bHeight, pixelFactor, colorString, layerName);
		m_newSVGs.append(pSvg);

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
		newSvg += QString("<rect fill='#ffbf00' x='%1' y='%2' width='%3' height='%4' id='connector%5pad' />\n")
			.arg(r.left() * MILS)
			.arg(r.top() * MILS)
			.arg(r.width() * MILS)
			.arg(MILS)
			.arg(ix++);
	}
	newSvg += "</g>\n</svg>\n";
	*/

}

void GroundPlaneGenerator::scanLines(QImage & image, int bWidth, int bHeight, QList<QRect> & rects)
{
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
				if (gray > 192) {			// qBlue(current) == 0xff    gray > 128
					// another white pixel, keep moving
					continue;
				}

				// got black: close up this segment;
				inWhite = false;
				if (x - whiteStart < MINYSECTION) {
					// not a big enough section
					continue;
				}

				rects.append(QRect(whiteStart, y, x - whiteStart + 1, 1));
			}
			else {
				if (gray <= 192) {		// qBlue(current) != 0xff				
					// another black pixel, keep moving
					continue;
				}

				inWhite = true;
				whiteStart = x;
			}
		}
		if (inWhite) {
			// close up the last segment
			if (bWidth - whiteStart + 1 >= MINYSECTION) {
				rects.append(QRect(whiteStart, y, bWidth - whiteStart + 1, 1));
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

QString GroundPlaneGenerator::makePolySvg(QList<QPolygon> & polygons, qreal res, qreal bWidth, qreal bHeight, qreal pixelFactor, const QString & colorString, const QString & layerName) 
{
	QString pSvg = QString("<svg xmlns='http://www.w3.org/2000/svg' width='%1in' height='%2in' viewBox='0 0 %3 %4' >\n")
		.arg(bWidth / res)
		.arg(bHeight / res)
		.arg(bWidth * pixelFactor)
		.arg(bHeight * pixelFactor);
	pSvg += QString("<g id='%1'>\n").arg(layerName);
	pSvg += "<g id='connector0pad'>\n";
	foreach (QPolygon poly, polygons) {
		pSvg += QString("<polygon fill='%1' points='\n").arg(colorString);
		int space = 0;
		foreach (QPoint p, poly) {
			pSvg += QString("%1,%2 %3").arg(p.x()).arg(p.y()).arg((++space % 8 == 0) ?  "\n" : "");
		}
		pSvg += "'/>\n";
	}
	pSvg += "</g></g>\n</svg>\n";

	return pSvg;
}


const QStringList & GroundPlaneGenerator::newSVGs() {
	return m_newSVGs;
}
