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

$Revision: 2776 $:
$Author: merunga $:
$Date: 2009-04-02 13:54:08 +0200 (Thu, 02 Apr 2009) $

********************************************************************/

#ifndef ADDPARTDIALOG_H_
#define ADDPARTDIALOG_H_

#include <QDialog>

class AddPartDialog : public QDialog {
	Q_OBJECT
	public:
		AddPartDialog(QWidget *parent=0);
		virtual ~AddPartDialog();

	public slots:
		void fromPartsEditor();
		void fromAllTheLibrary();
		void fromWebGenerator();
		void fromLocalFolder();

	public:
		static class ModelPart *getModelPart(QWidget *parent);

	protected:
		void addButton(const QString &btnText, const char *method);
};

#endif /* ADDPARTDIALOG_H_ */
