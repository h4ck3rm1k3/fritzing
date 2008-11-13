#ifndef PALETTEMODEL_H
#define PALETTEMODEL_H

#include "modelpart.h"
#include "modelbase.h"

#include <QDomDocument>
#include <QList>
#include <QDir>
#include <QStringList>
#include <QHash>

class PaletteModel : public ModelBase
{
Q_OBJECT
public:
	PaletteModel();
	PaletteModel(bool makeRoot, bool doInit=true);
	ModelPart * retrieveModelPart(const QString & moduleID);
	void updateOrAddModelPart(const QString & moduleID, ModelPart *modelPart);
	virtual bool containsModelPart(const QString & moduleID);
	virtual ModelPart * loadPart(const QString & path, bool update=false);
	void clear();
	bool loadedFromFile();
	QString loadedFrom();
	bool load(const QString & fileName, ModelBase* refModel, bool doConnections);
	ModelPart * addPart(QString newPartPath, bool addToReference, bool updateIdAlreadyExists=false);

signals:
	void newPartLoaded(ModelPart *);

protected:
	QHash<QString, ModelPart *> partHash;
	bool m_loadedFromFile;
	QString m_loadedFrom; // The file this was loaded from, only if m_loadedFromFile == true

	bool m_loadingCore;

protected:
	virtual void init();
	void loadParts();
	void loadPartsAux(QDir & dir, QStringList & nameFilters);

	void writeAllPartsBinHeader();
	void writeAllPartsBinFooter();
	void writeInstanceInAllPartsBin(const QString &moduleID, const QString &path);
	void writeToAllPartsBinAux(const QString &textToWrite, QIODevice::OpenMode openMode);

	static bool CreateAllPartsBinFile;
	static QString AllPartsBinFilePath;

};
#endif
