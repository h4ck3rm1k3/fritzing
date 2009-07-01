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

$Revision: 3051 $:
$Author: cohen@irascible.com $:
$Date: 2009-05-31 13:20:46 +0200 (Sun, 31 May 2009) $

********************************************************************/

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDialogButtonBox>

#include "fixfontsdialog.h"
#include "../fapplication.h"
#include "../debugdialog.h"

//////////////////////////////////////////////////////////////////////////////

class FixedFontComboBox : public QComboBox {
public:
	FixedFontComboBox(QWidget *parent, const QString &brokenFont)
		: QComboBox(parent)
	{
		m_brokenFont = brokenFont;
	}

	const QString &brokenFont() { return m_brokenFont; }
	void setBrokenFont(const QString &brokenFont) { m_brokenFont = brokenFont; }

protected:
	QString m_brokenFont;
};

//////////////////////////////////////////////////////////////////////////////


FixFontsDialog::FixFontsDialog(QWidget *parent, const QSet<QString> fontsTofix)
	: QDialog(parent)
{
	setWindowTitle(tr("Fonts fixer"));

	QScrollArea *container = new QScrollArea(this);
	container->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	QVBoxLayout *layout = new QVBoxLayout(container);
	layout->setSpacing(1);
	layout->setMargin(1);

	m_fontsToFix = fontsTofix - FApplication::InstalledFonts;
	QStringList availFonts = FApplication::InstalledFonts.toList();
	qSort(availFonts);
	availFonts.insert(0,tr("-- ignore --"));

	foreach(QString ftf, m_fontsToFix) {
		DebugDialog::debug("font not found: "+ftf);
		createLine(layout,ftf,availFonts);
	}
	layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *mainLO = new QVBoxLayout(this);
	mainLO->setMargin(2);
	mainLO->setSpacing(2);

	mainLO->addWidget(new QLabel(tr(
		"One or more fonts defined in the SVG file, couldn't be recognized.\n"
		"If you want, use these controls to fixed them."
		),this));
	mainLO->addWidget(container);
	QHBoxLayout *btnLayout = new QHBoxLayout();
	btnLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
	btnLayout->addWidget(buttonBox);
	mainLO->addLayout(btnLayout);
	setMinimumSize(400,200);
}

FixFontsDialog::~FixFontsDialog() {
	// TODO Auto-generated destructor stub
}

// assumes that the first item in items, is a default value
void FixFontsDialog::createLine(QLayout* layout, const QString &brokenFont, const QStringList &items) {
	FixedFontComboBox *cb = new FixedFontComboBox(this,brokenFont);

	int fontValue = -1;
	foreach(QString font, items) {
		cb->addItem(font, fontValue);
		fontValue++;
	}
	m_fixedFonts << cb;

	QFrame *line = new QFrame(this);
	QHBoxLayout *lineLO = new QHBoxLayout(line);
	lineLO->setSpacing(0);
	lineLO->setMargin(1);
	lineLO->addWidget(new QLabel(tr("Replace '%1' with ").arg(brokenFont), line));
	lineLO->addWidget(cb);

	layout->addWidget(line);
}

QSet<QString> FixFontsDialog::fontsToFix() {
	return m_fontsToFix;
}

FixedFontsHash FixFontsDialog::getFixedFontsHash() {
	FixedFontsHash retval;
	foreach(FixedFontComboBox* cb, m_fixedFonts) {
		int idx = cb->currentIndex();
		if( idx != -1) {
			int value = cb->itemData(idx).toInt();
			if(value > -1) {
				retval[cb->brokenFont()] = cb->itemText(idx);
			}
		}
	}
	return retval;
}

FixedFontsHash FixFontsDialog::fixFonts(QWidget *parent, const QSet<QString> fontsTofix, bool &cancelPressed) {
	FixedFontsHash retval;
	cancelPressed = false;
	FixFontsDialog *dlg = new FixFontsDialog(parent,fontsTofix);

	if(dlg->fontsToFix().size() > 0) {
		if(dlg->exec() == QDialog::Accepted) {
			retval = dlg->getFixedFontsHash();
		} else {
			cancelPressed = true;
		}
	}
	delete dlg;
	return retval;
}
