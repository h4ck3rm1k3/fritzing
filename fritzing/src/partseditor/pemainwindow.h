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

$Revision$:
$Author$:
$Date$

********************************************************************/


#ifndef PEMAINWINDOW_H_
#define PEMAINWINDOW_H_


#include "../mainwindow.h"
#include "../model/modelpartshared.h"
#include "../sketch/sketchwidget.h"

class IconSketchWidget : public SketchWidget
{
	Q_OBJECT

public:
    IconSketchWidget(ViewIdentifierClass::ViewIdentifier, QWidget *parent=0);

	void addViewLayers();
};


class PEMainWindow : public MainWindow
{
Q_OBJECT

public:
	PEMainWindow(class PaletteModel * paletteModel, class ReferenceModel * referenceModel, QWidget * parent);
	~PEMainWindow();

    void setInitialItem(class PaletteItem *);
    void changeTags(const QStringList &);
    void changeProperties(const QHash<QString, QString> &);
    void changeMetadata(const QString & name, const QString & value);

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
	void setTitle();
    void createViewMenuActions();
    void createViewMenu();

public slots:
    void metadataChanged(const QString & name, const QString & value);
    void propertiesChanged(const QHash<QString, QString> &);
    void tagsChanged(const QStringList &);

protected slots:
    void initZoom();
    void showMetadataView();
    void showIconView();

protected:
    QDomDocument m_fzpDocument;
    QDomDocument m_iconDocument;
    QDomDocument m_breadboardDocument;
    QDomDocument m_schematicDocument;
    QDomDocument m_pcbDocument;

    QAction * m_showMetadataAct;
    QAction * m_showIconAct;

	QPointer<SketchAreaWidget> m_iconWidget;
	QPointer<class IconSketchWidget> m_iconGraphicsView;
    class MetadataView * m_metadataView;

};

#endif /* PEMAINWINDOW_H_ */
