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

#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>

#include "tipsandtricks.h"

TipsAndTricks* TipsAndTricks::singleton = NULL;

TipsAndTricks::TipsAndTricks(QWidget *parent)
	: QDialog(parent)
{
	// Let's set the icon
	this->setWindowIcon(QIcon(QPixmap(":resources/images/fritzing_icon.png")));

	singleton = this;
	setWindowTitle(tr("Tips and Tricks"));
	resize(400, 300);
	m_textEdit = new QTextEdit();
	m_textEdit->setReadOnly(true);

	QVBoxLayout * vLayout = new QVBoxLayout(this);
	vLayout->addWidget(m_textEdit);

	QString html =
tr("<html><body>") +
tr("<h3>Fritzing Tips and Tricks</h3>") +
tr("<ul>") +
tr("<li>To drag a wire segment (a section of a wire between two bendpoints), drag it with the alt key down.  If you also hold down the shift key, the wire segment will be constrained to horizontal or vertical motion.</li>") +
tr("<li>Use shift-drag on a wire end or bendpoint to constrain its wire segment to an angle of 45 degrees (or some multiple of 45 degrees).  If the wire segment is connected to other wire segments, the segment you're dragging will snap to make 90 degree angles with the neighboring wire segment.</li>") +
tr("<li>In Schematic or PCB view, if you drag from a bendpoint with the alt key down, you will drag out a new wire from that bendpoint.</li>") +
tr("<li>To constrain the motion of a part to horizontal or vertical, hold down the shift key as you drag it.</li>") +
tr("<li>If you're having trouble selecting a part or a wire (segment), try selecting the part that's in the way and send it to the back: use the Raise and Lower functions on the Part menu or the context menu (right-click menu).</li>") +
tr("<li>When you export images from Fritzing, you can choose which layers are exported. Before you choose 'Export...', go into the 'View' menu and hide the layers you don't want to be visible.</li>") +
tr("</ul>") +
tr("</body></html>") 
;

	m_textEdit->setHtml(html);

}

TipsAndTricks::~TipsAndTricks()
{
}

void TipsAndTricks::hideTipsAndTricks() {
	if (singleton != NULL) {
		singleton->hide();
	}
}

void TipsAndTricks::showTipsAndTricks() {
	if (singleton == NULL) {
		new TipsAndTricks();
	}

	singleton->show();
}

void TipsAndTricks::cleanup() {
	if (singleton) {
		delete singleton;
		singleton = NULL;
	}
}
