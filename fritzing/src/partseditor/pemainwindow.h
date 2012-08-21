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

$Revision: 5291 $:
$Author: cohen@irascible.com $:
$Date: 2011-07-27 15:02:24 +0200 (Wed, 27 Jul 2011) $

********************************************************************/


#ifndef PEMAINWINDOW_H_
#define PEMAINWINDOW_H_


#include "../mainwindow.h"
#include "../model/modelpartshared.h"

class PEMainWindow : public MainWindow
{
Q_OBJECT

public:
	PEMainWindow(class PaletteModel * paletteModel, class ReferenceModel * referenceModel, QWidget * parent);
	~PEMainWindow();

    void setInitialModuleID(const QString & moduleID);

protected:
	void closeEvent(QCloseEvent * event);
    void initLockedFiles(bool lockFiles);
    void initSketchWidgets();
    void initDock();
    void initHelper();
    void moreInitDock();
    void createActions();
    void createMenus();
    QList<QWidget*> getButtonsForView(ViewIdentifierClass::ViewIdentifier);
    void connectPairs();
	QMenu *breadboardItemMenu();
	QMenu *schematicItemMenu();
	QMenu *pcbItemMenu();
	QMenu *pcbWireMenu();
	QMenu *schematicWireMenu();
	QMenu *breadboardWireMenu();
    bool eventFilter(QObject *obj, QEvent *event);

protected slots:
    void initZoom();

protected:
    QDomDocument m_fzpDocument;
    QDomDocument m_iconDocument;
    QDomDocument m_breadboardDocument;
    QDomDocument m_schematicDocument;
    QDomDocument m_pcbDocument;
};

#endif /* PEMAINWINDOW_H_ */
