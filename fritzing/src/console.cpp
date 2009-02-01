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

#include "console.h"
#include "debugdialog.h"


Console::Console( QWidget * parent )
	: QTextEdit(parent)
{
	// TODO: need to hook this up to more useful stuff
	//DebugDialog::connectToBroadcast(this, SLOT(receiveDebugBroadcast(const QString &, DebugDialog::DebugLevel, QObject *)));
}

void Console::receiveDebugBroadcast(const QString & message, DebugDialog::DebugLevel debugLevel, QObject * ancestor) {
	bool gotOne = false;
	if (ancestor == NULL) {
		gotOne = this->isActiveWindow();
	}
	else {
		while (ancestor->parent() != NULL) {
			ancestor = ancestor->parent();
		}

		for (QObject * pa = this; pa; pa = pa->parent()) {
			if (pa == ancestor) {
				gotOne = true;
				break;
			}
		}
	}

	if (!gotOne) return;

	this->append(message);
}
