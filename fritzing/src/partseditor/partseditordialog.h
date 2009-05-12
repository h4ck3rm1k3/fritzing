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

#ifndef PARTSEDITORDIALOG_H_
#define PARTSEDITORDIALOG_H_

#include <QDialog>


class PartsEditorDialog : QDialog {
	Q_OBJECT
	public:
		PartsEditorDialog(class MainWindow *parent, class ModelPart *mp=NULL);
		virtual ~PartsEditorDialog();

		ModelPart *modelPart();

	public slots:
		void loadPart(const QString &newPartPath);
		void partsEditorClosed(long id);
		void reject();

	public:
		static ModelPart* newPart(MainWindow *parent);
		static ModelPart* editPart(MainWindow *parent, ModelPart* mp);

	protected:
		void closeEvent(QCloseEvent *event);

		MainWindow *m_mainWindow;
		class PartsEditorMainWindow *m_partsEditorWindow;
		ModelPart* m_modelPart;
};

#endif /* PARTSEDITORDIALOG_H_ */
