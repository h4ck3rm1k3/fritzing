/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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


#ifndef PALETTEITEM_H
#define PALETTEITEM_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QComboBox>
#include <QRadioButton>
#include <QDoubleValidator>
#include <QLineEdit>

#include "paletteitembase.h"
#include "../viewlayer.h"

struct HoleClassThing {
    QString holeSize;
    QString ringThickness;
    QString holeSizeValue;
    QHash<QString, QString> holeSizes;
    QList<QString> holeSizeKeys;                // need this for menu item ordering purposes
	QPointF ringThicknessRange;
	QPointF holeDiameterRange;

};

struct HoleSettings
{
	QString holeDiameter;
	QString ringThickness;
    HoleClassThing * holeThing;
	QPointer<QDoubleValidator> diameterValidator;
	QPointer<QDoubleValidator> thicknessValidator;
	QPointer<QLineEdit> diameterEdit;
	QPointer<QLineEdit> thicknessEdit;
	QPointer<QRadioButton> inRadioButton;
	QPointer<QRadioButton> mmRadioButton;
	QPointer<QComboBox> sizesComboBox;

	QString currentUnits();
    QString holeSize();
};

class PaletteItem : public PaletteItemBase 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	PaletteItem(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~PaletteItem();

	void removeLayerKin();
	void addLayerKin(class LayerKinPaletteItem * lkpi);
	const QList<class ItemBase *> & layerKin();
 	virtual void loadLayerKin(const LayerHash & viewLayers, ViewLayer::ViewLayerSpec);
	void rotateItem(double degrees);
	void flipItem(Qt::Orientations orientation);
	void moveItem(ViewGeometry & viewGeometry);
	void transformItem2(const QMatrix &);
	void setItemPos(QPointF & pos);

	bool renderImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors, QString & error);

	void setTransforms();
	void syncKinMoved(QPointF offset, QPointF loc);

	void setInstanceTitle(const QString&);

	bool swap(ModelPart* newModelPart, const LayerHash &layerHash, bool reinit, class SwapCommand *);
	void setHidden(bool hidden);
	void setInactive(bool inactivate);
	bool collectFemaleConnectees(QSet<ItemBase *> & items);
	void collectWireConnectees(QSet<class Wire *> & wires);
	void clearModelPart();
	void mousePressEvent(PaletteItemBase * originalItem, QGraphicsSceneMouseEvent *);
	ItemBase * lowerConnectorLayerVisible(ItemBase *);
	void resetID();
	void slamZ(double z);
	void resetImage(class InfoGraphicsView *, QDomDocument *);
	void resetKinImage(ItemBase * layerKin, InfoGraphicsView * infoGraphicsView, QDomDocument *);
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	virtual bool changePinLabels(bool singleRow, bool sip);
	QStringList getPinLabels(bool & hasLocal);
	void renamePins(const QStringList & labels, bool singleRow);
	void resetConnectors();
	void resetConnectors(ItemBase * otherLayer, FSvgRenderer * otherLayerRenderer);
	void resetConnector(ItemBase * itemBase, SvgIdLayer * svgIdLayer);


public:
	static QString genFZP(const QString & moduleid, const QString & templateName, int minPins, int maxPins, int steps, bool smd);

	static QWidget * createHoleSettings(QWidget * parent, HoleSettings &, bool swappingEnabled, const QString & currentHoleSize, bool advanced);
	static void updateValidators(HoleSettings &);
	static void updateEditTexts(HoleSettings &);
	static void updateSizes(HoleSettings &);
    static void initHoleSettings(HoleSettings & holeSettings, HoleClassThing *);
	static bool setHoleSize(QString & holeSize, bool force, HoleSettings & holeSettings);
    static QString changeUnits(HoleSettings &);
	static bool changeDiameter(HoleSettings & holeSettings, QObject * sender);
	static bool changeThickness(HoleSettings & holeSettings, QObject * sender);

signals:
	void pinLabelSwap(ItemBase *, const QString & moduleID);

protected slots:
	void openPinLabelDialog();

protected:
	void syncKinSelection(bool selected, PaletteItemBase * originator);
 	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateConnections();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void figureHover();
	bool isSingleRow(QList<ConnectorItem *> & connectorItems);
	QList<ConnectorItem *> sortConnectorItems();
    QString hackSvgHoleSize(const QString & holeDiameter, const QString & ringThickness);
    QString hackFzpHoleSize(const QString & moduleID, const QString & pcbFilename, const QString & holeSize);
    QString appendHoleSize(const QString & moduleID, const QString & holeSize, const QString & ringThickness);
    void generateSwap(const QString & text, GenModuleID, GenFzp, GenSvg makeBreadboardSvg, GenSvg makeSchematicSvg, GenSvg makePcbSvg);

protected:
    void setUpHoleSizes(const QString & type, HoleClassThing &);
	void setUpHoleSizesAux(HoleClassThing &, const QString & type);
    bool collectHoleSizeInfo(const QString & defaultHoleSizeValue, QWidget * parent, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	static QStringList getSizes(QString & holeSize, HoleSettings &);
    static QString hackFzpHoleSize(QDomDocument & document, const QString & newModuleID, const QString & pcbFilename, const QString & newSize);
    static QString hackFzpHoleSize(const QString & fzp, const QString & moduleid, int hsix); 
    static QString hackSvgHoleSize(QDomDocument & domDocument, const QString & holeDiameter, const QString & ringThickness);
    static QString hackSvgHoleSizeAux(const QString & svg, const QString & expectedFileName);	

protected slots:
	virtual void changeHoleSize(const QString &);
	virtual void changeUnits(bool);
	void changeDiameter();
	void changeThickness();


public:
    static const QString HoleSizePrefix;

protected:
 	QList<class ItemBase *> m_layerKin;
    HoleSettings m_holeSettings;

};

#endif
