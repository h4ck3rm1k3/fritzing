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



#ifndef PARTSEDITORCONNECTORSVIEW_H_
#define PARTSEDITORCONNECTORSVIEW_H_

#include "partseditorconnectoritem.h"
#include "partseditorabstractview.h"
#include "partseditorconnectorspaletteitem.h"
#include "partseditorconnectorsconnectoritem.h"
#include "zoomcontrols.h"

class PartsEditorConnectorsView: public PartsEditorAbstractView {
	Q_OBJECT
	public:
		PartsEditorConnectorsView(ItemBase::ViewIdentifier, QDir tempDir, bool showingTerminalPoint, QWidget *parent=0, int size=150);
		void drawConector(Connector *conn, bool showTerminalPoint);
		void removeConnector(const QString &connId);
		void inFileDefinedConnectorChanged(PartsEditorConnectorsConnectorItem *connItem);
		void aboutToSave();


		void showTerminalPoints(bool show);
		bool showingTerminalPoints();

	public slots:
		void informConnectorSelection(const QString& connId);
		void informConnectorSelectionFromView(const QString& connId);
		virtual void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
		virtual void addItemInPartsEditor(ModelPart * modelPart, SvgAndPartFilePath * svgFilePath);
		void setMismatching(ItemBase::ViewIdentifier viewId, const QString &id, bool mismatching);

	signals:
		void connectorsFound(ItemBase::ViewIdentifier viewId, const QList<Connector*> &conns);
		void svgFileLoadNeeded(const QString &filepath);
		void connectorSelected(const QString& connId);

	protected:
		PartsEditorPaletteItem *newPartsEditorPaletteItem(ModelPart * modelPart);
		PartsEditorPaletteItem *newPartsEditorPaletteItem(ModelPart * modelPart, SvgAndPartFilePath *path);

		void wheelEvent(QWheelEvent* event);
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void connectItem();
		void createConnector(Connector *conn, const QSize &connSize, bool showTerminalPoint);
		void setItemProperties();
		bool isSupposedToBeRemoved(const QString& id);

		bool addConnectorsIfNeeded(QDomDocument *svgDom, const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId);
		bool removeConnectorsIfNeeded(QDomElement &docEle);
		bool updateTerminalPoints(QDomDocument *svgDom, const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId);
		void removeTerminalPoints(const QStringList &tpIdsToRemove, QDomElement &docElem);
		void addNewTerminalPoints(
				const QList<PartsEditorConnectorsConnectorItem*> &connsWithNewTPs, QDomDocument *svgDom,
				const QSizeF &sceneViewBox, const QRectF &svgViewBox, const QString &connectorsLayerId
		);
		QRectF mapFromSceneToSvg(const QRectF &sceneRect, const QSizeF &defaultSize, const QRectF &viewBox);
		void addRectToSvg(QDomDocument* svgDom, const QString &id, const QRectF &rect, const QString &connectorsLayerId);
		bool addRectToSvgAux(QDomElement &docElem, const QString &connectorsLayerId, QDomElement &rectElem);

		PartsEditorConnectorsPaletteItem *myItem();
		QTransform m_prevTransform;

		//ZoomControls *m_zoomControls;

		QList<PartsEditorConnectorsConnectorItem*> m_drawnConns;
		QStringList m_removedConnIds;

		QString m_lastSelectedConnId;
		bool m_showingTerminalPoints;

	protected:
		static int ConnDefaultWidth;
		static int ConnDefaultHeight;
};

#endif /* PARTSEDITORCONNECTORSVIEW_H_ */

