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

#include "palettemodel.h"
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QDomElement>

#include "debugdialog.h"
#include "modelpart.h"
#include "version/version.h"

#ifndef QT_NO_DEBUG
bool PaletteModel::CreateAllPartsBinFile = true;
#else
bool PaletteModel::CreateAllPartsBinFile = false;
#endif

QString PaletteModel::AllPartsBinFilePath = ___emptyString___;

PaletteModel::PaletteModel() : ModelBase(true) {
	m_loadedFromFile = false;
	m_loadingCore = false;
}

PaletteModel::PaletteModel(bool makeRoot, bool doInit) : ModelBase( makeRoot ) {
	m_loadedFromFile = false;
	m_loadingCore = false;

	if (doInit)
		init();
}

void PaletteModel::init() {
	loadParts();
	if (m_root == NULL) {
	    QMessageBox::information(NULL, QObject::tr("Fritzing"),
	                             QObject::tr("No parts found.") );
	}
}

ModelPart * PaletteModel::retrieveModelPart(const QString & moduleID) {
	ModelPart * modelPart = m_partHash[moduleID];
	if (modelPart != NULL) return modelPart;

	if (m_referenceModel != NULL) {
		return m_referenceModel->retrieveModelPart(moduleID);
	}

	return NULL;
}

bool PaletteModel::containsModelPart(const QString & moduleID) {
	return m_partHash.contains(moduleID);
}

void PaletteModel::updateOrAddModelPart(const QString & moduleID, ModelPart *newOne) {
	ModelPart *oldOne = m_partHash[moduleID];
	if(oldOne) {
		oldOne->copy(newOne);
	} else {
		m_partHash.insert(moduleID, newOne);
	}
}

void PaletteModel::loadParts() {
	QStringList nameFilters;
	nameFilters << "*" + FritzingPartExtension;

	if(CreateAllPartsBinFile) writeAllPartsBinHeader();

	QDir * dir = getApplicationSubFolder("parts");
	if (dir != NULL) {
		loadPartsAux(*dir, nameFilters);
		delete dir;
	}

	dir = new QDir(":/resources/parts");
	loadPartsAux(*dir, nameFilters);

	if(CreateAllPartsBinFile) writeAllPartsBinFooter();

	delete dir;
}

void PaletteModel::writeAllPartsBinHeader() {
	QString header =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"+
		QString("<module fritzingVersion='%1'>\n").arg(Version::versionString())+
		"\t<title>All Parts</title>\n"
		"\t<instances>\n";
	writeToAllPartsBinAux(header, QFile::WriteOnly);
}

void PaletteModel::writeAllPartsBinFooter() {
	QString footer = "\t</instances>\n</module>\n";
	writeToAllPartsBinAux(footer, QFile::Append);
}

void PaletteModel::writeInstanceInAllPartsBin(const QString &moduleID, const QString &path) {
	QString pathAux = path;
	pathAux.remove(getApplicationSubFolderPath("")+"/");
	QString instance =
        QString(
        	"\t\t<instance moduleIdRef=\"%1\" path=\"%2\">\n").arg(moduleID).arg(pathAux)+
            "\t\t\t<views>\n"
        	"\t\t\t\t<iconView layer=\"icon\">\n"
        	"\t\t\t\t\t<geometry z=\"-1\" x=\"-1\" y=\"-1\"></geometry>\n"
        	"\t\t\t\t</iconView>\n"
        	"\t\t\t</views>\n"
        	"\t\t</instance>\n";
	writeToAllPartsBinAux(instance, QFile::Append);
}

void PaletteModel::writeToAllPartsBinAux(const QString &textToWrite, QIODevice::OpenMode openMode) {
	if(AllPartsBinFilePath == ___emptyString___) {
		AllPartsBinFilePath = getApplicationSubFolderPath("bins")+"/allParts.dbg" + FritzingBinExtension;
	}
	QFile file(AllPartsBinFilePath);
	if (!file.open(openMode | QFile::Text)) {
		CreateAllPartsBinFile = false;
	} else {
		QTextStream out(&file);
		out << textToWrite;
		file.close();
	}
}

void PaletteModel::loadPartsAux(QDir & dir, QStringList & nameFilters) {
    QString temp = dir.absolutePath();
    QFileInfoList list = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks);
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString path = fileInfo.absoluteFilePath ();
        // DebugDialog::debug(QString("part path:%1 core? %2").arg(path).arg(m_loadingCore? "true" : "false"));
        loadPart(path);
    }

    QStringList dirs = dir.entryList(QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.size(); ++i) {
    	QString temp2 = dirs[i];
       	dir.cd(temp2);
       	m_loadingCore = temp2=="core";
    	loadPartsAux(dir, nameFilters);
    	dir.cdUp();
    }
}

ModelPart * PaletteModel::loadPart(const QString & path, bool update) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(NULL, QObject::tr("Fritzing"),
                             QObject::tr("Cannot read file %1:\n%2.")
                             .arg(path)
                             .arg(file.errorString()));
        return NULL;
    }


    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument* domDocument = new QDomDocument();

    if (!domDocument->setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
        QMessageBox::information(NULL, QObject::tr("Fritzing"),
							 QObject::tr("Parse error (2) at line %1, column %2:\n%3\n%4")
							 .arg(errorLine)
							 .arg(errorColumn)
							 .arg(errorStr)
							 .arg(path));
        return NULL;
    }

    QDomElement root = domDocument->documentElement();
   	if (root.isNull()) {
        //QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file is not a Fritzing file (8)."));
   		return NULL;
	}

    if (root.tagName() != "module") {
        //QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file is not a Fritzing file (9)."));
        return NULL;
    }

	QString moduleID = root.attribute("moduleId");
	if (moduleID.isNull() || moduleID.isEmpty()) {
		//QMessageBox::information(NULL, QObject::tr("Fritzing"), QObject::tr("The file is not a Fritzing file (10)."));
		return NULL;
	}

    ModelPart::ItemType type = ModelPart::Part;
    // check if it's a wire
	QDomElement properties = root.firstChildElement("properties");
	// FIXME: properties is nested right now
	if (properties.text().contains("wire", Qt::CaseInsensitive)) {
		type = ModelPart::Wire;
	}
	else if (properties.text().contains("breadboard", Qt::CaseInsensitive)) {
		type = ModelPart::Breadboard;
	}
	else if (properties.text().contains("arduino", Qt::CaseInsensitive)) {
		type = ModelPart::Board;
	}
	else if (properties.text().contains("plain pcb", Qt::CaseInsensitive)) {
		type = ModelPart::Board;
	}
	else if (properties.text().contains("note", Qt::CaseInsensitive)) {
		type = ModelPart::Note;
	}
	else if (properties.text().contains("group", Qt::CaseInsensitive)) {
		type = ModelPart::Module;
	}
	ModelPart * modelPart = new ModelPart(domDocument, path, type);
	if (modelPart == NULL) return NULL;

	modelPart->setCore(m_loadingCore);

	if (m_partHash.contains(moduleID) && m_partHash[moduleID]) {
		if(!update) {
			QMessageBox::warning(NULL, QObject::tr("Fritzing"),
							 QObject::tr("The part '%1' at '%2' does not have a unique module id '%3'.")
							 .arg(modelPart->title())
							 .arg(path)
							 .arg(moduleID));
			return NULL;
		} else {
			m_partHash[moduleID]->copyStuff(modelPart);
		}
	} else {
		m_partHash.insert(moduleID, modelPart);
	}

    if (m_root == NULL) {
		 m_root = modelPart;
	}
	else {
    	modelPart->setParent(m_root);
   	}

    emit newPartLoaded(modelPart);

    if(CreateAllPartsBinFile) writeInstanceInAllPartsBin(moduleID,path);

    return modelPart;
}

bool PaletteModel::load(const QString & fileName, ModelBase * refModel) {
	QList<ModelPart *> modelParts;
	bool result = ModelBase::load(fileName, refModel, modelParts);
	if (result) {
		m_loadedFromFile = true;
		m_loadedFrom = fileName;
	}
	return result;
}

bool PaletteModel::loadedFromFile() {
	return m_loadedFromFile;
}

QString PaletteModel::loadedFrom() {
	if(m_loadedFromFile) {
		return m_loadedFrom;
	} else {
		return QString::null;
	}
}

ModelPart * PaletteModel::addPart(QString newPartPath, bool addToReference, bool updateIdAlreadyExists) {
	/*ModelPart * modelPart = loadPart(newPartPath, updateIdAlreadyExists);;
	if (m_referenceModel != NULL && addToReference) {
		modelPart = m_referenceModel->addPart(newPartPath, addToReference);
		if (modelPart != NULL) {
			return addModelPart( m_root, modelPart);
		}
	}*/

	ModelPart * modelPart = loadPart(newPartPath, updateIdAlreadyExists);
	if (m_referenceModel && addToReference) {
		m_referenceModel->addPart(modelPart,updateIdAlreadyExists);
	}

	return modelPart;
}

void PaletteModel::removePart(const QString &moduleID) {
	ModelPart *mpToRemove = NULL;
	QList<QObject *>::const_iterator i;
    for (i = m_root->children().constBegin(); i != m_root->children().constEnd(); ++i) {
		ModelPart* mp = qobject_cast<ModelPart *>(*i);
		if (mp == NULL) continue;

		if(mp->moduleID() == moduleID) {
			mpToRemove = mp;
			break;
		}
	}
	if(mpToRemove) {
		mpToRemove->setParent(NULL);
		delete mpToRemove;
	}
}

void PaletteModel::clearPartHash() {
	foreach (ModelPart * modelPart, m_partHash.values()) {
		delete modelPart;
	}
	m_partHash.clear();
}
