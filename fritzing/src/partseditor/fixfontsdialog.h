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

#ifndef FIXFONTSDIALOG_H_
#define FIXFONTSDIALOG_H_

#include <QDialog>
#include <QSet>

#define FixedFontsHash QHash<QString /*borkenFont*/, QString /*replacementFont*/>

class FixFontsDialog : public QDialog {
public:
	FixFontsDialog(QWidget *parent, const QSet<QString> fontsTofix);
	virtual ~FixFontsDialog();
	FixedFontsHash getFixedFontsHash();
	QSet<QString> fontsToFix();

	static FixedFontsHash fixFonts(QWidget *parent, const QSet<QString> fontsTofix, bool &cancelPressed);

protected:
	void createLine(QLayout* layout, const QString &brokenFont, const QStringList &items, int defaultIdx);

	QSet<QString> m_fontsToFix;
	QList<class FixedFontComboBox *> m_fixedFonts;
};

#endif /* FIXFONTSDIALOG_H_ */
