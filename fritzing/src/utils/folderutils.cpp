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

#include "folderutils.h"
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSettings>
#include <QTextStream>

FolderUtils* FolderUtils::singleton = NULL;

FolderUtils::FolderUtils() {
	m_folders << "/bins"
		<< "/parts/user" << "/parts/contrib"
		<< "/parts/svg/user/icon" << "/parts/svg/user/breadboard"
		<< "/parts/svg/user/schematic" << "/parts/svg/user/pcb"
		<< "/parts/svg/contrib/icon" << "/parts/svg/contrib/breadboard"
		<< "/parts/svg/contrib/schematic" << "/parts/svg/contrib/pcb";
}

FolderUtils::~FolderUtils() {
}

// finds the user parts folder based on local desktop (OS) defaults
QString FolderUtils::getUserPartsFolder() {
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/UserCreatedParts";
}

// finds a subfolder of the application directory searching backward up the tree
QDir * FolderUtils::getApplicationSubFolder(QString search) {
	if (singleton == NULL) {
		singleton = new FolderUtils();
	}

	QString path = singleton->applicationDirPath();
    path += "/" + search;
	//DebugDialog::debug(QString("path %1").arg(path) );
    QDir* dir= new QDir(path);
    while (!dir->exists()) {
    	// if we're running from the debug or release folder, go up one to find things
    	dir->cdUp();
		dir->cdUp();
    	if (dir->isRoot()) return NULL;   // didn't find the search folder

		dir->setPath(dir->absolutePath() + "/" + search);
   	}

   	return dir;
}

QString FolderUtils::getApplicationSubFolderPath(QString search) {
	if (singleton == NULL) {
		singleton = new FolderUtils();
	}

	QDir * dir = getApplicationSubFolder(search);
	if (dir == NULL) return "";

	QString result = dir->path();
	delete dir;
	return result;
}

QString FolderUtils::getUserDataStorePath(QString folder) {
	QString settingsFile = QSettings(QSettings::IniFormat,QSettings::UserScope,"Fritzing","Fritzing").fileName();
	return QFileInfo(settingsFile).dir()
		.absolutePath()+(folder!=___emptyString___?"/"+folder:"");
}

const QStringList & FolderUtils::getUserDataStoreFolders() {
	if (singleton == NULL) {
		singleton = new FolderUtils();
	}

	return singleton->userDataStoreFolders();
}

bool FolderUtils::createFolderAnCdIntoIt(QDir &dir, QString newFolder) {
	if(!dir.mkdir(newFolder)) return false;
	if(!dir.cd(newFolder)) return false;

	return true;
}

bool FolderUtils::setApplicationPath(const QString & path) 
{
	if (singleton == NULL) {
		singleton = new FolderUtils();
	}

	return singleton->setApplicationPath2(path);
}

void FolderUtils::cleanup() {
	if (singleton) {
		delete singleton;
		singleton = NULL;
	}
}

/////////////////////////////////////////////////

const QString FolderUtils::getLibraryPath() 
{
	if (singleton == NULL) {
		singleton = new FolderUtils();
	}

	return singleton->libraryPath();
}


const QString FolderUtils::libraryPath() 
{
#ifdef Q_WS_MAC
	// mac plugins are always in the bundle
	return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../lib");
#endif

	return QDir::cleanPath(applicationDirPath() + "/lib");		
}

const QString FolderUtils::applicationDirPath() {
	if (m_appPath.isEmpty()) {
		return QCoreApplication::applicationDirPath();
	}

	return m_appPath;
}

bool FolderUtils::setApplicationPath2(const QString & path) 
{
	QDir dir(path);
	if (!dir.exists()) return false;

	m_appPath = path;
	return true;
}

const QStringList & FolderUtils::userDataStoreFolders() {
	return m_folders;
}

// this function searches by regexp
bool FolderUtils::containsText(const QString &filepath, const QString &searchText) {
	QRegExp re(searchText);
	if(!re.isValid()) return false;

    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly )) return false;

	QTextStream stream(&file);
	QString content = stream.readAll();
	file.close();

	return re.indexIn(content) != -1;
}

