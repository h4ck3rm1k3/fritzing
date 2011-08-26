/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "cursormaster.h"
#include "../debugdialog.h"


#include <QApplication>
#include <QBitmap>
#include <QString>

QCursor * CursorMaster::BendpointCursor = NULL;
QCursor * CursorMaster::NewBendpointCursor = NULL;
QCursor * CursorMaster::MakeWireCursor = NULL;
QCursor * CursorMaster::MakeCurveCursor = NULL;
QCursor * CursorMaster::RubberbandCursor = NULL;
QCursor * CursorMaster::MoveCursor = NULL;

CursorMaster CursorMaster::TheCursorMaster;
static QList<QObject *> Associates;

CursorMaster::CursorMaster() : QObject()
{
}

void CursorMaster::initCursors()
{
	if (BendpointCursor == NULL) {
		QBitmap bitmap1(":resources/images/cursor/bendpoint.bmp");
		QBitmap bitmap1m(":resources/images/cursor/bendpoint_mask.bmp");
		BendpointCursor = new QCursor(bitmap1, bitmap1m, 0, 0);

		QBitmap bitmap2(":resources/images/cursor/new_bendpoint.bmp");
		QBitmap bitmap2m(":resources/images/cursor/new_bendpoint_mask.bmp");
		NewBendpointCursor = new QCursor(bitmap2, bitmap2m, 0, 0);

		QBitmap bitmap3(":resources/images/cursor/make_wire.bmp");
		QBitmap bitmap3m(":resources/images/cursor/make_wire_mask.bmp");
		MakeWireCursor = new QCursor(bitmap3, bitmap3m, 0, 0);

		QBitmap bitmap4(":resources/images/cursor/curve.bmp");
		QBitmap bitmap4m(":resources/images/cursor/curve_mask.bmp");
		MakeCurveCursor = new QCursor(bitmap4, bitmap4m, 0, 0);

		QBitmap bitmap5(":resources/images/cursor/rubberband_move.bmp");
		QBitmap bitmap5m(":resources/images/cursor/rubberband_move_mask.bmp");
		RubberbandCursor = new QCursor(bitmap5, bitmap5m, 0, 0);

		QBitmap bitmap6(":resources/images/cursor/part_move.bmp");
		QBitmap bitmap6m(":resources/images/cursor/part_move_mask.bmp");
		MoveCursor = new QCursor(bitmap6, bitmap6m, 0, 0);
	}
}

CursorMaster * CursorMaster::instance()
{
	return &TheCursorMaster;
}

void CursorMaster::addCursor(QObject * associate, const QCursor & cursor)
{
	if (associate == NULL) return;

	if (Associates.contains(associate)) {
		DebugDialog::debug(QString("changing cursor %1").arg((long) associate));
		QApplication::changeOverrideCursor(cursor);
		return;
	}

	Associates.append(associate);
	connect(associate, SIGNAL(destroyed(QObject *)), this, SLOT(deleteCursor(QObject *)));
	QApplication::setOverrideCursor(cursor);
	DebugDialog::debug(QString("addding cursor %1").arg((long) associate));
}

void CursorMaster::removeCursor(QObject * associate)
{
	if (associate == NULL) return;

	if (Associates.contains(associate)) {
		disconnect(associate, SIGNAL(destroyed(QObject *)), this, SLOT(deleteCursor(QObject *)));
		Associates.removeOne(associate);
		QApplication::restoreOverrideCursor();
		DebugDialog::debug(QString("removing cursor %1").arg((long) associate));
	}
}

void CursorMaster::deleteCursor(QObject * associate)
{
	removeCursor(associate);
}
