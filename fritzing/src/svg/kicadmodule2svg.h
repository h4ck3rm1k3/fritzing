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

#ifndef KICADMODULE2SVG_H
#define KICADMODULE2SVG_H

#include <QString>
#include <QStringList>
#include <QTextStream>

#include "x2svg.h"

class KicadModule2Svg : public X2Svg
{

public:
	KicadModule2Svg();
	QString convert(const QString & filename, const QString & moduleName, bool allowPadsAndPins);

public:
	static QStringList listModules(const QString & filename);

public:
	enum PadLayer {
		ToCopper0,
		ToCopper1,
		UnableToTranslate
	};

protected:
	KicadModule2Svg::PadLayer convertPad(QTextStream & stream, QString & pad);
	int drawDSegment(const QString & ds, QString & line);
	int drawDArc(const QString & ds, QString & arc);
	int drawDCircle(const QString & ds, QString & arc);
	QString drawOblong(int posX, int posY, qreal xSize, qreal ySize, int drillX, int drillY, const QString & padName, const QString & padType);
	QString drawVerticalOblong(int posX, int posY, qreal xSize, qreal ySize, int drillX, int drillY, const QString & padName, const QString & padType);
	QString drawHorizontalOblong(int posX, int posY, qreal xSize, qreal ySize, int drillX, int drillY, const QString & padName, const QString & padType);
	void checkLimits(int posX, int xSize, int posY, int ySize);
	QString drawRPad(int posX, int posY, int xSize, int ySize, int drillX, int drillY, const QString & padName, const QString & padType);
	QString drawCPad(int posX, int posY, int xSize, int ySize, int drillX, int drillY, const QString & padName, const QString & padType);
};


#endif // KICADMODULE2SVG_H
